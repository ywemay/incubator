#include "wifi_manager.h"
#include "config.h"

// Only compile for ESP32
#ifdef ESP32

// Default AP credentials
#define AP_SSID "Incubator-Config"
#define AP_PASSWORD "setup12345"
#define CONFIG_PORTAL_TIMEOUT 300000 // 5 minutes

// Device hostname (for network discovery)
// Base name for the device
#define DEVICE_BASE_NAME "incubator-esp32"

WiFiManager::WiFiManager() :
    wifi_connected(false),
    ap_mode_active(false),
    last_connection_attempt(0),
    connection_timeout(30000), // 30 seconds
    config_server(nullptr),
    connection_retries(0),
    device_hostname("")
{
    // Initialize strings
    wifi_ssid[0] = '\0';
    wifi_password[0] = '\0';
}

WiFiManager::~WiFiManager() {
    if (config_server) {
        delete config_server;
        config_server = nullptr;
    }
}

void WiFiManager::begin() {
    Serial.println("[WiFi] Initializing WiFi Manager");
    
    // Generate unique hostname with multiple components
    String hostname_parts = "";
    
    // 1. Get MAC address for device uniqueness
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    String mac_id = mac.substring(mac.length() - 4); // Last 4 chars of MAC
    mac_id.toLowerCase();
    hostname_parts += mac_id;
    
    // 2. Add compilation timestamp for build uniqueness
    // Use __DATE__ and __TIME__ macros (predefined by compiler)
    const char* compile_date = __DATE__;  // Format: "Apr 14 2026"
    const char* compile_time = __TIME__;  // Format: "12:09:30"
    
    // Extract month abbreviation (3 chars)
    char month[4] = {0};
    strncpy(month, compile_date, 3);
    month[3] = '\0';
    
    // Extract day (1 or 2 chars)
    char day[3] = {0};
    // Day starts at position 4
    if (compile_date[4] == ' ') {
        // Single digit day
        day[0] = '0';
        day[1] = compile_date[5];
    } else {
        // Two digit day
        day[0] = compile_date[4];
        day[1] = compile_date[5];
    }
    day[2] = '\0';
    
    // Extract hour and minute from compile time (format: "12:09:30")
    char hour[3] = {compile_time[0], compile_time[1], '\0'};
    char minute[3] = {compile_time[3], compile_time[4], '\0'};
    
    // Create a short timestamp code
    char timestamp_code[13];
    snprintf(timestamp_code, sizeof(timestamp_code), "%s%s%s%s", month, day, hour, minute);
    
    // 3. Combine all parts
    char hostname_buf[64];
    snprintf(hostname_buf, sizeof(hostname_buf), "%s-%s-%s", 
             DEVICE_BASE_NAME, mac_id.c_str(), timestamp_code);
    
    device_hostname = String(hostname_buf);
    
    // Clean up the hostname (convert to lowercase, remove any spaces)
    device_hostname.toLowerCase();
    device_hostname.replace(" ", "");
    
    // Ensure hostname is valid (max 32 chars for WiFi hostname)
    if (device_hostname.length() > 32) {
        device_hostname = device_hostname.substring(0, 32);
    }
    
    // Set hostname for network discovery
    WiFi.setHostname(device_hostname.c_str());
    Serial.printf("[WiFi] Hostname set to: %s\n", device_hostname.c_str());
    Serial.printf("[WiFi] Compiled on: %s at %s\n", compile_date, compile_time);
    
    // Initialize preferences
    preferences.begin("incubator", false);
    
    // Load saved credentials
    loadCredentials();
    
    // Try to connect if credentials exist
    if (hasCredentials()) {
        Serial.println("[WiFi] Saved credentials found, attempting connection");
        connectToWiFi();
    } else {
        Serial.println("[WiFi] No saved credentials, starting config portal");
        startAPMode();
    }
}

