/**
 * Hive Monitor System - Power Management Header
 * 
 * Header file for the power management module that handles
 * sleep modes and battery monitoring.
 */

#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

#include <Arduino.h>

// Battery status enumeration
enum BatteryStatus {
  BATTERY_NORMAL,   // Normal operation, good battery level
  BATTERY_LOW,      // Low battery, enter power saving
  BATTERY_CRITICAL  // Critical battery, extreme power saving
};

// Pin definitions
#define VBAT_PIN A7
#define LED_PIN  13

// Threshold values
#define LOW_BATTERY_THRESHOLD     3.5f
#define CRITICAL_BATTERY_THRESHOLD 3.2f
#define EXTENDED_SLEEP_MINUTES    60

// Function prototypes
void setupPowerManagement();
void readBatteryVoltage();
float getBatteryVoltage();
BatteryStatus getBatteryStatus();
void enterSleep(uint16_t minutes);
void powerDownPeripherals();
void powerUpPeripherals();

#endif // POWER_MANAGEMENT_H