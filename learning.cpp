/**
 * Hive Monitor System - Learning Module
 * 
 * This module implements adaptive learning capabilities for the hive monitor.
 * It collects baseline data on normal colony behavior and adjusts 
 * detection thresholds based on observed patterns.
 */

#include "learning.h"
#include "config.h"
#include <SD.h>
#include <RTClib.h>
#include <ArduinoJson.h>

// File for storing learning data
#define LEARNING_FILE "LEARN.DAT"
#define LEARNING_JSON "LEARN.JSN"

// Learning state
static SensorBaseline colonyBaseline;
static bool baselineEstablished = false;
static uint16_t learningSampleCount = 0;
static uint8_t currentSeason = 0;

// Running statistics for incremental calculations
static RunningStats tempStats;
static RunningStats humidityStats;
static RunningStats pressureStats;
static RunningStats weightStats;
static RunningStats audioStats[NUM_AUDIO_BANDS];
static RunningStats lightStats;
static RunningStats motionStats;

// Daily patterns storage (hour by season)
static DailyPattern dailyPatterns[24][4];

/**
 * Initialize learning system
 */
void setupLearning() {
    Serial.println("Initializing learning system...");
    
    // Try to load existing learning data
    if (loadLearnedParameters()) {
        Serial.println("Loaded existing learning parameters");
        baselineEstablished = true;
    } else {
        Serial.println("No saved learning data, starting with defaults");
        resetLearningSystem();
    }
    
    // Determine current season
    DateTime now;
    if (rtcIsRunning()) {
        now = getRTCTime();
        currentSeason = getSeason(now.month());
    } else {
        currentSeason = 0; // Default to first season
    }
    
    Serial.print("Current season: ");
    Serial.println(currentSeason);
}

/**
 * Reset learning system to defaults
 */
void resetLearningSystem() {
    // Initialize with sensible defaults
    colonyBaseline.tempMean = 35.0;         // °C
    colonyBaseline.tempStdDev = 2.0;        // °C
    colonyBaseline.humidityMean = 60.0;     // %
    colonyBaseline.humidityStdDev = 5.0;    // %
    colonyBaseline.pressureMean = 1013.25;  // hPa
    colonyBaseline.pressureStdDev = 5.0;    // hPa
    colonyBaseline.weightMean = 0.0;        // kg
    colonyBaseline.weightStdDev = 1.0;      // kg
    colonyBaseline.weightDailyDelta = 0.2;  // kg
    
    // Audio energy defaults
    colonyBaseline.audioEnergy[0] = THRESH_B1; // Normal hum band
    colonyBaseline.audioEnergy[1] = THRESH_B2; // Queen piping band
    colonyBaseline.audioEnergy[2] = THRESH_B3; // Swarming band
    colonyBaseline.audioEnergy[3] = THRESH_B4; // Alarm band
    
    for (int i = 0; i < NUM_AUDIO_BANDS; i++) {
        colonyBaseline.audioStdDev[i] = 0.1; // Initial std deviation
        audioStats[i].reset();
    }
    
    // Reset running statistics
    tempStats.reset();
    humidityStats.reset();
    pressureStats.reset();
    weightStats.reset();
    lightStats.reset();
    motionStats.reset();
    
    // Reset learning counter
    learningSampleCount = 0;
    
    // Initialize daily patterns
    for (int h = 0; h < 24; h++) {
        for (int s = 0; s < 4; s++) {
            dailyPatterns[h][s].activityLevel = 0.5;    // Medium activity
            dailyPatterns[h][s].tempOffset = 0.0;       // No offset
            dailyPatterns[h][s].humidityOffset = 0.0;   // No offset
            dailyPatterns[h][s].sampleCount = 0;
        }
    }
    
    baselineEstablished = false;
}

/**
 * Process new sensor readings and update learning model
 */
