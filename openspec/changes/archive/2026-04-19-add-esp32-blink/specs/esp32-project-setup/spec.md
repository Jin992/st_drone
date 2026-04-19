## ADDED Requirements

### Requirement: ESP32-C3 project directory exists
The repository SHALL contain an `esp32/` directory at the project root with a valid ESP-IDF project structure, buildable independently of the STM32 CMake project.

#### Scenario: Developer builds ESP32 firmware
- **WHEN** a developer with ESP-IDF v5.x installed runs `idf.py build` inside `esp32/`
- **THEN** the build completes without errors and produces `esp32/build/esp32_drone.bin`

#### Scenario: STM32 build is unaffected
- **WHEN** a developer builds the STM32 project via the existing CMake preset
- **THEN** the build completes without errors and the `esp32/` directory is not referenced

### Requirement: ESP-IDF project uses sdkconfig.defaults
The `esp32/` directory SHALL contain an `sdkconfig.defaults` file that sets the target to `esp32c3` and flash size to 4MB. The generated `sdkconfig` SHALL be excluded from version control.

#### Scenario: First-time build generates sdkconfig
- **WHEN** a developer runs `idf.py build` for the first time with no existing `sdkconfig`
- **THEN** ESP-IDF merges `sdkconfig.defaults` with built-in defaults and generates a complete `sdkconfig` file

#### Scenario: sdkconfig is gitignored
- **WHEN** a developer inspects git status after building
- **THEN** the generated `esp32/sdkconfig` and `esp32/build/` directory do not appear as untracked files

### Requirement: LED blink on GPIO8
The ESP32-C3 firmware SHALL blink the onboard LED connected to GPIO8 at a 1 Hz rate (500 ms on, 500 ms off) as board bring-up verification. The application SHALL be written in C using ESP-IDF APIs.

#### Scenario: LED blinks after flash
- **WHEN** the compiled firmware is flashed to an ESP32-C3 SuperMini board
- **THEN** the blue onboard LED blinks at approximately 1 Hz

#### Scenario: No Python application code
- **WHEN** a developer inspects the `esp32/` directory
- **THEN** all application source files have `.c` or `.h` extensions (no `.py` files authored by the project)
