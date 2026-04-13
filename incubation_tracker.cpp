#include "incubation_tracker.h"

#ifdef ESP32
#include <time.h>

// Preset definitions for different bird species
const IncubationTracker::IncubationPreset IncubationTracker::presets[] = {
    // Quail
    {BIRD_QUAIL, "Quail", 37.5, 55.0, 17, 7, 14, 2 * 60 * 60}, // 2 hour turn interval
    
    // Chicken
    {BIRD_CHICKEN, "Chicken", 37.5, 55.0, 21, 7, 18, 4 * 60 * 60}, // 4 hour turn interval
    
    // Duck
    {BIRD_DUCK, "Duck", 37.5, 55.0, 28, 7, 25, 4 * 60 * 60}, // 4 hour turn interval
    
    // Goose
    {BIRD_GOOSE, "Goose", 37.5, 55.0, 30, 10, 27, 4 * 60 * 60}, // 4 hour turn interval
    
    // Peacock
    {BIRD_PEACOCK, "Peacock", 37.5, 55.0, 28, 10, 25, 4 * 60 * 60}, // 4 hour turn interval
    
    // Turkey
    {BIRD_TURKEY, "Turkey", 37.5, 55.0, 28, 7, 25, 4 * 60 * 60}, // 4 hour turn interval
    
    // Pheasant
    {BIRD_PHEASANT, "Pheasant", 37.5, 55.0, 24, 7, 21, 4 * 60 * 60}, // 4 hour turn interval
    
    // Guinea Fowl
    {BIRD_GUINEA_FOWL, "Guinea Fowl", 37.5, 55.0, 26, 7, 23, 4 * 60 * 60}, // 4 hour turn interval
    
    // Custom (default values)
    {BIRD_CUSTOM, "Custom", 38.0, 50.0, 21, 0, 18, 8 * 60 * 60} // 8 hour turn interval
};

IncubationTracker::IncubationTracker() 
    : initialized(false), incubation_start_time(0), 
      current_species(BIRD_CUSTOM), session_active(false) {
}

IncubationTracker::~IncubationTracker() {
    if (initialized) {
        end();
    }
}

bool IncubationTracker::begin() {
    if (initialized) {
        return true;
    }
    
    if (!preferences.begin("incubator", false)) {
        Serial.println("Failed to open preferences for incubation tracker");
        return false;
    }
    
    initialized = true;
    
    // Try to load existing session
    if (!loadSession()) {
        // No existing session, load defaults
        loadDefaults();
    }
    
    // Check if we need to recover from power loss
    recoverFromPowerLoss();
    
    return true;
}

void IncubationTracker::end() {
    if (initialized) {
        saveSession();
        preferences.end();
        initialized = false;
    }
}

bool IncubationTracker::startIncubation(BirdSpecies species, time_t start_time,
                                        float* out_target_temp, 
                                        unsigned int* out_turn_interval) {
    if (!initialized) {
        return false;
    }
    
    current_species = species;
    
    if (start_time == 0) {
        incubation_start_time = getCurrentTime();
    } else {
        incubation_start_time = start_time;
    }
    
    session_active = true;
    
    // Update global incubator state
    #ifdef ESP32
    incubatorState = INCUBATOR_INCUBATING;
    #endif
    
    // Return the preset values if output pointers are provided
    if (out_target_temp) {
        *out_target_temp = getCurrentTargetTemp();
    }
    
    if (out_turn_interval) {
        *out_turn_interval = getCurrentTurnInterval();
    }
    
    // Save the session
    return saveSession();
}

bool IncubationTracker::stopIncubation(float* out_target_temp, 
                                       unsigned int* out_turn_interval) {
    if (!initialized) {
        return false;
    }
    
    session_active = false;
    incubation_start_time = 0;
    
    // Update global incubator state
    #ifdef ESP32
    incubatorState = INCUBATOR_IDLE;
    #endif
    
    // Return default values if output pointers are provided
    if (out_target_temp) {
        *out_target_temp = 38.0; // Default temperature
    }
    
    if (out_turn_interval) {
        *out_turn_interval = 8 * 60 * 60; // Default 8 hours
    }
    
    return clearSession();
}

bool IncubationTracker::adjustIncubationDays(int days_adjustment) {
    if (!session_active || !initialized) {
        return false;
    }
    
    // Calculate new start time based on adjustment
    time_t now = getCurrentTime();
    unsigned int elapsed_days = getElapsedDays();
    
    // Adjust elapsed days (positive adjustment = fewer days elapsed)
    int new_elapsed_days = static_cast<int>(elapsed_days) + days_adjustment;
    
    // Ensure we don't go negative or beyond incubation period
    unsigned int incubation_days = getCurrentIncubationDays();
    if (new_elapsed_days < 0) {
        new_elapsed_days = 0;
    } else if (new_elapsed_days > static_cast<int>(incubation_days)) {
        new_elapsed_days = incubation_days;
    }
    
    // Calculate new start time
    incubation_start_time = now - (new_elapsed_days * 24 * 60 * 60);
    
    Serial.printf("[Incubation] Adjusted by %d days. Now at day %d of %u\n", 
                 days_adjustment, new_elapsed_days, incubation_days);
    
    return saveSession();
}

