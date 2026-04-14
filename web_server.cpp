#include "web_server.h"
#include "wifi_manager.h"
#include "config_storage.h"
#include "password_manager.h"

// Only compile for ESP32
#ifdef ESP32

// External managers
extern WiFiManager wifiManager;
extern ConfigStorage configStorage;
extern PasswordManager passwordManager;

WebServerManager::WebServerManager() : 
    server(nullptr),
    server_active(false)
{
}

WebServerManager::~WebServerManager() {
    if (server) {
        delete server;
        server = nullptr;
    }
}

void WebServerManager::begin() {
    // Only start if WiFi is connected
    if (!wifiManager.isConnected()) {
        Serial.println("[Web] WiFi not connected, skipping web server start");
        return;
    }
    
    Serial.println("[Web] Starting web server");
    
    server = new WebServer(80);
    setupRoutes();
    server->begin();
    
    server_active = true;
    
    // Sync time if connected
    syncTime();
    
    Serial.printf("[Web] Server started on http://%s\n", wifiManager.getIPAddress().c_str());
}

void WebServerManager::loop() {
    if (server_active && server) {
        server->handleClient();
    }
    
    // Restart server if WiFi connection state changes
    static bool last_wifi_state = false;
    bool current_wifi_state = wifiManager.isConnected();
    
    if (current_wifi_state != last_wifi_state) {
        if (current_wifi_state && !server_active) {
            Serial.println("[Web] WiFi connected, starting web server");
            begin();
        } else if (!current_wifi_state && server_active) {
            Serial.println("[Web] WiFi disconnected, stopping web server");
            if (server) {
                server->stop();
                delete server;
                server = nullptr;
            }
            server_active = false;
        }
        last_wifi_state = current_wifi_state;
    }
}

void WebServerManager::syncTime() {
    if (!wifiManager.isConnected()) {
        Serial.println("[Web] Cannot sync time: WiFi not connected");
        return;
    }
    
    Serial.println("[Web] Synchronizing time with NTP servers");
    
    configTime(gmt_offset_sec, daylight_offset_sec, ntp_server1, ntp_server2);
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("[Web] Failed to obtain time");
        return;
    }
    
    Serial.println(&timeinfo, "[Web] Time synchronized: %A, %B %d %Y %H:%M:%S");
}

String WebServerManager::getFormattedTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Not synchronized";
    }
    
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
    return String(buffer);
}

String WebServerManager::getFormattedDate() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Not synchronized";
    }
    
    char buffer[30];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %A", &timeinfo);
    return String(buffer);
}

bool WebServerManager::isTimeSynced() {
    struct tm timeinfo;
    return getLocalTime(&timeinfo);
}

void WebServerManager::setupRoutes() {
    if (!server) return;
    
    server->on("/", std::bind(&WebServerManager::handleRoot, this));
    server->on("/api/status", std::bind(&WebServerManager::handleAPIStatus, this));
    server->on("/api/config", std::bind(&WebServerManager::handleAPIConfig, this));
    server->on("/api/command", std::bind(&WebServerManager::handleAPICommand, this));
    server->on("/api/time", std::bind(&WebServerManager::handleAPITime, this));
    server->on("/api/system", std::bind(&WebServerManager::handleSystemInfo, this));
    server->on("/api/incubation", std::bind(&WebServerManager::handleIncubationAPI, this));
    server->on("/api/auth", std::bind(&WebServerManager::handleAuthAPI, this));
    server->onNotFound(std::bind(&WebServerManager::handleNotFound, this));
}

