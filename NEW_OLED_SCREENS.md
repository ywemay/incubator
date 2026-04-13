# New OLED Screens: Bird Species & Candling Schedule

## Overview
Added two new screens to the OLED display cycle (now 8 screens total):
1. **Screen 7: Bird Species** - Shows what bird is being incubated
2. **Screen 8: Candling Schedule** - Shows when to candle the eggs

## Screen 7: Bird Species 🐦

### What It Shows:
- **Bird icon** and species name (e.g., "Chicken", "Duck")
- **Current incubation day** (e.g., "Day 7 of 21")
- **Progress percentage** (e.g., "Progress: 33%")

### Layout Examples:

**Active Incubation:**
```
Bird Species
🐦 Chicken
Day 7 of 21
Progress: 33%
Screen 7/8
```

**No Species Selected:**
```
Bird Species
🐦 Not set
Select species
to start
Screen 7/8
```

### Features:
- **Truncates long names**: Automatically shortens long species names
- **Progress calculation**: Shows percentage completion
- **Empty state**: Helpful message when no species selected
- **Bird icon**: Visual indicator for species screen

## Screen 8: Candling Schedule 🕯️

### What It Shows:
- **Candle icon** and candling day (e.g., "Day 7")
- **Days until candling** (e.g., "Candle in: 3 days")
- **Current day** (e.g., "Today: Day 4")
- **Special alerts** for candling day

### Layout Examples:

**Future Candling (Day 4, Candling Day 7):**
```
Candling Schedule
🕯️ Day 7
Candle in: 3 days
Today: Day 4
Screen 8/8
```

**TODAY IS CANDLING DAY! (Day 7):**
```
Candling Schedule
🕯️ Day 7
*** TODAY! ***
Candle eggs now
Screen 8/8
```

**Candling Passed (Day 10, Candling was Day 7):**
```
Candling Schedule
🕯️ Day 7
Candling passed
Day 3 days ago
Screen 8/8
```

**No Candling Scheduled:**
```
Candling Schedule
🕯️ No candling
scheduled
for species
Screen 8/8
```

### Features:
- **Smart timing**: Calculates days until candling
- **Today alert**: Special display when it's candling day
- **Past tracking**: Shows how many days since candling
- **Species-specific**: Only shows if species has candling day
- **Candle icon**: Visual indicator for candling

## New Icons Added

### 1. Bird Icon 🐦 (8x8 pixels)
```
   ##   
  ####  
 ###### 
 ##  ## 
########
###  ###
 ###### 
  ####  
```

### 2. Candle Icon 🕯️ (8x8 pixels)
```
   ##   
  ####  
  ####  
  ####  
 ###### 
 ###### 
  ####  
   ##   
```

## Technical Implementation

### Updated Method Signatures:
- `Oled::displayIncubatorInfo()` now accepts `bird_species` and `candling_day` parameters
- `Feedback::displayIncubatorInfo()` updated to pass through parameters
- Main loop retrieves species name and candling day from `IncubationTracker`

### Data Flow:
1. Main loop gets `bird_species` from `incubationTracker.getSpeciesName()`
2. Main loop gets `candling_day` from `incubationTracker.getCurrentCandlingDay()`
3. Parameters passed to `feedback.displayIncubatorInfo()`
4. OLED system displays appropriate screens based on data

### Screen Cycling:
- **Now 8 screens total** (increased from 6)
- **Still 5-second cycles** per screen
- **Complete cycle**: 40 seconds for all screens
- **Screen numbers updated**: All screens show correct numerator (e.g., "3/8")

## User Benefits

### 1. Quick Species Identification
- Immediately see what bird is being incubated
- No need to remember or check settings
- Visual confirmation of active species

### 2. Candling Reminders
- Never miss candling day
- Countdown to candling
- Clear "TODAY!" alert when it's time
- Track past candling dates

### 3. Comprehensive Information
All 8 screens now provide complete incubator overview:
1. Current status (temp/humidity)
2. Temperature control
3. Egg turner info
4. Incubation progress
5. Network status
6. System summary
7. **Bird species** ← NEW
8. **Candling schedule** ← NEW

### 4. Visual Consistency
- Icons match screen content
- Consistent layout across all screens
- Clear screen numbering
- Professional appearance

## Use Cases

### For New Users:
- Quickly identify what's in the incubator
- Learn when to check egg development
- Understand incubation progress

### For Experienced Users:
- Confirm correct species settings
- Track multiple incubation batches
- Plan candling schedule
- Monitor overall incubator status

### For Shared Incubators:
- Multiple users can see what's incubating
- Clear instructions for candling
- No confusion about settings

## Testing Scenarios

### 1. Species Display Test
- Start incubation for "Chicken"
- Verify screen shows "Chicken" and "Day 7 of 21"
- Start incubation for "Duck"  
- Verify screen updates to "Duck" and "Day 7 of 28"

### 2. Candling Schedule Test
- Set to Chicken (candling day 7)
- On day 4: Verify "Candle in: 3 days"
- On day 7: Verify "*** TODAY! ***" alert
- On day 10: Verify "Candling passed"

### 3. Empty States Test
- No species selected: Verify "Not set" message
- Species with no candling: Verify "No candling scheduled"
- No active incubation: Verify appropriate messages

## Files Modified

### `oled.h`
- Added `bird_icon` and `candle_icon` declarations
- Updated `displayIncubatorInfo()` method signature

### `oled.cpp`
- Implemented `bird_icon` and `candle_icon` (8x8 pixel arrays)
- Added `drawScreen6()` for bird species display
- Added `drawScreen7()` for candling schedule
- Updated screen cycling to 8 screens
- Updated all screen numbers to "x/8"

### `feedback.h/cpp`
- Updated method signatures to include bird species and candling day
- Passes parameters through to OLED system

### `incubator_ESP32C_roll_turn.ino`
- Retrieves bird species name from incubation tracker
- Retrieves candling day from incubation tracker
- Passes both to display system

## Complete Screen Cycle (40 seconds)

1. **Screen 1** (0-5s): Current temperature & humidity
2. **Screen 2** (5-10s): Temperature control (current vs target)
3. **Screen 3** (10-15s): Egg turner information
4. **Screen 4** (15-20s): Incubation progress with progress bar
5. **Screen 5** (20-25s): Network status (WiFi, IP, hostname)
6. **Screen 6** (25-30s): System summary (all statuses)
7. **Screen 7** (30-35s): **Bird species** ← NEW
8. **Screen 8** (35-40s): **Candling schedule** ← NEW

Repeat every 40 seconds for continuous monitoring!