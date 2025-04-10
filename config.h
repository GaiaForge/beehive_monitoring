/**
 * Hive Monitor System - Configuration File
 * 
 * This file contains all configurable parameters for the hive monitor system.
 * Edit this file to customize thresholds, sampling rates, and system behavior.
 * Parameters can be overridden by a CONFIG.TXT file on the SD card.
 */

 #ifndef HIVE_MONITOR_CONFIG_H
 #define HIVE_MONITOR_CONFIG_H
 
 #include <Arduino.h>
 
 // System identification
 #define DEVICE_ID                "HIVE01"    // Unique identifier for this monitor
 #define FIRMWARE_VERSION         "1.1"       // Current firmware version
 
 // General system configuration
 #define WAKE_INTERVAL_MINUTES    10          // How often to wake and record data (1, 5, 10, 30, 60)
 #define ENABLE_BLE               1           // Enable Bluetooth communications (1=on, 0=off)
 #define ENABLE_STATUS_LED        1           // Enable status LED blinking (1=on, 0=off in field deployment)
 #define ENABLE_LEARNING          1           // Enable adaptive learning system (1=on, 0=off)
 #define SERIAL_BAUD              115200      // Serial baud rate for debugging
 
 // Pin definitions
 #define SD_CS_PIN                5           // SD card chip select pin
 #define HX711_DATA_PIN           6           // HX711 data pin
 #define HX711_CLOCK_PIN          5           // HX711 clock pin
 #define VBAT_PIN                 A7          // Battery voltage monitoring pin
 #define LED_PIN                  13          // Status LED pin
 
 // Microphone sensing configuration
 #define MIC_SAMPLING_RATE        16000       // Sampling rate in Hz
 #define MIC_SAMPLE_DURATION      1000        // Duration to sample in ms
 #define FFT_SIZE                 512         // FFT size for audio processing
 
 // Audio classification thresholds (can be overridden by learning system)
 #define THRESH_B1                0.6f        // Normal hum (200-300 Hz)
 #define THRESH_B2                0.4f        // Queen piping (300-600 Hz)
 #define THRESH_B3                0.3f        // Swarming agitation (600-1000 Hz)
 #define THRESH_B4                0.2f        // Alarm or disturbance (1000-3000 Hz)
 #define THRESH_SILENT            0.1f        // Possible absconding
 #define MIN_AUDIO_THRESHOLD      0.05f       // Minimum threshold regardless of learning
 
 // Environmental thresholds (can be overridden by learning system)
 #define TEMP_ALERT_LOW           30.0f       // Lower temperature threshold in °C
 #define TEMP_ALERT_HIGH          38.0f       // Upper temperature threshold in °C
 #define HUM_ALERT_LOW            50.0f       // Lower humidity threshold in %
 #define HUM_ALERT_HIGH           70.0f       // Upper humidity threshold in %
 #define MIN_SAFE_TEMP            25.0f       // Absolute minimum safe temperature
 #define MAX_SAFE_TEMP            42.0f       // Absolute maximum safe temperature
 #define MIN_SAFE_HUMIDITY        30.0f       // Absolute minimum safe humidity
 #define MAX_SAFE_HUMIDITY        90.0f       // Absolute maximum safe humidity
 
 // Anomaly detection thresholds
 #define TEMP_ANOMALY_THRESHOLD   3.0f        // Z-score threshold for temperature anomalies
 #define HUMIDITY_ANOMALY_THRESHOLD 3.0f      // Z-score threshold for humidity anomalies
 #define WEIGHT_ANOMALY_THRESHOLD 3.5f        // Z-score threshold for weight anomalies
 #define WEIGHT_CHANGE_THRESHOLD  2.0f        // Std deviations for significant weight change
 
 // Light sensing thresholds
 #define LIGHT_THRESHOLD          100         // Threshold for detecting lid removal (lux)
 
 // Motion sensing thresholds
 #define MOTION_ALERT_THRESHOLD   12.0f       // Motion alert threshold (m/s²)
 #define MOTION_WARNING_THRESHOLD 10.5f       // Motion warning threshold (m/s²)
 
 // Weight sensing configuration
 #define WEIGHT_CALIBRATION       22000.0f    // Calibration factor for load cell
 #define WEIGHT_SAMPLES           5           // Number of weight samples to average
 #define WEIGHT_CHANGE_ALERT      2.0f        // Significant weight change threshold (kg)
 
 // Power management
 #define LOW_BATTERY_THRESHOLD    3.5f        // Low battery voltage threshold (V)
 #define CRITICAL_BATTERY_THRESHOLD 3.2f      // Critical battery level (V)
 #define EXTENDED_SLEEP_MINUTES   60          // Extended sleep time when battery low (minutes)
 
 // Data logging configuration
 #define LOG_FILE_PREFIX          "HIVE_"     // Prefix for log filenames
 #define LOG_FORMAT_CSV           1           // Use CSV format for logs (1=CSV, 0=plain text)
 #define ROTATE_LOGS_DAILY        1           // Create new log files daily (1=yes, 0=no)
 
 // Learning system configuration
 #define LEARNING_PERIOD_DAYS     7           // Initial learning period in days
 #define LEARNING_SAMPLES_MIN     100         // Minimum samples needed for valid baseline
 #define LEARNING_ADAPTATION_RATE 0.05f       // Rate at which baseline adapts to changes (0.01-0.1)
 #define LEARNING_UPDATE_INTERVAL 50          // Update baseline every N samples
 #define LEARNING_SAVE_INTERVAL   20          // Save learning data every N samples
 
 // Bluetooth configuration (if enabled)
 #define BLE_NAME                 "HiveMonitor"  // Bluetooth device name
 #define BLE_TX_POWER             0           // Transmit power (0=normal, -20=lowest)
 #define BLE_ADVERTISING_INTERVAL 1000        // Advertising interval (ms)
 #define BLE_CONNECTION_INTERVAL  100         // Connection interval (ms)
 
 // Function prototypes
 bool loadConfigFromSD();
 void printConfig();
 const char* getConfigValueStr(const char* key);
 int getConfigValueInt(const char* key, int defaultValue);
 float getConfigValueFloat(const char* key, float defaultValue);
 
 #endif // HIVE_MONITOR_CONFIG_H