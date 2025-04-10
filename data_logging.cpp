/**
 * Hive Monitor System - Data Logging Module
 * 
 * This module handles the data logging subsystem, writing sensor data
 * to SD card in appropriate formats for later analysis.
 */

#include "data_logging.h"
#include "config.h"
#include "audio_processing.h"
#include <SD.h>
#include <RTClib.h>

// Private variables
static int sdCardPin = 0;
static RTC_PCF8523 *rtcPtr = NULL;
static bool sdCardAvailable = false;

/**
 * Initialize data logging system
 */
bool setupDataLogging(int csPin, RTC_PCF8523 *rtc) {
  sdCardPin = csPin;
  rtcPtr = rtc;
  
  // Check if SD card is available
  sdCardAvailable = SD.begin(sdCardPin);
  
  if (!sdCardAvailable) {
    Serial.println("Data logging initialization failed - SD card not available");
    return false;
  }
  
  Serial.println("Data logging system initialized");
  return true;
}

/**
 * Check if SD card is available for logging
 */
bool isSDCardAvailable() {
  return sdCardAvailable;
}

/**
 * Generate ISO8601 timestamp string
 */
void getTimestampString(DateTime time, char* buffer, size_t bufferSize) {
  snprintf(buffer, bufferSize, "%04d-%02d-%02dT%02d:%02d:%02dZ", 
          time.year(), time.month(), time.day(),
          time.hour(), time.minute(), time.second());
}

/**
 * Log environmental data to a dedicated file
 */
bool logEnvironmentalData(DateTime time, EnvData envData) {
  if (!sdCardAvailable) {
    return false;
  }
  
  char filename[32];
  char timestamp[24];
  
  // Generate filename with ENV_ prefix
  getLogFilename(time, "ENV_", filename, sizeof(filename));
  
  // Generate timestamp
  getTimestampString(time, timestamp, sizeof(timestamp));
  
  // Open log file
  File logFile = SD.open(filename, FILE_WRITE);
  
  if (logFile) {
    logFile.print(timestamp);
    logFile.print(" | Temp: ");
    logFile.print(envData.temperature, 1);
    logFile.print("C | Hum: ");
    logFile.print(envData.humidity, 1);
    logFile.print("% | Pressure: ");
    logFile.print(envData.pressure, 1);
    logFile.print(" hPa | Status: ");
    logFile.println(getEnvStatusString());
    
    logFile.close();
    return true;
  } else {
    Serial.print("Error opening environmental log file: ");
    Serial.println(filename);
    return false;
  }
}

/**
 * Log weight data to a dedicated file
 */
bool logWeightData(DateTime time, float weight, WeightStatus status) {
  if (!sdCardAvailable) {
    return false;
  }
  
  char filename[32];
  char timestamp[24];
  
  // Generate filename with WEIGHT_ prefix
  getLogFilename(time, "WEIGHT_", filename, sizeof(filename));
  
  // Generate timestamp
  getTimestampString(time, timestamp, sizeof(timestamp));
  
  // Open log file
  File logFile = SD.open(filename, FILE_WRITE);
  
  if (logFile) {
    logFile.print(timestamp);
    logFile.print(" | Weight: ");
    logFile.print(weight, 2);
    logFile.print(" kg | Status: ");
    
    // Convert status to string
    const char* statusStr = "Unknown";
    switch (status) {
      case WEIGHT_STABLE: statusStr = "Stable"; break;
      case WEIGHT_INCREASE: statusStr = "Increase"; break;
      case WEIGHT_DECREASE: statusStr = "Decrease"; break;
      case WEIGHT_DROP_ALERT: statusStr = "Weight Drop Alert"; break;
    }
    
    logFile.println(statusStr);
    
    logFile.close();
    return true;
  } else {
    Serial.print("Error opening weight log file: ");
    Serial.println(filename);
    return false;
  }
}

/**
 * Log motion data to a dedicated file
 */
bool logMotionData(DateTime time, MotionData motionData, MotionStatus status) {
  if (!sdCardAvailable) {
    return false;
  }
  
  char filename[32];
  char timestamp[24];
  
  // Generate filename with MOTION_ prefix
  getLogFilename(time, "MOTION_", filename, sizeof(filename));
  
  // Generate timestamp
  getTimestampString(time, timestamp, sizeof(timestamp));
  
  // Open log file
  File logFile = SD.open(filename, FILE_WRITE);
  
  if (logFile) {
    logFile.print(timestamp);
    logFile.print(" | X: ");
    logFile.print(motionData.accelX, 2);
    logFile.print("g Y: ");
    logFile.print(motionData.accelY, 2);
    logFile.print("g Z: ");
    logFile.print(motionData.accelZ, 2);
    logFile.print("g | Orientation: ");
    
    // Determine orientation
    if (abs(motionData.accelZ - 1.0) < 0.1) {
      logFile.print("Stable");
    } else if (motionData.accelZ < 0.8) {
      logFile.print("Tilted");
    } else {
      logFile.print("Shifted");
    }
    
    logFile.print(" | Motion Status: ");
    
    // Convert status to string
    const char* statusStr = "Unknown";
    switch (status) {
      case MOTION_NOMINAL: statusStr = "Nominal"; break;
      case MOTION_WARNING: statusStr = "Warning"; break;
      case MOTION_ALERT: statusStr = "Movement Alert"; break;
    }
    
    logFile.println(statusStr);
    
    logFile.close();
    return true;
  } else {
    Serial.print("Error opening motion log file: ");
    Serial.println(filename);
    return false;
  }
}

