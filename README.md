# Hive Monitor System

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A modular, solar-powered beehive monitoring system with adaptive learning capabilities. Designed to help beekeepers track colony health and improve yields, especially in regions with limited infrastructure and technology access.

<img src="docs/images/hive-monitor-concept.png" alt="Hive Monitor Concept" width="600"/>

## 🐝 Overview

The Hive Monitor system provides continuous monitoring of beehive conditions by collecting data on:
- Temperature and humidity patterns
- Colony sounds across different frequency bands
- Hive weight for honey production tracking
- Vibration and movement
- Light levels for detecting hive opening

What makes this system unique is its **adaptive learning capability** that builds a customized baseline for each colony, adapting thresholds based on observed patterns and reducing false alarms.

## ✨ Features

- **Acoustic Analysis:** Monitors bee sounds in specific frequency bands to detect normal activity, queen piping, swarming preparation, and disturbances
- **Environmental Tracking:** Records temperature, humidity, and barometric pressure inside the hive
- **Weight Monitoring:** Tracks honey production and detects sudden changes that could indicate swarming or theft
- **Intelligent Alerting:** Learns normal patterns for each colony and only alerts on significant deviations
- **Solar Powered:** Designed for long-term, maintenance-free operation with efficient power management
- **Durable & Weather-resistant:** Built to withstand outdoor conditions
- **Data Logging:** Records all sensor data to SD card for later analysis
- **BLE Connectivity:** Optional wireless access to readings and configuration via smartphone

## 🧠 Adaptive Learning

Unlike fixed-threshold monitoring systems, Hive Monitor includes an advanced learning system that:

1. **Builds Colony-specific Baselines:** Learns what's normal for your specific hive
2. **Recognizes Daily & Seasonal Patterns:** Understands how colony behavior changes throughout the day and year
3. **Improves Over Time:** Continuously refines its understanding of normal behavior
4. **Adapts to Environment:** Automatically adjusts to local climate and conditions
5. **Reduces False Alarms:** Generates alerts only when conditions truly warrant attention

This makes the system more useful over time as it learns the specific characteristics of each colony.

## 🔧 Hardware

### Required Components:
- Adafruit Feather nRF52840 Sense board
- HX711 load cell amplifier with strain gauge (for weight sensing)
- MicroSD card adapter
- LiFePO4 battery
- Solar panel (10W recommended)
- Solar charge controller
- Weatherproof enclosure

### Optional Components:
- OLED display for status information
- Additional external sensors
- GSM/LoRa modules for remote connectivity

## 💻 Software Architecture

The system is organized into modular components:

```
hive-monitor/
├── main.ino                 # Main application entry point
├── config.h                 # Configuration parameters
├── config.cpp               # Configuration management
├── audio_processing.cpp     # Sound analysis 
├── audio_processing.h
├── env_sensors.cpp          # Environmental monitoring
├── env_sensors.h
├── motion_sensing.cpp       # Motion detection
├── motion_sensing.h
├── light_sensing.cpp        # Light level monitoring
├── light_sensing.h
├── weight_sensing.cpp       # Weight measurement
├── weight_sensing.h
├── data_logging.cpp         # Data storage
├── data_logging.h
├── power_management.cpp     # Battery/solar management
├── power_management.h
├── learning.cpp             # Adaptive learning system
└── learning.h
```

## 🚀 Getting Started

### Installation

1. Clone this repository
```bash
git clone https://github.com/yourusername/hive-monitor.git
```

2. Open the project in Arduino IDE

3. Install required libraries:
   - Adafruit nRF52 board package
   - Adafruit Sensor library
   - Adafruit BME280 library
   - Adafruit SHT31 library
   - Adafruit LSM6DS library
   - Adafruit LIS3MDL library
   - Adafruit APDS9960 library
   - ArduinoJSON
   - HX711 Arduino library
   - Arduino Low Power library
   - RTClib
   - SPI
   - SD

4. Configure the `config.h` file with your specific settings

5. Upload to your Adafruit Feather nRF52840 Sense board

### Configuration

The system can be configured by editing `config.h` or by placing a `CONFIG.TXT` or `CONFIG.JSON` file on the SD card. Example configuration:

```json
{
  "DEVICE_ID": "HIVE01",
  "WAKE_INTERVAL_MINUTES": 5,
  "ENABLE_LEARNING": 1,
  "TEMP_ALERT_LOW": 32.0,
  "TEMP_ALERT_HIGH": 38.0
}
```

## 📊 Data Format

Data is logged to the SD card in CSV format with the following files:
- `LOG_YYYYMMDD.CSV` - Combined sensor data
- `AUDIO_YYYYMMDD.CSV` - Sound frequency analysis
- `ENV_YYYYMMDD.CSV` - Environmental readings
- `WEIGHT_YYYYMMDD.CSV` - Weight measurements

Example log entry:
```
2025-04-10T18:00:00Z,34.7,62.1,1012.3,42.78,3,0.03,-0.02,0.98,0.72,0.14,0.04,0.01,3.9,Nominal
```

## 📱 Mobile Interface

A companion mobile app is planned that will provide:
- Real-time hive status
- Historical data visualization
- Alert notifications
- Configuration updates
- Data download without removing SD card

## 🌱 Future Development

Planned enhancements:
- Web dashboard for data visualization
- Machine learning for behavior prediction
- Remote connectivity options (LoRa, GSM)
- Multi-hive networking
- Rich data analytics and comparative analysis

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 📚 Acknowledgments

- Developed to support sustainable beekeeping operations
- Special thanks to GaiaForge for the initiative
- Inspired by beekeepers in Kisuno village
