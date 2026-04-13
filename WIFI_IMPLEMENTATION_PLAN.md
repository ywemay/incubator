# WiFi Implementation Plan for ESP32 Incubator

## Step-by-Step Implementation Guide

### Step 1: Update Dependencies

Add these libraries to your `platformio.ini` or Arduino Library Manager:

```ini
[env:esp32-c3]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
lib_deps = 
    bblanchon/ArduinoJson@^6.21.3
    ottowinter/ESPAsyncWebServer-esphome@^3.1.0
    me-no-dev/AsyncTCP-esphome@^1.2.2
    adafruit/Adafruit GFX Library@^1.11.9
    adafruit/Adafruit SSD1306@^2.5.10
    bodmer/TFT_eSPI@^2.5.0
    knolleary/PubSubClient@^2.8
```

### Step 2: Create Network Manager Module

Create `network_manager.h`:

```cpp
#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <time.h>
#include "config.h"

class NetworkManager {
private:
    AsyncWebServer* server;
    bool wifiConnected;
    unsigned long lastReconnectAttempt;
    
public:
    NetworkManager();
    void begin();
    void loop();
    bool isConnected();
    String getIPAddress();
    
    // Web server handlers
    void handleRoot(AsyncWebServerRequest *request);
    void handleStatus(AsyncWebServerRequest *request);
    void handleConfig(AsyncWebServerRequest *request);
    void handleCommand(AsyncWebServerRequest *request);
    
private:
    void connectWiFi();
    void setupWebServer();
    void setupNTP();
    String getSystemStatusJSON();
};

extern NetworkManager networkManager;

#endif
```

Create `network_manager.cpp`:

```cpp
#include "network_manager.h"
#include "thermo.h"
#include "turner.h"
#include "feedback.h"

extern Thermo thermo;
extern Turner turner;
extern Feedback feedback;

NetworkManager networkManager;

NetworkManager::NetworkManager() : server(nullptr), wifiConnected(false), lastReconnectAttempt(0) {}

void NetworkManager::begin() {
    connectWiFi();
    if (wifiConnected) {
        setupWebServer();
        setupNTP();
    }
}

void NetworkManager::connectWiFi() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(WIFI_SID, WIFI_PASS);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi connection failed");
        wifiConnected = false;
    }
}

void NetworkManager::setupWebServer() {
    server = new AsyncWebServer(80);
    
    // Serve static files from SPIFFS (optional)
    // server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
    
    // API endpoints
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleRoot(request);
    });
    
    server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleStatus(request);
    });
    
    server->on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleConfig(request);
    });
    
    server->on("/api/command", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleCommand(request);
    });
    
    // WebSocket for real-time updates
    // AsyncWebSocket ws("/ws");
    // server->addHandler(&ws);
    
    server->begin();
    Serial.println("HTTP server started");
}

void NetworkManager::setupNTP() {
    configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.println(&timeinfo, "Time synchronized: %A, %B %d %Y %H:%M:%S");
}

void NetworkManager::handleRoot(AsyncWebServerRequest *request) {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Incubator Controller</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .card { border: 1px solid #ddd; padding: 15px; margin: 10px 0; border-radius: 5px; }
        .temp { color: #e74c3c; font-size: 24px; font-weight: bold; }
        .humidity { color: #3498db; font-size: 24px; font-weight: bold; }
        .status { padding: 5px 10px; border-radius: 3px; }
        .online { background: #2ecc71; color: white; }
        .offline { background: #e74c3c; color: white; }
    </style>
</head>
<body>
    <h1>ESP32 Incubator Controller</h1>
    
    <div class="card">
        <h2>Current Status</h2>
        <div id="status">Loading...</div>
    </div>
    
    <div class="card">
        <h2>Controls</h2>
        <button onclick="manualTurn()">Turn Eggs Now</button>
        <button onclick="restartSystem()">Restart System</button>
    </div>
    
    <div class="card">
        <h2>Configuration</h2>
        <label>Target Temperature: </label>
        <input type="number" id="targetTemp" step="0.1" min="35" max="42">
        <button onclick="updateConfig()">Update</button>
    </div>
    
    <script>
        function updateStatus() {
            fetch('/api/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('status').innerHTML = `
                        <p>Temperature: <span class="temp">${data.temperature.toFixed(1)}°C</span></p>
                        <p>Humidity: <span class="humidity">${data.humidity.toFixed(1)}%</span></p>
                        <p>Target: ${data.target_temp}°C</p>
                        <p>Next turn in: ${Math.floor(data.next_turn_seconds / 3600)}h ${Math.floor((data.next_turn_seconds % 3600) / 60)}m</p>
                        <p>Heater: <span class="status ${data.heater_on ? 'online' : 'offline'}">${data.heater_on ? 'ON' : 'OFF'}</span></p>
                        <p>Fan: <span class="status ${data.fan_on ? 'online' : 'offline'}">${data.fan_on ? 'ON' : 'OFF'}</span></p>
                    `;
                });
        }
        
        function manualTurn() {
            fetch('/api/command', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({command: 'turn_now'})
            });
        }
        
        function updateConfig() {
            const temp = document.getElementById('targetTemp').value;
            fetch('/api/config', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({target_temp: parseFloat(temp)})
            });
        }
        
        function restartSystem() {
            fetch('/api/command', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({command: 'restart'})
            });
        }
        
        // Update every 5 seconds
        setInterval(updateStatus, 5000);
        updateStatus();
    </script>
</body>
</html>
)rawliteral";
    
    request->send(200, "text/html", html);
}

void NetworkManager::handleStatus(AsyncWebServerRequest *request) {
    String json = getSystemStatusJSON();
    request->send(200, "application/json", json);
}

void NetworkManager::handleConfig(AsyncWebServerRequest *request) {
    if (request->method() == HTTP_POST) {
        // Parse JSON configuration
        // Update system settings
        request->send(200, "application/json", "{\"status\":\"updated\"}");
    } else {
        // Return current configuration
        String json = "{";
        json += "\"target_temp\":" + String(targetTemp) + ",";
        json += "\"turn_interval\":" + String(EGGS_TURNING_INTERVAL);
        json += "}";
        request->send(200, "application/json", json);
    }
}

void NetworkManager::handleCommand(AsyncWebServerRequest *request) {
    if (request->method() == HTTP_POST) {
        // Parse command and execute
        // Example: {"command": "turn_now"}
        request->send(200, "application/json", "{\"status\":\"command_received\"}");
    }
}

String NetworkManager::getSystemStatusJSON() {
    StaticJsonDocument<256> doc;
    doc["temperature"] = thermo.temperature();
    doc["humidity"] = thermo.humidity();
    doc["target_temp"] = targetTemp;
    doc["next_turn_seconds"] = turner.remained();
    doc["heater_on"] = digitalRead(HEATER_PIN);
    doc["fan_on"] = thermo.fanOn;
    doc["wifi_connected"] = wifiConnected;
    doc["ip_address"] = WiFi.localIP().toString();
    
    String json;
    serializeJson(doc, json);
    return json;
}

void NetworkManager::loop() {
    // Handle WiFi reconnection
    if (!wifiConnected && millis() - lastReconnectAttempt > 30000) {
        lastReconnectAttempt = millis();
        connectWiFi();
    }
    
    // Handle WebSocket updates if implemented
}

bool NetworkManager::isConnected() {
    return wifiConnected;
}

String NetworkManager::getIPAddress() {
    return WiFi.localIP().toString();
}
```

