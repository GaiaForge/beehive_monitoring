/**
 * Hive Monitor System - Light Sensing Module
 * 
 * This module handles the light sensing subsystem, using the onboard
 * APDS-9960 sensor to monitor ambient light levels and detect lid
 * removal or unusual light exposure.
 */

#include "light_sensing.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_APDS9960.h>

// Sensor object
Adafruit_APDS9960 apds;

// Current light data
LightData lightData;

/**
 * Initialize light sensor
 */
bool setupLightSensor() {
  // Initialize APDS9960
  if (!apds.begin()) {
    Serial.println("Failed to initialize APDS9960 light sensor");
    return false;
  }
  
  // Enable light sensing
  apds.enableColor(true);
  
  Serial.println("APDS9960 light sensor initialized");
  return true;
}

/**
 * Read data from light sensor
 */
void readLightSensor() {
  // Default values
  lightData.lightLevel = 0;
  lightData.red = 0;
  lightData.green = 0;
  lightData.blue = 0;
  lightData.clear = 0;
  lightData.status = LIGHT_ENCLOSED;
  
  // Check if color data is available
  if (apds.colorDataReady()) {
    // Read the light sensor data
    apds.getColorData(&lightData.red, &lightData.green, &lightData.blue, &lightData.clear);
    
    // The clear channel provides overall brightness
    lightData.lightLevel = lightData.clear;
    
    // Determine if lid is open based on threshold
    if (lightData.lightLevel > LIGHT_THRESHOLD) {
      lightData.status = LIGHT_OPEN;
    } else {
      lightData.status = LIGHT_ENCLOSED;
    }
    
    // Print light data
    Serial.println("Light Sensor Readings:");
    Serial.print("Light Level: "); Serial.println(lightData.lightLevel);
    Serial.print("RGBC Values: ");
    Serial.print(lightData.red); Serial.print(", ");
    Serial.print(lightData.green); Serial.print(", ");
    Serial.print(lightData.blue); Serial.print(", ");
    Serial.println(lightData.clear);
    
    Serial.print("Lid Status: ");
    Serial.println(lightData.status == LIGHT_ENCLOSED ? "Enclosed" : "Lid Removed");
  } else {
    Serial.println("Light sensor data not ready");
  }
}

/**
 * Get the current light data
 */
LightData getLightData() {
  return lightData;
}

/**
 * Get the current light status
 */
LightStatus getLightStatus() {
  return lightData.status;
}

/**
 * Check if the hive lid may have been removed
 */
bool isLidRemoved() {
  return (lightData.status == LIGHT_OPEN);
}