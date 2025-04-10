/**
 * Hive Monitor System - Light Sensing Header
 * 
 * Header file for the light sensing module that detects lid
 * removal and unusual light exposure using the APDS9960 sensor.
 */

#ifndef LIGHT_SENSING_H
#define LIGHT_SENSING_H

#include <Arduino.h>

// Light status enumeration
enum LightStatus {
  LIGHT_ENCLOSED,  // Normal, dark conditions
  LIGHT_OPEN       // Lid removed or unusual light exposure
};

// Structure to hold light data
typedef struct {
  uint16_t lightLevel;  // Overall brightness
  uint16_t red;         // Red channel
  uint16_t green;       // Green channel
  uint16_t blue;        // Blue channel
  uint16_t clear;       // Clear channel (overall light)
  LightStatus status;   // Current status
} LightData;

// Function prototypes
bool setupLightSensor();
void readLightSensor();
LightData getLightData();
LightStatus getLightStatus();
bool isLidRemoved();

#endif // LIGHT_SENSING_H