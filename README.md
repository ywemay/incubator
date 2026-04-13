# ESP32-C3 Incubator Controller with Egg Turner

A smart incubator controller based on ESP32-C3 microcontroller with temperature/humidity control and automatic egg turning system.

## Features

- **Temperature Control**: Maintains precise temperature (default: 38°C) using DHT22/AM2302 sensor
- **Humidity Monitoring**: Tracks humidity levels for optimal incubation conditions
- **Automatic Egg Turning**: Turns eggs every 8 hours (configurable) using either:
  - DC motor with relay control
  - Servo motor for precise angular movement
- **Visual Feedback**: OLED display (SSD1306) for real-time monitoring
- **Audible Alerts**: Beeper for system status and alerts
- **LED Indicators**: Optional LED feedback for temperature status
- **Error Handling**: Robust sensor failure detection and recovery

## Hardware Requirements

### Microcontroller
- ESP32-C3 development board

### Sensors
- DHT22/AM2302 temperature and humidity sensor
- Optional NTC thermistor for backup temperature sensing

### Actuators
- Heater element with relay/SSR control
- Cooling fan
- Egg turner: DC motor with relay OR servo motor (MG996R/SG90)

### Display & Indicators
- OLED SSD1306 (128x64, I2C)
- Buzzer/beeper
- Optional status LEDs (cold/ok/hot)

### Power
- 5V/12V power supply for microcontroller and peripherals
- Separate power for heater (AC/DC depending on heater type)

## Pin Configuration

See `config.h` for complete pin assignments:

| Component | Pin | Notes |
|-----------|-----|-------|
| DHT22 Sensor | 5 | AM2302_SENSOR_PIN |
| Heater Relay | 6 | HEATER_PIN |
| Beeper | 7 | BEEPER_PIN |
| Egg Turner Relay | 10 | EGGS_TURNER_PIN |
| Fan Control | 20 | FAN_PIN (ESP32) |
| Light Control | 21 | LIGHT_PIN (ESP32) |
| Optional Servo | 21 | EGGS_TURNER_SERVO_PIN |

## Software Architecture

The project follows a modular C++ design:

### Core Modules

1. **`incubator_ESP32C_roll_turn.ino`** - Main program loop
   - Initializes all components
   - Main control loop with 1-second intervals
   - Error handling and recovery

2. **`thermo.cpp/.h`** - Temperature/Humidity Control
   - Reads DHT22/AM2302 sensor
   - Optional NTC thermistor support
   - PID-like temperature control logic
   - Heater and fan control

3. **`turner.cpp/.h`** - Egg Turning System
   - Manages turning interval (default: 8 hours)
   - Supports both relay-controlled DC motor and servo
   - Tracks remaining time until next turn

4. **`feedback.cpp/.h`** - User Interface
   - OLED display management
   - LED status indicators
   - Beeper alerts

5. **`config.h`** - Configuration
   - Pin assignments
   - Target temperature (38°C)
   - Timing constants
   - Feature toggles (OLED, LEDs)

## Installation & Setup

### Prerequisites
- Arduino IDE or PlatformIO
- ESP32 board support package
- Required libraries:
  - `AM2302-Sensor` (for DHT22)
  - `Adafruit_GFX` and `Adafruit_SSD1306` (for OLED)
  - `ESP32Servo` (if using servo egg turner)

### Configuration Steps

1. **WiFi Setup** (if adding network features):
   - Copy `wifi.loc.h.example` to `wifi.loc.h`
   - Enter your WiFi credentials

2. **Feature Selection** in `config.h`:
   - Enable/disable OLED: `#define OLED_ON`
   - Enable/disable LEDs: `#define LEDS_ON`
   - Choose egg turner type: `EGGS_TURNER_PIN` or `EGGS_TURNER_SERVO_PIN`
   - Set target temperature: `const float targetTemp = 38.0`

3. **Hardware Calibration**:
   - Verify sensor readings with known references
   - Adjust `intermitentHeatFrequency` in `thermo.cpp` for finer temperature control
   - Calibrate servo angles if using servo turner