void WebServerManager::handleRoot() {
    if (!server) return;
    
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Incubator Controller</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
    <style>
        * { box-sizing: border-box; }
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
            margin: 0; 
            padding: 20px; 
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            color: #333;
        }
        
        /* Password Modal */
        .password-modal {
            display: none;
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(0, 0, 0, 0.7);
            z-index: 1000;
            justify-content: center;
            align-items: center;
        }
        .password-modal.active {
            display: flex;
        }
        .password-modal-content {
            background: white;
            padding: 30px;
            border-radius: 15px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            max-width: 400px;
            width: 90%;
        }
        .password-modal h3 {
            margin-top: 0;
            color: #2d3748;
        }
        .password-input {
            width: 100%;
            padding: 12px;
            border: 2px solid #e2e8f0;
            border-radius: 8px;
            font-size: 1em;
            margin-bottom: 20px;
        }
        .password-input:focus {
            outline: none;
            border-color: #4c51bf;
        }
        .password-buttons {
            display: flex;
            gap: 10px;
            justify-content: flex-end;
        }
        .password-error {
            color: #e53e3e;
            margin-bottom: 15px;
            display: none;
        }
        .password-error.active {
            display: block;
        }
        .security-badge {
            display: inline-block;
            padding: 4px 12px;
            border-radius: 20px;
            font-size: 0.85em;
            font-weight: 600;
            margin-left: 10px;
        }
        .security-on { background: #c6f6d5; color: #22543d; }
        .security-off { background: #fed7d7; color: #742a2a; }
        .security-warning { background: #feebc8; color: #744210; }
        .container { 
            max-width: 1200px; 
            margin: 0 auto; 
        }
        header { 
            text-align: center; 
            margin-bottom: 30px; 
            color: white;
            text-shadow: 0 2px 4px rgba(0,0,0,0.2);
        }
        h1 { 
            margin: 0; 
            font-size: 2.5em; 
        }
        .subtitle { 
            opacity: 0.9; 
            font-size: 1.1em; 
            margin-top: 5px;
        }
        .dashboard {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        .card {
            background: white;
            border-radius: 15px;
            padding: 25px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.1);
            transition: transform 0.3s, box-shadow 0.3s;
        }
        .card:hover {
            transform: translateY(-5px);
            box-shadow: 0 15px 35px rgba(0,0,0,0.15);
        }
        .card h2 {
            margin-top: 0;
            color: #4a5568;
            border-bottom: 2px solid #e2e8f0;
            padding-bottom: 10px;
            margin-bottom: 20px;
        }
        .stat {
            margin-bottom: 15px;
        }
        .stat-label {
            font-weight: 600;
            color: #718096;
            font-size: 0.9em;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        .stat-value {
            font-size: 1.8em;
            font-weight: 700;
            color: #2d3748;
            margin: 5px 0;
        }
        .stat-unit {
            color: #a0aec0;
            font-size: 0.9em;
        }
        .temp-hot { color: #e53e3e; }
        .temp-ok { color: #38a169; }
        .temp-cold { color: #3182ce; }
        .controls {
            display: flex;
            gap: 10px;
            flex-wrap: wrap;
            margin-top: 20px;
        }
        button {
            background: #4c51bf;
            color: white;
            border: none;
            padding: 12px 24px;
            border-radius: 8px;
            cursor: pointer;
            font-weight: 600;
            transition: background 0.3s;
            flex: 1;
            min-width: 120px;
        }
        button:hover {
            background: #434190;
        }
        button.danger {
            background: #e53e3e;
        }
        button.danger:hover {
            background: #c53030;
        }
        button.success {
            background: #38a169;
        }
        button.success:hover {
            background: #2f855a;
        }
        .status-badge {
            display: inline-block;
            padding: 4px 12px;
            border-radius: 20px;
            font-size: 0.85em;
            font-weight: 600;
            margin-left: 10px;
        }
        .status-online { background: #c6f6d5; color: #22543d; }
        .status-offline { background: #fed7d7; color: #742a2a; }
        .status-warning { background: #feebc8; color: #744210; }
        .config-form {
            display: grid;
            gap: 15px;
        }
        .form-group {
            display: flex;
            flex-direction: column;
        }
        label {
            margin-bottom: 5px;
            font-weight: 600;
            color: #4a5568;
        }
        input, select {
            padding: 10px;
            border: 2px solid #e2e8f0;
            border-radius: 8px;
            font-size: 1em;
            transition: border-color 0.3s;
        }
        input:focus, select:focus {
            outline: none;
            border-color: #4c51bf;
        }
        .last-update {
            text-align: center;
            color: #a0aec0;
            font-size: 0.9em;
            margin-top: 20px;
        }
        .network-info {
            background: #f7fafc;
            padding: 15px;
            border-radius: 8px;
            margin-top: 15px;
        }
        @media (max-width: 768px) {
            .dashboard {
                grid-template-columns: 1fr;
            }
            body {
                padding: 10px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>🥚 ESP32 Incubator Controller</h1>
            <div class="subtitle">Smart incubation with remote monitoring</div>
            <div class="security-status" id="securityStatus">
                Security: <span id="securityBadge" class="security-badge security-off">OFF</span>
            </div>
        </header>
        
        <!-- Password Modal -->
        <div class="password-modal" id="passwordModal">
            <div class="password-modal-content">
                <h3>Authentication Required</h3>
                <div class="password-error" id="passwordError">
                    Incorrect password. Please try again.
                </div>
                <p>This operation requires authentication. Please enter your password:</p>
                <input type="password" class="password-input" id="passwordInput" placeholder="Enter password" autocomplete="current-password">
                <div class="password-buttons">
                    <button onclick="hidePasswordModal()">Cancel</button>
                    <button class="success" onclick="submitPassword()">Authenticate</button>
                </div>
            </div>
        </div>
        
        <div class="dashboard">
            <!-- Temperature & Humidity Card -->
            <div class="card">
                <h2>Climate Control</h2>
                <div class="stat">
                    <div class="stat-label">Current Temperature</div>
                    <div class="stat-value" id="temperature">--.-</div>
                    <div class="stat-unit">°C</div>
                </div>
                <div class="stat">
                    <div class="stat-label">Current Humidity</div>
                    <div class="stat-value" id="humidity">--.-</div>
                    <div class="stat-unit">%</div>
                </div>
                <div class="stat">
                    <div class="stat-label">Target Temperature</div>
                    <div class="stat-value" id="targetTemp">--.-</div>
                    <div class="stat-unit">°C</div>
                </div>
                <div class="stat">
                    <div class="stat-label">Heater Status</div>
                    <div>
                        <span id="heaterStatus">--</span>
                        <span class="status-badge" id="heaterBadge">--</span>
                    </div>
                </div>
                <div class="stat">
                    <div class="stat-label">Fan Status</div>
                    <div>
                        <span id="fanStatus">--</span>
                        <span class="status-badge" id="fanBadge">--</span>
                    </div>
                </div>
            </div>
            
            <!-- Egg Turner Card -->
            <div class="card">
                <h2>Egg Turner</h2>
                <div class="stat">
                    <div class="stat-label">Next Turn In</div>
                    <div class="stat-value" id="nextTurn">--:--</div>
                    <div class="stat-unit">HH:MM</div>
                </div>
                <div class="stat">
                    <div class="stat-label">Turn Interval</div>
                    <div class="stat-value" id="turnInterval">--</div>
                    <div class="stat-unit">hours</div>
                </div>
                <div class="stat">
                    <div class="stat-label">Turner Status</div>
                    <div>
                        <span id="turnerStatus">--</span>
                        <span class="status-badge" id="turnerBadge">--</span>
                    </div>
                </div>
                <div class="controls">
                    <button class="success" id="turnButton" onclick="toggleTurn()">Turn Now</button>
                    <button onclick="sendCommand('reset_timer')">Reset Timer</button>
                </div>
            </div>
            
            <!-- System Info Card -->
            <div class="card">
                <h2>System Information</h2>
                <div class="stat">
                    <div class="stat-label">Current Time</div>
                    <div class="stat-value" id="currentTime">--:--:--</div>
                    <div class="stat-unit" id="currentDate">---- -- --</div>
                </div>
                <div class="stat">
                    <div class="stat-label">WiFi Connection</div>
                    <div>
                        <span id="wifiStatus">--</span>
                        <span class="status-badge" id="wifiBadge">--</span>
                    </div>
                </div>
                <div class="stat">
                    <div class="stat-label">Hostname</div>
                    <div class="stat-value" id="hostname">incubator-esp32</div>
                </div>
                <div class="stat">
                    <div class="stat-label">IP Address</div>
                    <div class="stat-value" id="ipAddress">---.---.---.---</div>
                </div>
                <div class="stat">
                    <div class="stat-label">Uptime</div>
                    <div class="stat-value" id="uptime">--:--:--</div>
                </div>
                <div class="controls">
                    <button onclick="sendCommand('restart')">Restart System</button>
                    <button class="danger" onclick="sendCommand('factory_reset')">Factory Reset</button>
                </div>
            </div>
            
            <!-- Configuration Card -->
            <div class="card">
                <h2>Configuration</h2>
                <form class="config-form" onsubmit="return updateConfig()">
                    <div class="form-group">
                        <label for="configTargetTemp">Target Temperature (°C)</label>
                        <input type="number" id="configTargetTemp" step="0.1" min="35" max="42" required>
                    </div>
                    <div class="form-group">
                        <label for="configTurnInterval">Turn Interval (hours)</label>
                        <input type="number" id="configTurnInterval" min="1" max="24" required>
                    </div>
                    <div class="form-group">
                        <label for="configTurnDuration">Turn Duration (seconds)</label>
                        <input type="number" id="configTurnDuration" min="1" max="60" required>
                        <div class="text-sm text-gray-500 mt-1">
                            How long the egg turner motor runs each time (1-60 seconds)
                        </div>
                    </div>
                    <button type="submit" class="success">Update Configuration</button>
                </form>
                <div class="network-info">
                    <div class="stat-label">Connected to WiFi:</div>
                    <div class="stat-value" id="connectedSSID">--</div>
                </div>
            </div>
            
            <!-- Password Configuration Card -->
            <div class="card">
                <h2>Password Configuration</h2>
                <div class="text-sm text-gray-500 mb-4">
                    Configure security password separately from other settings.
                </div>
                <form class="config-form" onsubmit="return updatePasswordConfig()">
                    <div class="form-group">
                        <label for="passwordConfigCurrent">Current Password (if set)</label>
                        <input type="password" id="passwordConfigCurrent" placeholder="Enter current password if changing">
                    </div>
                    <div class="form-group">
                        <label for="passwordConfigNew">New Password</label>
                        <input type="password" id="passwordConfigNew" placeholder="Enter new password">
                        <div class="text-sm text-gray-500 mt-1">
                            Leave empty to disable password protection
                        </div>
                    </div>
                    <div class="form-group">
                        <label for="passwordConfigConfirm">Confirm New Password</label>
                        <input type="password" id="passwordConfigConfirm" placeholder="Confirm new password">
                    </div>
                    <button type="submit" class="success">Update Password</button>
                </form>
                <div class="mt-4 text-sm">
                    <div class="stat-label">Password Status:</div>
                    <div class="stat-value" id="passwordStatus">--</div>
                </div>
            </div>
            
            <!-- Incubation Tracking Card -->
            <div class="card">
                <h2>Incubation Tracking</h2>
                <div class="stat">
                    <div class="stat-label">Current Status</div>
                    <div>
                        <span id="incubationStatus">--</span>
                        <span class="status-badge" id="incubationBadge">--</span>
                    </div>
                </div>
                <div class="stat">
                    <div class="stat-label">Species</div>
                    <div class="stat-value" id="incubationSpecies">--</div>
                </div>
                <div class="stat">
                    <div class="stat-label">Day</div>
                    <div class="stat-value" id="incubationDay">--</div>
                    <div class="stat-unit" id="incubationTotalDays">of -- days</div>
                </div>
                <div class="stat">
                    <div class="stat-label">Time Remaining</div>
                    <div class="stat-value" id="incubationRemaining">--</div>
                </div>
                <div class="controls">
                    <select id="birdSpecies">
                        <option value="0">Quail</option>
                        <option value="1">Chicken</option>
                        <option value="2">Duck</option>
                        <option value="3">Goose</option>
                        <option value="4">Peacock</option>
                        <option value="5">Turkey</option>
                        <option value="6">Pheasant</option>
                        <option value="7">Guinea Fowl</option>
                        <option value="8">Custom</option>
                    </select>
                    <button class="success" id="startIncubationBtn" onclick="startIncubation()">Start Incubation</button>
                    <button class="danger" id="stopIncubationBtn" onclick="stopIncubation()">Stop Incubation</button>
                </div>
                <div class="controls" style="margin-top: 15px;">
                    <input type="number" id="adjustDays" placeholder="Days to adjust" min="-30" max="30" style="flex: 2;">
                    <button onclick="adjustIncubationDays()" style="flex: 1;">Adjust Days</button>
                </div>
                <div class="controls" style="margin-top: 15px;">
                    <input type="number" id="setDay" placeholder="Set to day" min="0" max="50" style="flex: 2;">
                    <button onclick="setIncubationDay()" style="flex: 1;">Set Day</button>
                </div>
                <div class="alerts" id="incubationAlerts">
                    <!-- Alerts will appear here -->
                </div>
            </div>
        </div>
        
        <div class="last-update" id="lastUpdate">
            Last updated: <span id="updateTime">--:--:--</span>
        </div>
    </div>
    
    <script>
        let updateInterval;
        
        // Password authentication
        let pendingCommand = null;
        let pendingCommandData = null;
        
        function showPasswordModal(command, data = null) {
            pendingCommand = command;
            pendingCommandData = data;
            document.getElementById('passwordModal').classList.add('active');
            document.getElementById('passwordInput').focus();
            document.getElementById('passwordError').classList.remove('active');
        }
        
        function hidePasswordModal() {
            document.getElementById('passwordModal').classList.remove('active');
            pendingCommand = null;
            pendingCommandData = null;
            document.getElementById('passwordInput').value = '';
        }
        
        function submitPassword() {
            const password = document.getElementById('passwordInput').value;
            
            fetch('/api/auth', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ password: password })
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    hidePasswordModal();
                    // Execute the pending command
                    if (pendingCommand === 'sendCommand') {
                        executeCommand(pendingCommandData.command, pendingCommandData.data);
                    } else if (pendingCommand === 'updateConfig') {
                        executeUpdateConfig(pendingCommandData);
                    }
                } else {
                    document.getElementById('passwordError').classList.add('active');
                    document.getElementById('passwordInput').value = '';
                    document.getElementById('passwordInput').focus();
                }
            })
            .catch(error => {
                console.error('Password authentication error:', error);
                document.getElementById('passwordError').textContent = 'Network error';
                document.getElementById('passwordError').classList.add('active');
            });
        }
        
        // Check if authentication is required before executing command
        function checkAuthAndExecute(command, data = null) {
            // First check current authentication status
            fetch('/api/auth')
                .then(response => response.json())
                .then(authData => {
                    if (authData.requires_password) {
                        // Show password modal
                        showPasswordModal('sendCommand', { command: command, data: data });
                    } else {
                        // Already authenticated or no password required
                        executeCommand(command, data);
                    }
                })
                .catch(error => {
                    console.error('Auth check error:', error);
                    executeCommand(command, data);
                });
        }
        
        function updateDashboard() {
            fetch('/api/status')
                .then(response => response.json())
                .then(data => {
                    // Update temperature/humidity
                    document.getElementById('temperature').textContent = data.temperature.toFixed(1);
                    document.getElementById('humidity').textContent = data.humidity.toFixed(1);
                    document.getElementById('targetTemp').textContent = data.target_temp.toFixed(1);
                    
                    // Update heater/fan status
                    document.getElementById('heaterStatus').textContent = data.heater_on ? 'ON' : 'OFF';
                    document.getElementById('heaterBadge').textContent = data.heater_on ? 'HEATING' : 'IDLE';
                    document.getElementById('heaterBadge').className = 'status-badge ' + (data.heater_on ? 'status-warning' : 'status-online');
                    
                    document.getElementById('fanStatus').textContent = data.fan_on ? 'ON' : 'OFF';
                    document.getElementById('fanBadge').textContent = data.fan_on ? 'RUNNING' : 'STOPPED';
                    document.getElementById('fanBadge').className = 'status-badge ' + (data.fan_on ? 'status-online' : 'status-offline');
                    
                    // Update egg turner
                    const hours = Math.floor(data.next_turn_seconds / 3600);
                    const minutes = Math.floor((data.next_turn_seconds % 3600) / 60);
                    document.getElementById('nextTurn').textContent = 
                        hours.toString().padStart(2, '0') + ':' + minutes.toString().padStart(2, '0');
                    
                    document.getElementById('turnInterval').textContent = (data.turn_interval / 3600).toFixed(0);
                    document.getElementById('turnerStatus').textContent = data.turner_active ? 'ACTIVE' : 'IDLE';
                    document.getElementById('turnerBadge').textContent = data.turner_active ? 'TURNING' : 'WAITING';
                    document.getElementById('turnerBadge').className = 'status-badge ' + (data.turner_active ? 'status-warning' : 'status-online');
                    
                    // Update turn button based on turner status
                    const turnButton = document.getElementById('turnButton');
                    if (data.turner_turning) {
                        turnButton.textContent = 'Stop Turning';
                        turnButton.className = 'danger';
                    } else {
                        turnButton.textContent = 'Turn Now';
                        turnButton.className = 'success';
                    }
                    
                    // Update system info
                    document.getElementById('currentTime').textContent = data.current_time;
                    document.getElementById('currentDate').textContent = data.current_date;
                    document.getElementById('wifiStatus').textContent = data.wifi_connected ? 'CONNECTED' : 'DISCONNECTED';
                    document.getElementById('wifiBadge').textContent = data.wifi_connected ? 'ONLINE' : 'OFFLINE';
                    document.getElementById('wifiBadge').className = 'status-badge ' + (data.wifi_connected ? 'status-online' : 'status-offline');
                    document.getElementById('hostname').textContent = data.hostname;
                    document.getElementById('ipAddress').textContent = data.ip_address;
                    document.getElementById('uptime').textContent = data.uptime;
                    document.getElementById('connectedSSID').textContent = data.wifi_ssid;
                    
                    // Update configuration form
                    document.getElementById('configTargetTemp').value = data.target_temp;
                    document.getElementById('configTurnInterval').value = data.turn_interval / 3600;
                    if (data.turn_duration !== undefined) {
                        document.getElementById('configTurnDuration').value = data.turn_duration;
                    }
                    
                    // Update incubation tracking
                    document.getElementById('incubationStatus').textContent = 
                        data.incubation_active ? 'ACTIVE' : 'INACTIVE';
                    document.getElementById('incubationBadge').textContent = 
                        data.incubation_active ? 'RUNNING' : 'STOPPED';
                    document.getElementById('incubationBadge').className = 'status-badge ' + 
                        (data.incubation_active ? 'status-online' : 'status-offline');
                    document.getElementById('incubationSpecies').textContent = data.incubation_species;
                    document.getElementById('incubationDay').textContent = data.incubation_day;
                    document.getElementById('incubationTotalDays').textContent = 'of ' + 
                        (data.incubation_day + data.incubation_remaining_days) + ' days';
                    document.getElementById('incubationRemaining').textContent = data.incubation_remaining_days + ' days';
                    
                    // Update password status
                    const passwordStatus = data.password_enabled ? 
                        (data.authenticated ? 'ENABLED (Authenticated)' : 'ENABLED (Locked)') : 
                        'DISABLED';
                    document.getElementById('passwordStatus').textContent = passwordStatus;
                    
                    // Update alerts
                    const alertsDiv = document.getElementById('incubationAlerts');
                    alertsDiv.innerHTML = '';
                    
                    if (data.incubation_active) {
                        if (data.is_candling_day) {
                            alertsDiv.innerHTML += '<div class="status-badge status-warning">🎯 TODAY: Candling Day!</div>';
                        }
                        if (data.is_lockdown_day) {
                            alertsDiv.innerHTML += '<div class="status-badge status-warning">🔒 LOCKDOWN: Stop Turning!</div>';
                        }
                        if (data.is_hatching_day) {
                            alertsDiv.innerHTML += '<div class="status-badge status-online">🐣 HATCHING DAY!</div>';
                        }
                    }
                    
                    // Update incubation controls based on incubator state
                    const speciesSelect = document.getElementById('birdSpecies');
                    const startBtn = document.getElementById('startIncubationBtn');
                    const stopBtn = document.getElementById('stopIncubationBtn');
                    const isIncubating = data.incubator_state === 'incubating' || data.incubation_active;
                    
                    if (isIncubating) {
                        // When incubating: hide species selector and Start button, show Stop button
                        speciesSelect.style.display = 'none';
                        startBtn.style.display = 'none';
                        stopBtn.style.display = 'block';
                    } else {
                        // When idle: show species selector and Start button, hide Stop button
                        speciesSelect.style.display = 'block';
                        startBtn.style.display = 'block';
                        stopBtn.style.display = 'none';
                    }
                    
                    // Update security status
                    const securityBadge = document.getElementById('securityBadge');
                    if (data.password_enabled) {
                        if (data.authenticated) {
                            securityBadge.textContent = 'AUTHENTICATED';
                            securityBadge.className = 'security-badge security-on';
                        } else {
                            securityBadge.textContent = 'LOCKED';
                            securityBadge.className = 'security-badge security-warning';
                        }
                    } else {
                        securityBadge.textContent = 'OFF';
                        securityBadge.className = 'security-badge security-off';
                    }
                    
                    // Update last update time
                    const now = new Date();
                    document.getElementById('updateTime').textContent = 
                        now.getHours().toString().padStart(2, '0') + ':' +
                        now.getMinutes().toString().padStart(2, '0') + ':' +
                        now.getSeconds().toString().padStart(2, '0');
                })
                .catch(error => {
                    console.error('Error updating dashboard:', error);
                });
        }
        
        function toggleTurn() {
            const turnButton = document.getElementById('turnButton');
            const isTurning = turnButton.textContent === 'Stop Turning';
            
            const command = isTurning ? 'stop_turning' : 'turn_now';
            const payload = { command: command };
            
            // Check authentication before executing
            checkAuthAndExecute(command, payload);
        }
        
        // Original sendCommand function (now used internally)
        function executeCommand(command, payload) {
            fetch('/api/command', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    if (command === 'start_incubation') {
                        alert('Incubation started successfully');
                    } else if (command === 'stop_incubation') {
                        alert('Incubation stopped successfully');
                    } else if (command === 'turn_now' || command === 'stop_turning') {
                        // Success message for turn commands
                        const message = command === 'turn_now' ? 'Turning started' : 'Turning stopped';
                        alert(message + ' successfully');
                    }
                    updateDashboard(); // Refresh data
                } else {
                    if (data.message && data.message.includes('Authentication')) {
                        alert('Authentication required. Please enter password.');
                        showPasswordModal('sendCommand', { command: command, data: payload });
                    } else {
                        alert('Error: ' + (data.message || 'Unknown error'));
                    }
                }
            })
            .catch(error => {
                console.error('Error sending command:', error);
                alert('Network error sending command');
            });
        }
        
        function sendCommand(command) {
            const payload = { command: command };
            
            fetch('/api/command', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    alert('Command executed successfully');
                    updateDashboard(); // Refresh data
                } else {
                    alert('Error: ' + (data.message || 'Unknown error'));
                }
            })
            .catch(error => {
                console.error('Error sending command:', error);
                alert('Network error sending command');
            });
        }
        
        function updateConfig() {
            const targetTemp = parseFloat(document.getElementById('configTargetTemp').value);
            const turnInterval = parseInt(document.getElementById('configTurnInterval').value);
            const turnDuration = parseInt(document.getElementById('configTurnDuration').value);
            
            if (isNaN(targetTemp) || targetTemp < 35 || targetTemp > 42) {
                alert('Target temperature must be between 35°C and 42°C');
                return false;
            }
            
            if (isNaN(turnInterval) || turnInterval < 1 || turnInterval > 24) {
                alert('Turn interval must be between 1 and 24 hours');
                return false;
            }
            
            if (isNaN(turnDuration) || turnDuration < 1 || turnDuration > 60) {
                alert('Turn duration must be between 1 and 60 seconds');
                return false;
            }
            
            const payload = {
                target_temp: targetTemp,
                turn_interval: turnInterval * 3600, // Convert hours to seconds
                turn_duration: turnDuration
            };
            
            // Check authentication before updating config
            fetch('/api/auth')
                .then(response => response.json())
                .then(authData => {
                    if (authData.requires_password) {
                        // Show password modal
                        showPasswordModal('updateConfig', payload);
                    } else {
                        // Already authenticated or no password required
                        executeUpdateConfig(payload);
                    }
                })
                .catch(error => {
                    console.error('Auth check error:', error);
                    executeUpdateConfig(payload);
                });
            
            return false; // Prevent form submission
        }
        
        function executeUpdateConfig(payload) {
            fetch('/api/config', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    alert('Configuration updated successfully');
                    updateDashboard(); // Refresh data
                } else {
                    if (data.message && data.message.includes('Authentication')) {
                        alert('Authentication required. Please enter password.');
                        showPasswordModal('updateConfig', payload);
                    } else {
                        alert('Error: ' + (data.message || 'Unknown error'));
                    }
                }
            })
            .catch(error => {
                console.error('Error updating config:', error);
                alert('Network error updating configuration');
            });
            
            return false; // Prevent form submission
        }
        
        function updatePasswordConfig() {
            const currentPassword = document.getElementById('passwordConfigCurrent').value;
            const newPassword = document.getElementById('passwordConfigNew').value;
            const confirmPassword = document.getElementById('passwordConfigConfirm').value;
            
            // Validate passwords match
            if (newPassword !== confirmPassword) {
                alert('New password and confirmation do not match');
                return false;
            }
            
            // Prepare payload
            const payload = {
                command: 'update_password',
                current_password: currentPassword,
                new_password: newPassword
            };
            
            // Check authentication before updating password
            fetch('/api/auth')
                .then(response => response.json())
                .then(authData => {
                    if (authData.requires_password) {
                        // Show password modal
                        showPasswordModal('sendCommand', payload);
                    } else {
                        // Already authenticated or no password required
                        executePasswordUpdate(payload);
                    }
                })
                .catch(error => {
                    console.error('Auth check error:', error);
                    executePasswordUpdate(payload);
                });
            
            return false; // Prevent form submission
        }
        
        function executePasswordUpdate(payload) {
            fetch('/api/command', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    alert('Password updated successfully');
                    // Clear password fields
                    document.getElementById('passwordConfigCurrent').value = '';
                    document.getElementById('passwordConfigNew').value = '';
                    document.getElementById('passwordConfigConfirm').value = '';
                    updateDashboard(); // Refresh data
                } else {
                    if (data.message && data.message.includes('Authentication')) {
                        alert('Authentication required. Please enter password.');
                        showPasswordModal('sendCommand', payload);
                    } else {
                        alert('Error: ' + (data.message || 'Unknown error'));
                    }
                }
            })
            .catch(error => {
                console.error('Error updating password:', error);
                alert('Network error updating password');
            });
        }
        
        function startIncubation() {
            const speciesSelect = document.getElementById('birdSpecies');
            const species = parseInt(speciesSelect.value);
            const speciesName = speciesSelect.options[speciesSelect.selectedIndex].text;
            
            if (isNaN(species)) {
                alert('Please select a bird species');
                return;
            }
            
            // Ask for confirmation
            if (!confirm(`Are you sure you want to start incubation for ${speciesName}?\n\nThis will:\n• Set target temperature based on species\n• Set turn interval based on species\n• Start incubation tracking`)) {
                return;
            }
            
            const payload = {
                command: 'start_incubation',
                species: species
            };
            
            // Check authentication before executing
            checkAuthAndExecute('start_incubation', payload)
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    alert('Incubation started successfully');
                    updateDashboard(); // Refresh data
                } else {
                    alert('Error: ' + (data.message || 'Unknown error'));
                }
            })
            .catch(error => {
                console.error('Error starting incubation:', error);
                alert('Network error starting incubation');
            });
        }
        
        function stopIncubation() {
            if (!confirm('Are you sure you want to stop the current incubation session?\n\nThis will:\n• Reset to default temperature (38.0°C)\n• Reset to default turn interval (8 hours)\n• Clear incubation tracking data')) {
                return;
            }
            
            const payload = {
                command: 'stop_incubation'
            };
            
            // Check authentication before executing
            checkAuthAndExecute('stop_incubation', payload);
        }
        
        function adjustIncubationDays() {
            const daysInput = document.getElementById('adjustDays');
            const days = parseInt(daysInput.value);
            
            if (isNaN(days)) {
                alert('Please enter a valid number of days to adjust');
                return;
            }
            
            if (days === 0) {
                alert('Please enter a non-zero number of days');
                return;
            }
            
            const payload = {
                action: 'adjust_days',
                days: days
            };
            
            fetch('/api/incubation', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    alert('Incubation days adjusted by ' + days + ' days');
                    updateDashboard(); // Refresh data
                    daysInput.value = ''; // Clear input
                } else {
                    alert('Error: ' + (data.message || 'Unknown error'));
                }
            })
            .catch(error => {
                console.error('Error adjusting incubation days:', error);
                alert('Network error adjusting incubation days');
            });
        }
        
        function setIncubationDay() {
            const dayInput = document.getElementById('setDay');
            const day = parseInt(dayInput.value);
            
            if (isNaN(day) || day < 0) {
                alert('Please enter a valid day number (0 or higher)');
                return;
            }
            
            const payload = {
                action: 'set_day',
                day: day
            };
            
            fetch('/api/incubation', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    alert('Incubation set to day ' + day);
                    updateDashboard(); // Refresh data
                    dayInput.value = ''; // Clear input
                } else {
                    alert('Error: ' + (data.message || 'Unknown error'));
                }
            })
            .catch(error => {
                console.error('Error setting incubation day:', error);
                alert('Network error setting incubation day');
            });
        }
        
        // Start auto-update every 5 seconds
        updateInterval = setInterval(updateDashboard, 5000);
        
        // Initial update
        updateDashboard();
        
        // Stop auto-update when page is hidden
        document.addEventListener('visibilitychange', function() {
            if (document.hidden) {
                clearInterval(updateInterval);
            } else {
                updateInterval = setInterval(updateDashboard, 5000);
                updateDashboard();
            }
        });
    </script>
