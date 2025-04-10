/**
 * Hive Monitor System - Environmental Sensors Module
 * 
 * This module handles the environmental monitoring subsystem, collecting
 * temperature, humidity, and barometric pressure data from onboard sensors.
 */

#include "env_sensors.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SHT31.h>

// Sensor instances
Adafruit_BME280 bme;  // Pressure and temperature
Adafruit_SHT31 sht;   // Humidity and temperature

// Current environmental readings
EnvData envData;

// Alert status
EnvAlertStatus envAlertStatus = ENV_STATUS_NOMINAL;

/**
 * Initialize environmental sensors
 */
bool setupEnvSensors() {
  bool success = true;
  
  // Initialize I2C bus if not already initialized
  Wire.begin();
  
  // Initialize BME280 (pressure, temperature)
  if (!bme.begin(0x76)) {
    Serial.println("Could not find BME280 sensor!");
    success = false;
  } else {
    Serial.println("BME280 sensor initialized");
    
    // Weather monitoring settings
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1,  // temperature
                    Adafruit_BME280::SAMPLING_X1,  // pressure
                    Adafruit_BME280::SAMPLING_X1,  // humidity
                    Adafruit_BME280::FILTER_OFF);
  }
  
  // Initialize SHT31 (humidity, temperature)
  if (!sht.begin(0x44)) {
    Serial.println("Could not find SHT31 sensor!");
    success = false;
  } else {
    Serial.println("SHT31 sensor initialized");
    sht.heater(false);  // Turn off heater to save power
  }
  
  return success;
}

/**
 * Read data from environmental sensors
 */
void readEnvSensors() {
  // Clear previous data
  envData.temperature = 0.0f;
  envData.humidity = 0.0f;
  envData.pressure = 0.0f;
  
  // Read SHT31 (primary temperature and humidity)
  envData.temperature = sht.readTemperature();
  envData.humidity = sht.readHumidity();
  
  // Check if readings are valid
  if (isnan(envData.temperature) || isnan(envData.humidity)) {
    Serial.println("SHT31 read failed!");
    
    // Fall back to BME280 for temperature if available
    if (bme.takeForcedMeasurement()) {
      envData.temperature = bme.readTemperature();
      Serial.println("Using BME280 for temperature fallback");
    }
  }
  
  // Read BME280 for pressure
  if (bme.takeForcedMeasurement()) {
    envData.pressure = bme.readPressure() / 100.0F; // Convert Pa to hPa
  } else {
    Serial.println("BME280 forced measurement failed!");
  }
  
  // Print readings
  Serial.println("Environmental Readings:");
  Serial.print("Temperature: "); Serial.print(envData.temperature); Serial.println(" °C");
  Serial.print("Humidity: "); Serial.print(envData.humidity); Serial.println(" %");
  Serial.print("Pressure: "); Serial.print(envData.pressure); Serial.println(" hPa");
  
  // Determine alert status based on thresholds
  checkEnvAlerts();
}

/**
 * Check if environmental readings are beyond alert thresholds
 */
void checkEnvAlerts() {
  // Default to nominal status
  envAlertStatus = ENV_STATUS_NOMINAL;
  
  // Check temperature
  if (envData.temperature < TEMP_ALERT_LOW || envData.temperature > TEMP_ALERT_HIGH) {
    envAlertStatus = ENV_STATUS_ALERT;
  }
  
  // Check humidity
  if (envData.humidity < HUM_ALERT_LOW || envData.humidity > HUM_ALERT_HIGH) {
    envAlertStatus = ENV_STATUS_ALERT;
  }
  
  // Print alert status
  Serial.print("Environmental Status: ");
  if (envAlertStatus == ENV_STATUS_NOMINAL) {
    Serial.println("Nominal");
  } else {
    Serial.println("Alert");
  }
}

/**
 * Get the current environmental data
 */
EnvData getEnvData() {
  return envData;
}

/**
 * Get the current environmental alert status
 */
EnvAlertStatus getEnvAlertStatus() {
  return envAlertStatus;
}

/**
 * Get environment status as string
 */
const char* getEnvStatusString() {
  switch (envAlertStatus) {
    case ENV_STATUS_NOMINAL:
      return "Nominal";
    case ENV_STATUS_ALERT:
      return "Alert";
    default:
      return "Unknown";
  }
}

/**
 * Format environmental data for logging
 */
void formatEnvDataString(char* buffer, size_t bufferSize) {
  snprintf(buffer, bufferSize, 
           "Temp: %.1fC | Hum: %.1f%% | Pressure: %.1f hPa | Status: %s", 
           envData.temperature, envData.humidity, envData.pressure,
           getEnvStatusString());
}