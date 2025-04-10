/**
 * Hive Monitor System - Weight Sensing Module
 * 
 * This module handles the weight sensing subsystem, using a load
 * cell and HX711 amplifier to measure and log the hive's weight.
 */

 #include "weight_sensing.h"
 #include "config.h"
 #include <HX711.h>
 
 // HX711 instance
 HX711 scale;
 
 // Weight variables
 float currentWeight = 0.0f;
 float previousWeight = 0.0f;
 WeightStatus weightStatus = WEIGHT_STABLE;
 
 /**
  * Initialize weight sensor
  */
 bool setupWeightSensor() {
   // Initialize HX711
   scale.begin(HX711_DATA_PIN, HX711_CLOCK_PIN);
   
   // Check if HX711 is found
   if (!scale.is_ready()) {
     Serial.println("HX711 not found or not ready!");
     return false;
   }
   
   // Set calibration factor (should be adjusted for specific load cell)
   scale.set_scale(WEIGHT_CALIBRATION);
   
   // Reset the scale to zero
   scale.tare();
   
   Serial.println("HX711 weight sensor initialized");
   return true;
 }
 
 /**
  * Read weight from load cell
  */
 void readWeightSensor() {
   // Store previous reading for change detection
   previousWeight = currentWeight;
   
   // Take multiple readings and average them
   if (scale.is_ready()) {
     float totalWeight = 0.0f;
     
     for (int i = 0; i < WEIGHT_SAMPLES; i++) {
       totalWeight += scale.get_units();
       delay(50); // Short delay between samples
     }
     
     // Calculate average
     currentWeight = totalWeight / WEIGHT_SAMPLES;
     
     // Determine weight status
     float weightDifference = currentWeight - previousWeight;
     
     // Reset status to stable by default
     weightStatus = WEIGHT_STABLE;
     
     // Check for significant changes
     if (abs(weightDifference) > WEIGHT_CHANGE_ALERT) {
       if (weightDifference < 0) {
         // Weight decrease (could be honey removal, absconding, etc.)
         if (weightDifference < -WEIGHT_CHANGE_ALERT * 2) {
           weightStatus = WEIGHT_DROP_ALERT; // Severe drop
         } else {
           weightStatus = WEIGHT_DECREASE;
         }
       } else {
         // Weight increase (honey production, rain ingress, etc.)
         weightStatus = WEIGHT_INCREASE;
       }
     }
     
     // Print weight
     Serial.print("Current Weight: ");
     Serial.print(currentWeight, 2);
     Serial.println(" kg");
     
     Serial.print("Weight Change: ");
     Serial.print(weightDifference, 2);
     Serial.println(" kg");
     
     Serial.print("Weight Status: ");
     switch (weightStatus) {
       case WEIGHT_STABLE: Serial.println("Stable"); break;
       case WEIGHT_INCREASE: Serial.println("Increase"); break;
       case WEIGHT_DECREASE: Serial.println("Decrease"); break;
       case WEIGHT_DROP_ALERT: Serial.println("Weight Drop Alert!"); break;
       default: Serial.println("Unknown"); break;
     }
   } else {
     Serial.println("HX711 not ready for reading");
   }
 }
 
 /**
  * Get the current weight reading
  */
 float getWeight() {
   return currentWeight;
 }
 
 /**
  * Get the current weight status
  */
 WeightStatus getWeightStatus() {
   return weightStatus;
 }
 
 /**
  * Calibrate the weight sensor with a known reference weight
  */
 void calibrateWeightSensor(float knownWeight) {
   if (!scale.is_ready()) {
     Serial.println("HX711 not ready for calibration");
     return;
   }
   
   Serial.println("Starting calibration procedure...");
   Serial.println("Please remove all weight from the scale");
   delay(5000);
   
   // Tare scale
   Serial.println("Taring scale...");
   scale.tare();
   delay(1000);
   
   // Prompt to add calibration weight
   Serial.print("Please place the ");
   Serial.print(knownWeight);
   Serial.println("kg calibration weight on the scale");
   delay(10000);
   
   // Get raw reading
   long rawReading = scale.read_average(20);
   
   // Calculate calibration factor
   float calibrationFactor = rawReading / knownWeight;
   
   Serial.print("New calibration factor: ");
   Serial.println(calibrationFactor);
   
   // Update scale with new calibration factor
   scale.set_scale(calibrationFactor);
   
   // Test the calibration
   Serial.print("Measured weight: ");
   Serial.print(scale.get_units(10), 2);
   Serial.println(" kg");
   
   // Prompt to remove weight
   Serial.println("Please remove the calibration weight");
   delay(5000);
   
   // Verify zero
   Serial.print("Zero reading: ");
   Serial.print(scale.get_units(10), 2);
   Serial.println(" kg");
 }