void updateLearningModel(EnvData envData, float* audioEnergy, 
                        MotionData motionData, LightData lightData, 
                        float weight, DateTime timestamp) {
    
    // Increment sample counter
    learningSampleCount++;
    
    // Update running statistics
    tempStats.addSample(envData.temperature);
    humidityStats.addSample(envData.humidity);
    pressureStats.addSample(envData.pressure);
    weightStats.addSample(weight);
    
    // Audio energy for each band
    for (int i = 0; i < NUM_AUDIO_BANDS; i++) {
        audioStats[i].addSample(audioEnergy[i]);
    }
    
    // Motion magnitude
    float motionMag = sqrt(motionData.accelX*motionData.accelX + 
                          motionData.accelY*motionData.accelY + 
                          motionData.accelZ*motionData.accelZ);
    motionStats.addSample(motionMag);
    
    // Light level
    lightStats.addSample(lightData.lightLevel);
    
    // Update daily pattern for current hour and season
    int hour = timestamp.hour();
    int season = getSeason(timestamp.month());
    
    // Activity level is based on audio energy in normal band and motion
    float activity = (audioEnergy[0] / colonyBaseline.audioEnergy[0]) * 0.8f + 
                    (motionMag / motionStats.mean()) * 0.2f;
    
    updateDailyPattern(hour, season, activity, envData.temperature, envData.humidity);
    
    // After sufficient samples, establish baseline
    if (learningSampleCount >= LEARNING_SAMPLES_MIN && !baselineEstablished) {
        updateBaseline();
        baselineEstablished = true;
        Serial.println("Baseline established!");
        saveLearnedParameters();
    }
    
    // Periodically update baseline with slow adaptation rate
    if (baselineEstablished && learningSampleCount % LEARNING_UPDATE_INTERVAL == 0) {
        updateBaselineAdaptive();
        saveLearnedParameters();
    }
    
    // Periodically save learning data
    if (learningSampleCount % LEARNING_SAVE_INTERVAL == 0) {
        saveLearnedParameters();
    }
    
    // Log learning progress
    if (learningSampleCount % 10 == 0 || learningSampleCount == LEARNING_SAMPLES_MIN) {
        Serial.print("Learning progress: ");
        Serial.print(learningSampleCount);
        if (!baselineEstablished) {
            Serial.print("/");
            Serial.print(LEARNING_SAMPLES_MIN);
            Serial.print(" (");
            Serial.print((learningSampleCount * 100) / LEARNING_SAMPLES_MIN);
            Serial.println("%)");
        } else {
            Serial.println(" samples collected");
        }
    }
}

/**
 * Update baseline from collected statistics
 */
void updateBaseline() {
    // Update environmental baselines
    colonyBaseline.tempMean = tempStats.mean();
    colonyBaseline.tempStdDev = tempStats.standardDeviation();
    
    colonyBaseline.humidityMean = humidityStats.mean();
    colonyBaseline.humidityStdDev = humidityStats.standardDeviation();
    
    colonyBaseline.pressureMean = pressureStats.mean();
    colonyBaseline.pressureStdDev = pressureStats.standardDeviation();
    
    colonyBaseline.weightMean = weightStats.mean();
    colonyBaseline.weightStdDev = weightStats.standardDeviation();
    
    // Update audio energy baselines
    for (int i = 0; i < NUM_AUDIO_BANDS; i++) {
        colonyBaseline.audioEnergy[i] = audioStats[i].mean();
        colonyBaseline.audioStdDev[i] = audioStats[i].standardDeviation();
    }
    
    // Log the new baseline
    printBaseline();
}

/**
 * Update baseline with adaptive rate (slower changes over time)
 */
void updateBaselineAdaptive() {
    float adaptRate = LEARNING_ADAPTATION_RATE;
    
    // Slowly adapt baseline to seasonal changes
    colonyBaseline.tempMean = (1-adaptRate) * colonyBaseline.tempMean + 
                             adaptRate * tempStats.mean();
    colonyBaseline.tempStdDev = (1-adaptRate) * colonyBaseline.tempStdDev + 
                               adaptRate * tempStats.standardDeviation();
    
    colonyBaseline.humidityMean = (1-adaptRate) * colonyBaseline.humidityMean + 
                                adaptRate * humidityStats.mean();
    colonyBaseline.humidityStdDev = (1-adaptRate) * colonyBaseline.humidityStdDev + 
                                  adaptRate * humidityStats.standardDeviation();
    
    colonyBaseline.pressureMean = (1-adaptRate) * colonyBaseline.pressureMean + 
                                adaptRate * pressureStats.mean();
    
    // Weight changes more slowly with seasons
    colonyBaseline.weightMean = (1-adaptRate/2) * colonyBaseline.weightMean + 
                              (adaptRate/2) * weightStats.mean();
    
    // Update audio energy baselines (more responsive)
    for (int i = 0; i < NUM_AUDIO_BANDS; i++) {
        colonyBaseline.audioEnergy[i] = (1-adaptRate*2) * colonyBaseline.audioEnergy[i] + 
                                       (adaptRate*2) * audioStats[i].mean();
        colonyBaseline.audioStdDev[i] = (1-adaptRate) * colonyBaseline.audioStdDev[i] + 
                                      adaptRate * audioStats[i].standardDeviation();
    }
    
    // Reset statistics for next adaptation period
    tempStats.reset();
    humidityStats.reset();
    pressureStats.reset();
    audioStats[0].reset();
    audioStats[1].reset();
    audioStats[2].reset();
    audioStats[3].reset();
    
    // Keep weight history longer
    weightStats.partialReset(0.8);
    
    // Log the updated baseline
    Serial.println("Updated adaptive baseline:");
    printBaseline();
}

