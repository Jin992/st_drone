#!/usr/bin/env python3
"""
JSBSim ↔ STM32 HITL bridge.

Runs a JSBSim flight dynamics model on the PC and connects it to the
STM32 HITL firmware over a USB-CDC serial port using MAVLink HIL messages.

Data flow:
  JSBSim FDM state  →  MAVLink HIL_SENSOR  →  STM32 (hal/hitl backend)
  STM32 control output  →  MAVLink HIL_ACTUATOR_CONTROLS  →  JSBSim inputs

Usage:
  python jsbsim_hitl_bridge.py --port /dev/tty.usbmodem* --jsbsim-root ~/jsbsim

Dependencies:
  pip install jsbsim pymavlink
"""

import argparse
import math
import signal
import sys
import time
import logging
from pathlib import Path

# vpython is optional — only imported when --viz is passed
_vp = None

try:
    import jsbsim
except ImportError:
    sys.exit("jsbsim package not found.  Install with: pip install jsbsim")

try:
    from pymavlink import mavutil
except ImportError:
    sys.exit("pymavlink package not found.  Install with: pip install pymavlink")


logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s  %(levelname)-8s  %(message)s",
    datefmt="%H:%M:%S",
)
log = logging.getLogger("hitl_bridge")

# Feet-per-second² → m/s²
FT_S2_TO_M_S2 = 0.3048

# Simulation step — must match the STM32 control loop rate (1 kHz).
# JSBSim will be stepped at this rate; pymavlink is non-blocking.
DT_S = 0.001


def parse_args():
    p = argparse.ArgumentParser(description="JSBSim HITL bridge for STM32 drone FC")
    p.add_argument(
        "--port",
        default="/dev/tty.usbmodem*",
        help="Serial port of the STM32 USB-CDC device (default: /dev/tty.usbmodem*)",
    )
    p.add_argument(
        "--baud",
        type=int,
        default=115200,
        help="Serial baud rate (default: 115200)",
    )
    p.add_argument(
        "--jsbsim-root",
        default=None,
        help="Path to JSBSim root directory containing aircraft/ and engine/ folders",
    )
    p.add_argument(
        "--model",
        default="F450",
        help="JSBSim aircraft model name (default: F450)",
    )
    p.add_argument(
        "--sysid",
        type=int,
        default=1,
        help="MAVLink system ID of the flight controller (default: 1)",
    )
    p.add_argument(
        "--flightgear",
        action="store_true",
        help="Stream FDM state to FlightGear on UDP port 5550",
    )
    p.add_argument(
        "--viz",
        action="store_true",
        help="Open a 3D vpython visualiser in the browser",
    )
    p.add_argument(
        "--verbose",
        action="store_true",
        help="Print every HIL_ACTUATOR_CONTROLS packet received",
    )
    return p.parse_args()


# ── JSBSim helpers ────────────────────────────────────────────────────────────

def init_jsbsim(root_dir: str | None, model: str) -> jsbsim.FGFDMExec:
    fdm = jsbsim.FGFDMExec(root_dir)
    fdm.set_debug_level(0)

    if not fdm.load_model(model):
        sys.exit(f"JSBSim could not load model '{model}'. "
                 f"Check --jsbsim-root and --model arguments.")

    # Initialise at 100 m AGL, stationary.
    fdm['ic/h-sl-ft'] = 328.0   # ~100 m
    fdm['ic/vt-kts'] = 0.0
    fdm.run_ic()

    # Pre-set motors to approximate hover so JSBSim doesn't free-fall
    # before the FC has a chance to spin up its control loop.
    for i in range(4):
        try:
            fdm[f'fcs/throttle-cmd-norm[{i}]'] = 0.55
        except Exception:
            pass

    log.info("JSBSim model '%s' loaded, dt=%.4f s", model, fdm.get_delta_t())
    return fdm


def enable_flightgear_output(fdm: jsbsim.FGFDMExec) -> None:
    directive = str(
        (Path(__file__).parent / "flightgear_out.xml").resolve()
    )
    if not fdm.set_output_directive(directive):
        log.warning("FlightGear output directive could not be loaded: %s", directive)
    else:
        log.info("FlightGear output enabled → udp://localhost:5550")