/**
 * Log light data to a dedicated file
 */
bool logLightData(DateTime time, LightData lightData) {
  if (!sdCardAvailable) {
    return false;
  }
  
  char filename[32];
  char timestamp[24];
  
  // Generate filename with LIGHT_ prefix
  getLogFilename(time, "LIGHT_", filename, sizeof(filename));
  
  // Generate timestamp
  getTimestampString(time, timestamp, sizeof(timestamp));
  
  // Open log file
  File logFile = SD.open(filename, FILE_WRITE);
  
  if (logFile) {
    logFile.print(timestamp);
    logFile.print(" | Light: ");
    logFile.print(lightData.lightLevel);
    logFile.print(" lux | Status: ");
    
    // Status based on light level
    const char* statusStr = (lightData.status == LIGHT_ENCLOSED) ? "Enclosed" : "Lid Removed";
    logFile.println(statusStr);
    
    logFile.close();
    return true;
  } else {
    Serial.print("Error opening light log file: ");
    Serial.println(filename);
    return false;
  }
}

/**
 * Generate filename based on date and prefix
 */
void getLogFilename(DateTime time, const char* prefix, char* buffer, size_t bufferSize) {
  snprintf(buffer, bufferSize, "%s%04d%02d%02d.CSV", 
          prefix, time.year(), time.month(), time.day());
}

/**
 * Log all sensor data to a combined file
 */
bool logSensorData(DateTime time, EnvData envData, float* audioEnergy,
                  MotionData motionData, LightData lightData, 
                  float weight, float batteryVoltage) {
  
  if (!sdCardAvailable) {
    return false;
  }
  
  char filename[32];
  char timestamp[24];
  
  // Generate filename with LOG_ prefix
  getLogFilename(time, "LOG_", filename, sizeof(filename));
  
  // Generate timestamp
  getTimestampString(time, timestamp, sizeof(timestamp));
  
  // Open log file
  File logFile = SD.open(filename, FILE_WRITE);
  
  if (logFile) {
    // If file is newly created, write header
    if (logFile.size() == 0) {
      logFile.println("Timestamp,Temperature(C),Humidity(%),Pressure(hPa),Weight(kg),Light,AccelX,AccelY,AccelZ,B1,B2,B3,B4,Battery(V),Status");
    }
    
    // Log data
    logFile.print(timestamp);
    logFile.print(",");
    logFile.print(envData.temperature);
    logFile.print(",");
    logFile.print(envData.humidity);
    logFile.print(",");
    logFile.print(envData.pressure);
    logFile.print(",");
    logFile.print(weight);
    logFile.print(",");
    logFile.print(lightData.lightLevel);
    logFile.print(",");
    logFile.print(motionData.accelX);
    logFile.print(",");
    logFile.print(motionData.accelY);
    logFile.print(",");
    logFile.print(motionData.accelZ);
    logFile.print(",");
    logFile.print(audioEnergy[0]);
    logFile.print(",");
    logFile.print(audioEnergy[1]);
    logFile.print(",");
    logFile.print(audioEnergy[2]);
    logFile.print(",");
    logFile.print(audioEnergy[3]);
    logFile.print(",");
    logFile.print(batteryVoltage);
    logFile.print(",");
    
    // Overall status (most critical of all subsystems)
    const char* status = "Nominal";
    if (getEnvAlertStatus() != ENV_STATUS_NOMINAL || 
        getCurrentSoundClass() == SOUND_ALARM ||
        getMotionStatus() != MOTION_NOMINAL ||
        getLightStatus() != LIGHT_ENCLOSED ||
        getWeightStatus() != WEIGHT_STABLE) {
      status = "Alert";
    }
    logFile.println(status);
    
    logFile.close();
    return true;
  } else {
    Serial.print("Error opening log file: ");
    Serial.println(filename);
    return false;
  }
}

/**
 * Log audio data to a dedicated file
 */
bool logAudioData(DateTime time, float* audioEnergy, SoundClass soundClass) {
  if (!sdCardAvailable) {
    return false;
  }
  
  char filename[32];
  char timestamp[24];
  
  // Generate filename with AUDIO_ prefix
  getLogFilename(time, "AUDIO_", filename, sizeof(filename));
  
  // Generate timestamp
  getTimestampString(time, timestamp, sizeof(timestamp));
  
  // Open log file
  File logFile = SD.open(filename, FILE_WRITE);
  
  if (logFile) {
    // Format according to microphone sensing spec
    logFile.print(timestamp);
    logFile.print(" | B1: ");
    logFile.print(audioEnergy[0], 2);
    logFile.print(" | B2: ");
    logFile.print(audioEnergy[1], 2);
    logFile.print(" | B3: ");
    logFile.print(audioEnergy[2], 2);
    logFile.print(" | B4: ");
    logFile.print(audioEnergy[3], 2);
    logFile.print(" | Status: ");
    logFile.println(getSoundClassName(soundClass));
    
    logFile.close();
    return true;
  } else {
    Serial.print("Error opening audio log file: ");
    Serial.println(filename);
    return false;
  }
}