</body>
</html>
)rawliteral";
    
    server->send(200, "text/html", html);
}

void WebServerManager::handleAPIStatus() {
    if (!server) return;
    
    String json = getSystemStatusJSON();
    server->send(200, "application/json", json);
}

void WebServerManager::handleAPIConfig() {
    if (!server) return;
    
    if (server->method() == HTTP_GET) {
        String json = getSystemConfigJSON();
        server->send(200, "application/json", json);
    } else if (server->method() == HTTP_POST) {
        String body = server->arg("plain");
        
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, body);
        
        if (error) {
            server->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
            return;
        }
        
        bool success = updateConfig(doc);
        
        if (success) {
            server->send(200, "application/json", "{\"success\":true,\"message\":\"Configuration updated\"}");
        } else {
            server->send(400, "application/json", "{\"success\":false,\"message\":\"Failed to update configuration\"}");
        }
    }
}

void WebServerManager::handleAPICommand() {
    if (!server) return;
    
    if (server->method() != HTTP_POST) {
        server->send(405, "application/json", "{\"success\":false,\"message\":\"Method not allowed\"}");
        return;
    }
    
    String body = server->arg("plain");
    
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (error || !doc.containsKey("command")) {
        server->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid command\"}");
        return;
    }
    
    String command = doc["command"].as<String>();
    bool success = executeCommand(command, doc);
    
    if (success) {
        server->send(200, "application/json", "{\"success\":true,\"message\":\"Command executed\"}");
    } else {
        server->send(400, "application/json", "{\"success\":false,\"message\":\"Failed to execute command\"}");
    }
}

