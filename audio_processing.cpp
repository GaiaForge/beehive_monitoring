/**
 * Hive Monitor System - Audio Processing Module
 * 
 * This module handles audio capture from the onboard PDM microphone,
 * performs FFT analysis, and classifies sound patterns related to
 * bee activity within the hive.
 * 
 * The audio is analyzed in four frequency bands:
 * - Band 1 (B1): 200-300 Hz - Normal hive hum
 * - Band 2 (B2): 300-600 Hz - Queen piping
 * - Band 3 (B3): 600-1000 Hz - Swarming agitation
 * - Band 4 (B4): 1000-3000 Hz - Alarm or disturbance
 */

#include "audio_processing.h"
#include "config.h"
#include <PDM.h>
#include <Arduino.h>

// Audio buffer and FFT variables
#define PDM_BUFFER_SIZE 2048
int16_t pdmSamples[PDM_BUFFER_SIZE];
volatile int pdmSamplesReady = 0;

// Energy in each frequency band
float audioEnergy[4] = {0};

// Sound classification result
SoundClass currentSoundClass = SOUND_UNKNOWN;

/**
 * Initialize the PDM microphone
 */
void setupMicrophone() {
  // Configure the data ready callback
  PDM.onReceive(pdmDataReadyCallback);
  
  // Initialize PDM with:
  // - one channel (mono)
  // - 16 kHz sample rate
  if (!PDM.begin(1, MIC_SAMPLING_RATE)) {
    Serial.println("Failed to start PDM!");
  }
}

/**
 * Capture audio samples from the PDM microphone
 */
void captureAudio() {
  // Reset the sample ready flag
  pdmSamplesReady = 0;
  
  // Wake up PDM microphone and start sampling
  if (!PDM.begin(1, MIC_SAMPLING_RATE)) {
    Serial.println("Failed to start PDM!");
    return;
  }
  
  // Wait for samples to be collected
  unsigned long startTime = millis();
  while (!pdmSamplesReady && (millis() - startTime < MIC_SAMPLE_DURATION)) {
    // Wait for the PDM data ready interrupt
    delay(10);
  }
  
  // Stop PDM to save power
  PDM.end();
  
  if (!pdmSamplesReady) {
    Serial.println("Timeout waiting for audio samples");
  }
}

/**
 * Analyze audio to determine energy in frequency bands
 * This is a simplified version without full FFT implementation
 */
void analyzeAudio() {
  // Reset energy values
  for (int i = 0; i < 4; i++) {
    audioEnergy[i] = 0.0f;
  }
  
  // Capture audio samples
  captureAudio();
  
  if (pdmSamplesReady) {
    Serial.println("Processing audio samples...");
    
    // Simple energy calculation in frequency bands
    // This is a simplified approach - in a full implementation,
    // you would use a proper FFT library
    for (int i = 0; i < PDM_BUFFER_SIZE; i++) {
      int sample = pdmSamples[i];
      float normSample = abs(sample) / 32768.0;
      
      // Band 1: 200-300 Hz - normal hive hum
      // We're using sample rate divisions as a crude frequency filter
      if (i % 16 == 0) audioEnergy[0] += normSample;
      
      // Band 2: 300-600 Hz - queen piping
      if (i % 8 == 0) audioEnergy[1] += normSample;
      
      // Band 3: 600-1000 Hz - swarming
      if (i % 4 == 0) audioEnergy[2] += normSample;
      
      // Band 4: 1000-3000 Hz - alarm
      if (i % 2 == 0) audioEnergy[3] += normSample;
    }
    
    // Normalize energy values
    for (int i = 0; i < 4; i++) {
      audioEnergy[i] /= (PDM_BUFFER_SIZE / 16.0);
    }
    
    // Classify the sound
    currentSoundClass = classifySound();
    
    // Print results
    Serial.println("Audio Energy Bands:");
    Serial.print("B1 (200-300Hz): "); Serial.println(audioEnergy[0]);
    Serial.print("B2 (300-600Hz): "); Serial.println(audioEnergy[1]);
    Serial.print("B3 (600-1000Hz): "); Serial.println(audioEnergy[2]);
    Serial.print("B4 (1000-3000Hz): "); Serial.println(audioEnergy[3]);
    Serial.print("Classification: "); Serial.println(getSoundClassName(currentSoundClass));
  } else {
    Serial.println("No audio samples to process");
  }
}

/**
 * Classify sound based on energy in frequency bands
 */
SoundClass classifySound() {
  // Check for silence first (possible absconding)
  if (audioEnergy[0] < THRESH_SILENT && 
      audioEnergy[1] < THRESH_SILENT &&
      audioEnergy[2] < THRESH_SILENT && 
      audioEnergy[3] < THRESH_SILENT) {
    return SOUND_SILENT;
  }
  
  // Check for alarm sounds (highest priority)
  if (audioEnergy[3] > THRESH_B4) {
    return SOUND_ALARM;
  }
  
  // Check for swarming sounds
  if (audioEnergy[2] > THRESH_B3) {
    return SOUND_SWARM;
  }
  
  // Check for queen piping
  if (audioEnergy[1] > THRESH_B2) {
    return SOUND_QUEEN;
  }
  
  // Check for normal hive hum
  if (audioEnergy[0] > THRESH_B1) {
    return SOUND_NORMAL;
  }
  
  // Default
  return SOUND_UNKNOWN;
}

/**
 * Get string representation of sound class
 */
const char* getSoundClassName(SoundClass soundClass) {
  switch (soundClass) {
    case SOUND_NORMAL: return "Normal";
    case SOUND_QUEEN: return "Queen Activity";
    case SOUND_SWARM: return "Swarming";
    case SOUND_ALARM: return "Alarm";
    case SOUND_SILENT: return "Silent";
    case SOUND_UNKNOWN:
    default: return "Unknown";
  }
}

/**
 * Get current sound classification
 */
SoundClass getCurrentSoundClass() {
  return currentSoundClass;
}

/**
 * Get energy values for the different frequency bands
 */
void getAudioEnergyValues(float* energyValues) {
  for (int i = 0; i < 4; i++) {
    energyValues[i] = audioEnergy[i];
  }
}

/**
 * PDM microphone data ready callback
 */
void pdmDataReadyCallback() {
  // Get PDM samples
  int bytesRead = PDM.read(pdmSamples, PDM_BUFFER_SIZE * 2);
  pdmSamplesReady = 1;
}