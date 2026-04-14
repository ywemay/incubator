#include "config_storage.h"

// Only compile for ESP32
#ifdef ESP32

ConfigStorage::ConfigStorage() : initialized(false) {
}

ConfigStorage::~ConfigStorage() {
    if (initialized) {
        end();
    }
}

bool ConfigStorage::begin() {
    if (initialized) {
        return true;
    }
    
    initialized = preferences.begin("incubator", false);
    if (!initialized) {
        Serial.println("[Config] Failed to initialize preferences");
        return false;
    }
    
    Serial.println("[Config] Preferences initialized");
    return true;
}

void ConfigStorage::end() {
    if (initialized) {
        preferences.end();
        initialized = false;
        Serial.println("[Config] Preferences closed");
    }
}

bool ConfigStorage::saveTargetTemperature(float temp) {
    if (!begin()) return false;
    
    if (temp < 35.0 || temp > 42.0) {
        Serial.printf("[Config] Invalid temperature: %.1f°C\n", temp);
        return false;
    }
    
    bool success = preferences.putFloat("target_temp", temp);
    end();
    
    if (success) {
        Serial.printf("[Config] Saved target temperature: %.1f°C\n", temp);
    } else {
        Serial.println("[Config] Failed to save target temperature");
    }
    
    return success;
}

float ConfigStorage::loadTargetTemperature(float default_temp) {
    if (!begin()) return default_temp;
    
    float temp = preferences.getFloat("target_temp", default_temp);
    end();
    
    // Validate loaded temperature
    if (temp < 35.0 || temp > 42.0) {
        Serial.printf("[Config] Loaded invalid temperature (%.1f°C), using default\n", temp);
        return default_temp;
    }
    
    Serial.printf("[Config] Loaded target temperature: %.1f°C\n", temp);
    return temp;
}

bool ConfigStorage::saveTurnInterval(unsigned int interval_seconds) {
    if (!begin()) return false;
    
    // Validate interval (1 hour to 24 hours)
    if (interval_seconds < 3600 || interval_seconds > 86400) {
        Serial.printf("[Config] Invalid turn interval: %u seconds\n", interval_seconds);
        return false;
    }
    
    bool success = preferences.putUInt("turn_interval", interval_seconds);
    end();
    
    if (success) {
        unsigned int hours = interval_seconds / 3600;
        unsigned int minutes = (interval_seconds % 3600) / 60;
        Serial.printf("[Config] Saved turn interval: %u hours %u minutes\n", hours, minutes);
    } else {
        Serial.println("[Config] Failed to save turn interval");
    }
    
    return success;
}

unsigned int ConfigStorage::loadTurnInterval(unsigned int default_interval) {
    if (!begin()) return default_interval;
    
    unsigned int interval = preferences.getUInt("turn_interval", default_interval);
    end();
    
    // Validate loaded interval
    if (interval < 3600 || interval > 86400) {
        Serial.printf("[Config] Loaded invalid interval (%u seconds), using default\n", interval);
        return default_interval;
    }
    
    unsigned int hours = interval / 3600;
    unsigned int minutes = (interval % 3600) / 60;
    Serial.printf("[Config] Loaded turn interval: %u hours %u minutes\n", hours, minutes);
    
    return interval;
}

bool ConfigStorage::saveTurnDuration(unsigned int duration_seconds) {
    if (!begin()) return false;
    
    // Validate duration (1 second to 60 seconds)
    if (duration_seconds < 1 || duration_seconds > 60) {
        Serial.printf("[Config] Invalid turn duration: %u seconds\n", duration_seconds);
        return false;
    }
    
    bool success = preferences.putUInt("turn_duration", duration_seconds);
    end();
    
    if (success) {
        Serial.printf("[Config] Saved turn duration: %u seconds\n", duration_seconds);
    } else {
        Serial.println("[Config] Failed to save turn duration");
    }
    
    return success;
}

unsigned int ConfigStorage::loadTurnDuration(unsigned int default_duration) {
    if (!begin()) return default_duration;
    
    unsigned int duration = preferences.getUInt("turn_duration", default_duration);
    end();
    
    // Validate loaded duration
    if (duration < 1 || duration > 60) {
        Serial.printf("[Config] Loaded invalid duration (%u seconds), using default\n", duration);
        return default_duration;
    }
    
    Serial.printf("[Config] Loaded turn duration: %u seconds\n", duration);
    return duration;
}

bool ConfigStorage::saveSystemConfig(const char* key, const char* value) {
    if (!begin()) return false;
    
    String full_key = getKeyName(key);
    bool success = preferences.putString(full_key.c_str(), value);
    end();
    
    if (success) {
        Serial.printf("[Config] Saved system config: %s = %s\n", full_key.c_str(), value);
    } else {
        Serial.printf("[Config] Failed to save system config: %s\n", full_key.c_str());
    }
    
    return success;
}