void WebServerManager::handleAPITime() {
    if (!server) return;
    
    StaticJsonDocument<200> doc;
    doc["current_time"] = getFormattedTime();
    doc["current_date"] = getFormattedDate();
    doc["time_synced"] = isTimeSynced();
    doc["timestamp"] = time(nullptr);
    
    String json;
    serializeJson(doc, json);
    server->send(200, "application/json", json);
}

void WebServerManager::handleSystemInfo() {
    if (!server) return;
    
    String json = getSystemInfoJSON();
    server->send(200, "application/json", json);
}

void WebServerManager::handleIncubationAPI() {
    if (!server) return;
    
    if (server->method() == HTTP_GET) {
        // Get incubation status
        StaticJsonDocument<512> doc;
        
        doc["session_active"] = incubationTracker.isSessionActive();
        doc["species"] = incubationTracker.getSpeciesName();
        doc["elapsed_days"] = incubationTracker.getElapsedDays();
        doc["remaining_days"] = incubationTracker.getRemainingDays();
        doc["is_candling_day"] = incubationTracker.isCandlingDay();
        doc["is_lockdown_day"] = incubationTracker.isLockdownDay();
        doc["is_hatching_day"] = incubationTracker.isHatchingDay();
        doc["time_remaining"] = incubationTracker.getTimeRemainingString();
        
        // Add preset information if session is active
        if (incubationTracker.isSessionActive()) {
            doc["target_temp"] = incubationTracker.getCurrentTargetTemp();
            doc["target_humidity"] = incubationTracker.getCurrentTargetHumidity();
            doc["incubation_days"] = incubationTracker.getCurrentIncubationDays();
            doc["candling_day"] = incubationTracker.getCurrentCandlingDay();
            doc["lockdown_day"] = incubationTracker.getCurrentLockdownDay();
            doc["turn_interval"] = incubationTracker.getCurrentTurnInterval();
        }
        
        String json;
        serializeJson(doc, json);
        server->send(200, "application/json", json);
        
    } else if (server->method() == HTTP_POST) {
        // Start/stop incubation session
        String body = server->arg("plain");
        
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, body);
        
        if (error) {
            server->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
            return;
        }
        
        bool success = false;
        String message;
        
        if (doc.containsKey("action")) {
            String action = doc["action"].as<String>();
            
            if (action == "start") {
                if (doc.containsKey("species")) {
                    int species = doc["species"].as<int>();
                    time_t start_time = 0;
                    
                    if (doc.containsKey("start_time")) {
                        start_time = doc["start_time"].as<time_t>();
                    }
                    
                    success = incubationTracker.startIncubation(static_cast<BirdSpecies>(species), start_time);
                    message = success ? "Incubation session started" : "Failed to start incubation session";
                } else {
                    message = "Missing species parameter";
                }
                
            } else if (action == "stop") {
                success = incubationTracker.stopIncubation();
                message = success ? "Incubation session stopped" : "Failed to stop incubation session";
                
            } else if (action == "adjust_days") {
                if (doc.containsKey("days")) {
                    int days = doc["days"].as<int>();
                    success = incubationTracker.adjustIncubationDays(days);
                    message = success ? "Incubation days adjusted" : "Failed to adjust incubation days";
                } else {
                    message = "Missing days parameter";
                }
                
            } else if (action == "set_day") {
                if (doc.containsKey("day")) {
                    unsigned int day = doc["day"].as<unsigned int>();
                    success = incubationTracker.setIncubationDay(day);
                    message = success ? "Incubation day set" : "Failed to set incubation day";
                } else {
                    message = "Missing day parameter";
                }
                
            } else if (action == "status") {
                // Already handled by GET
                success = true;
                message = "Status retrieved";
            }
        }
        
        StaticJsonDocument<128> response;
        response["success"] = success;
        response["message"] = message;
        
        String json;
        serializeJson(response, json);
        server->send(success ? 200 : 400, "application/json", json);
        
    } else {
        server->send(405, "application/json", "{\"success\":false,\"message\":\"Method not allowed\"}");
    }
}

