#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>

// Conditional compilation for ESP32 only
#ifdef ESP32
#include <WebServer.h>
#include <ArduinoJson.h>
#include <time.h>
#include "config.h"
#include "thermo.h"
#include "turner.h"
#include "feedback.h"
#include "incubation_tracker.h"

// Forward declarations
extern Thermo thermo;
extern Turner turner;
extern Feedback feedback;
extern IncubationTracker incubationTracker;

class WebServerManager {
private:
    WebServer* server;
    bool server_active;
    
    // NTP configuration
    const char* ntp_server1 = "pool.ntp.org";
    const char* ntp_server2 = "time.nist.gov";
    const long gmt_offset_sec = 8 * 3600; // GMT+8 for Asia/Shanghai
    const int daylight_offset_sec = 0;
    
public:
    WebServerManager();
    ~WebServerManager();
    
    // Initialization
    void begin();
    void loop();
    
    // Status
    bool isActive() const { return server_active; }
    
    // Time synchronization
    void syncTime();
    String getFormattedTime();
    String getFormattedDate();
    bool isTimeSynced();
    
private:
    // Server setup
    void setupRoutes();
    
    // Route handlers
    void handleRoot();
    void handleAPIStatus();
    void handleAPIConfig();
    void handleAPICommand();
    void handleAPITime();
    void handleSystemInfo();
    void handleIncubationAPI();
    void handleNotFound();
    
    // Helper functions
    String getSystemStatusJSON();
    String getSystemConfigJSON();
    String getSystemInfoJSON();
    String htmlEncode(const String& str);
    
    // Configuration management
    bool updateConfig(const JsonDocument& doc);
    bool executeCommand(const String& command, const JsonDocument& data);
};

#else
// Dummy implementation for non-ESP32 platforms
class WebServerManager {
public:
    WebServerManager() {}
    void begin() { /* No-op */ }
    void loop() { /* No-op */ }
    bool isActive() const { return false; }
    void syncTime() { /* No-op */ }
    String getFormattedTime() { return "N/A"; }
    String getFormattedDate() { return "N/A"; }
    bool isTimeSynced() { return false; }
};

#endif // ESP32

#endif // WEB_SERVER_H