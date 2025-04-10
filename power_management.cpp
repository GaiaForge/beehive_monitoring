/**
 * Hive Monitor System - Power Management Module
 * 
 * This module handles the power management subsystem, controlling
 * sleep modes, battery monitoring, and power conservation strategies.
 */

#include "power_management.h"
#include "config.h"
#include <Arduino.h>
#include <ArduinoLowPower.h>

// Battery variables
float batteryVoltage = 0.0f;
BatteryStatus batteryStatus = BATTERY_NORMAL;

/**
 * Initialize power management
 */
void setupPowerManagement() {
  // Set up analog reference for battery measurement
  analogReference(AR_INTERNAL_3_0);
  
  // Set up battery monitoring pin
  pinMode(VBAT_PIN, INPUT);
  
  // Take initial battery reading
  readBatteryVoltage();
  
  Serial.println("Power management initialized");
}

/**
 * Read battery voltage
 */
void readBatteryVoltage() {
  // Stabilize ADC reference
  analogRead(VBAT_PIN);
  delay(10);
  
  // Take multiple readings for accuracy
  float totalReading = 0.0f;
  for (int i = 0; i < 5; i++) {
    totalReading += analogRead(VBAT_PIN);
    delay(10);
  }
  float avgReading = totalReading / 5.0f;
  
  // Convert to actual voltage
  // On nRF52840 Feather, VBAT is routed through a 1/2 voltage divider
  batteryVoltage = avgReading * 2.0f * 3.0f / 1023.0f;
  
  // Determine battery status
  if (batteryVoltage < CRITICAL_BATTERY_THRESHOLD) {
    batteryStatus = BATTERY_CRITICAL;
  } else if (batteryVoltage < LOW_BATTERY_THRESHOLD) {
    batteryStatus = BATTERY_LOW;
  } else {
    batteryStatus = BATTERY_NORMAL;
  }
  
  // Print battery status
  Serial.print("Battery Voltage: ");
  Serial.print(batteryVoltage, 2);
  Serial.println(" V");
  
  Serial.print("Battery Status: ");
  switch (batteryStatus) {
    case BATTERY_NORMAL: Serial.println("Normal"); break;
    case BATTERY_LOW: Serial.println("Low"); break;
    case BATTERY_CRITICAL: Serial.println("Critical"); break;
    default: Serial.println("Unknown"); break;
  }
}

/**
 * Get the current battery voltage
 */
float getBatteryVoltage() {
  return batteryVoltage;
}

/**
 * Get the current battery status
 */
BatteryStatus getBatteryStatus() {
  return batteryStatus;
}

/**
 * Enter sleep mode for power conservation
 */
void enterSleep(uint16_t minutes) {
  // If battery is low, extend sleep time to conserve power
  uint16_t sleepMinutes = minutes;
  
  if (batteryStatus == BATTERY_LOW) {
    sleepMinutes *= 2; // Sleep twice as long when battery is low
    Serial.print("Low battery - extended sleep to ");
    Serial.print(sleepMinutes);
    Serial.println(" minutes");
  } else if (batteryStatus == BATTERY_CRITICAL) {
    sleepMinutes = EXTENDED_SLEEP_MINUTES; // Use max sleep time when battery is critical
    Serial.print("Critical battery - maximum sleep of ");
    Serial.print(sleepMinutes);
    Serial.println(" minutes");
  }
  
  // Calculate sleep time in milliseconds
  uint32_t sleepTime = sleepMinutes * 60 * 1000;
  
  // Make sure all peripherals are powered down
  powerDownPeripherals();
  
  // Enter deep sleep
  Serial.print("Entering deep sleep for ");
  Serial.print(sleepMinutes);
  Serial.println(" minutes...");
  
  Serial.flush(); // Make sure all serial data is sent
  
  // Use low power library to put device into deep sleep
  LowPower.deepSleep(sleepTime);
  
  // Code will continue from here after waking
  Serial.println("Waking from deep sleep");
  
  // Power up peripherals
  powerUpPeripherals();
}

/**
 * Power down peripherals to save energy
 */
void powerDownPeripherals() {
  // Disable peripherals or set them to low power states
  // This will vary based on the specific hardware configuration
  
  // Disable BLE to save power
  if (ENABLE_BLE) {
    // Placeholder for BLE shutdown code
    // Bluefruit.Advertising.stop();
  }
  
  // Disable sensors via I2C
  Wire.end();
  
  // Disable unused pins
  if (ENABLE_STATUS_LED) {
    pinMode(LED_PIN, INPUT);
  }
}

/**
 * Power up peripherals after sleep
 */
void powerUpPeripherals() {
  // Re-initialize I2C
  Wire.begin();
  
  // Set up LED pin
  if (ENABLE_STATUS_LED) {
    pinMode(LED_PIN, OUTPUT);
  }
  
  // Re-enable BLE
  if (ENABLE_BLE) {
    // Placeholder for BLE restart code
    // Bluefruit.Advertising.start(0);
  }
}