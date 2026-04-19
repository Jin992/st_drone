## Why

The STM32F405RG flight controller lacks WiFi and Bluetooth. An ESP32-C3 SuperMini co-processor will fill this gap, connected via UART for debug, configuration, and simulation when the drone is not armed. Before UART integration can happen, the ESP32-C3 needs its own firmware project — starting with a verified LED blink to confirm the toolchain, build structure, and board bring-up work.

## What Changes

- Add `esp32/` directory at project root as a standalone ESP-IDF project
- Add `esp32/CMakeLists.txt` (ESP-IDF project root, built separately from STM32)
- Add `esp32/main/CMakeLists.txt` and `esp32/main/main.c` with LED blink on GPIO8
- Add `esp32/sdkconfig.defaults` committing target and flash size configuration
- Update `.gitignore` to exclude `esp32/build/`

## Capabilities

### New Capabilities

- `esp32-project-setup`: Standalone ESP-IDF project structure for the ESP32-C3 co-processor firmware, with its own CMake build, sdkconfig, and initial LED blink application

### Modified Capabilities

<!-- none -->

## Impact

- No changes to the STM32 CMake build or any existing source files
- Requires ESP-IDF v5.x installed on the developer machine with `IDF_PATH` set and `idf.py` in PATH
- New `esp32/` directory and `esp32/build/` (gitignored) added to repo
- LED on GPIO8 (onboard blue LED on ESP32-C3 SuperMini)
