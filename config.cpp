/**
 * Hive Monitor System - Configuration Implementation
 * 
 * This file implements loading and managing configuration settings
 * for the hive monitor system. It handles loading overrides from 
 * the SD card and providing access to configuration values.
 */

#include "config.h"
#include <SD.h>
#include <ArduinoJson.h>

// Maximum config items to store
#define MAX_CONFIG_ITEMS 30

// Config key-value pairs
struct ConfigItem {
    char key[32];
    char value[32];
};

// Config storage
static ConfigItem configItems[MAX_CONFIG_ITEMS];
static int configItemCount = 0;

/**
 * Load configuration overrides from SD card
 */
bool loadConfigFromSD() {
    // Reset config count
    configItemCount = 0;
    
    // Check if SD is available
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("Failed to initialize SD card - using default configuration");
        return false;
    }
    
    // Try loading JSON config first
    if (SD.exists("/CONFIG.JSON")) {
        return loadConfigJSON();
    }
    
    // Fall back to text config
    if (SD.exists("/CONFIG.TXT")) {
        return loadConfigTXT();
    }
    
    Serial.println("No configuration file found - using default configuration");
    return false;
}

/**
 * Load configuration from JSON file
 */
bool loadConfigJSON() {
    File configFile = SD.open("/CONFIG.JSON", FILE_READ);
    if (!configFile) {
        Serial.println("Failed to open CONFIG.JSON");
        return false;
    }
    
    Serial.println("Loading configuration from CONFIG.JSON");
    
    // Allocate JsonDocument
    StaticJsonDocument<2048> doc;
    
    // Parse JSON
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();
    
    if (error) {
        Serial.print("Failed to parse CONFIG.JSON: ");
        Serial.println(error.c_str());
        return false;
    }
    
    // Copy values to config storage
    for (JsonPair kv : doc.as<JsonObject>()) {
        if (configItemCount >= MAX_CONFIG_ITEMS) {
            Serial.println("Too many config items - some will be ignored");
            break;
        }
        
        // Store key
        strncpy(configItems[configItemCount].key, kv.key().c_str(), sizeof(configItems[0].key)-1);
        configItems[configItemCount].key[sizeof(configItems[0].key)-1] = '\0';
        
        // Store value as string
        if (kv.value().is<const char*>()) {
            strncpy(configItems[configItemCount].value, kv.value().as<const char*>(), sizeof(configItems[0].value)-1);
        } else if (kv.value().is<int>()) {
            snprintf(configItems[configItemCount].value, sizeof(configItems[0].value), "%d", kv.value().as<int>());
        } else if (kv.value().is<float>()) {
            snprintf(configItems[configItemCount].value, sizeof(configItems[0].value), "%f", kv.value().as<float>());
        } else if (kv.value().is<bool>()) {
            strncpy(configItems[configItemCount].value, kv.value().as<bool>() ? "1" : "0", sizeof(configItems[0].value)-1);
        }
        configItems[configItemCount].value[sizeof(configItems[0].value)-1] = '\0';
        
        configItemCount++;
    }
    
    Serial.print("Loaded ");
    Serial.print(configItemCount);
    Serial.println(" configuration items");
    
    return true;
}

/**
 * Load configuration from plain text file
 */
bool loadConfigTXT() {
    File configFile = SD.open("/CONFIG.TXT", FILE_READ);
    if (!configFile) {
        Serial.println("Failed to open CONFIG.TXT");
        return false;
    }
    
    Serial.println("Loading configuration from CONFIG.TXT");
    
    // Read file line by line
    while (configFile.available() && configItemCount < MAX_CONFIG_ITEMS) {
        String line = configFile.readStringUntil('\n');
        line.trim();
        
        // Skip empty lines and comments
        if (line.length() == 0 || line.startsWith("#") || line.startsWith("//")) {
            continue;
        }
        
        // Find equals sign
        int equalsPos = line.indexOf('=');
        if (equalsPos <= 0) {
            continue;
        }
        
        // Extract key and value
        String key = line.substring(0, equalsPos);
        String value = line.substring(equalsPos + 1);
        
        // Trim whitespace
        key.trim();
        value.trim();
        
        // Store in config array
        strncpy(configItems[configItemCount].key, key.c_str(), sizeof(configItems[0].key)-1);
        configItems[configItemCount].key[sizeof(configItems[0].key)-1] = '\0';
        
        strncpy(configItems[configItemCount].value, value.c_str(), sizeof(configItems[0].value)-1);
        configItems[configItemCount].value[sizeof(configItems[0].value)-1] = '\0';
        
        configItemCount++;
    }
    
    configFile.close();
    
    Serial.print("Loaded ");
    Serial.print(configItemCount);
    Serial.println(" configuration items");
    
    return true;
}