def read_sensors(fdm: jsbsim.FGFDMExec) -> dict:
    """Extract sensor-equivalent values from JSBSim FDM state."""
    return {
        # Accelerations in body frame, converted to m/s²
        "ax": fdm["accelerations/udot-ft_sec2"] * FT_S2_TO_M_S2,
        "ay": fdm["accelerations/vdot-ft_sec2"] * FT_S2_TO_M_S2,
        "az": fdm["accelerations/wdot-ft_sec2"] * FT_S2_TO_M_S2,
        # Angular rates in rad/s
        "gx": fdm["velocities/p-rad_sec"],
        "gy": fdm["velocities/q-rad_sec"],
        "gz": fdm["velocities/r-rad_sec"],
        # Barometric pressure (Pa) and altitude (m)
        "abs_pressure": fdm["atmosphere/P-psf"] * 47.8803,  # lbf/ft² → Pa
        "pressure_alt": fdm["position/h-sl-ft"] * 0.3048,   # ft → m
        "temperature": fdm["atmosphere/T-R"] * 5.0 / 9.0 - 273.15,  # Rankine → °C
    }


def apply_controls(fdm: jsbsim.FGFDMExec, controls: list[float]) -> None:
    """Write HIL_ACTUATOR_CONTROLS values [0..1] into JSBSim motor inputs."""
    for i, val in enumerate(controls[:4]):
        try:
            fdm[f"fcs/throttle-cmd-norm[{i}]"] = max(0.0, min(1.0, val))
        except Exception:
            pass  # property may not exist for this model


# ── MAVLink helpers ───────────────────────────────────────────────────────────

def connect_mavlink(port: str, baud: int) -> mavutil.mavfile:
    log.info("Connecting to STM32 on %s @ %d baud ...", port, baud)
    conn = mavutil.mavlink_connection(port, baud=baud)
    log.info("Waiting for HEARTBEAT from FC ...")
    conn.wait_heartbeat(timeout=10)
    log.info(
        "HEARTBEAT received — system %d component %d",
        conn.target_system,
        conn.target_component,
    )
    return conn


def _safe(v: float, default: float = 0.0) -> float:
    """Return v if finite and packable as float32, else default."""
    if not math.isfinite(v):
        return default
    if abs(v) > 3.4e38:
        return default
    return v


def send_hil_sensor(conn: mavutil.mavfile, sensors: dict, time_usec: int) -> None:
    conn.mav.hil_sensor_send(
        time_usec=time_usec,
        xacc=_safe(sensors["ax"]),
        yacc=_safe(sensors["ay"]),
        zacc=_safe(sensors["az"]),
        xgyro=_safe(sensors["gx"]),
        ygyro=_safe(sensors["gy"]),
        zgyro=_safe(sensors["gz"]),
        xmag=0.0,
        ymag=0.0,
        zmag=0.0,
        abs_pressure=_safe(sensors["abs_pressure"], 101325.0),
        diff_pressure=0.0,
        pressure_alt=_safe(sensors["pressure_alt"]),
        temperature=_safe(sensors["temperature"], 25.0),
        fields_updated=0x1FFF,  # all fields valid
        id=0,
    )


def recv_actuator_controls(conn: mavutil.mavfile) -> list[float] | None:
    msg = conn.recv_match(type="HIL_ACTUATOR_CONTROLS", blocking=False)
    if msg is None:
        return None
    return list(msg.controls)


# ── vpython visualiser ────────────────────────────────────────────────────────

def init_viz():
    global _vp
    try:
        import vpython as vp
        _vp = vp
    except ImportError:
        sys.exit("vpython not found.  Install with: pip install vpython")

    vp.scene.title  = "HITL Drone"
    vp.scene.width  = 900
    vp.scene.height = 600
    vp.scene.background = vp.color.cyan
    vp.scene.forward = vp.vector(-1, -0.5, -1)

    # Ground
    vp.box(pos=vp.vector(0, -0.05, 0),
           size=vp.vector(200, 0.1, 200),
           color=vp.color.green, opacity=0.4)

    # Drone body: two crossing arms + centre hub
    drone = vp.compound([
        vp.box(size=vp.vector(0.55, 0.04, 0.12), color=vp.color.red),   # fore-aft arm
        vp.box(size=vp.vector(0.12, 0.04, 0.55), color=vp.color.red),   # left-right arm
        vp.sphere(radius=0.06, color=vp.color.white),                    # hub
    ])
    drone.pos = vp.vector(0, 30, 0)

    # Altitude text
    alt_label = vp.label(pos=vp.vector(0, 0, 0), text="alt: --",
                         xoffset=20, yoffset=20, border=4,
                         font="monospace", height=14)

    log.info("vpython visualiser open in browser.")
    return drone, alt_label


