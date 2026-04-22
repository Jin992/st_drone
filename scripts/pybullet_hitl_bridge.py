#!/usr/bin/env python3
"""
gym-pybullet-drones ↔ STM32 HITL bridge.

Replaces JSBSim with a PyBullet quadrotor simulation that opens a 3D GUI window
and communicates with the STM32 HITL firmware over USB-CDC via MAVLink.

Data flow:
  PyBullet FDM state  →  MAVLink HIL_SENSOR  →  STM32 (hal/hitl backend)
  STM32 control output  →  MAVLink HIL_ACTUATOR_CONTROLS  →  PyBullet inputs

Usage:
  python scripts/pybullet_hitl_bridge.py --port /dev/tty.usbmodem*

Dependencies:
  pip install git+https://github.com/utiasDSL/gym-pybullet-drones.git pymavlink
"""

import argparse
import math
import signal
import sys
import time
import logging
import warnings
import numpy as np

warnings.filterwarnings("ignore")

try:
    from gym_pybullet_drones.envs import CtrlAviary
    from gym_pybullet_drones.utils.enums import DroneModel, Physics
except ImportError:
    sys.exit("gym-pybullet-drones not found.\n"
             "Install: pip install git+https://github.com/utiasDSL/gym-pybullet-drones.git")

try:
    from pymavlink import mavutil
except ImportError:
    sys.exit("pymavlink not found.  Install: pip install pymavlink")


logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s  %(levelname)-8s  %(message)s",
    datefmt="%H:%M:%S",
)
log = logging.getLogger("pybullet_hitl")


def parse_args():
    p = argparse.ArgumentParser(description="PyBullet HITL bridge for STM32 drone FC")
    p.add_argument("--port", default="/dev/tty.usbmodem*",
                   help="Serial port of the STM32 USB-CDC device")
    p.add_argument("--baud", type=int, default=115200)
    p.add_argument("--no-gui", action="store_true",
                   help="Disable the PyBullet 3D window")
    p.add_argument("--verbose", action="store_true",
                   help="Print every HIL_ACTUATOR_CONTROLS packet")
    return p.parse_args()


# ── Quaternion helper ─────────────────────────────────────────────────────────

def quat_rotate_inv(v: np.ndarray, q: np.ndarray) -> np.ndarray:
    """Rotate vector v from world frame to body frame using quaternion q (x,y,z,w)."""
    qx, qy, qz, qw = q
    t = 2.0 * np.cross([qx, qy, qz], v)
    return v - qw * t + np.cross([qx, qy, qz], t)


# ── Simulation ────────────────────────────────────────────────────────────────

def init_sim(gui: bool):
    env = CtrlAviary(
        drone_model=DroneModel.CF2X,
        num_drones=1,
        physics=Physics.PYB,
        gui=gui,
        record=False,
    )
    obs, _ = env.reset()
    hover_frac = env.HOVER_RPM / env.MAX_RPM
    dt = env.CTRL_TIMESTEP
    log.info("PyBullet ready — hover throttle=%.2f  dt=%.4f s (%.0f Hz)",
             hover_frac, dt, 1.0 / dt)
    return env, obs, hover_frac, dt


def step_sim(env, obs, motor_cmds: list[float]):
    """Step one physics frame. motor_cmds: 4 values in [0, 1]."""
    rpms = np.clip(motor_cmds[:4], 0.0, 1.0) * env.MAX_RPM
    action = rpms.reshape(1, 4)
    obs, _, _, _, _ = env.step(action)
    return obs


def get_imu(obs, prev_vel: np.ndarray, dt: float):
    """Extract IMU-equivalent data from PyBullet observation."""
    state = obs[0]
    pos   = state[0:3]
    quat  = state[3:7]    # (x, y, z, w)
    vel   = state[10:13]
    ang_world = state[13:16]

    # Linear acceleration in world frame (finite-difference) + gravity correction
    accel_world = (vel - prev_vel) / dt
    accel_world[2] += 9.81   # IMU reads +g upward when stationary

    # Rotate to body frame
    accel_body = quat_rotate_inv(accel_world, quat)
    gyro_body  = quat_rotate_inv(ang_world,   quat)

    # Simple barometric model
    alt = pos[2]
    pressure = 101325.0 * (1.0 - 2.25577e-5 * alt) ** 5.25588

    return {
        "ax": float(accel_body[0]), "ay": float(accel_body[1]), "az": float(accel_body[2]),
        "gx": float(gyro_body[0]),  "gy": float(gyro_body[1]),  "gz": float(gyro_body[2]),
        "abs_pressure": float(pressure),
        "pressure_alt": float(alt),
        "temperature": 25.0,
    }, vel.copy()