/**
 * Update daily pattern for specific hour and season
 */
void updateDailyPattern(uint8_t hour, uint8_t season, float activity, 
                       float temp, float humidity) {
    
    DailyPattern* pattern = &dailyPatterns[hour][season];
    
    // Calculate adaptation rate (faster when fewer samples)
    float adaptRate = min(0.5f, 5.0f / (pattern->sampleCount + 10.0f));
    
    // Update pattern
    pattern->activityLevel = (1-adaptRate) * pattern->activityLevel + 
                           adaptRate * activity;
    
    pattern->tempOffset = (1-adaptRate) * pattern->tempOffset + 
                         adaptRate * (temp - colonyBaseline.tempMean);
    
    pattern->humidityOffset = (1-adaptRate) * pattern->humidityOffset + 
                            adaptRate * (humidity - colonyBaseline.humidityMean);
    
    pattern->sampleCount++;
}

/**
 * Get the current season (0-3) based on month
 */
uint8_t getSeason(uint8_t month) {
    // Simple season calculation
    // 0 = Winter (Dec-Feb), 1 = Spring (Mar-May), 
    // 2 = Summer (Jun-Aug), 3 = Fall (Sep-Nov)
    if (month == 12 || month == 1 || month == 2) return 0;
    if (month >= 3 && month <= 5) return 1;
    if (month >= 6 && month <= 8) return 2;
    return 3; // Fall (Sep-Nov)
}

/**
 * Check if a temperature reading is anomalous based on learned patterns
 */
bool isTemperatureAnomaly(float temperature, uint8_t hour) {
    // Get expected temperature for current time and season
    float expectedTemp = colonyBaseline.tempMean + 
                        dailyPatterns[hour][currentSeason].tempOffset;
    
    // Calculate z-score (standard deviations from mean)
    float zScore = (temperature - expectedTemp) / colonyBaseline.tempStdDev;
    
    // Consider anomaly if more than ANOMALY_THRESHOLD standard deviations from mean
    return abs(zScore) > TEMP_ANOMALY_THRESHOLD;
}

/**
 * Check if humidity is anomalous based on learned patterns
 */
bool isHumidityAnomaly(float humidity, uint8_t hour) {
    float expectedHumidity = colonyBaseline.humidityMean + 
                           dailyPatterns[hour][currentSeason].humidityOffset;
    
    float zScore = (humidity - expectedHumidity) / colonyBaseline.humidityStdDev;
    
    return abs(zScore) > HUMIDITY_ANOMALY_THRESHOLD;
}

/**
 * Check if audio pattern is anomalous based on learned baselines
 */
bool isAudioAnomaly(float* audioLevels) {
    // Check each frequency band
    for (int i = 0; i < NUM_AUDIO_BANDS; i++) {
        float zScore = (audioLevels[i] - colonyBaseline.audioEnergy[i]) / 
                     max(0.01f, colonyBaseline.audioStdDev[i]);
        
        // Different thresholds for different bands
        float threshold = (i == 0) ? 3.0f : 2.5f; // Normal band can vary more
        
        if (abs(zScore) > threshold) {
            return true;
        }
    }
    return false;
}

/**
 * Check if weight change is anomalous
 */
bool isWeightAnomaly(float weight, float previousWeight) {
    // Short term change (sudden)
    float change = weight - previousWeight;
    if (abs(change) > WEIGHT_CHANGE_THRESHOLD * colonyBaseline.weightStdDev) {
        return true;
    }
    
    // Long term deviation from baseline
    float zScore = (weight - colonyBaseline.weightMean) / colonyBaseline.weightStdDev;
    if (abs(zScore) > WEIGHT_ANOMALY_THRESHOLD) {
        return true;
    }
    
    return false;
}

/**
 * Get adapted temperature thresholds based on learning
 */