def update_viz(drone, alt_label, fdm) -> None:
    phi   = fdm["attitude/phi-rad"]    # roll
    theta = fdm["attitude/theta-rad"]  # pitch
    psi   = fdm["attitude/psi-rad"]    # yaw
    alt   = fdm["position/h-sl-ft"] * 0.3048

    if not (math.isfinite(phi) and math.isfinite(theta) and
            math.isfinite(psi) and math.isfinite(alt)):
        return

    vp = _vp
    # Body forward axis in NED, mapped to vpython (x=E, y=U, z=-N)
    cx, cy, cz = math.cos(phi), math.cos(theta), math.cos(psi)
    sx, sy, sz = math.sin(phi), math.sin(theta), math.sin(psi)
    fwd = vp.vector(cy * sz, -sy, cy * cz)          # drone forward
    up  = vp.vector(sx * sy * sz + cx * cz,
                    sx * cy,
                    sx * sy * cz - cx * sz)         # drone up

    drone.pos  = vp.vector(0, max(alt, 0), 0)
    drone.axis = fwd * 0.55
    drone.up   = up
    alt_label.pos  = drone.pos
    alt_label.text = f"alt: {alt:.1f} m  roll: {math.degrees(phi):.1f}°  pitch: {math.degrees(theta):.1f}°"


# ── Main loop ─────────────────────────────────────────────────────────────────

def run(args):
    fdm = init_jsbsim(args.jsbsim_root, args.model)
    if args.flightgear:
        enable_flightgear_output(fdm)

    drone_obj = alt_label = None
    if args.viz:
        drone_obj, alt_label = init_viz()

    conn = connect_mavlink(args.port, args.baud)

    last_controls: list[float] = [0.55, 0.55, 0.55, 0.55] + [0.0] * 12
    step = 0
    t_sim = 0.0

    log.info("HITL loop running at %.0f Hz.  Ctrl-C to stop.", 1.0 / DT_S)

    def _shutdown(sig, frame):
        log.info("Shutting down.")
        sys.exit(0)

    signal.signal(signal.SIGINT, _shutdown)

    while True:
        loop_start = time.monotonic()

        # 1. Advance simulation
        fdm.run()
        t_sim += DT_S

        # 2. Apply last known motor commands
        apply_controls(fdm, last_controls)

        # 3. Read sensor outputs and send HIL_SENSOR
        sensors = read_sensors(fdm)
        send_hil_sensor(conn, sensors, int(t_sim * 1e6))

        # 4. Collect any actuator commands the FC sent back
        controls = recv_actuator_controls(conn)
        if controls is not None:
            last_controls = controls
            if args.verbose:
                log.debug(
                    "HIL_ACTUATOR_CONTROLS  M0=%.3f  M1=%.3f  M2=%.3f  M3=%.3f",
                    *controls[:4],
                )

        # 5. Update visualiser at 30 Hz
        if args.viz and step % 33 == 0:
            update_viz(drone_obj, alt_label, fdm)

        # 6. Log position every 200 steps (5 Hz)
        if step % 200 == 0:
            alt = fdm["position/h-sl-ft"] * 0.3048
            lat = fdm["position/lat-geod-deg"]
            lon = fdm["position/long-gc-deg"]
            log.info("t=%.1f s  alt=%.1f m  lat=%.4f  lon=%.4f", t_sim, alt, lat, lon)
        step += 1

        # 7. Pace the loop to DT_S
        elapsed = time.monotonic() - loop_start
        sleep_time = DT_S - elapsed
        if sleep_time > 0:
            time.sleep(sleep_time)


def main():
    args = parse_args()
    if args.verbose:
        log.setLevel(logging.DEBUG)
    run(args)


if __name__ == "__main__":
    main()