# ── MAVLink ───────────────────────────────────────────────────────────────────

def connect_mavlink(port: str, baud: int):
    log.info("Connecting to STM32 on %s @ %d baud ...", port, baud)
    conn = mavutil.mavlink_connection(port, baud=baud)
    log.info("Waiting for HEARTBEAT ...")
    conn.wait_heartbeat(timeout=15)
    log.info("HEARTBEAT received — system %d component %d",
             conn.target_system, conn.target_component)
    return conn


def _safe(v: float, default: float = 0.0) -> float:
    return v if math.isfinite(v) and abs(v) < 3.4e38 else default


def send_hil_sensor(conn, sensors: dict, time_usec: int) -> None:
    conn.mav.hil_sensor_send(
        time_usec=time_usec,
        xacc=_safe(sensors["ax"]),
        yacc=_safe(sensors["ay"]),
        zacc=_safe(sensors["az"]),
        xgyro=_safe(sensors["gx"]),
        ygyro=_safe(sensors["gy"]),
        zgyro=_safe(sensors["gz"]),
        xmag=0.0, ymag=0.0, zmag=0.0,
        abs_pressure=_safe(sensors["abs_pressure"], 101325.0),
        diff_pressure=0.0,
        pressure_alt=_safe(sensors["pressure_alt"]),
        temperature=_safe(sensors["temperature"], 25.0),
        fields_updated=0x1FFF,
        id=0,
    )


def recv_actuator_controls(conn) -> list[float] | None:
    msg = conn.recv_match(type="HIL_ACTUATOR_CONTROLS", blocking=False)
    return list(msg.controls) if msg else None


# ── Main loop ─────────────────────────────────────────────────────────────────

def run(args):
    env, obs, hover_frac, dt = init_sim(gui=not args.no_gui)
    conn = connect_mavlink(args.port, args.baud)

    motor_cmds = [hover_frac] * 4
    prev_vel   = np.zeros(3)
    t_sim      = 0.0
    step       = 0

    log.info("HITL loop running at %.0f Hz.  Ctrl-C to stop.", 1.0 / dt)

    def _shutdown(sig, frame):
        log.info("Shutting down.")
        env.close()
        sys.exit(0)

    signal.signal(signal.SIGINT, _shutdown)

    while True:
        loop_start = time.monotonic()

        # 1. Step physics
        obs = step_sim(env, obs, motor_cmds)
        t_sim += dt

        # 2. Read IMU and send HIL_SENSOR
        sensors, prev_vel = get_imu(obs, prev_vel, dt)
        send_hil_sensor(conn, sensors, int(t_sim * 1e6))

        # 3. Collect motor commands from FC
        cmds = recv_actuator_controls(conn)
        if cmds is not None:
            motor_cmds = cmds[:4]
            if args.verbose:
                log.debug("M0=%.3f  M1=%.3f  M2=%.3f  M3=%.3f", *motor_cmds)

        # 4. Log at 1 Hz
        if step % int(1.0 / dt) == 0:
            state = obs[0]
            log.info("t=%.1f s  alt=%.2f m  roll=%.1f°  pitch=%.1f°  yaw=%.1f°",
                     t_sim, state[2],
                     math.degrees(state[7]),
                     math.degrees(state[8]),
                     math.degrees(state[9]))
        step += 1

        # 5. Pace to physics timestep
        elapsed = time.monotonic() - loop_start
        sleep = dt - elapsed
        if sleep > 0:
            time.sleep(sleep)


def main():
    args = parse_args()
    if args.verbose:
        log.setLevel(logging.DEBUG)
    run(args)


if __name__ == "__main__":
    main()
