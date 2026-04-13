# Additional Features Implemented

## 1. Adjustable Incubation Days

### Problem
When eggs are partially incubated in another incubator and transferred, the incubation timer needs to be adjusted to reflect the actual incubation progress.

### Solution
Added two methods to adjust incubation days:

#### `adjustIncubationDays(int days_adjustment)`
- Adjusts the incubation timer by the specified number of days
- Positive values = move forward in time (eggs are more developed)
- Negative values = move backward in time (eggs are less developed)
- Automatically bounds the adjustment to valid range (0 to total incubation days)

#### `setIncubationDay(unsigned int day_number)`
- Sets the incubation to a specific day number
- Useful when you know exactly what day the eggs are on
- Automatically bounds to valid range

### Web Interface Controls
Added to the Incubation Tracking card:
- **Adjust Days**: Input field + button to adjust by ± days
- **Set Day**: Input field + button to set to specific day

### API Endpoints
- `POST /api/incubation` with `action: "adjust_days"` and `days: <number>`
- `POST /api/incubation` with `action: "set_day"` and `day: <number>`

### Use Cases
1. **Transfer from another incubator**: Set eggs to day 7 if they've been incubating for 7 days elsewhere
2. **Timer correction**: Adjust by -1 day if you realize the timer started too early
3. **Partial incubation**: Start at day 10 if eggs were incubated elsewhere for 10 days

## 2. OLED Display Cycling

### Problem
The OLED display only showed remaining time until next egg turn, but users also want to see the IP address without navigating menus.

### Solution
Implemented automatic cycling display on the bottom line of the OLED:

#### How It Works
1. **Cycle Interval**: Switches every 5 seconds (configurable)
2. **Display Modes**:
   - **Mode 1**: Shows remaining time until next egg turn (e.g., "3h 25m")
   - **Mode 2**: Shows IP address when connected to WiFi (e.g., "192.168.1.100")
3. **Automatic Detection**: Only cycles to IP address when WiFi is connected
4. **Memory**: Stores IP address when WiFi connects, clears when disconnected

#### Technical Implementation
- Modified `Feedback::stats()` to track time and switch display modes
- Added `Feedback::setIPAddress()` to store current IP
- Modified `Oled::stats()` to accept alternate display text
- Updated main loop to store IP when WiFi connects/disconnects

#### User Experience
- Normal operation: Shows temperature, humidity, and remaining time
- Every 5 seconds: Briefly shows IP address (if connected to WiFi)
- No interaction required - automatic and non-intrusive

## 3. Network Hostname for Discovery

### Problem
When scanning the local network with tools like `nmap`, it's hard to identify which device is the incubator among many IoT devices.

### Solution
Set a unique hostname for the ESP32 incubator:

#### Hostname: `incubator-esp32`

#### Implementation
1. **WiFi Configuration**: Set hostname using `WiFi.setHostname()` during initialization
2. **Network Discovery**: Device appears as `incubator-esp32` in:
   - `nmap` scans
   - Router device lists
   - mDNS/Bonjour (if enabled)
   - Network discovery tools
3. **Web Interface**: Hostname displayed prominently in System Information card
4. **API**: Hostname included in `/api/status` and `/api/system` responses

#### Benefits
1. **Easy Identification**: Quickly find the incubator in network scans
2. **Consistent Naming**: Always appears as `incubator-esp32` regardless of IP address
3. **Professional**: Looks like a proper IoT device rather than generic ESP32
4. **Troubleshooting**: Easier to identify when multiple ESP32 devices on network

#### Network Commands
Users can now:
```bash
# Scan for device
nmap -sn 192.168.1.0/24 | grep incubator

# Ping by hostname (if mDNS works)
ping incubator-esp32.local

# Connect by hostname in browser
http://incubator-esp32.local
```

## Files Modified

### `incubation_tracker.h/cpp`
- Added `adjustIncubationDays()` and `setIncubationDay()` methods
- Updated dummy implementation

### `feedback.h/cpp`
- Added `setIPAddress()` and `getIPAddress()` methods
- Modified `stats()` to cycle display every 5 seconds
- Added display state tracking variables

### `oled.h/cpp`
- Modified `stats()` to accept alternate display text parameter
- Added logic to display IP address on bottom line

### `wifi_manager.h/cpp`
- Added `getHostname()` method
- Set hostname during WiFi initialization
- Updated dummy implementation

### `web_server.cpp`
- Added API endpoints for day adjustment
- Added web interface controls for day adjustment
- Updated HTML to show hostname
- Updated JSON responses to include hostname

### `incubator_ESP32C_roll_turn.ino`
- Updated to store IP address when WiFi connects
- Clears IP address when WiFi disconnects

## Testing Instructions

### 1. Day Adjustment Test
1. Start incubation for Chicken (21 days)
2. Use "Set Day" to set to day 10
3. Verify display shows "Day: 10 of 21"
4. Use "Adjust Days" with +2
5. Verify display shows "Day: 12 of 21"

### 2. OLED Cycling Test
1. Connect to WiFi
2. Observe OLED display
3. Wait 5 seconds - should show IP address
4. Wait 5 more seconds - should return to time display
5. Disconnect WiFi - should only show time

### 3. Hostname Test
1. Connect incubator to network
2. Run `nmap -sn <your-network>` 
3. Look for `incubator-esp32` in results
4. Check router device list for hostname
5. Verify web interface shows correct hostname

## Notes
- All features are conditionally compiled for ESP32 only
- Arduino builds use dummy implementations
- Backward compatibility maintained
- Memory usage optimized for ESP32-C3 constraints