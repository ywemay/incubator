# Incubation Settings Application Fix

## Problem
When clicking "Start Incubation" in the web interface, the incubation tracking system was only displaying information but not actually applying the bird species preset values to the control system.

## Solution
Modified the code to automatically apply incubation preset values when starting an incubation session:

### 1. Updated `IncubationTracker` Class
- Modified `startIncubation()` method to return preset values via output parameters
- Modified `stopIncubation()` method to return default values via output parameters
- This allows the caller to get the actual temperature and interval values

### 2. Updated Web Server Command Handling
- Enhanced `executeCommand()` for `start_incubation`:
  - Gets target temperature and turn interval from incubation tracker
  - Updates global `targetTemp` variable (used by temperature control)
  - Updates global `EGGS_TURNING_INTERVAL` variable (used by egg turner)
  - Saves new values to configuration storage
  - Resets egg turner timer to use new interval immediately
  - Logs the changes to serial monitor

- Enhanced `executeCommand()` for `stop_incubation`:
  - Gets default temperature and interval values
  - Restores global variables to defaults
  - Saves defaults to configuration storage
  - Resets egg turner timer
  - Logs the changes

### 3. Added Timer Reset Functionality
- Added `resetTimer()` method to `Turner` class
  - Immediately resets `eggsTurnCounter` to `EGGS_TURNING_INTERVAL`
  - Ensures new interval takes effect immediately

- Updated `updateConfig()` function:
  - Now calls `resetTimer()` when turn interval is changed
  - Ensures manual configuration changes also reset the timer

### 4. Updated Dummy Implementation
- Updated non-ESP32 dummy class to match new method signatures
- Returns appropriate default values

## How It Works Now

### Starting Incubation:
1. User selects bird species and clicks "Start Incubation"
2. Web server calls `incubationTracker.startIncubation()`
3. Gets species-specific preset values:
   - Target temperature (e.g., 37.5°C for chickens)
   - Turn interval (e.g., 4 hours for chickens)
4. Updates global control variables:
   - `targetTemp` = preset temperature
   - `EGGS_TURNING_INTERVAL` = preset interval
5. Resets egg turner timer to use new interval
6. Saves values to non-volatile storage
7. Temperature control and egg turning immediately use new values

### Stopping Incubation:
1. User clicks "Stop Incubation"
2. Web server calls `incubationTracker.stopIncubation()`
3. Gets default values (38.0°C, 8 hours)
4. Restores global variables to defaults
5. Resets egg turner timer
6. Saves defaults to storage

### Manual Configuration Changes:
1. User changes temperature or interval in Configuration card
2. `updateConfig()` is called
3. Updates global variables
4. Resets egg turner timer for interval changes
5. Saves to storage

## Files Modified

### `incubation_tracker.h`
- Updated method signatures for `startIncubation()` and `stopIncubation()`
- Added output parameters for temperature and interval

### `incubation_tracker.cpp`
- Implemented output parameter handling
- Returns preset/default values through pointers

### `turner.h`
- Added `resetTimer()` method declaration

### `turner.cpp`
- Implemented `resetTimer()` method
- Resets `eggsTurnCounter` to current `EGGS_TURNING_INTERVAL`

### `web_server.cpp`
- Updated `executeCommand()` for `start_incubation` and `stop_incubation`
- Added `resetTimer()` calls to apply changes immediately
- Updated `updateConfig()` to reset timer on interval changes
- Enhanced logging for debugging

## Testing
The system should now:
1. Apply correct temperature when starting incubation for any bird species
2. Apply correct turn interval when starting incubation
3. Reset to defaults when stopping incubation
4. Immediately apply interval changes (no waiting for current countdown)
5. Persist settings across power cycles
6. Work with both manual configuration and incubation presets

## Preset Values Applied
Each bird species preset includes:
- **Target Temperature**: Optimal incubation temperature
- **Turn Interval**: Recommended turning frequency
- **Incubation Days**: Total days until hatching
- **Candling Day**: Day for egg inspection
- **Lockdown Day**: Day to stop turning eggs

Example for Chicken:
- Temperature: 37.5°C
- Interval: 4 hours (14,400 seconds)
- Days: 21
- Candling: Day 7
- Lockdown: Day 18