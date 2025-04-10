/**
 * Hive Monitor System - Audio Processing Header
 * 
 * Header file for the audio processing module that handles microphone
 * capture and frequency analysis for bee activity monitoring.
 */

#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

// Sound classification types
enum SoundClass {
  SOUND_NORMAL,    // Normal hive hum
  SOUND_QUEEN,     // Queen piping
  SOUND_SWARM,     // Swarming agitation
  SOUND_ALARM,     // Alarm/disturbance 
  SOUND_SILENT,    // Quiet/possible absconding
  SOUND_UNKNOWN    // Unable to classify
};

// Function prototypes
void setupMicrophone();
void captureAudio();
void analyzeAudio();
SoundClass classifySound();
SoundClass getCurrentSoundClass();
const char* getSoundClassName(SoundClass soundClass);
void getAudioEnergyValues(float* energyValues);
void pdmDataReadyCallback();

#endif // AUDIO_PROCESSING_H