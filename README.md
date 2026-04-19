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