void getAdaptedTempThresholds(float* lowThreshold, float* highThreshold, uint8_t hour) {
    float seasonalOffset = dailyPatterns[hour][currentSeason].tempOffset;
    
    // Base thresholds adjusted for this colony's normal patterns
    *lowThreshold = max(MIN_SAFE_TEMP, TEMP_ALERT_LOW + 
                      (colonyBaseline.tempMean - 35.0f) + 
                      seasonalOffset - colonyBaseline.tempStdDev);
    
    *highThreshold = min(MAX_SAFE_TEMP, TEMP_ALERT_HIGH + 
                       (colonyBaseline.tempMean - 35.0f) + 
                       seasonalOffset + colonyBaseline.tempStdDev);
}

/**
 * Get adapted humidity thresholds based on learning
 */
void getAdaptedHumidityThresholds(float* lowThreshold, float* highThreshold, uint8_t hour) {
    float seasonalOffset = dailyPatterns[hour][currentSeason].humidityOffset;
    
    *lowThreshold = max(MIN_SAFE_HUMIDITY, HUM_ALERT_LOW + 
                     seasonalOffset - colonyBaseline.humidityStdDev);
    
    *highThreshold = min(MAX_SAFE_HUMIDITY, HUM_ALERT_HIGH + 
                      seasonalOffset + colonyBaseline.humidityStdDev);
}

/**
 * Get adapted audio thresholds for each band
 */
void getAdaptedAudioThresholds(float thresholds[NUM_AUDIO_BANDS]) {
    thresholds[0] = max(MIN_AUDIO_THRESHOLD, colonyBaseline.audioEnergy[0] * 0.7f); // Normal hum
    thresholds[1] = max(MIN_AUDIO_THRESHOLD, colonyBaseline.audioEnergy[1] * 1.5f); // Queen
    thresholds[2] = max(MIN_AUDIO_THRESHOLD, colonyBaseline.audioEnergy[2] * 1.5f); // Swarming
    thresholds[3] = max(MIN_AUDIO_THRESHOLD, colonyBaseline.audioEnergy[3] * 1.8f); // Alarm
}

/**
 * Save learned parameters to SD card
 */
bool saveLearnedParameters() {
    if (!isSDCardAvailable()) {
        Serial.println("Cannot save learning data - SD card not available");
        return false;
    }
    
    // First save binary version for internal use
    File dataFile = SD.open(LEARNING_FILE, FILE_WRITE);
    if (dataFile) {
        // Write the baseline data
        dataFile.write((uint8_t*)&colonyBaseline, sizeof(SensorBaseline));
        
        // Write the daily patterns
        dataFile.write((uint8_t*)dailyPatterns, sizeof(dailyPatterns));
        
        // Write learning progress
        dataFile.write((uint8_t*)&learningSampleCount, sizeof(learningSampleCount));
        dataFile.write((uint8_t*)&currentSeason, sizeof(currentSeason));
        
        dataFile.close();
        
        // Now save human-readable JSON version
        saveJsonParameters();
        
        Serial.println("Learning data saved");
        return true;
    } else {
        Serial.println("Failed to open learning data file for writing");
        return false;
    }
}

/**
 * Save parameters in human-readable JSON format
 */
bool saveJsonParameters() {
    if (!isSDCardAvailable()) {
        return false;
    }
    
    File jsonFile = SD.open(LEARNING_JSON, FILE_WRITE);
    if (jsonFile) {
        // Create JSON document
        StaticJsonDocument<1024> doc;
        
        // Store baseline data
        JsonObject baseline = doc.createNestedObject("baseline");
        baseline["tempMean"] = colonyBaseline.tempMean;
        baseline["tempStdDev"] = colonyBaseline.tempStdDev;
        baseline["humidityMean"] = colonyBaseline.humidityMean;
        baseline["humidityStdDev"] = colonyBaseline.humidityStdDev;
        baseline["weightMean"] = colonyBaseline.weightMean;
        baseline["weightStdDev"] = colonyBaseline.weightStdDev;
        
        JsonArray audio = baseline.createNestedArray("audio");
        for (int i = 0; i < NUM_AUDIO_BANDS; i++) {
            JsonObject band = audio.createNestedObject();
            band["energy"] = colonyBaseline.audioEnergy[i];
            band["stdDev"] = colonyBaseline.audioStdDev[i];
        }
        
        doc["sampleCount"] = learningSampleCount;
        doc["baselineEstablished"] = baselineEstablished;
        doc["currentSeason"] = currentSeason;
        
        // Serialize to file
        if (serializeJson(doc, jsonFile) == 0) {
            Serial.println("Failed to write JSON data");
        }
        
        jsonFile.close();
        return true;
    }
    return false;
}

/**
 * Load learned parameters from SD card
 */
