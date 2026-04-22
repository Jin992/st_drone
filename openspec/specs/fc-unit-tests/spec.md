## ADDED Requirements

### Requirement: Pure-logic modules are unit tested on host
The project SHALL include a `tests/` directory with unit tests for `flight/pid.c`
and `flight/mixer.c` that compile and run as a native x86 binary. No STM32 toolchain
or hardware SHALL be required to run the tests.

#### Scenario: Tests run on dev machine
- **WHEN** a developer runs `cmake --preset host-debug && cmake --build --preset host-debug && ctest --preset host-debug`
- **THEN** all tests pass on the developer's Mac or Linux machine without a connected STM32

#### Scenario: Test binary links no platform headers
- **WHEN** the host-debug target is compiled
- **THEN** no STM32 HAL headers are included in the compilation unit graph

### Requirement: PID unit test verifies step response
The `tests/test_pid.c` file SHALL verify that the PID controller produces the correct
output for a known step input, and that the integrator clamps at its configured limit.

#### Scenario: Proportional step response
- **WHEN** a PID with Kp=1.0, Ki=0, Kd=0 is updated with error=1.0
- **THEN** the output equals 1.0

#### Scenario: Integrator windup clamping
- **WHEN** a PID with Ki>0 and an integral limit is updated with a constant error for many steps
- **THEN** the integral term does not exceed the configured limit

### Requirement: Mixer unit test verifies quad-X output
The `tests/test_mixer.c` file SHALL verify that given known roll/pitch/yaw/throttle
inputs the mixer produces the correct per-motor outputs for a quad-X configuration.

#### Scenario: Pure throttle produces equal motor outputs
- **WHEN** mixer is called with throttle=0.5, roll=0, pitch=0, yaw=0
- **THEN** all four motor outputs equal 0.5

#### Scenario: Roll input differentiates left and right motors
- **WHEN** mixer is called with throttle=0.5, roll=0.1, pitch=0, yaw=0
- **THEN** motors on the right side increase and motors on the left side decrease by equal amounts