/**
 * Print current configuration to serial
 */
void printConfig() {
    Serial.println("Current Configuration:");
    Serial.println("----------------------");
    
    // Print defaults from config.h
    Serial.print("DEVICE_ID: ");
    Serial.println(DEVICE_ID);
    
    Serial.print("FIRMWARE_VERSION: ");
    Serial.println(FIRMWARE_VERSION);
    
    Serial.print("WAKE_INTERVAL_MINUTES: ");
    Serial.println(WAKE_INTERVAL_MINUTES);
    
    Serial.print("ENABLE_BLE: ");
    Serial.println(ENABLE_BLE);
    
    Serial.print("ENABLE_LEARNING: ");
    Serial.println(ENABLE_LEARNING);
    
    // Print overrides
    if (configItemCount > 0) {
        Serial.println("\nConfiguration Overrides:");
        Serial.println("------------------------");
        
        for (int i = 0; i < configItemCount; i++) {
            Serial.print(configItems[i].key);
            Serial.print(": ");
            Serial.println(configItems[i].value);
        }
    }
    
    Serial.println("----------------------");
}

/**
 * Get string configuration value
 */
const char* getConfigValueStr(const char* key) {
    // Check for override
    for (int i = 0; i < configItemCount; i++) {
        if (strcasecmp(configItems[i].key, key) == 0) {
            return configItems[i].value;
        }
    }
    
    // No override found, return NULL
    return NULL;
}

/**
 * Get integer configuration value
 */
int getConfigValueInt(const char* key, int defaultValue) {
    const char* strValue = getConfigValueStr(key);
    if (strValue == NULL) {
        return defaultValue;
    }
    
    return atoi(strValue);
}

/**
 * Get float configuration value
 */
float getConfigValueFloat(const char* key, float defaultValue) {
    const char* strValue = getConfigValueStr(key);
    if (strValue == NULL) {
        return defaultValue;
    }
    
    return atof(strValue);
}

/**
 * Get actual wake interval adjusted for any overrides
 */
int getWakeIntervalMinutes() {
    return getConfigValueInt("WAKE_INTERVAL_MINUTES", WAKE_INTERVAL_MINUTES);
}

/**
 * Check if BLE is enabled
 */
bool isBLEEnabled() {
    return getConfigValueInt("ENABLE_BLE", ENABLE_BLE) != 0;
}

/**
 * Check if learning system is enabled
 */
bool isLearningEnabled() {
    return getConfigValueInt("ENABLE_LEARNING", ENABLE_LEARNING) != 0;
}

/**
 * Get temperature thresholds (possibly adjusted by learning)
 */
void getTemperatureThresholds(float* lowThreshold, float* highThreshold) {
    // Start with configured defaults
    *lowThreshold = getConfigValueFloat("TEMP_ALERT_LOW", TEMP_ALERT_LOW);
    *highThreshold = getConfigValueFloat("TEMP_ALERT_HIGH", TEMP_ALERT_HIGH);
    
    // Let learning system adjust if enabled and available
    if (isLearningEnabled() && isBaselineEstablished()) {
        DateTime now = getRTCTime();
        getAdaptedTempThresholds(lowThreshold, highThreshold, now.hour());
    }
}

/**
 * Get humidity thresholds (possibly adjusted by learning)
 */
void getHumidityThresholds(float* lowThreshold, float* highThreshold) {
    // Start with configured defaults
    *lowThreshold = getConfigValueFloat("HUM_ALERT_LOW", HUM_ALERT_LOW);
    *highThreshold = getConfigValueFloat("HUM_ALERT_HIGH", HUM_ALERT_HIGH);
    
    // Let learning system adjust if enabled and available
    if (isLearningEnabled() && isBaselineEstablished()) {
        DateTime now = getRTCTime();
        getAdaptedHumidityThresholds(lowThreshold, highThreshold, now.hour());
    }
}

/**
 * Get audio thresholds (possibly adjusted by learning)
 */
void getAudioThresholds(float thresholds[4]) {
    // Start with configured defaults
    thresholds[0] = getConfigValueFloat("THRESH_B1", THRESH_B1);
    thresholds[1] = getConfigValueFloat("THRESH_B2", THRESH_B2);
    thresholds[2] = getConfigValueFloat("THRESH_B3", THRESH_B3);
    thresholds[3] = getConfigValueFloat("THRESH_B4", THRESH_B4);
    
    // Let learning system adjust if enabled and available
    if (isLearningEnabled() && isBaselineEstablished()) {
        getAdaptedAudioThresholds(thresholds);
    }
}