/**
 * Hive Monitor System - Motion Sensing Header
 * 
 * Header file for the motion sensing module that tracks hive
 * movement and orientation using 9-DoF sensors.
 */

#ifndef MOTION_SENSING_H
#define MOTION_SENSING_H

#include <Arduino.h>

// Structure to hold motion data
typedef struct {
  float accelX;   // X-axis acceleration in G
  float accelY;   // Y-axis acceleration in G
  float accelZ;   // Z-axis acceleration in G
  float gyroX;    // X-axis rotation in degrees per second
  float gyroY;    // Y-axis rotation in degrees per second
  float gyroZ;    // Z-axis rotation in degrees per second
  float magX;     // X-axis magnetic field in uT
  float magY;     // Y-axis magnetic field in uT
  float magZ;     // Z-axis magnetic field in uT
} MotionData;

// Motion status enumeration
enum MotionStatus {
  MOTION_NOMINAL,  // Normal, stable
  MOTION_WARNING,  // Some movement detected
  MOTION_ALERT     // Significant movement or impact
};

// Function prototypes
bool setupMotionSensors();
void readMotionSensors();
MotionData getMotionData();
MotionStatus getMotionStatus();
bool hasOrientationChanged();

#endif // MOTION_SENSING_H