## Usage

1. **Initialization**:
   - System performs sensor check on startup
   - "Setup OK" displayed on OLED if all components initialize correctly

2. **Normal Operation**:
   - Current temperature and humidity displayed
   - Time remaining until next egg turn shown
   - Heater/fan automatically controlled to maintain target temperature

3. **Error States**:
   - Sensor failures trigger audible alerts
   - Error codes displayed on OLED
   - Automatic recovery attempts for sensor issues

## WiFi & Network Capabilities (Potential Enhancements)

The current project has basic WiFi configuration files but doesn't implement network features. The ESP32-C3's WiFi capabilities can be leveraged for:

### Recommended Network Features to Add:

1. **Web Interface**:
   - Real-time monitoring dashboard
   - Remote configuration changes
   - Historical data graphs

2. **API Endpoints** (REST/WebSocket):
   - `GET /api/status` - Current temperature, humidity, system state
   - `POST /api/config` - Update target temperature, turning interval
   - `GET /api/history` - Temperature/humidity logs

3. **Time Synchronization**:
   - NTP client for accurate timekeeping
   - Scheduled operations based on real time

4. **Remote Notifications**:
   - Email/SMS alerts for temperature deviations
   - Mobile push notifications
   - Telegram/WhatsApp bot integration

5. **Data Logging**:
   - Local SD card storage
   - Cloud upload (Google Sheets, InfluxDB, MQTT)
   - Periodic reports

### Implementation Suggestions:

```cpp
// Example Web Server using AsyncWebServer
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

void setupWiFi() {
  WiFi.begin(WIFI_SID, WIFI_PASS);
  // Add web server routes
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{";
    json += "\"temperature\":" + String(thermo.temperature()) + ",";
    json += "\"humidity\":" + String(thermo.humidity()) + ",";
    json += "\"nextTurn\":" + String(turner.remained());
    json += "}";
    request->send(200, "application/json", json);
  });
  server.begin();
}
```

## Troubleshooting

### Common Issues

1. **Sensor Read Errors**:
   - Check wiring and pull-up resistors for DHT22
   - Verify power supply stability
   - Increase delay between sensor reads

2. **Temperature Oscillations**:
   - Adjust `intermitentHeatFrequency` in `thermo.cpp`
   - Add hysteresis to control logic
   - Consider implementing proper PID control

3. **WiFi Connection Issues** (if implemented):
   - Check `wifi.loc.h` credentials
   - Verify ESP32-C3 WiFi antenna connection
   - Consider adding WiFi manager with captive portal

### Error Codes

| Code | Meaning | Action |
|------|---------|--------|
| -5 | Sensor communication failure | Check wiring, power cycle |
| Other | DHT22 sensor errors | Refer to AM2302-Sensor library docs |

## Safety Considerations

1. **Electrical Safety**:
   - Use proper isolation for AC heater circuits
   - Ensure adequate wire gauge for current loads
   - Add fuses/circuit breakers for protection

2. **Fire Safety**:
   - Never leave unattended for extended periods
   - Implement maximum temperature cutoff
   - Use thermal fuses on heater elements

3. **Data Safety**:
   - Regular backups of configuration
   - Log critical events for diagnostics

## Future Enhancements

1. **Advanced Control**:
   - PID temperature control algorithm
   - Humidity control with mister/ventilation
   - Different temperature profiles for various egg types

2. **Smart Features**:
   - Machine learning for optimal conditions
   - Predictive maintenance alerts
   - Energy usage optimization

3. **Integration**:
   - Home Assistant MQTT integration
   - IFTTT/Webhook support
   - Voice control (Alexa/Google Home)

## License

Open source - modify and distribute as needed.

## Contributing

1. Fork the repository
2. Create feature branch
3. Test thoroughly with hardware
4. Submit pull request with documentation

## Support

For issues and questions:
1. Check wiring and configuration
2. Review error codes in serial monitor
3. Consult ESP32 and sensor documentation