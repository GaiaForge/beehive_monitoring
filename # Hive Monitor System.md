# Hive Monitor System

A modular, solar-powered beehive monitoring system designed to provide critical insights into colony health and environmental conditions for beekeepers in regions with limited access to technology and infrastructure.

## Project Overview

The Hive Monitor system is an initiative led by GaiaForge to support grassroots beekeeping operations. It's specifically designed for deployment in Kisuno village (2.7438° S, 31.2471° E), where 76 bee colonies are managed across two stationary apiary locations surrounded by forest and farmland.

## Features

- **Environmental Monitoring**: Temperature, humidity, and barometric pressure sensing
- **Acoustic Analysis**: Detects hive sounds in different frequency bands to identify colony behavior
- **Motion Detection**: Monitors orientation and disturbances to the hive
- **Light Sensing**: Detects when the hive lid is removed or unusual light exposure
- **Weight Measurement**: Tracks honey production and colony size changes
- **Solar Powered**: Operates off-grid with solar panel and battery
- **Data Logging**: Records all sensor data to SD card in standardized formats
- **Low Power**: Sleep modes and power management for extended operation

## Hardware Components

- **Microcontroller**: Adafruit Feather nRF52840 Sense
- **Sensors**:
  - Built-in PDM microphone (acoustic monitoring)
  - SHT31 temperature and humidity sensor
  - BMP280 barometric pressure sensor
  - LSM6DS33 accelerometer and gyroscope
  - LIS3MDL magnetometer
  - APDS9960 light and color sensor
  - HX711 load cell amplifier with strain gauge (weight)
- **Power**:
  - 10W solar panel
  - LiFePO4 battery pack
  - BQ24650 MPPT solar charge controller
- **Storage**: microSD card for data logging
- **Communication**: Bluetooth Low Energy (BLE)

## Software Architecture

The software is organized into modular subsystems:

1. **Environmental Sensing**: Monitors temperature, humidity, and pressure
2. **Audio Processing**: Analyzes hive sounds for behavior patterns
3. **Motion Sensing**: Detects hive movement and orientation
4. **Light Sensing**: Monitors light levels for lid removal detection
5. **Weight Sensing**: Tracks hive weight changes
6. **Data Logging**: Records sensor data to SD card
7. **Power Management**: Controls sleep cycles and power conservation

## File Structure

```
hive-monitor/
├── main.ino                 # Main application entry point
├── config.h                 # Configuration parameters and thresholds
├── audio_processing.cpp     # Audio analysis module
├── audio_processing.h
├── env_sensors.cpp          # Environmental sensors module
├── env_sensors.h
├── motion_sensing.cpp       # Motion sensing module
├── motion_sensing.h
├── light_sensing.cpp        # Light sensing module
├── light_sensing.h
├── weight_sensing.cpp       # Weight measurement module
├── weight_sensing.h
├── data_logging.cpp         # Data storage module
├── data_logging.h
├── power_management.cpp     # Power management module
└── power_management.h
```

## Installation and Setup

1. Clone this repository
2. Open the project in Arduino IDE
3. Install required libraries:
   - Adafruit nRF52 board package
   - Adafruit Sensor library
   - Adafruit BME280 library
   - Adafruit SHT31 library
   - Adafruit LSM6DS library
   - Adafruit LIS3MDL library
   - Adafruit APDS9960 library
   - HX711 Arduino library
   - Arduino Low Power library
   - RTClib
   - SPI
   - SD
4. Configure the `config.h` file with your specific thresholds and settings
5. Upload to your Adafruit Feather nRF52840 Sense board

## Power Configuration

The system uses deep sleep between readings to conserve power. The default wake interval is 10 minutes, but this can be adjusted in the configuration. When battery voltage drops below thresholds, the system extends sleep time to preserve power.

## Data Logging Format

Data is logged to the SD card in CSV format with the following naming conventions:
- `LOG_YYYYMMDD.CSV` - Combined sensor data
- `AUDIO_YYYYMMDD.CSV` - Audio frequency bands and classification
- `ENV_YYYYMMDD.CSV` - Environmental readings
- `WEIGHT_YYYYMMDD.CSV` - Weight measurements
- `MOTION_YYYYMMDD.CSV` - Motion and orientation data
- `LIGHT_YYYYMMDD.CSV` - Light level readings

## Future Enhancements

- Web dashboard for data visualization
- MQTT or HTTP push from gateway
- LoRa or mesh networking for remote apiaries
- Machine learning for behavior prediction
- Mobile app for beekeepers

## License

This project is open source and available under the MIT License.

## Acknowledgments

Developed for GaiaForge to support sustainable beekeeping operations in regions with limited access to technology and infrastructure.