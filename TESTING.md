# Testing the WiFi-Enabled Incubator

## Prerequisites

### Hardware Requirements
- ESP32-C3 development board
- DHT22/AM2302 temperature/humidity sensor
- Heater relay module
- Egg turner (DC motor with relay OR servo)
- OLED display (SSD1306, optional)
- USB cable for programming

### Software Requirements
- Arduino IDE 2.0+ or PlatformIO
- Required libraries:
  - `AM2302-Sensor` (for DHT22)
  - `Adafruit GFX Library` and `Adafruit SSD1306` (for OLED)
  - `ESP32Servo` (if using servo egg turner)
  - `ArduinoJson` (version 6.x)
  - `WebServer` and `DNSServer` (included with ESP32 core)

## Compilation Test

### Step 1: Basic Compilation (No WiFi)
1. Open the project in Arduino IDE
2. Select a non-ESP32 board (e.g., Arduino Uno)
3. Verify compilation:
   - Should compile successfully
   - WiFi-related code should be excluded via `#ifdef ESP32`
   - Serial output should show basic incubator functionality

### Step 2: ESP32 Compilation
1. Select ESP32-C3 board in Arduino IDE:
   - Board: "ESP32-C3 Dev Module"
   - Upload Speed: "921600"
   - Flash Mode: "DIO"
   - Flash Frequency: "80MHz"
   - Partition Scheme: "Default 4MB with spiffs"
2. Verify compilation:
   - Should include all WiFi and web server code
   - No compilation errors
   - Serial output should show WiFi initialization

## Hardware Testing

### Step 1: Basic Functionality Test
1. Upload to ESP32-C3
2. Open Serial Monitor (115200 baud)
3. Expected output:
   ```
   === ESP32 Incubator Controller ===
   Configuration loaded: Temp=38.0°C, Interval=28800 sec
   [WiFi] Initializing WiFi Manager
   [WiFi] No saved credentials, starting config portal
   [WiFi] Starting configuration portal
   [WiFi] AP started: Incubator-Config
   [WiFi] AP IP: 192.168.4.1
   System initialization complete
   ```

### Step 2: WiFi Configuration Portal Test
1. Using your phone/laptop, scan for WiFi networks
2. Connect to: `Incubator-Config` (password: `setup12345`)
3. Open browser and go to: `http://192.168.4.1`
4. Expected: Configuration portal with network scanner
5. Select your home WiFi network and enter password
6. Click "Save & Connect"
7. Expected: Incubator connects to your WiFi and starts web server

### Step 3: Web Interface Test
1. After successful WiFi connection, check Serial Monitor for IP address:
   ```
   [WiFi] Connected!
   [WiFi] IP address: 192.168.1.100
   [Web] Starting web server
   [Web] Server started on http://192.168.1.100
   ```
2. Open browser and go to the IP address shown
3. Expected: Full dashboard with:
   - Temperature and humidity readings
   - Egg turner status
   - System information
   - Configuration controls

## API Testing

### Basic API Endpoints
Test using curl or browser:

1. **System Status**: `GET http://[IP]/api/status`
   ```json
   {
     "temperature": 25.5,
     "humidity": 45.2,
     "target_temp": 38.0,
     "heater_on": false,
     "fan_on": false,
     "next_turn_seconds": 28765,
     "turn_interval": 28800,
     "turner_active": false,
     "current_time": "14:30:15",
     "current_date": "2026-04-13 Monday",
     "wifi_connected": true,
     "ip_address": "192.168.1.100",
     "wifi_ssid": "Your-WiFi",
     "uptime": "00:05:30"
   }
   ```

2. **Configuration**: `GET http://[IP]/api/config`
   ```json
   {
     "target_temp": 38.0,
     "turn_interval": 28800,
     "wifi_ssid": "Your-WiFi"
   }
   ```

3. **Update Configuration**: `POST http://[IP]/api/config`
   ```json
   {
     "target_temp": 37.5,
     "turn_interval": 21600  // 6 hours
   }
   ```

4. **Send Command**: `POST http://[IP]/api/command`
   ```json
   {
     "command": "turn_now"
   }
   ```

## Functional Testing

### Temperature Control Test
1. Set target temperature to 25°C (room temperature)
2. Verify heater doesn't turn on
3. Set target temperature to 40°C
4. Verify heater turns on (check LED/relay)
5. Monitor temperature rise on web interface
6. Verify heater turns off when target reached

### Egg Turner Test
1. Set turn interval to 1 minute (60 seconds) for testing
2. Monitor "Next Turn In" countdown on web interface
3. Verify egg turner activates when timer reaches 0
4. Test manual turn via "Turn Now" button
5. Verify timer resets after turn