bool loadLearnedParameters() {
    if (!isSDCardAvailable()) {
        return false;
    }
    
    if (!SD.exists(LEARNING_FILE)) {
        Serial.println("No learning data file found");
        return false;
    }
    
    File dataFile = SD.open(LEARNING_FILE, FILE_READ);
    if (dataFile) {
        // Read the baseline data
        dataFile.read((uint8_t*)&colonyBaseline, sizeof(SensorBaseline));
        
        // Read the daily patterns
        dataFile.read((uint8_t*)dailyPatterns, sizeof(dailyPatterns));
        
        // Read learning progress
        dataFile.read((uint8_t*)&learningSampleCount, sizeof(learningSampleCount));
        dataFile.read((uint8_t*)&currentSeason, sizeof(currentSeason));
        
        dataFile.close();
        
        // Initialize stats with baseline values
        tempStats.setStats(colonyBaseline.tempMean, colonyBaseline.tempStdDev);
        humidityStats.setStats(colonyBaseline.humidityMean, colonyBaseline.humidityStdDev);
        pressureStats.setStats(colonyBaseline.pressureMean, colonyBaseline.pressureStdDev);
        weightStats.setStats(colonyBaseline.weightMean, colonyBaseline.weightStdDev);
        
        for (int i = 0; i < NUM_AUDIO_BANDS; i++) {
            audioStats[i].setStats(colonyBaseline.audioEnergy[i], colonyBaseline.audioStdDev[i]);
        }
        
        printBaseline();
        return true;
    } else {
        Serial.println("Failed to open learning data file");
        return false;
    }
}

/**
 * Print the current baseline values to serial
 */
void printBaseline() {
    Serial.println("Current Baseline Values:");
    Serial.print("Temperature: "); 
    Serial.print(colonyBaseline.tempMean);
    Serial.print("°C ±"); 
    Serial.print(colonyBaseline.tempStdDev);
    Serial.println("°C");
    
    Serial.print("Humidity: "); 
    Serial.print(colonyBaseline.humidityMean);
    Serial.print("% ±"); 
    Serial.print(colonyBaseline.humidityStdDev);
    Serial.println("%");
    
    Serial.print("Weight: "); 
    Serial.print(colonyBaseline.weightMean);
    Serial.print("kg ±"); 
    Serial.print(colonyBaseline.weightStdDev);
    Serial.println("kg");
    
    Serial.println("Audio bands:");
    for (int i = 0; i < NUM_AUDIO_BANDS; i++) {
        Serial.print("  Band ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(colonyBaseline.audioEnergy[i], 3);
        Serial.print(" ±");
        Serial.println(colonyBaseline.audioStdDev[i], 3);
    }
    
    Serial.print("Learning samples: ");
    Serial.println(learningSampleCount);
}

/**
 * Get current learning status
 */
bool isBaselineEstablished() {
    return baselineEstablished;
}

/**
 * Get learning progress percentage
 */
uint8_t getLearningProgress() {
    if (baselineEstablished) return 100;
    
    return min(99, (learningSampleCount * 100) / LEARNING_SAMPLES_MIN);
}

/**
 * Reset statistics without losing all history
 */
void RunningStats::partialReset(float keepRatio) {
    if (count_ > 0) {
        count_ = max(1, (int)(count_ * keepRatio));
    }
}

/**
 * Set statistics to known values
 */
void RunningStats::setStats(float mean, float stdDev) {
    mean_ = mean;
    // To set a known standard deviation with a valid M2, we need:
    // stdDev² = M2 / (count - 1)
    // So: M2 = stdDev² * (count - 1)
    count_ = 30; // Arbitrary sample size that gives reasonable weight
    M2_ = stdDev * stdDev * (count_ - 1);
}

/**
 * Add a sample to running statistics
 */
void RunningStats::addSample(float value) {
    count_++;
    
    // Welford's online algorithm
    float delta = value - mean_;
    mean_ += delta / count_;
    float delta2 = value - mean_;
    M2_ += delta * delta2;
}

/**
 * Reset statistics
 */
void RunningStats::reset() {
    count_ = 0;
    mean_ = 0.0;
    M2_ = 0.0;
}

/**
 * Get current mean
 */
float RunningStats::mean() const {
    return (count_ > 0) ? mean_ : 0.0f;
}

/**
 * Get current variance
 */
float RunningStats::variance() const {
    return (count_ > 1) ? M2_ / (count_ - 1) : 0.0f;
}

/**
 * Get current standard deviation
 */
float RunningStats::standardDeviation() const {
    return sqrt(variance());
}

/**
 * Get sample count
 */
unsigned long RunningStats::count() const {
    return count_;
}