### Step 3: Update Main Program

Update `incubator_ESP32C_roll_turn.ino`:

```cpp
#include "beeper.h"
#include "feedback.h"
#include "thermo.h"
#include "turner.h"
#include "network_manager.h"

Thermo thermo;
Turner turner;
Feedback feedback;
NetworkManager networkManager;

void setup() {
  Serial.begin(115200);
  
  // Initialize hardware
  feedback.setup();
  if(thermo.setup()) {
    feedback.sensor_ok();
  } else {
    feedback.sensor_setup_fail();
  }
  turner.setup();  
  feedback.setup_ok();
  
  // Initialize network (non-blocking)
  networkManager.begin();
  
  Serial.println("System initialized");
}

int8_t state;
uint8_t errorsCount = 0;
uint8_t restoreTries = 0;

void handleErrors() {
  if (state != 0) {
    errorsCount++;
    if (errorsCount > 2) {
      if (state == -5) {
        if (restoreTries > 2) {
          feedback.restarting();
          // ESP.restart();
        } 
        feedback.restoreSensor();
        thermo.sensorBegin();
        delay(1000);
        restoreTries++;
      }
      feedback.error();
    }
  } else {
    errorsCount = 0;
    restoreTries = 0;
  }
}

void loop() {
  // Handle temperature control
  state = thermo.adjust();
  handleErrors();

  if (state != 0) {
    feedback.display_sensor(state);
  } else {
    unsigned int turning = turner.process();
    feedback.stats(
      thermo.temperature(),
      thermo.humidity(),
      turning,
      turner.remained(),
      state
    );
  }
  
  // Handle network operations
  networkManager.loop();
  
  delay(1000);
}
```

### Step 4: Create Configuration Storage

Create `config_storage.h`:

```cpp
#ifndef CONFIG_STORAGE_H
#define CONFIG_STORAGE_H

#include <Preferences.h>

class ConfigStorage {
private:
    Preferences preferences;
    
public:
    void begin();
    void saveTargetTemperature(float temp);
    float loadTargetTemperature();
    void saveTurnInterval(unsigned int interval);
    unsigned int loadTurnInterval();
    void saveWiFiCredentials(const char* ssid, const char* password);
    void loadWiFiCredentials(char* ssid, char* password, size_t maxLen);
};

#endif
```