### WiFi Resilience Test
1. Disconnect WiFi router
2. Verify incubator detects disconnection
3. Check Serial Monitor for reconnection attempts
4. Reconnect WiFi router
5. Verify automatic reconnection
6. Web interface should resume automatically

### Configuration Persistence Test
1. Change target temperature via web interface
2. Restart ESP32 (unplug/replug or use restart command)
3. Verify settings are restored after reboot
4. Check Serial Monitor for "Configuration loaded" message

## Security Testing

### WiFi Security
1. Configuration portal uses WPA2 (password: `setup12345`)
2. Portal auto-disables after 5 minutes of inactivity
3. Credentials stored encrypted in ESP32 flash
4. Factory reset clears all credentials

### Web Interface Security
1. No authentication on local network (by design)
2. Consider adding .htaccess if exposed to internet
3. All API endpoints validate input ranges
4. No sensitive data exposed in API responses

## Performance Testing

### Memory Usage
Check Serial Monitor for:
```
[Web] Free heap: 123456 bytes
```
- Should maintain > 50KB free heap during operation
- No memory leaks over 24+ hours

### Network Performance
1. Web page loads in < 3 seconds
2. API responses in < 100ms
3. Concurrent connections: 5+ clients
4. Data updates every 5 seconds (configurable)

### Temperature Stability
1. Maintain temperature within ±0.5°C of target
2. Heater cycling: < 10 cycles per hour at stable temperature
3. Response time: < 30 seconds to adjust to setpoint change

## Troubleshooting

### Common Issues

1. **WiFi Connection Fails**
   - Check credentials in configuration portal
   - Verify router supports 2.4GHz (ESP32 doesn't support 5GHz)
   - Check signal strength (> -70dBm recommended)

2. **Web Interface Not Loading**
   - Verify IP address in Serial Monitor
   - Check firewall/antivirus blocking
   - Try different browser

3. **Temperature Readings Unstable**
   - Check DHT22 wiring (3.3V, GND, Data with pull-up resistor)
   - Ensure sensor not near heater/fan
   - Add 100nF capacitor between VCC and GND

4. **Heater Not Turning On**
   - Check relay wiring
   - Verify HEATER_PIN definition in config.h
   - Test relay with simple blink sketch

5. **Configuration Not Saving**
   - Check flash partition scheme
   - Verify Preferences library initialization
   - Monitor Serial Monitor for storage errors

### Serial Debug Commands
Add to `setup()` for debugging:
```cpp
Serial.setDebugOutput(true);
```

Monitor debug output for:
- WiFi connection state changes
- DNS/HTTP server errors
- Memory allocation issues

## Long-term Testing

### 24-Hour Stability Test
1. Run incubator for 24+ hours continuously
2. Monitor:
   - Temperature stability graph
   - WiFi disconnection/reconnection events
   - Memory usage trends
   - Heater cycling frequency

### Power Cycle Test
1. Perform 10+ power cycles
2. Verify:
   - Configuration persistence
   - WiFi auto-reconnection
   - Web server auto-start
   - Temperature control resumes correctly

### Network Stress Test
1. Connect 5+ devices to web interface
2. Continuously refresh pages for 1 hour
3. Verify:
   - No crashes or freezes
   - Responsive interface
   - Stable temperature control

## Success Criteria

### Must Have
- [ ] Compiles for both ESP32 and Arduino
- [ ] WiFi configuration portal works
- [ ] Web interface displays real-time data
- [ ] Temperature control functions
- [ ] Egg turner operates on schedule
- [ ] Configuration persists after reboot
- [ ] No memory leaks in 24-hour test

### Should Have
- [ ] Responsive web interface (mobile/desktop)
- [ ] API endpoints for automation
- [ ] Time synchronization via NTP
- [ ] Automatic WiFi reconnection
- [ ] Factory reset functionality

### Nice to Have
- [ ] Data logging to SD card
- [ ] MQTT integration
- [ ] OTA updates
- [ ] Multiple temperature profiles
- [ ] Energy usage monitoring

## Next Steps After Testing

1. **Documentation**: Update README with tested configurations
2. **Optimization**: Profile and optimize memory usage
3. **Features**: Add requested features based on testing feedback
4. **Deployment**: Prepare for actual incubation use
5. **Monitoring**: Set up long-term monitoring system

## Support

For issues:
1. Check Serial Monitor output
2. Review wiring diagram
3. Test individual components
4. Search GitHub issues
5. Contact project maintainer