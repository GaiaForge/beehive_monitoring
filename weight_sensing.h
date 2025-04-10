/**
 * Hive Monitor System - Weight Sensing Header
 * 
 * Header file for the weight sensing module that tracks hive
 * weight using a load cell and HX711 amplifier.
 */

#ifndef WEIGHT_SENSING_H
#define WEIGHT_SENSING_H

#include <Arduino.h>

// Weight status enumeration
enum WeightStatus {
  WEIGHT_STABLE,      // Weight relatively unchanged
  WEIGHT_INCREASE,    // Significant weight increase detected
  WEIGHT_DECREASE,    // Significant weight decrease detected
  WEIGHT_DROP_ALERT   // Sudden large weight drop (possible theft/swarming)
};

// Pin definitions for HX711
#define HX711_DATA_PIN  6
#define HX711_CLOCK_PIN 5

// Function prototypes
bool setupWeightSensor();
void readWeightSensor();
float getWeight();
WeightStatus getWeightStatus();
void calibrateWeightSensor(float knownWeight);

#endif // WEIGHT_SENSING_H