void WebServerManager::handleAuthAPI() {
    if (!server) return;
    
    if (server->method() == HTTP_POST) {
        String body = server->arg("plain");
        
        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, body);
        
        if (error || !doc.containsKey("password")) {
            server->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid request\"}");
            return;
        }
        
        String password = doc["password"].as<String>();
        bool authenticated = passwordManager.authenticate(password);
        
        if (authenticated) {
            server->send(200, "application/json", "{\"success\":true,\"message\":\"Authenticated\"}");
        } else {
            server->send(401, "application/json", "{\"success\":false,\"message\":\"Authentication failed\"}");
        }
    } else if (server->method() == HTTP_GET) {
        // Return authentication status
        bool authenticated = checkAuthentication();
        bool passwordEnabled = passwordManager.isPasswordEnabled();
        
        StaticJsonDocument<128> doc;
        doc["authenticated"] = authenticated;
        doc["password_enabled"] = passwordEnabled;
        doc["requires_password"] = passwordEnabled && !authenticated;
        
        String json;
        serializeJson(doc, json);
        server->send(200, "application/json", json);
    } else {
        server->send(405, "application/json", "{\"success\":false,\"message\":\"Method not allowed\"}");
    }
}

void WebServerManager::handleNotFound() {
    if (!server) return;
    
    server->send(404, "text/plain", "Not found");
}