void WiFiManager::loop() {
    if (ap_mode_active && config_server) {
        // Handle config portal requests
        config_server->handleClient();
        dns_server.processNextRequest();
        
        // Auto-disable AP mode after timeout if not connected
        if (millis() - last_connection_attempt > CONFIG_PORTAL_TIMEOUT && !wifi_connected) {
            Serial.println("[WiFi] Config portal timeout, restarting");
            stopAPMode();
            delay(1000);
            startAPMode();
        }
    }
    
    // Handle WiFi reconnection
    if (!wifi_connected && !ap_mode_active && hasCredentials()) {
        if (millis() - last_connection_attempt > connection_timeout) {
            connection_retries++;
            Serial.printf("[WiFi] Reconnection attempt %d\n", connection_retries);
            
            if (connection_retries >= 3) {
                Serial.println("[WiFi] Too many failed attempts, starting config portal");
                startAPMode();
            } else {
                connectToWiFi();
            }
        }
    }
    
    // Check WiFi status
    if (wifi_connected && WiFi.status() != WL_CONNECTED) {
        Serial.println("[WiFi] Connection lost");
        wifi_connected = false;
        last_connection_attempt = millis();
    }
}

void WiFiManager::connectToWiFi() {
    Serial.printf("[WiFi] Connecting to: %s\n", wifi_ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_password);
    
    last_connection_attempt = millis();
    unsigned long start = millis();
    
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        delay(500);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifi_connected = true;
        connection_retries = 0;
        Serial.println("\n[WiFi] Connected!");
        Serial.printf("[WiFi] IP address: %s\n", WiFi.localIP().toString().c_str());
        
        // Stop AP mode if it was active
        if (ap_mode_active) {
            stopAPMode();
        }
    } else {
        wifi_connected = false;
        Serial.println("\n[WiFi] Connection failed");
        
        // Start AP mode for configuration
        if (!ap_mode_active) {
            startAPMode();
        }
    }
}

void WiFiManager::startAPMode() {
    Serial.println("[WiFi] Starting configuration portal");
    
    // Stop any existing connection
    WiFi.disconnect(true);
    delay(100);
    
    // Start AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    
    Serial.printf("[WiFi] AP started: %s\n", AP_SSID);
    Serial.printf("[WiFi] AP IP: %s\n", WiFi.softAPIP().toString().c_str());
    
    // Setup DNS server for captive portal
    dns_server.start(53, "*", WiFi.softAPIP());
    
    // Setup web server
    if (config_server) {
        delete config_server;
    }
    config_server = new WebServer(80);
    setupConfigPortal();
    config_server->begin();
    
    ap_mode_active = true;
    last_connection_attempt = millis();
}

void WiFiManager::stopAPMode() {
    if (!ap_mode_active) return;
    
    Serial.println("[WiFi] Stopping configuration portal");
    
    if (config_server) {
        config_server->stop();
        delete config_server;
        config_server = nullptr;
    }
    
    dns_server.stop();
    WiFi.softAPdisconnect(true);
    
    ap_mode_active = false;
}

void WiFiManager::setupConfigPortal() {
    if (!config_server) return;
    
    config_server->on("/", std::bind(&WiFiManager::handleRoot, this));
    config_server->on("/save", std::bind(&WiFiManager::handleSave, this));
    config_server->on("/status", std::bind(&WiFiManager::handleStatus, this));
    config_server->on("/reset", std::bind(&WiFiManager::handleReset, this));
    config_server->on("/scan", std::bind(&WiFiManager::handleScan, this));
    config_server->onNotFound(std::bind(&WiFiManager::handleNotFound, this));
}

