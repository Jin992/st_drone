# ESP32-C3 Firmware

Co-processor firmware for the drone's WiFi/BLE interface.

## Prerequisites

ESP-IDF v6.1 installed at `~/esp/esp-idf`. One-time setup:

```bash
git clone --recursive https://github.com/espressif/esp-idf.git ~/esp/esp-idf
cd ~/esp/esp-idf && ./install.sh esp32c3
```

## Build

```bash
source env.sh
idf.py --preset debug build
```

## Flash

### Enter Download Mode

1. Hold down the **BOOT** button
2. Press the **RESET** button once
3. Release **RESET** first, then release **BOOT**

The board will enter download mode and appear as a serial port.

### Flash the firmware

```bash
idf.py --preset debug -p /dev/tty.usbmodem* flash
```

Replace `/dev/tty.usbmodem*` with the actual port (check `ls /dev/tty.usbmodem*`).

## Presets

| Preset    | Description              |
|-----------|--------------------------|
| `debug`   | Development build        |
| `release` | Optimized release build  |
