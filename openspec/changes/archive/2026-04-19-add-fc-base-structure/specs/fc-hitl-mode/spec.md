## ADDED Requirements

### Requirement: HITL mode connects to JSBSim over USB-CDC
In HITL mode the STM32F405 SHALL communicate with a PC running JSBSim using MAVLink
over a USB-CDC virtual serial port on PA11/PA12. No additional hardware adapter SHALL
be required beyond a USB-C cable.

#### Scenario: STM32 appears as serial port on PC
- **WHEN** the STM32 running the hitl-debug firmware is connected via USB-C
- **THEN** the PC enumerates it as a virtual COM port (e.g., /dev/tty.usbmodem* on macOS)

#### Scenario: JSBSim connects to the virtual port
- **WHEN** JSBSim is configured to send MAVLink HIL_SENSOR packets to the serial port
- **THEN** the STM32 receives and parses the packets without error

### Requirement: hal/hitl/ reads sensor data from HIL_SENSOR MAVLink messages
The `hal/hitl/` backend SHALL implement `hal_imu_read()` by parsing incoming
`HIL_SENSOR` (#107) MAVLink messages from the USB-CDC stream and populating
the `imu_data_t` struct.

#### Scenario: IMU data populated from sim
- **WHEN** a valid HIL_SENSOR message is received from JSBSim
- **THEN** `hal_imu_read()` returns the accelerometer and gyroscope values from that message

#### Scenario: Stale data returned when no message received
- **WHEN** no HIL_SENSOR message has been received within 100 ms
- **THEN** `hal_imu_read()` returns the last known values and sets a staleness flag

### Requirement: hal/hitl/ sends actuator commands as HIL_ACTUATOR_CONTROLS
The `hal/hitl/` backend SHALL implement `hal_pwm_set()` by encoding motor throttle
values into a `HIL_ACTUATOR_CONTROLS` (#93) MAVLink message and transmitting it
over the USB-CDC stream.

#### Scenario: Motor outputs forwarded to simulator
- **WHEN** the flight control loop calls `hal_pwm_set()` with four motor values
- **THEN** a HIL_ACTUATOR_CONTROLS packet is sent to JSBSim within the same 1 ms tick

### Requirement: hitl-debug CMake preset selects HITL backend
The project SHALL provide a `hitl-debug` CMake preset that uses the ARM toolchain,
links `hal/hitl/` instead of `hal/stm32/`, and enables USB-CDC init in `main.c`.

#### Scenario: HITL binary builds without errors
- **WHEN** a developer runs `cmake --preset hitl-debug && cmake --build --preset hitl-debug`
- **THEN** the build completes and produces a flashable `.bin` file
