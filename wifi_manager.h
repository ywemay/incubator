#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

// Conditional compilation for ESP32 only
#ifdef ESP32
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

class WiFiManager {
private:
    // Configuration
    char wifi_ssid[33];      // Maximum SSID length is 32 + null terminator
    char wifi_password[65];   // Maximum password length is 64 + null terminator
    
    // Network state
    bool wifi_connected;
    bool ap_mode_active;
    unsigned long last_connection_attempt;
    unsigned long connection_timeout;
    
    // Web server for configuration portal
    WebServer* config_server;
    DNSServer dns_server;
    
    // Preferences for storing credentials
    Preferences preferences;
    
    // Connection retry counter
    uint8_t connection_retries;
    
    // Device hostname
    String device_hostname;
    
public:
    WiFiManager();
    ~WiFiManager();
    
    // Initialization
    void begin();
    void loop();
    
    // Status
    bool isConnected() const { return wifi_connected; }
    bool isAPModeActive() const { return ap_mode_active; }
    String getIPAddress() const;
    String getSSID() const { return String(wifi_ssid); }
    String getHostname() const;
    
    // Configuration
    bool hasCredentials() const;
    void clearCredentials();
    
private:
    // WiFi operations
    void connectToWiFi();
    void startAPMode();
    void stopAPMode();
    bool attemptConnection();
    
    // Configuration portal handlers
    void setupConfigPortal();
    void handleRoot();
    void handleSave();
    void handleStatus();
    void handleReset();
    void handleScan();
    void handleNotFound();
    
    // Storage operations
    void loadCredentials();
    void saveCredentials();
    
    // Network scanning
    String scanNetworks();
    
    // Helper functions
    String htmlEncode(const String& str);
};

#else
// Dummy implementation for non-ESP32 platforms (Arduino)
class WiFiManager {
public:
    WiFiManager() {}
    void begin() { /* No-op */ }
    void loop() { /* No-op */ }
    bool isConnected() const { return false; }
    bool isAPModeActive() const { return false; }
    String getIPAddress() const { return "N/A"; }
    String getSSID() const { return "N/A"; }
    String getHostname() const { 
        // For non-ESP32, return a generic name
        return "incubator-esp32-generic"; 
    }
    bool hasCredentials() const { return false; }
    void clearCredentials() { /* No-op */ }
};

#endif // ESP32

#endif // WIFI_MANAGER_H