void WiFiManager::handleRoot() {
    if (!config_server) return;
    
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Incubator WiFi Configuration</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
        .container { max-width: 500px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #333; text-align: center; }
        .form-group { margin-bottom: 15px; }
        label { display: block; margin-bottom: 5px; font-weight: bold; }
        input[type="text"], input[type="password"] { width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }
        button { background: #4CAF50; color: white; border: none; padding: 12px 20px; border-radius: 4px; cursor: pointer; width: 100%; font-size: 16px; }
        button:hover { background: #45a049; }
        .status { padding: 10px; border-radius: 4px; margin-bottom: 15px; }
        .success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .info { background: #d1ecf1; color: #0c5460; border: 1px solid #bee5eb; }
        .scan-btn { background: #007bff; margin-bottom: 15px; }
        .scan-btn:hover { background: #0069d9; }
        .network-list { margin-top: 10px; }
        .network-item { padding: 8px; border: 1px solid #ddd; margin-bottom: 5px; border-radius: 4px; cursor: pointer; }
        .network-item:hover { background: #f0f0f0; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Incubator WiFi Setup</h1>
        
        <div id="status" class="status info">
            Connect to your WiFi network to enable remote monitoring.
        </div>
        
        <div class="form-group">
            <button class="scan-btn" onclick="scanNetworks()">Scan for Networks</button>
            <div id="networkList" class="network-list"></div>
        </div>
        
        <form id="wifiForm" onsubmit="return saveConfig()">
            <div class="form-group">
                <label for="ssid">WiFi SSID:</label>
                <input type="text" id="ssid" name="ssid" required>
            </div>
            
            <div class="form-group">
                <label for="password">WiFi Password:</label>
                <input type="password" id="password" name="password">
            </div>
            
            <button type="submit">Save & Connect</button>
        </form>
        
        <div style="margin-top: 20px; text-align: center;">
            <button onclick="resetConfig()" style="background: #dc3545;">Reset to Factory</button>
        </div>
    </div>
    
    <script>
        function scanNetworks() {
            fetch('/scan')
                .then(response => response.json())
                .then(data => {
                    const list = document.getElementById('networkList');
                    list.innerHTML = '';
                    
                    if (data.networks && data.networks.length > 0) {
                        data.networks.forEach(network => {
                            const div = document.createElement('div');
                            div.className = 'network-item';
                            div.textContent = network.ssid + ' (' + network.rssi + ' dBm)';
                            div.onclick = function() {
                                document.getElementById('ssid').value = network.ssid;
                            };
                            list.appendChild(div);
                        });
                    } else {
                        list.innerHTML = '<div class="status error">No networks found</div>';
                    }
                })
                .catch(error => {
                    console.error('Error:', error);
                });
        }
        
        function saveConfig() {
            const ssid = document.getElementById('ssid').value;
            const password = document.getElementById('password').value;
            
            const formData = new FormData();
            formData.append('ssid', ssid);
            formData.append('password', password);
            
            fetch('/save', {
                method: 'POST',
                body: formData
            })
            .then(response => response.text())
            .then(data => {
                document.getElementById('status').className = 'status success';
                document.getElementById('status').innerHTML = 'Configuration saved! Connecting to WiFi...<br>This page will close in 10 seconds.';
                
                // Redirect to status page after delay
                setTimeout(() => {
                    window.location.href = '/status';
                }, 10000);
            })
            .catch(error => {
                document.getElementById('status').className = 'status error';
                document.getElementById('status').innerHTML = 'Error saving configuration';
                console.error('Error:', error);
            });
            
            return false;
        }
        
        function resetConfig() {
            if (confirm('Are you sure? This will erase all saved WiFi credentials.')) {
                fetch('/reset')
                    .then(() => {
                        document.getElementById('status').className = 'status info';
                        document.getElementById('status').innerHTML = 'Configuration reset. Please refresh the page.';
                        setTimeout(() => location.reload(), 2000);
                    });
            }
        }
        
        // Auto-scan on page load
        window.onload = scanNetworks;
    </script>
</body>
</html>
)rawliteral";
    
    config_server->send(200, "text/html", html);
}

void WiFiManager::handleSave() {
    if (!config_server) return;
    
    if (config_server->hasArg("ssid")) {
        String new_ssid = config_server->arg("ssid");
        String new_password = config_server->hasArg("password") ? config_server->arg("password") : "";
        
        // Validate SSID length
        if (new_ssid.length() > 0 && new_ssid.length() <= 32) {
            strncpy(wifi_ssid, new_ssid.c_str(), sizeof(wifi_ssid) - 1);
            wifi_ssid[sizeof(wifi_ssid) - 1] = '\0';
            
            strncpy(wifi_password, new_password.c_str(), sizeof(wifi_password) - 1);
            wifi_password[sizeof(wifi_password) - 1] = '\0';
            
            saveCredentials();
            
            config_server->send(200, "text/plain", "Configuration saved. Attempting to connect...");
            
            // Try to connect
            delay(1000);
            connectToWiFi();
        } else {
            config_server->send(400, "text/plain", "Invalid SSID");
        }
    } else {
        config_server->send(400, "text/plain", "Missing SSID");
    }
}

void WiFiManager::handleStatus() {
    if (!config_server) return;
    
    String json = "{";
    json += "\"wifi_connected\":" + String(wifi_connected ? "true" : "false") + ",";
    json += "\"ap_mode_active\":" + String(ap_mode_active ? "true" : "false") + ",";
    json += "\"ip_address\":\"" + getIPAddress() + "\",";
    json += "\"ssid\":\"" + htmlEncode(getSSID()) + "\"";
    json += "}";
    
    config_server->send(200, "application/json", json);
}

void WiFiManager::handleReset() {
    if (!config_server) return;
    
    clearCredentials();
    config_server->send(200, "text/plain", "Configuration reset. Restarting config portal...");
    
    delay(1000);
    stopAPMode();
    startAPMode();
}

void WiFiManager::handleScan() {
    if (!config_server) return;
    
    String networks = scanNetworks();
    config_server->send(200, "application/json", networks);
}

void WiFiManager::handleNotFound() {
    if (!config_server) return;
    
    // Redirect to root for captive portal
    config_server->sendHeader("Location", "http://" + WiFi.softAPIP().toString(), true);
    config_server->send(302, "text/plain", "");
}

void WiFiManager::loadCredentials() {
    wifi_ssid[0] = '\0';
    wifi_password[0] = '\0';
    
    String saved_ssid = preferences.getString("wifi_ssid", "");
    String saved_password = preferences.getString("wifi_password", "");
    
    if (saved_ssid.length() > 0) {
        strncpy(wifi_ssid, saved_ssid.c_str(), sizeof(wifi_ssid) - 1);
        wifi_ssid[sizeof(wifi_ssid) - 1] = '\0';
        
        if (saved_password.length() > 0) {
            strncpy(wifi_password, saved_password.c_str(), sizeof(wifi_password) - 1);
            wifi_password[sizeof(wifi_password) - 1] = '\0';
        }
        
        Serial.printf("[WiFi] Loaded credentials for: %s\n", wifi_ssid);
    }
}

void WiFiManager::saveCredentials() {
    preferences.putString("wifi_ssid", wifi_ssid);
    preferences.putString("wifi_password", wifi_password);
    preferences.end(); // Close to ensure write
    
    Serial.printf("[WiFi] Saved credentials for: %s\n", wifi_ssid);
}

void WiFiManager::clearCredentials() {
    preferences.clear();
    preferences.end();
    
    wifi_ssid[0] = '\0';
    wifi_password[0] = '\0';
    
    Serial.println("[WiFi] Credentials cleared");
}

String WiFiManager::scanNetworks() {
    Serial.println("[WiFi] Scanning networks...");
    
    int n = WiFi.scanNetworks();
    String json = "{\"networks\":[";
    
    for (int i = 0; i < n; ++i) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"ssid\":\"" + htmlEncode(WiFi.SSID(i)) + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
        json += "\"encryption\":" + String((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "false" : "true");
        json += "}";
    }
    
    json += "]}";
    WiFi.scanDelete();
    
    return json;
}

String WiFiManager::getIPAddress() const {
    if (ap_mode_active) {
        return WiFi.softAPIP().toString();
    } else if (wifi_connected) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

String WiFiManager::getHostname() const {
    // Return the generated hostname
    return device_hostname;
}

bool WiFiManager::hasCredentials() const {
    return wifi_ssid[0] != '\0';
}

String WiFiManager::htmlEncode(const String& str) {
    String encoded = str;
    encoded.replace("&", "&amp;");
    encoded.replace("\"", "&quot;");
    encoded.replace("'", "&#39;");
    encoded.replace("<", "&lt;");
    encoded.replace(">", "&gt;");
    return encoded;
}

#endif // ESP32