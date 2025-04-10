/**
 * Hive Monitor System - Environmental Sensors Header
 * 
 * Header file for the environmental monitoring subsystem.
 */

#ifndef ENV_SENSORS_H
#define ENV_SENSORS_H

#include <Arduino.h>

// Structure to hold environmental data
typedef struct {
  float temperature;  // Temperature in Celsius
  float humidity;     // Relative humidity percentage
  float pressure;     // Barometric pressure in hPa
} EnvData;

// Environment alert status
enum EnvAlertStatus {
  ENV_STATUS_NOMINAL,  // All readings normal
  ENV_STATUS_ALERT     // One or more readings outside threshold
};

// Function prototypes
bool setupEnvSensors();
void readEnvSensors();
void checkEnvAlerts();
EnvData getEnvData();
EnvAlertStatus getEnvAlertStatus();
const char* getEnvStatusString();
void formatEnvDataString(char* buffer, size_t bufferSize);

#endif // ENV_SENSORS_H