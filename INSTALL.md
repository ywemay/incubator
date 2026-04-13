# Installation Guide

## Quick Start

### Option 1: Arduino IDE
1. Install [Arduino IDE 2.0+](https://www.arduino.cc/en/software)
2. Install ESP32 board support:
   - File → Preferences → Additional Boards Manager URLs
   - Add: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
   - Tools → Board → Boards Manager → Search "esp32" → Install
3. Install required libraries (Sketch → Include Library → Manage Libraries):
   - "DHT sensor library" by Adafruit
   - "Adafruit Unified Sensor" by Adafruit
   - "Adafruit GFX Library" by Adafruit
   - "Adafruit SSD1306" by Adafruit
   - "ArduinoJson" by Benoit Blanchon
4. Open `incubator_ESP32C_roll_turn.ino`
5. Select board: Tools → Board → "ESP32-C3 Dev Module"
6. Upload to your ESP32

### Option 2: PlatformIO (Recommended)
1. Install [VS Code](https://code.visualstudio.com/)
2. Install [PlatformIO extension](https://platformio.org/install/ide?install=vscode)
3. Open project folder in VS Code
4. PlatformIO will automatically install dependencies
5. Click Upload button (→) to compile and upload

## Hardware Setup

### Required Components
- ESP32-C3 development board
- DHT22/AM2302 temperature/humidity sensor
- Heater relay module (SSR or mechanical)
- Cooling fan (12V DC)
- Egg turner (choose one):
  - DC motor with relay
  - Servo motor (MG996R/SG90)
- OLED display SSD1306 (128x64, I2C)
- Buzzer/beeper
- Power supplies:
  - 5V/3.3V for ESP32 and sensors
  - 12V for fan
  - Appropriate voltage for heater

### Wiring Diagram

```
ESP32-C3 Pinout:
┌─────────────────────────────────────┐
│ 3.3V  ────┬─── DHT22 VCC            │
│ GND   ────┼─── DHT22 GND            │
│ GPIO5 ────┴─── DHT22 DATA           │
│                                     │
│ GPIO6 ──────── Heater Relay IN      │
│ GPIO20 ─────── Fan Relay IN         │
│ GPIO10 ─────── Egg Turner Relay IN  │
│ GPIO21 ─────── Servo (optional)     │
│                                     │
│ GPIO7 ──────── Buzzer (+)           │
│ GND   ──────── Buzzer (-)           │
│                                     │
│ SDA   ──────── OLED SDA             │
│ SCL   ──────── OLED SCL             │
│ 3.3V  ──────── OLED VCC             │
│ GND   ──────── OLED GND             │
└─────────────────────────────────────┘
```

### Power Connections
```
Main Power (12V) ────┬─── Fan (+)
                     ├─── Heater (+)
                     └─── DC-DC Converter (12V→5V)
                                 │
                    5V Output ───┴─── ESP32 5V
                                         ├─── DHT22 VCC
                                         ├─── OLED VCC
                                         └─── Relay Module VCC
```

## Configuration

### Pin Configuration (config.h)
Edit `config.h` to match your wiring:

```cpp
// Temperature sensor
#define AM2302_SENSOR_PIN 5

// Actuators
#define HEATER_PIN 6
#define FAN_PIN 20
#define EGGS_TURNER_PIN 10  // For relay control
// OR
// #define EGGS_TURNER_SERVO_PIN 21  // For servo control

// Indicators
#define BEEPER_PIN 7

// Display
#define OLED_ON  // Comment to disable OLED

// Target temperature (default 38.0°C)
const float targetTemp = 38.0;

// Egg turning interval (default 8 hours)
const unsigned int EGGS_TURNING_INTERVAL = 8 * 60 * 60;
```

### WiFi Configuration
No manual configuration needed! The ESP32 will:
1. Start in Access Point mode as "Incubator-Config"
2. You connect to it and configure your WiFi
3. Credentials are saved automatically

Default AP credentials:
- SSID: `Incubator-Config`
- Password: `setup12345`

## First Time Setup

### Step 1: Initial Upload
1. Connect ESP32 via USB
2. Upload the sketch
3. Open Serial Monitor (115200 baud)
4. You should see:
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

### Step 2: Configure WiFi
1. On your phone/laptop, connect to WiFi: `Incubator-Config`
2. Password: `setup12345`
3. Open browser to: `http://192.168.4.1`
4. Select your home WiFi network and enter password
5. Click "Save & Connect"
6. The incubator will connect to your WiFi and display its IP address

### Step 3: Access Web Interface
1. Check Serial Monitor for IP address:
   ```
   [WiFi] Connected!
   [WiFi] IP address: 192.168.1.100
   [Web] Starting web server
   [Web] Server started on http://192.168.1.100
   ```
2. Open browser to that IP address
3. You should see the incubator dashboard

## Calibration

### Temperature Calibration
1. Place a reference thermometer next to DHT22
2. Compare readings on web interface
3. If needed, adjust in code:
   ```cpp
   // Add offset in thermo.cpp
   float Thermo::temperature() {
     float raw = am2302.get_Temperature();
     return raw + 0.5;  // Add 0.5°C offset
   }
   ```

### Egg Turner Calibration
For servo-based turners:
1. Adjust servo angles in `turner.cpp`:
   ```cpp
   // Default: 10° to 180°
   if (currentServoDegrees > 180 || currentServoDegrees < 10) {
     servoDirection = -servoDirection;
   }
   ```

For relay-based turners:
1. Adjust turn duration in `config.h`:
   ```cpp
   const unsigned int EGGS_TURN_SECONDS = 3;  // 3 seconds
   ```

## Testing

### Basic Function Test
1. Set target temperature to room temperature + 2°C
2. Verify heater turns on
3. Monitor temperature rise on web interface
4. Verify heater turns off at target temperature
5. Test egg turner manually via web interface

### Safety Test
1. Disconnect temperature sensor
2. Verify error appears on display/web interface
3. Reconnect sensor
4. Verify automatic recovery

### Network Test
1. Disconnect WiFi router
2. Verify incubator detects disconnection
3. Reconnect router
4. Verify automatic reconnection

## Troubleshooting

### Common Issues

**ESP32 not detected by computer:**
- Install correct USB drivers (CP210x/CH340)
- Try different USB cable (some are power-only)
- Press BOOT button while uploading

**WiFi connection fails:**
- Check router supports 2.4GHz (ESP32 doesn't support 5GHz)
- Move closer to router
- Check WiFi password

**Temperature readings unstable:**
- Add 4.7kΩ pull-up resistor on DHT22 data line
- Add 100nF capacitor between DHT22 VCC and GND
- Keep sensor away from heater/fan

**Web interface not loading:**
- Check IP address in Serial Monitor
- Try different browser
- Disable firewall/antivirus temporarily

**Heater not turning on:**
- Check relay wiring
- Verify HEATER_PIN definition
- Test relay with simple blink sketch

### Serial Monitor Debug Commands
Add to `setup()` for detailed debugging:
```cpp
Serial.setDebugOutput(true);
WiFi.setSleep(false);
```

## Advanced Configuration

### Customizing Web Interface
Edit `web_server.cpp`:
- Change colors in CSS section
- Modify dashboard layout
- Add/remove controls

### Adding Features
1. **Data Logging**: Add SD card module
2. **MQTT**: Add PubSubClient library
3. **OTA Updates**: Enable in Arduino IDE
4. **Multiple Sensors**: Add additional DHT22 sensors

### Security Considerations
1. **Change default AP password** in `wifi_manager.cpp`:
   ```cpp
   #define AP_PASSWORD "your-strong-password"
   ```
2. **Add web authentication** in `web_server.cpp`
3. **Use HTTPS** with SSL certificate
4. **Regular updates** for security patches

## Maintenance

### Regular Checks
- Weekly: Verify temperature calibration
- Monthly: Clean dust from sensors/fan
- Quarterly: Check wiring connections
- Annually: Replace DHT22 sensor (recommended)

### Firmware Updates
1. Backup configuration via web interface
2. Upload new firmware
3. Restore configuration
4. Test all functions

### Data Backup
Configuration is stored in ESP32 flash. To backup:
1. Use web interface to export settings
2. Or note down: target temperature, turn interval, WiFi credentials

## Support

### Getting Help
1. Check [TESTING.md](TESTING.md) for troubleshooting
2. Review Serial Monitor output
3. Search GitHub issues
4. Contact maintainer

### Reporting Issues
Include:
1. ESP32 board model
2. Arduino/PlatformIO version
3. Serial Monitor output
4. Steps to reproduce
5. Photos of wiring

### Contributing
1. Fork repository
2. Create feature branch
3. Test thoroughly
4. Submit pull request

## License
MIT License - see [LICENSE](LICENSE) file

## Acknowledgments
- ESP32 Arduino core developers
- Adafruit for sensor libraries
- PlatformIO team
- All contributors and testers