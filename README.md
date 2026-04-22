# ST Drone

Flight controller firmware for STM32F405RGT6 with ESP32-C3 SuperMini as WiFi/BLE co-processor.

## Hardware Wiring

### UART Connection (STM32 ↔ ESP32-C3)

```
WeAct STM32F405RGT6          ESP32-C3 SuperMini
───────────────────          ──────────────────
        PA2  (TX) ──────────► GPIO3 (RX)
        PA3  (RX) ◄────────── GPIO4 (TX)
             GND  ──────────── GND
             3V3  ──────────── 3V3
```

> Both boards run at 3.3V — no level shifter needed.
> USART2 (PA2/PA3) is used — physically adjacent to the 3V3 pin on the WeAct board.

### Full Board Diagram

```
  WeAct STM32F405RGT6            ESP32-C3 SuperMini
  ┌─────────────────┐            ┌──────────────┐
  │             3V3 │────────────│ 3V3          │
  │             GND │────────────│ GND          │
  │             PA2 │──── TX ───►│ GPIO3        │
  │             PA3 │◄─── RX ───│ GPIO4        │
  │                 │            │              │
  │  [SWD header]   │            │ [USB-C]      │
  │  [USB-C]        │            │ GPIO8 ── LED │
  └─────────────────┘            └──────────────┘
```

---

## STM32 Firmware

### Prerequisites

- `arm-none-eabi-gcc` 14.x
- CMake ≥ 3.21
- Ninja
- Conan 2.x
- OpenOCD (for flashing)

### Build

```bash
# Install dependencies (first time)
conan install . --profile profiles/arm-none-eabi --build=missing

# Configure
cmake --preset debug

# Build
cmake --build --preset debug
```

### Flash

```bash
cmake --build --preset debug --target flash
```

This runs OpenOCD via ST-Link.

### Unit Tests (host, no hardware)

```bash
cd tests
cmake --preset host-debug
cmake --build --preset host-debug
ctest --preset host-debug
```

Tests compile and run natively on a Mac/Linux machine. No STM32 required.

---

## HITL Simulation (PyBullet)

Hardware-In-The-Loop runs the real STM32 firmware with simulated sensor data from
gym-pybullet-drones on the PC. The flight control code is unchanged — only the HAL backend differs.

### How it works

```
gym-pybullet-drones (PC)
    │  MAVLink HIL_SENSOR (#107)            ← synthetic IMU/baro from PyBullet
    │  MAVLink HIL_ACTUATOR_CONTROLS (#93)  → motor RPMs back into simulation
    │
    │  USB-C cable  (USB-CDC virtual serial port)
    ▼
STM32F405  —  HITL firmware  (hal/hitl/ backend)
    hal_imu_read()   ← parses HIL_SENSOR
    hal_pwm_set()    → sends HIL_ACTUATOR_CONTROLS
```

### Build HITL firmware

```bash
conan install . --profile profiles/arm-none-eabi --build=missing
cmake --preset hitl-debug
cmake --build --preset hitl-debug
```

Flash via ST-Link:

```bash
cmake --build --preset hitl-debug --target flash
```

### Connect STM32 to PC

Disconnect ST-Link. Connect the WeAct board's **USB-C port** to the PC.
The HITL firmware enumerates as a virtual COM port:

- **macOS**: `/dev/tty.usbmodem*`
- **Linux**: `/dev/ttyACM0`

### Run the bridge

The bridge script steps the PyBullet physics engine, sends `HIL_SENSOR` to the STM32,
and feeds `HIL_ACTUATOR_CONTROLS` back into the simulation. A 3D GUI window opens
automatically showing the CF2X quadrotor in real time.

```bash
pip install git+https://github.com/utiasDSL/gym-pybullet-drones.git pymavlink

python scripts/pybullet_hitl_bridge.py \
    --port /dev/tty.usbmodem*
```

| Argument | Default | Description |
|---|---|---|
| `--port` | `/dev/tty.usbmodem*` | STM32 USB-CDC serial port |
| `--baud` | `115200` | Serial baud rate |
| `--no-gui` | off | Disable the PyBullet 3D window |
| `--verbose` | off | Print every actuator packet |

The script runs at ~240 Hz (the CF2X control timestep).
It waits for a MAVLink `HEARTBEAT` from the FC before starting the loop.
The drone starts with motors pre-set to hover throttle so it doesn't free-fall
before the FC control loop takes over.

### Prerequisites

- `pip install git+https://github.com/utiasDSL/gym-pybullet-drones.git pymavlink`
- STM32 flashed with the HITL firmware (`HAL_BACKEND=hitl`)

---

## ESP32-C3 Firmware

See [esp32/README.md](esp32/README.md) for full instructions.

### Quick start

```bash
source esp32/env.sh
idf.py --preset debug build
idf.py --preset debug -p /dev/tty.usbmodem* flash
```

### Enter Download Mode (if not auto-detected)

1. Hold down the **BOOT** button
2. Press the **RESET** button once
3. Release **RESET** first, then release **BOOT**

The board will enter download mode and appear as a serial port.