String ConfigStorage::loadSystemConfig(const char* key, const char* default_value) {
    if (!begin()) return String(default_value);
    
    String full_key = getKeyName(key);
    String value = preferences.getString(full_key.c_str(), default_value);
    end();
    
    Serial.printf("[Config] Loaded system config: %s = %s\n", full_key.c_str(), value.c_str());
    return value;
}

bool ConfigStorage::saveSystemConfig(const char* key, int value) {
    if (!begin()) return false;
    
    String full_key = getKeyName(key);
    bool success = preferences.putInt(full_key.c_str(), value);
    end();
    
    if (success) {
        Serial.printf("[Config] Saved system config: %s = %d\n", full_key.c_str(), value);
    } else {
        Serial.printf("[Config] Failed to save system config: %s\n", full_key.c_str());
    }
    
    return success;
}

int ConfigStorage::loadSystemConfig(const char* key, int default_value) {
    if (!begin()) return default_value;
    
    String full_key = getKeyName(key);
    int value = preferences.getInt(full_key.c_str(), default_value);
    end();
    
    Serial.printf("[Config] Loaded system config: %s = %d\n", full_key.c_str(), value);
    return value;
}

bool ConfigStorage::saveSystemConfig(const char* key, float value) {
    if (!begin()) return false;
    
    String full_key = getKeyName(key);
    bool success = preferences.putFloat(full_key.c_str(), value);
    end();
    
    if (success) {
        Serial.printf("[Config] Saved system config: %s = %.2f\n", full_key.c_str(), value);
    } else {
        Serial.printf("[Config] Failed to save system config: %s\n", full_key.c_str());
    }
    
    return success;
}

float ConfigStorage::loadSystemConfig(const char* key, float default_value) {
    if (!begin()) return default_value;
    
    String full_key = getKeyName(key);
    float value = preferences.getFloat(full_key.c_str(), default_value);
    end();
    
    Serial.printf("[Config] Loaded system config: %s = %.2f\n", full_key.c_str(), value);
    return value;
}

bool ConfigStorage::saveIncubationStart(time_t start_time) {
    if (!begin()) return false;
    
    bool success = preferences.putULong64("incubation_start", static_cast<uint64_t>(start_time));
    end();
    
    if (success) {
        Serial.printf("[Config] Saved incubation start time: %llu\n", static_cast<unsigned long long>(start_time));
    } else {
        Serial.println("[Config] Failed to save incubation start time");
    }
    
    return success;
}

time_t ConfigStorage::loadIncubationStart(time_t default_value) {
    if (!begin()) return default_value;
    
    uint64_t stored_value = preferences.getULong64("incubation_start", static_cast<uint64_t>(default_value));
    end();
    
    time_t start_time = static_cast<time_t>(stored_value);
    Serial.printf("[Config] Loaded incubation start time: %llu\n", static_cast<unsigned long long>(start_time));
    return start_time;
}

bool ConfigStorage::saveBirdSpecies(int species) {
    if (!begin()) return false;
    
    bool success = preferences.putInt("bird_species", species);
    end();
    
    if (success) {
        Serial.printf("[Config] Saved bird species: %d\n", species);
    } else {
        Serial.println("[Config] Failed to save bird species");
    }
    
    return success;
}

int ConfigStorage::loadBirdSpecies(int default_value) {
    if (!begin()) return default_value;
    
    int species = preferences.getInt("bird_species", default_value);
    end();
    
    Serial.printf("[Config] Loaded bird species: %d\n", species);
    return species;
}

bool ConfigStorage::saveIncubatorName(const String& name) {
    if (!begin()) return false;
    
    // Validate name length (max 32 characters for safety)
    String trimmed_name = name;
    trimmed_name.trim();
    
    if (trimmed_name.length() > 32) {
        trimmed_name = trimmed_name.substring(0, 32);
    }
    
    bool success = preferences.putString("incubator_name", trimmed_name.c_str());
    end();
    
    if (success) {
        Serial.printf("[Config] Saved incubator name: %s\n", trimmed_name.c_str());
    } else {
        Serial.println("[Config] Failed to save incubator name");
    }
    
    return success;
}

String ConfigStorage::loadIncubatorName(const String& default_name) {
    if (!begin()) return default_name;
    
    String name = preferences.getString("incubator_name", default_name.c_str());
    end();
    
    Serial.printf("[Config] Loaded incubator name: %s\n", name.c_str());
    return name;
}

bool ConfigStorage::factoryReset() {
    if (!begin()) return false;
    
    bool success = preferences.clear();
    end();
    
    if (success) {
        Serial.println("[Config] Factory reset completed");
    } else {
        Serial.println("[Config] Factory reset failed");
    }
    
    return success;
}

String ConfigStorage::getKeyName(const char* base_key) {
    return String("sys_") + base_key;
}

#endif // ESP32