String WebServerManager::getSystemStatusJSON() {
    StaticJsonDocument<512> doc;
    
    // Climate data
    doc["temperature"] = thermo.temperature();
    doc["humidity"] = thermo.humidity();
    doc["target_temp"] = targetTemp;
    doc["heater_on"] = digitalRead(HEATER_PIN);
    doc["fan_on"] = thermo.fanOn;
    
    // Egg turner data
    doc["next_turn_seconds"] = turner.remained();
    doc["turn_interval"] = EGGS_TURNING_INTERVAL;
    #ifdef EGGS_TURNER_PIN
    // Read the turner pin state to determine if motor is currently ON
    // Note: This only works for relay mode, not servo mode
    bool turner_active = digitalRead(EGGS_TURNER_PIN);
    doc["turner_active"] = turner_active;
    doc["turner_turning"] = turner_active;
    doc["turn_duration"] = EGGS_TURN_SECONDS;
    #else
    // For servo mode or when turner pin is not defined
    doc["turner_active"] = false;
    doc["turner_turning"] = false;
    #ifdef EGGS_TURNER_SERVO_PIN
    doc["turn_duration"] = EGGS_TURN_SECONDS;
    #endif
    #endif
    
    // System data
    doc["current_time"] = getFormattedTime();
    doc["current_date"] = getFormattedDate();
    doc["wifi_connected"] = wifiManager.isConnected();
    doc["hostname"] = wifiManager.getHostname();
    doc["ip_address"] = wifiManager.getIPAddress();
    doc["wifi_ssid"] = wifiManager.getSSID();
    
    // Incubation data
    bool incubation_active = incubationTracker.isSessionActive();
    doc["incubation_active"] = incubation_active;
    doc["incubation_species"] = incubationTracker.getSpeciesName();
    doc["incubation_day"] = incubationTracker.getElapsedDays();
    doc["incubation_remaining_days"] = incubationTracker.getRemainingDays();
    doc["is_candling_day"] = incubationTracker.isCandlingDay();
    doc["is_lockdown_day"] = incubationTracker.isLockdownDay();
    doc["is_hatching_day"] = incubationTracker.isHatchingDay();
    
    // Incubator state
    doc["incubator_state"] = incubatorState == INCUBATOR_INCUBATING ? "incubating" : "idle";
    doc["incubator_state_code"] = incubatorState;
    
    // Uptime
    unsigned long uptime = millis() / 1000;
    unsigned long hours = uptime / 3600;
    unsigned long minutes = (uptime % 3600) / 60;
    unsigned long seconds = uptime % 60;
    char uptimeStr[20];
    snprintf(uptimeStr, sizeof(uptimeStr), "%02lu:%02lu:%02lu", hours, minutes, seconds);
    doc["uptime"] = uptimeStr;
    
    // Security status
    doc["password_enabled"] = passwordManager.isPasswordEnabled();
    doc["authenticated"] = checkAuthentication();
    
    String json;
    serializeJson(doc, json);
    return json;
}

