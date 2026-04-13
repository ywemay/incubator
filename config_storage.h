#ifndef CONFIG_STORAGE_H
#define CONFIG_STORAGE_H

#include <Arduino.h>

// Conditional compilation for ESP32 only
#ifdef ESP32
#include <Preferences.h>

class ConfigStorage {
private:
    Preferences preferences;
    bool initialized;
    
public:
    ConfigStorage();
    ~ConfigStorage();
    
    // Initialization
    bool begin();
    void end();
    
    // Temperature configuration
    bool saveTargetTemperature(float temp);
    float loadTargetTemperature(float default_temp = 38.0);
    
    // Egg turner configuration
    bool saveTurnInterval(unsigned int interval_seconds);
    unsigned int loadTurnInterval(unsigned int default_interval = 8 * 60 * 60);
    
    bool saveTurnDuration(unsigned int duration_seconds);
    unsigned int loadTurnDuration(unsigned int default_duration = 2);
    
    // System configuration
    bool saveSystemConfig(const char* key, const char* value);
    String loadSystemConfig(const char* key, const char* default_value = "");
    
    bool saveSystemConfig(const char* key, int value);
    int loadSystemConfig(const char* key, int default_value = 0);
    
    bool saveSystemConfig(const char* key, float value);
    float loadSystemConfig(const char* key, float default_value = 0.0);
    
    // Incubation tracking
    bool saveIncubationStart(time_t start_time);
    time_t loadIncubationStart(time_t default_value = 0);
    
    bool saveBirdSpecies(int species);
    int loadBirdSpecies(int default_value = 0);
    
    // Factory reset
    bool factoryReset();
    
    // Status
    bool isInitialized() const { return initialized; }
    
private:
    String getKeyName(const char* base_key);
};

#else
// Dummy implementation for non-ESP32 platforms
class ConfigStorage {
public:
    ConfigStorage() {}
    bool begin() { return true; }
    void end() { /* No-op */ }
    bool saveTargetTemperature(float temp) { return false; }
    float loadTargetTemperature(float default_temp = 38.0) { return default_temp; }
    bool saveTurnInterval(unsigned int interval_seconds) { return false; }
    unsigned int loadTurnInterval(unsigned int default_interval = 8 * 60 * 60) { return default_interval; }
    bool saveTurnDuration(unsigned int duration_seconds) { return false; }
    unsigned int loadTurnDuration(unsigned int default_duration = 2) { return default_duration; }
    bool saveSystemConfig(const char* key, const char* value) { return false; }
    String loadSystemConfig(const char* key, const char* default_value = "") { return String(default_value); }
    bool saveSystemConfig(const char* key, int value) { return false; }
    int loadSystemConfig(const char* key, int default_value = 0) { return default_value; }
    bool saveSystemConfig(const char* key, float value) { return false; }
    float loadSystemConfig(const char* key, float default_value = 0.0) { return default_value; }
    bool factoryReset() { return false; }
    bool isInitialized() const { return false; }
};

#endif // ESP32

#endif // CONFIG_STORAGE_H