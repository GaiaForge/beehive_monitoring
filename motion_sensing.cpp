/**
 * Hive Monitor System - Motion Sensing Module
 * 
 * This module handles the motion sensing subsystem, using the onboard
 * 9-DoF motion sensors to detect hive disturbances, monitor orientation,
 * and log subtle changes in movement.
 */

#include "motion_sensing.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_LSM6DS33.h>
#include <Adafruit_LIS3MDL.h>

// Sensor objects
Adafruit_LSM6DS33 lsm6ds33; // Accelerometer and gyroscope
Adafruit_LIS3MDL lis3mdl;   // Magnetometer

// Current motion data and status
MotionData motionData;
MotionStatus motionStatus = MOTION_NOMINAL;

/**
 * Initialize motion sensors
 */
bool setupMotionSensors() {
  bool success = true;
  
  // Initialize accelerometer/gyro
  if (!lsm6ds33.begin_I2C()) {
    Serial.println("Failed to find LSM6DS33 accelerometer/gyro");
    success = false;
  } else {
    Serial.println("LSM6DS33 accelerometer/gyro initialized");
    
    // Configure accelerometer
    lsm6ds33.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
    lsm6ds33.setAccelDataRate(LSM6DS_RATE_52_HZ);
    
    // Configure gyro
    lsm6ds33.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
    lsm6ds33.setGyroDataRate(LSM6DS_RATE_52_HZ);
  }
  
  // Initialize magnetometer
  if (!lis3mdl.begin_I2C()) {
    Serial.println("Failed to find LIS3MDL magnetometer");
    success = false;
  } else {
    Serial.println("LIS3MDL magnetometer initialized");
    
    // Configure magnetometer
    lis3mdl.setPerformanceMode(LIS3MDL_MEDIUMMODE);
    lis3mdl.setOperationMode(LIS3MDL_CONTINUOUSMODE);
    lis3mdl.setDataRate(LIS3MDL_DATARATE_10_HZ);
    lis3mdl.setRange(LIS3MDL_RANGE_4_GAUSS);
  }
  
  return success;
}

/**
 * Read data from motion sensors
 */
void readMotionSensors() {
  // Clear previous data
  motionData.accelX = 0.0f;
  motionData.accelY = 0.0f;
  motionData.accelZ = 0.0f;
  motionData.gyroX = 0.0f;
  motionData.gyroY = 0.0f;
  motionData.gyroZ = 0.0f;
  motionData.magX = 0.0f;
  motionData.magY = 0.0f;
  motionData.magZ = 0.0f;
  
  // Read accelerometer and gyroscope
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  
  if (lsm6ds33.getEvent(&accel, &gyro, &temp)) {
    // Convert accelerometer values to G (9.8 m/s²)
    motionData.accelX = accel.acceleration.x / 9.8f;
    motionData.accelY = accel.acceleration.y / 9.8f;
    motionData.accelZ = accel.acceleration.z / 9.8f;
    
    // Store gyroscope values in degrees per second
    motionData.gyroX = gyro.gyro.x;
    motionData.gyroY = gyro.gyro.y;
    motionData.gyroZ = gyro.gyro.z;
  } else {
    Serial.println("Failed to read accel/gyro sensors");
  }
  
  // Read magnetometer
  sensors_event_t mag;
  if (lis3mdl.getEvent(&mag)) {
    motionData.magX = mag.magnetic.x;
    motionData.magY = mag.magnetic.y;
    motionData.magZ = mag.magnetic.z;
  } else {
    Serial.println("Failed to read magnetometer");
  }
  
  // Calculate motion magnitude
  float motionMagnitude = sqrt(motionData.accelX * motionData.accelX + 
                              motionData.accelY * motionData.accelY + 
                              motionData.accelZ * motionData.accelZ);
  
  // Classify motion status based on magnitude
  if (motionMagnitude > MOTION_ALERT_THRESHOLD) {
    motionStatus = MOTION_ALERT;
    Serial.println("Motion ALERT detected!");
  } else if (motionMagnitude > MOTION_WARNING_THRESHOLD) {
    motionStatus = MOTION_WARNING;
    Serial.println("Motion warning detected");
  } else {
    motionStatus = MOTION_NOMINAL;
  }
  
  // Print motion data
  Serial.println("Motion Sensor Readings:");
  Serial.print("Accel X/Y/Z (G): ");
  Serial.print(motionData.accelX); Serial.print(", ");
  Serial.print(motionData.accelY); Serial.print(", ");
  Serial.println(motionData.accelZ);
  
  Serial.print("Gyro X/Y/Z (dps): ");
  Serial.print(motionData.gyroX); Serial.print(", ");
  Serial.print(motionData.gyroY); Serial.print(", ");
  Serial.println(motionData.gyroZ);
  
  Serial.print("Mag X/Y/Z (uT): ");
  Serial.print(motionData.magX); Serial.print(", ");
  Serial.print(motionData.magY); Serial.print(", ");
  Serial.println(motionData.magZ);
  
  Serial.print("Motion Status: ");
  switch (motionStatus) {
    case MOTION_NOMINAL: Serial.println("Nominal"); break;
    case MOTION_WARNING: Serial.println("Warning"); break;
    case MOTION_ALERT: Serial.println("Alert"); break;
    default: Serial.println("Unknown"); break;
  }
}

/**
 * Get the current motion data
 */
MotionData getMotionData() {
  return motionData;
}

/**
 * Get the current motion status
 */
MotionStatus getMotionStatus() {
  return motionStatus;
}

/**
 * Check for significant orientation change
 */
bool hasOrientationChanged() {
  // Check if Z acceleration has changed significantly from ~1G
  // which would indicate the hive has been tilted
  if (abs(motionData.accelZ - 1.0f) > 0.3f) {
    return true;
  }
  
  return false;
}