Create `config_storage.cpp`:

```cpp
#include "config_storage.h"
#include "config.h"

void ConfigStorage::begin() {
    preferences.begin("incubator", false);
}

void ConfigStorage::saveTargetTemperature(float temp) {
    preferences.putFloat("target_temp", temp);
}

float ConfigStorage::loadTargetTemperature() {
    return preferences.getFloat("target_temp", targetTemp);
}

void ConfigStorage::saveTurnInterval(unsigned int interval) {
    preferences.putUInt("turn_interval", interval);
}

unsigned int ConfigStorage::loadTurnInterval() {
    return preferences.getUInt("turn_interval", EGGS_TURNING_INTERVAL);
}

void ConfigStorage::saveWiFiCredentials(const char* ssid, const char* password) {
    preferences.putString("wifi_ssid", ssid);
    preferences.putString("wifi_pass", password);
}

void ConfigStorage::loadWiFiCredentials(char* ssid, char* password, size_t maxLen) {
    String savedSSID = preferences.getString("wifi_ssid", "");
    String savedPass = preferences.getString("wifi_pass", "");
    
    if (savedSSID.length() > 0) {
        strncpy(ssid, savedSSID.c_str(), maxLen);
    }
    if (savedPass.length() > 0) {
        strncpy(password, savedPass.c_str(), maxLen);
    }
}
```

### Step 5: Update Configuration System

Update `config.h` to support dynamic configuration:

```cpp
#ifndef __CONFIG_H__
#define __CONFIG_H__

// use or not Oled screen
#define OLED_ON
// LED based feedback
// #define LEDS_ON

// Default target temperature, celsius
extern float targetTemp;  // Now extern, defined in main .ino file

// comment/uncomment to enable/disable the usage of NTC type sersor 
// #define NTC_SENSOR_PIN A3
//#define NTC_SENSOR_POWER_PIN A4

// NTC related
#ifdef NTC_SENSOR_PIN
#define nominal_resistance 10900       //Nominal resistance at 25⁰C
#define nominal_temeprature 25   // temperature for nominal 
#define beta 4000  // The beta coefficient or the B value of the thermistor (usually 3000-4000) check the datasheet for the accurate value.
#define Rref 9890   //Value of  resistor used for the voltage divider
#endif

// --- Pin constants definition --

#ifdef LEDS_ON
#define LED_COLD A0
#define LED_OK A1
#define LED_HOT A2
#endif

#if defined(ARDUINO_AVR_UNO)
  #define FAN_PIN 9
  #define LIGHT_PIN 10
#else
  #define FAN_PIN 20
  #define LIGHT_PIN 21
#endif

#define AM2302_SENSOR_PIN 5
#define HEATER_PIN 6
#define EGGS_TURNER_PIN 10
// #define EGGS_TURNER_SERVO_PIN 21
#define BEEPER_PIN 7

#ifdef EGGS_TURNER_PIN
extern unsigned int EGGS_TURN_SECONDS;  // Now extern
#endif

#ifdef EGGS_TURNER_SERVO_PIN
extern unsigned int EGGS_TURN_SERVER_STEPS;  // Now extern
extern unsigned int EGGS_TURN_SECONDS;  // Now extern
#endif

extern unsigned int EGGS_TURNING_INTERVAL;  // Now extern

// WiFi configuration (can be overridden by stored credentials)
#ifndef WIFI_SID
#define WIFI_SID "YOUR-WIFI"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "12345678"
#endif

#endif
```

### Step 6: Create WiFi Configuration Portal

Create `wifi_portal.h`:

```cpp
#ifndef WIFI_PORTAL_H
#define WIFI_PORTAL_H

#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

class WiFiPortal {
private:
    DNSServer dnsServer;
    WebServer webServer;
    bool portalActive;
    
public:
    WiFiPortal();
    void begin();
    void loop();
    bool isPortalActive();
    
private:
    void handleRoot();
    void handleSave();
    void handleScan();
    String generateNetworkList();
};

#endif
```

### Step 7: Testing Procedure

1. **Basic WiFi Test**:
   - Upload code with basic WiFi connection
   - Check serial monitor for connection status
   - Verify IP address assignment

2. **Web Interface Test**:
   - Connect to IP address in browser
   - Verify status page loads
   - Check API endpoints return JSON

3. **Configuration Test**:
   - Test configuration updates via web interface
   - Verify settings persist after restart
   - Test manual egg turn command

4. **Long-term Stability**:
   - Run for 24+ hours
   - Monitor memory usage
   - Check for WiFi disconnections/reconnections

### Step 8: Deployment Checklist

- [ ] Update all library dependencies
- [ ] Test