bool IncubationTracker::setIncubationDay(unsigned int day_number) {
    if (!session_active || !initialized) {
        return false;
    }
    
    // Ensure day number is valid
    unsigned int incubation_days = getCurrentIncubationDays();
    if (day_number > incubation_days) {
        day_number = incubation_days;
    }
    
    // Calculate new start time
    time_t now = getCurrentTime();
    incubation_start_time = now - (day_number * 24 * 60 * 60);
    
    Serial.printf("[Incubation] Set to day %u of %u\n", day_number, incubation_days);
    
    return saveSession();
}

unsigned long IncubationTracker::getElapsedSeconds() const {
    if (!session_active || incubation_start_time == 0) {
        return 0;
    }
    
    time_t now = getCurrentTime();
    if (now < incubation_start_time) {
        return 0; // Time travel not supported
    }
    
    return static_cast<unsigned long>(now - incubation_start_time);
}

unsigned int IncubationTracker::getElapsedDays() const {
    unsigned long seconds = getElapsedSeconds();
    return static_cast<unsigned int>(seconds / (24 * 60 * 60));
}

unsigned int IncubationTracker::getRemainingDays() const {
    if (!session_active) {
        return 0;
    }
    
    unsigned int incubation_days = getCurrentIncubationDays();
    unsigned int elapsed_days = getElapsedDays();
    if (elapsed_days >= incubation_days) {
        return 0;
    }
    
    return incubation_days - elapsed_days;
}

bool IncubationTracker::isCandlingDay() const {
    if (!session_active) {
        return false;
    }
    
    unsigned int candling_day = getCurrentCandlingDay();
    if (candling_day == 0) {
        return false;
    }
    
    unsigned int elapsed_days = getElapsedDays();
    return (elapsed_days == candling_day);
}

bool IncubationTracker::isLockdownDay() const {
    if (!session_active) {
        return false;
    }
    
    unsigned int lockdown_day = getCurrentLockdownDay();
    if (lockdown_day == 0) {
        return false;
    }
    
    unsigned int elapsed_days = getElapsedDays();
    return (elapsed_days >= lockdown_day);
}

bool IncubationTracker::isHatchingDay() const {
    if (!session_active) {
        return false;
    }
    
    unsigned int incubation_days = getCurrentIncubationDays();
    unsigned int elapsed_days = getElapsedDays();
    return (elapsed_days >= incubation_days);
}

float IncubationTracker::getCurrentTargetTemp() const {
    for (int i = 0; i < PRESET_COUNT; i++) {
        if (presets[i].species == current_species) {
            return presets[i].target_temp;
        }
    }
    return presets[PRESET_COUNT - 1].target_temp; // Return custom preset as fallback
}

float IncubationTracker::getCurrentTargetHumidity() const {
    for (int i = 0; i < PRESET_COUNT; i++) {
        if (presets[i].species == current_species) {
            return presets[i].target_humidity;
        }
    }
    return presets[PRESET_COUNT - 1].target_humidity;
}

unsigned int IncubationTracker::getCurrentIncubationDays() const {
    for (int i = 0; i < PRESET_COUNT; i++) {
        if (presets[i].species == current_species) {
            return presets[i].incubation_days;
        }
    }
    return presets[PRESET_COUNT - 1].incubation_days;
}

unsigned int IncubationTracker::getCurrentCandlingDay() const {
    for (int i = 0; i < PRESET_COUNT; i++) {
        if (presets[i].species == current_species) {
            return presets[i].candling_day;
        }
    }
    return presets[PRESET_COUNT - 1].candling_day;
}

unsigned int IncubationTracker::getCurrentLockdownDay() const {
    for (int i = 0; i < PRESET_COUNT; i++) {
        if (presets[i].species == current_species) {
            return presets[i].lockdown_day;
        }
    }
    return presets[PRESET_COUNT - 1].lockdown_day;
}

unsigned int IncubationTracker::getCurrentTurnInterval() const {
    for (int i = 0; i < PRESET_COUNT; i++) {
        if (presets[i].species == current_species) {
            return presets[i].turn_interval;
        }
    }
    return presets[PRESET_COUNT - 1].turn_interval;
}