String WebServerManager::getSystemConfigJSON() {
    StaticJsonDocument<256> doc;
    
    doc["target_temp"] = targetTemp;
    doc["turn_interval"] = EGGS_TURNING_INTERVAL;
    #ifdef EGGS_TURNER_PIN
    doc["turn_duration"] = EGGS_TURN_SECONDS;
    #endif
    doc["wifi_ssid"] = wifiManager.getSSID();
    
    String json;
    serializeJson(doc, json);
    return json;
}

String WebServerManager::getSystemInfoJSON() {
    StaticJsonDocument<512> doc;
    
    // Hardware info
    doc["chip_model"] = "ESP32-C3";
    doc["chip_revision"] = ESP.getChipRevision();
    doc["cpu_freq_mhz"] = ESP.getCpuFreqMHz();
    doc["flash_size"] = ESP.getFlashChipSize();
    doc["free_heap"] = ESP.getFreeHeap();
    doc["sketch_size"] = ESP.getSketchSize();
    doc["free_sketch_space"] = ESP.getFreeSketchSpace();
    
    // Network info
    doc["mac_address"] = WiFi.macAddress();
    doc["hostname"] = wifiManager.getHostname();
    doc["rssi"] = WiFi.RSSI();
    
    // Software info
    doc["sdk_version"] = ESP.getSdkVersion();
    doc["compile_date"] = __DATE__;
    doc["compile_time"] = __TIME__;
    
    String json;
    serializeJson(doc, json);
    return json;
}

