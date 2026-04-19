## 1. Repository Structure

- [x] 1.1 Create `esp32/` directory at project root
- [x] 1.2 Add `esp32/build/` and `esp32/sdkconfig` to `.gitignore`

## 2. ESP-IDF Project Files

- [x] 2.1 Create `esp32/CMakeLists.txt` as ESP-IDF project root (includes `$ENV{IDF_PATH}/tools/cmake/project.cmake`, project name `esp32_drone`)
- [x] 2.2 Create `esp32/main/` directory
- [x] 2.3 Create `esp32/main/CMakeLists.txt` registering the main component via `idf_component_register`
- [x] 2.4 Create `esp32/sdkconfig.defaults` with `CONFIG_IDF_TARGET="esp32c3"` and `CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y`

## 3. LED Blink Application

- [x] 3.1 Create `esp32/main/main.c` with `app_main` entry point
- [x] 3.2 Configure GPIO8 as output using `gpio_reset_pin` and `gpio_set_direction`
- [x] 3.3 Implement blink loop using `gpio_set_level` and `vTaskDelay(pdMS_TO_TICKS(500))`

## 4. Verification

- [x] 4.1 Run `idf.py build` inside `esp32/` and confirm it completes without errors
- [x] 4.2 Flash to ESP32-C3 SuperMini with `idf.py -p <port> flash` and confirm LED blinks at ~1 Hz
- [x] 4.3 Confirm STM32 CMake build still works after repo changes
- [x] 4.4 Confirm `git status` shows no untracked `esp32/build/` or `esp32/sdkconfig` files
