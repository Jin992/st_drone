## Context

The project currently builds only STM32F405RG firmware using CMake + Conan with an ARM Cortex-M4 toolchain. The ESP32-C3 SuperMini (RISC-V, riscv32-esp-elf toolchain) requires a completely separate build system — ESP-IDF — which cannot be mixed into the same CMake configure invocation as the STM32 build.

ESP-IDF is CMake-based internally but managed through its own tooling (`idf.py`), component system, and KConfig configuration. The two projects coexist in the same repository but are built independently.

## Goals / Non-Goals

**Goals:**
- Create a working ESP-IDF project in `esp32/` that compiles and flashes to the ESP32-C3 SuperMini
- LED blink on GPIO8 (onboard blue LED) as board bring-up verification
- Committed `sdkconfig.defaults` so any developer can build without running `menuconfig`
- Zero impact on the existing STM32 build

**Non-Goals:**
- UART communication between STM32 and ESP32 (future change)
- Power switching / armed-state disable logic (future change)
- WiFi or Bluetooth initialization
- Integration of ESP32 build into the STM32 CMake invocation

## Decisions

### Separate build, not integrated CMake
**Decision**: `esp32/` is a standalone ESP-IDF project built with `idf.py build`, not linked to the root CMakeLists.txt via `add_subdirectory` or `ExternalProject_Add`.

**Rationale**: The STM32 and ESP32-C3 have different CPU architectures (ARM vs RISC-V) and entirely different toolchains. CMake is single-toolchain per configure invocation. Integrating them would require complex `ExternalProject_Add` wiring that adds friction with no benefit at this stage — the two firmwares are flashed to separate physical chips independently.

**Alternative considered**: `add_custom_target` in root CMakeLists calling `idf.py build`. Rejected because it silently fails if ESP-IDF is not installed and creates false coupling between unrelated builds.

### sdkconfig.defaults over committed sdkconfig
**Decision**: Commit `sdkconfig.defaults` (2-3 lines of overrides) and gitignore the generated `sdkconfig`.

**Rationale**: The full `sdkconfig` is 600+ auto-generated lines that changes with every ESP-IDF version update, creating noisy diffs. `sdkconfig.defaults` contains only intentional overrides and is stable. ESP-IDF merges defaults with built-in values at build time.

### C (not C++) for ESP32 application code
**Decision**: `main.c` in plain C, matching the STM32 side of the project.

**Rationale**: Consistent with project convention. ESP-IDF supports both; C is sufficient for the blink and for the eventual UART comms layer.

### GPIO8 for LED
**Decision**: Onboard LED is on GPIO8 on the ESP32-C3 SuperMini board.

**Rationale**: Hardware-fixed. The SuperMini routes its blue onboard LED to GPIO8. No configuration choice here.

## Risks / Trade-offs

**ESP-IDF not installed** → `idf.py build` fails with a clear error. Mitigation: document the prerequisite in a comment in `esp32/CMakeLists.txt` and in the repo README when it gets one.

**sdkconfig regeneration** → First `idf.py build` generates a full `sdkconfig` from defaults; if a developer commits it accidentally, future diffs are noisy. Mitigation: `.gitignore` entry for `esp32/build/` and `esp32/sdkconfig`.

**ESP-IDF version drift** → Different ESP-IDF versions may generate different `sdkconfig` defaults. Mitigation: document the target version (v5.x) in `sdkconfig.defaults` as a comment.