String IncubationTracker::getCurrentPresetName() const {
    for (int i = 0; i < PRESET_COUNT; i++) {
        if (presets[i].species == current_species) {
            return String(presets[i].name);
        }
    }
    return String(presets[PRESET_COUNT - 1].name);
}

bool IncubationTracker::saveSession() {
    if (!initialized) {
        return false;
    }
    
    preferences.putUInt("species", static_cast<unsigned int>(current_species));
    preferences.putULong64("start_time", static_cast<uint64_t>(incubation_start_time));
    preferences.putBool("session_active", session_active);
    
    return true;
}

bool IncubationTracker::loadSession() {
    if (!initialized) {
        return false;
    }
    
    current_species = static_cast<BirdSpecies>(preferences.getUInt("species", BIRD_CUSTOM));
    incubation_start_time = static_cast<time_t>(preferences.getULong64("start_time", 0));
    session_active = preferences.getBool("session_active", false);
    
    // Validate the loaded session
    if (session_active && incubation_start_time > 0) {
        time_t now = getCurrentTime();
        unsigned int incubation_days = getCurrentIncubationDays();
        
        // Check if the session has expired (past incubation period + 2 days grace)
        if ((now - incubation_start_time) > ((incubation_days + 2) * 24 * 60 * 60)) {
            session_active = false;
            incubation_start_time = 0;
            Serial.println("Loaded incubation session has expired. Clearing.");
        }
    }
    
    return true;
}

bool IncubationTracker::clearSession() {
    if (!initialized) {
        return false;
    }
    
    preferences.remove("species");
    preferences.remove("start_time");
    preferences.remove("session_active");
    
    return true;
}

String IncubationTracker::getStatusString() const {
    if (!session_active) {
        return "No active incubation session";
    }
    
    unsigned int incubation_days = getCurrentIncubationDays();
    unsigned int elapsed_days = getElapsedDays();
    unsigned int remaining_days = getRemainingDays();
    
    String status = "Species: " + getCurrentPresetName() + "\n";
    status += "Day: " + String(elapsed_days) + " of " + String(incubation_days) + "\n";
    status += "Remaining: " + String(remaining_days) + " days\n";
    
    if (isCandlingDay()) {
        status += "** TODAY: Candling Day **\n";
    }
    
    if (isLockdownDay()) {
        status += "** LOCKDOWN: Stop turning **\n";
    }
    
    if (isHatchingDay()) {
        status += "** HATCHING DAY! **\n";
    }
    
    return status;
}

String IncubationTracker::getSpeciesName() const {
    return getCurrentPresetName();
}

String IncubationTracker::getTimeRemainingString() const {
    if (!session_active) {
        return "No active session";
    }
    
    unsigned long remaining_seconds = 0;
    unsigned int incubation_days = getCurrentIncubationDays();
    
    unsigned long elapsed_seconds = getElapsedSeconds();
    unsigned long total_seconds = incubation_days * 24 * 60 * 60;
    
    if (elapsed_seconds < total_seconds) {
        remaining_seconds = total_seconds - elapsed_seconds;
    }
    
    return formatTime(remaining_seconds);
}

void IncubationTracker::recoverFromPowerLoss() {
    if (!session_active || incubation_start_time == 0) {
        return;
    }
    
    // Check if we had a power loss by comparing stored start time with current time
    time_t now = getCurrentTime();
    
    // If current time is before stored start time (system clock reset),
    // we've likely had a power loss and the RTC lost time
    if (now < incubation_start_time) {
        Serial.println("Detected possible power loss. Incubation time may be inaccurate.");
        // In a real implementation, you might want to adjust the start time
        // or notify the user about the time discrepancy
    }
}

time_t IncubationTracker::getCurrentTime() const {
    time_t now;
    time(&now);
    return now;
}

String IncubationTracker::formatTime(unsigned long seconds) const {
    unsigned long days = seconds / (24 * 60 * 60);
    seconds %= (24 * 60 * 60);
    unsigned long hours = seconds / (60 * 60);
    seconds %= (60 * 60);
    unsigned long minutes = seconds / 60;
    seconds %= 60;
    
    char buffer[64];
    if (days > 0) {
        snprintf(buffer, sizeof(buffer), "%lud %02luh %02lum", days, hours, minutes);
    } else if (hours > 0) {
        snprintf(buffer, sizeof(buffer), "%luh %02lum %02lus", hours, minutes, seconds);
    } else {
        snprintf(buffer, sizeof(buffer), "%lum %02lus", minutes, seconds);
    }
    
    return String(buffer);
}

void IncubationTracker::loadDefaults() {
    current_species = BIRD_CUSTOM;
    incubation_start_time = 0;
    session_active = false;
}

#endif // ESP32