bool WebServerManager::updateConfig(const JsonDocument& doc) {
    // Check authentication for configuration updates
    if (!checkAuthentication()) {
        Serial.println("[Web] Authentication required for configuration update");
        return false;
    }
    
    bool updated = false;
    
    // Handle password update if provided
    if (doc.containsKey("password")) {
        String password = doc["password"].as<String>();
        if (passwordManager.setPassword(password)) {
            Serial.println("[Web] Password updated");
            updated = true;
        } else {
            Serial.println("[Web] Failed to update password");
        }
    }
    
    // Update target temperature
    if (doc.containsKey("target_temp")) {
        float new_temp = doc["target_temp"].as<float>();
        if (new_temp >= 35.0 && new_temp <= 42.0) {
            targetTemp = new_temp;
            
            // Save to storage
            configStorage.saveTargetTemperature(new_temp);
            
            Serial.printf("[Web] Target temperature updated to: %.1f°C\n", new_temp);
            updated = true;
        } else {
            Serial.printf("[Web] Invalid target temperature: %.1f°C\n", new_temp);
        }
    }
    
    // Update turn interval
    if (doc.containsKey("turn_interval")) {
        unsigned int new_interval = doc["turn_interval"].as<unsigned int>();
        if (new_interval >= 3600 && new_interval <= 86400) { // 1 hour to 24 hours
            EGGS_TURNING_INTERVAL = new_interval;
            
            // Reset the egg turner timer to use the new interval immediately
            turner.resetTimer();
            
            // Save to storage
            configStorage.saveTurnInterval(new_interval);
            
            unsigned int hours = new_interval / 3600;
            unsigned int minutes = (new_interval % 3600) / 60;
            Serial.printf("[Web] Turn interval updated to: %u hours %u minutes\n", hours, minutes);
            updated = true;
        } else {
            Serial.printf("[Web] Invalid turn interval: %u seconds\n", new_interval);
        }
    }
    
    // Update turn duration (if using relay)
    if (doc.containsKey("turn_duration")) {
        unsigned int new_duration = doc["turn_duration"].as<unsigned int>();
        if (new_duration >= 1 && new_duration <= 60) {
            #ifdef EGGS_TURNER_PIN
            EGGS_TURN_SECONDS = new_duration;
            
            // Save to storage
            configStorage.saveTurnDuration(new_duration);
            
            Serial.printf("[Web] Turn duration updated to: %u seconds\n", new_duration);
            updated = true;
            #endif
        }
    }
    
    return updated;
}

bool WebServerManager::checkAuthentication() {
    // Check if password is enabled
    if (!passwordManager.isPasswordEnabled()) {
        return true; // No password required
    }
    
    // Check session
    if (passwordManager.checkSession()) {
        return true; // Session is valid
    }
    
    return false; // Authentication required
}

bool WebServerManager::executeCommand(const String& command, const JsonDocument& data) {
    // Check authentication for critical commands
    bool requiresAuth = false;
    
    if (command == "turn_now" || command == "stop_turning" || 
        command == "reset_timer" || command == "start_incubation" || 
        command == "stop_incubation" || command == "restart" || 
        command == "factory_reset" || command == "update_password") {
        requiresAuth = true;
    }
    
    if (requiresAuth && !checkAuthentication()) {
        Serial.println("[Web] Authentication required for command: " + command);
        return false;
    }
    
    if (command == "turn_now") {
        // Trigger egg turner immediately
        turner.turn();
        return true;
    } else if (command == "reset_timer") {
        // Reset egg turner timer
        turner.resetTimer();
        return true;
    } else if (command == "restart") {
        // Restart ESP32
        server->send(200, "application/json", "{\"success\":true,\"message\":\"Restarting...\"}");
        delay(1000);
        ESP.restart();
        return true;
    } else if (command == "factory_reset") {
        // Clear all settings
        wifiManager.clearCredentials();
        // Add other reset logic here
        return true;
    } else if (command == "stop_turning") {
        // Stop the egg turner immediately
        turner.stop();
        return true;
    } else if (command == "start_incubation") {
        // Start incubation with specified species
        if (data.containsKey("species")) {
            int species = data["species"].as<int>();
            float new_target_temp;
            unsigned int new_turn_interval;
            
            bool success = incubationTracker.startIncubation(
                static_cast<BirdSpecies>(species), 
                0, // current time
                &new_target_temp, 
                &new_turn_interval
            );
            
            if (success) {
                // Apply the preset values
                targetTemp = new_target_temp;
                EGGS_TURNING_INTERVAL = new_turn_interval;
                
                // Reset the egg turner timer to use the new interval immediately
                turner.resetTimer();
                
                // Save to configuration storage
                configStorage.saveTargetTemperature(new_target_temp);
                configStorage.saveTurnInterval(new_turn_interval);
                
                Serial.printf("[Web] Incubation started. Temp: %.1f°C, Interval: %u seconds\n", 
                             new_target_temp, new_turn_interval);
            }
            
            return success;
        }
        return false;
    } else if (command == "stop_incubation") {
        // Stop current incubation session
        float default_temp;
        unsigned int default_interval;
        
        bool success = incubationTracker.stopIncubation(&default_temp, &default_interval);
        
        if (success) {
            // Restore default values
            targetTemp = default_temp;
            EGGS_TURNING_INTERVAL = default_interval;
            
            // Reset the egg turner timer to use the new interval immediately
            turner.resetTimer();
            
            // Save to configuration storage
            configStorage.saveTargetTemperature(default_temp);
            configStorage.saveTurnInterval(default_interval);
            
            Serial.printf("[Web] Incubation stopped. Restored defaults. Temp: %.1f°C, Interval: %u seconds\n", 
                         default_temp, default_interval);
        }
        
        return success;
    } else if (command == "update_password") {
        // Update password
        if (!data.containsKey("new_password")) {
            Serial.println("[Web] Missing new_password in update_password command");
            return false;
        }
        
        String new_password = data["new_password"].as<String>();
        
        // If current_password is provided, verify it first
        if (data.containsKey("current_password")) {
            String current_password = data["current_password"].as<String>();
            if (!passwordManager.authenticate(current_password)) {
                Serial.println("[Web] Current password verification failed");
                return false;
            }
        } else {
            // If no current password provided, check if we're already authenticated
            if (!checkAuthentication()) {
                Serial.println("[Web] Authentication required for password update");
                return false;
            }
        }
        
        // Set the new password
        bool success = passwordManager.setPassword(new_password);
        if (success) {
            Serial.println("[Web] Password updated successfully");
        } else {
            Serial.println("[Web] Failed to update password");
        }
        
        return success;
    }
    
    return false;
}

String WebServerManager::htmlEncode(const String& str) {
    String encoded = str;
    encoded.replace("&", "&amp;");
    encoded.replace("\"", "&quot;");
    encoded.replace("'", "&#39;");
    encoded.replace("<", "&lt;");
    encoded.replace(">", "&gt;");
    return encoded;
}

#endif // ESP32