/**
 * Hive Monitor System - Learning Header
 * 
 * Header file for the adaptive learning module that collects baseline data
 * about colony behavior and adjusts thresholds accordingly.
 */

 #ifndef LEARNING_H
 #define LEARNING_H
 
 #include <Arduino.h>
 #include <RTClib.h>
 #include "env_sensors.h"
 #include "motion_sensing.h"
 #include "light_sensing.h"
 #include "data_logging.h"
 
 // Number of audio frequency bands
 #define NUM_AUDIO_BANDS 4
 
 // Structure to hold baseline sensor data
 typedef struct {
     float tempMean;          // Mean temperature
     float tempStdDev;        // Temperature standard deviation
     float humidityMean;      // Mean humidity
     float humidityStdDev;    // Humidity standard deviation
     float pressureMean;      // Mean barometric pressure
     float pressureStdDev;    // Pressure standard deviation
     float weightMean;        // Mean hive weight
     float weightStdDev;      // Weight standard deviation
     float weightDailyDelta;  // Normal daily weight fluctuation
     float audioEnergy[NUM_AUDIO_BANDS];  // Mean energy in each freq band
     float audioStdDev[NUM_AUDIO_BANDS];  // StdDev in each freq band
 } SensorBaseline;
 
 // Structure to hold time-of-day patterns
 typedef struct {
     float activityLevel;    // Relative activity level (0.0-1.0)
     float tempOffset;       // Temperature offset from baseline
     float humidityOffset;   // Humidity offset from baseline
     uint16_t sampleCount;   // Number of samples for this time period
 } DailyPattern;
 
 // Class for maintaining running statistics with low memory usage
 class RunningStats {
 public:
     RunningStats() : count_(0), mean_(0), M2_(0) {}
     
     void addSample(float value);
     void reset();
     void partialReset(float keepRatio);
     void setStats(float mean, float stdDev);
     
     float mean() const;
     float variance() const;
     float standardDeviation() const;
     unsigned long count() const;
     
 private:
     unsigned long count_;  // Number of samples
     float mean_;           // Running mean
     float M2_;             // Sum of squared differences (for variance)
 };
 
 // Function prototypes
 void setupLearning();
 void resetLearningSystem();
 void updateLearningModel(EnvData envData, float* audioEnergy, 
                        MotionData motionData, LightData lightData, 
                        float weight, DateTime timestamp);
 void updateBaseline();
 void updateBaselineAdaptive();
 void updateDailyPattern(uint8_t hour, uint8_t season, float activity, 
                       float temp, float humidity);
 uint8_t getSeason(uint8_t month);
 
 // Anomaly detection
 bool isTemperatureAnomaly(float temperature, uint8_t hour);
 bool isHumidityAnomaly(float humidity, uint8_t hour);
 bool isAudioAnomaly(float* audioLevels);
 bool isWeightAnomaly(float weight, float previousWeight);
 
 // Get adapted thresholds
 void getAdaptedTempThresholds(float* lowThreshold, float* highThreshold, uint8_t hour);
 void getAdaptedHumidityThresholds(float* lowThreshold, float* highThreshold, uint8_t hour);
 void getAdaptedAudioThresholds(float thresholds[NUM_AUDIO_BANDS]);
 
 // Parameter persistence
 bool saveLearnedParameters();
 bool saveJsonParameters();
 bool loadLearnedParameters();
 void printBaseline();
 
 // Status
 bool isBaselineEstablished();
 uint8_t getLearningProgress();
 
 // These functions should be provided by other modules
 extern bool isSDCardAvailable();
 extern DateTime getRTCTime();
 extern bool rtcIsRunning();
 
 #endif // LEARNING_H