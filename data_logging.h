/**
 * Hive Monitor System - Data Logging Header
 * 
 * Header file for the data logging module that writes sensor data
 * to the SD card in standardized formats.
 */

#ifndef DATA_LOGGING_H
#define DATA_LOGGING_H

#include <Arduino.h>
#include <SD.h>
#include <RTClib.h>
#include "env_sensors.h"
#include "motion_sensing.h"
#include "light_sensing.h"
#include "weight_sensing.h"
#include "audio_processing.h"

// Function prototypes
bool setupDataLogging(int csPin, RTC_PCF8523 *rtc);
bool isSDCardAvailable();
void getTimestampString(DateTime time, char* buffer, size_t bufferSize);
void getLogFilename(DateTime time, const char* prefix, char* buffer, size_t bufferSize);

// Main logging functions
bool logSensorData(DateTime time, EnvData envData, float* audioEnergy,
                   MotionData motionData, LightData lightData, 
                   float weight, float batteryVoltage);
                   
// Subsystem-specific logging
bool logAudioData(DateTime time, float* audioEnergy, SoundClass soundClass);
bool logEnvironmentalData(DateTime time, EnvData envData);
bool logWeightData(DateTime time, float weight, WeightStatus status);
bool logMotionData(DateTime time, MotionData motionData, MotionStatus status);
bool logLightData(DateTime time, LightData lightData);

#endif // DATA_LOGGING_H