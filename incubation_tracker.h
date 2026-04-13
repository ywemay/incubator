#ifndef INCUBATION_TRACKER_H
#define INCUBATION_TRACKER_H

#include <Arduino.h>

// Conditional compilation for ESP32 only
#ifdef ESP32
#include <Preferences.h>
#include "config.h"

// Bird species presets
enum BirdSpecies {
    BIRD_QUAIL = 0,
    BIRD_CHICKEN,
    BIRD_DUCK,
    BIRD_GOOSE,
    BIRD_PEACOCK,
    BIRD_TURKEY,
    BIRD_PHEASANT,
    BIRD_GUINEA_FOWL,
    BIRD_CUSTOM
};

class IncubationTracker {
private:
    // Incubation preset structure
    struct IncubationPreset {
        BirdSpecies species;
        const char* name;
        float target_temp;      // Target temperature in Celsius
        float target_humidity;  // Target humidity in percentage
        unsigned int incubation_days;  // Total incubation days
        unsigned int candling_day;     // Day for candling (0 = no candling)
        unsigned int lockdown_day;     // Day for lockdown (stop turning)
        unsigned int turn_interval;    // Turning interval in seconds
    };
private:
    Preferences preferences;
    bool initialized;
    
    // Current incubation session
    time_t incubation_start_time;  // When incubation started (Unix timestamp)
    BirdSpecies current_species;
    bool session_active;
    
    // Presets for different bird species
    static const IncubationPreset presets[];
    static const int PRESET_COUNT = 9;
    
public:
    IncubationTracker();
    ~IncubationTracker();
    
    // Initialization
    bool begin();
    void end();
    
    // Session management
    bool startIncubation(BirdSpecies species, time_t start_time = 0, 
                        float* out_target_temp = nullptr, 
                        unsigned int* out_turn_interval = nullptr);
    bool stopIncubation(float* out_target_temp = nullptr, 
                       unsigned int* out_turn_interval = nullptr);
    bool adjustIncubationDays(int days_adjustment);
    bool setIncubationDay(unsigned int day_number);
    bool isSessionActive() const { return session_active; }
    
    // Time calculations
    unsigned long getElapsedSeconds() const;
    unsigned int getElapsedDays() const;
    unsigned int getRemainingDays() const;
    
    // Important dates
    bool isCandlingDay() const;
    bool isLockdownDay() const;
    bool isHatchingDay() const;
    
    // Preset management
    BirdSpecies getCurrentSpecies() const { return current_species; }
    
    // Preset data accessors
    float getCurrentTargetTemp() const;
    float getCurrentTargetHumidity() const;
    unsigned int getCurrentIncubationDays() const;
    unsigned int getCurrentCandlingDay() const;
    unsigned int getCurrentLockdownDay() const;
    unsigned int getCurrentTurnInterval() const;
    String getCurrentPresetName() const;
    
    // Configuration persistence
    bool saveSession();
    bool loadSession();
    bool clearSession();
    
    // Status information
    String getStatusString() const;
    String getSpeciesName() const;
    String getTimeRemainingString() const;
    
    // Power failure recovery
    void recoverFromPowerLoss();
    
private:
    // Helper functions
    time_t getCurrentTime() const;
    String formatTime(unsigned long seconds) const;
    void loadDefaults();
};

#else
// Dummy implementation for non-ESP32 platforms
class IncubationTracker {
public:
    IncubationTracker() {}
    bool begin() { return true; }
    void end() { /* No-op */ }
    bool startIncubation(int species, time_t start_time = 0, 
                        float* out_target_temp = nullptr, 
                        unsigned int* out_turn_interval = nullptr) { 
        if (out_target_temp) *out_target_temp = 38.0;
        if (out_turn_interval) *out_turn_interval = 8 * 60 * 60;
        return false; 
    }
    bool stopIncubation(float* out_target_temp = nullptr, 
                       unsigned int* out_turn_interval = nullptr) { 
        if (out_target_temp) *out_target_temp = 38.0;
        if (out_turn_interval) *out_turn_interval = 8 * 60 * 60;
        return false; 
    }
    bool adjustIncubationDays(int days_adjustment) { return false; }
    bool setIncubationDay(unsigned int day_number) { return false; }
    bool isSessionActive() const { return false; }
    unsigned long getElapsedSeconds() const { return 0; }
    unsigned int getElapsedDays() const { return 0; }
    unsigned int getRemainingDays() const { return 0; }
    bool isCandlingDay() const { return false; }
    bool isLockdownDay() const { return false; }
    bool isHatchingDay() const { return false; }
    int getCurrentSpecies() const { return 0; }
    float getCurrentTargetTemp() const { return 38.0; }
    float getCurrentTargetHumidity() const { return 50.0; }
    unsigned int getCurrentIncubationDays() const { return 21; }
    unsigned int getCurrentCandlingDay() const { return 0; }
    unsigned int getCurrentLockdownDay() const { return 0; }
    unsigned int getCurrentTurnInterval() const { return 8 * 60 * 60; }
    String getCurrentPresetName() const { return "Custom"; }
    bool saveSession() { return false; }
    bool loadSession() { return false; }
    bool clearSession() { return false; }
    String getStatusString() const { return "N/A"; }
    String getSpeciesName() const { return "N/A"; }
    String getTimeRemainingString() const { return "N/A"; }
    void recoverFromPowerLoss() { /* No-op */ }
};

#endif // ESP32

#endif // INCUBATION_TRACKER_H