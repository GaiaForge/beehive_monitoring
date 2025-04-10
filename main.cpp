/**
 * Hive Monitor System - Main Application
 * 
 * This is the main application file that coordinates all subsystems
 * of the solar-powered hive monitoring system. It manages the sensor
 * modules, data logging, power management, and communication.
 * 
 * Hardware:
 * - Adafruit Feather nRF52840 Sense
 * - Built-in sensors: temperature, humidity, pressure, light, microphone, motion
 * - External: HX711 for weight sensing
 * - Power: Solar panel with LiFePO4 battery
 */

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include <ArduinoLowPower.h>
#include <bluefruit.h>

// Include our module headers
#include "config.h"
#include "audio_processing.h"
#include "env_sensors.h"
#include "motion_sensing.h"
#include "light_sensing.h"
#include "weight_sensing.h"
#include "data_logging.h"
#include "power_management.h"

// Pin definitions
#define SD_CS_PIN        5
#define LED_PIN          13

// RTC instance
RTC_PCF8523 rtc;

// Forward declarations
void setupSystem();
void blinkLED(int times);
void performMeasurementCycle();
void logAllSensorData();

/**
 * Setup function - runs once at startup
 */
void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(3000); // Wait for serial console to open
  
  Serial.println("Hive Monitor System Starting...");
  Serial.println("--------------------------------");
  
  // Set up LED
  pinMode(LED_PIN, OUTPUT);
  blinkLED(3); // Signal startup
  
  // Initialize all subsystems
  setupSystem();
  
  // Take initial readings
  performMeasurementCycle();
  
  Serial.println("Hive Monitor initialized!");
  Serial.println("--------------------------------");
}

/**
 * Main loop
 */
void loop() {
  // Take readings from all sensors
  performMeasurementCycle();
  
  // Log data to SD card
  logAllSensorData();
  
  // Enter low power sleep
  Serial.println("Entering low power sleep mode...");
  Serial.flush(); // Make sure all serial data is sent
  
  // Sleep for configured interval
  enterSleep(WAKE_INTERVAL_MINUTES);
}

/**
 * Initialize all subsystems
 */
void setupSystem() {
  // Initialize SD card
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    blinkLED(10); // Error indicator
  } else {
    Serial.println("SD card initialized.");
  }
  
  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
  } else {
    Serial.println("RTC found!");
    if (!rtc.initialized() || rtc.lostPower()) {
      Serial.println("RTC needs time setting!");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  }
  
  // Initialize each subsystem
  setupEnvSensors();
  setupMicrophone();
  setupMotionSensors();
  setupLightSensor();
  setupWeightSensor();
  setupDataLogging(SD_CS_PIN, &rtc);
  setupPowerManagement();
  
  // Load configuration (if available)
  loadConfigFromSD();
  
  // Initialize BLE if enabled
  if (ENABLE_BLE) {
    setupBLE();
  }
}

/**
 * Take a complete measurement cycle from all sensors
 */
void performMeasurementCycle() {
  DateTime now = rtc.now();
  char timestamp[24];
  sprintf(timestamp, "%04d-%02d-%02dT%02d:%02d:%02dZ", 
          now.year(), now.month(), now.day(),
          now.hour(), now.minute(), now.second());
  
  Serial.println("--------------------------------");
  Serial.print("Taking measurements at: ");
  Serial.println(timestamp);
  
  // Read battery voltage first
  readBatteryVoltage();
  
  // Read from each sensor module
  readEnvSensors();
  analyzeAudio();
  readMotionSensors();
  readLightSensor();
  readWeightSensor();
  
  // Print summary to serial
  Serial.println("Measurement cycle complete!");
}

/**
 * Log data from all sensors to SD card
 */
void logAllSensorData() {
  if (!isSDCardAvailable()) {
    Serial.println("SD card not available for logging!");
    return;
  }
  
  // Get current time from RTC
  DateTime now = rtc.now();
  
  // Log environmental data
  EnvData envData = getEnvData();
  LightData lightData = getLightData();
  MotionData motionData = getMotionData();
  float weight = getWeight();
  float batteryVoltage = getBatteryVoltage();
  float audioEnergy[4];
  getAudioEnergyValues(audioEnergy);
  
  // Create combined log entry
  logSensorData(now, envData, audioEnergy, motionData, lightData, weight, batteryVoltage);
  
  // Log audio status specifically
  logAudioData(now, audioEnergy, getCurrentSoundClass());
  
  // Log environmental data specifically
  logEnvironmentalData(now, envData);
  
  // Log weight data specifically
  logWeightData(now, weight, getWeightStatus());
  
  // Log motion data
  logMotionData(now, motionData, getMotionStatus());
  
  // Log light data
  logLightData(now, lightData);
  
  Serial.println("Data logging complete!");
}

/**
 * Blink the LED a specified number of times
 */
void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
}

/**
 * Load configuration from SD card
 */
void loadConfigFromSD() {
  Serial.println("Checking for configuration file...");
  if (SD.exists("/CONFIG.TXT")) {
    Serial.println("Configuration file found, loading settings");
    // Implementation of config loading would go here
  } else {
    Serial.println("No configuration file found, using defaults");
  }
}

/**
 * Initialize Bluetooth Low Energy
 */
void setupBLE() {
  Serial.println("Initializing Bluetooth...");
  
  // Initialize Bluefruit with maximum connections as peripheral
  Bluefruit.begin(1, 0);
  Bluefruit.setName(BLE_NAME);
  
  // Set the advertised device name (keep it short!)
  Bluefruit.setName(BLE_NAME);
  
  // Set tx power level to BLE_TX_POWER, see bluefruit.h for supported values
  Bluefruit.setTxPower(BLE_TX_POWER);
  
  // Start advertising
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addName();
  
  // Start advertising with specified interval
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(BLE_ADVERTISING_INTERVAL, BLE_ADVERTISING_INTERVAL*2);
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0);
  
  Serial.println("Bluetooth initialized");
}