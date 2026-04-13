# Incubator ESP32C API - Technical White Paper

## Executive Summary

The Incubator ESP32C Roll Turn system provides a comprehensive REST API for remote monitoring and control of egg incubation parameters. This document outlines the technical specifications, endpoints, and integration guidelines for the API.

## 1. Architecture Overview

### 1.1 System Components
- **ESP32-C3 Microcontroller**: Main processing unit
- **DHT22 Sensor**: Temperature and humidity monitoring
- **OLED Display**: Real-time status display (cycles between turn time and IP)
- **Web Server**: Embedded HTTP server on port 80
- **WiFi Manager**: Network connectivity with hostname `incubator-esp32c`

### 1.2 Network Architecture
```
[Client Device] ←HTTP→ [ESP32 Web Server:80] → [Control Logic] → [Hardware Components]
                    ↑
               [WiFi Network]
```

## 2. API Specification

### 2.1 Base Information
- **Protocol**: HTTP/1.1
- **Port**: 80
- **Authentication**: None (local network only)
- **Response Format**: JSON
- **Character Encoding**: UTF-8

### 2.2 Endpoint Matrix

| Endpoint | Method | Description | Requires Auth |
|----------|--------|-------------|---------------|
| `/` | GET | Web dashboard interface | No |
| `/api/status` | GET | System status | No |
| `/api/config` | GET, POST | Configuration management | No |
| `/api/command` | POST | System commands | No |
| `/api/time` | GET | Time information | No |
| `/api/system` | GET | System information | No |
| `/api/incubation` | GET, POST | Incubation management | No |

## 3. Detailed Endpoint Documentation

### 3.1 Status Endpoint (`GET /api/status`)

**Purpose**: Real-time monitoring of incubator state

**Response Schema**:
```json
{
  "type": "object",
  "properties": {
    "temperature": {"type": "number", "format": "float", "description": "Current temperature in °C"},
    "humidity": {"type": "number", "format": "float", "description": "Current humidity in %"},
    "target_temperature": {"type": "number", "format": "float"},
    "target_humidity": {"type": "number", "format": "float"},
    "heater_on": {"type": "boolean"},
    "fan_on": {"type": "boolean"},
    "light_on": {"type": "boolean"},
    "egg_turner_active": {"type": "boolean"},
    "time_until_next_turn": {"type": "integer", "description": "Seconds until next turn"},
    "incubation_active": {"type": "boolean"},
    "wifi_connected": {"type": "boolean"},
    "ip_address": {"type": "string", "format": "ipv4"},
    "hostname": {"type": "string", "default": "incubator-esp32c"},
    "uptime": {"type": "integer", "description": "Seconds since boot"}
  },
  "required": ["temperature", "humidity", "wifi_connected"]
}
```

### 3.2 Command Endpoint (`POST /api/command`)

**Purpose**: Execute control commands

**Request Schema**:
```json
{
  "type": "object",
  "properties": {
    "command": {
      "type": "string",
      "enum": ["turn_now", "reset_timer", "restart", "factory_reset", "start_incubation", "stop_incubation"]
    },
    "species": {
      "type": "integer",
      "minimum": 0,
      "maximum": 8,
      "description": "Required for start_incubation command"
    }
  },
  "required": ["command"]
}
```

**Command Reference**:

| Command | Parameters | Effect | Response Time |
|---------|------------|--------|---------------|
| `turn_now` | None | Immediate egg turning | < 1s |
| `reset_timer` | None | Reset turn timer to zero | < 100ms |
| `restart` | None | System reboot | 5-10s |
| `factory_reset` | None | Clear all settings | < 1s |
| `start_incubation` | `species` (int) | Start incubation session | < 1s |
| `stop_incubation` | None | Stop incubation session | < 1s |

### 3.3 Incubation Management (`GET/POST /api/incubation`)

**GET Response Schema**:
```json
{
  "session_active": {"type": "boolean"},
  "species": {"type": "string"},
  "elapsed_days": {"type": "integer"},
  "remaining_days": {"type": "integer"},
  "is_candling_day": {"type": "boolean"},
  "is_lockdown_day": {"type": "boolean"},
  "is_hatching_day": {"type": "boolean"},
  "time_remaining": {"type": "string"},
  "target_temp": {"type": "number"},
  "target_humidity": {"type": "number"},
  "incubation_days": {"type": "integer"},
  "candling_day": {"type": "integer"},
  "lockdown_day": {"type": "integer"},
  "turn_interval": {"type": "integer"}
}
```

**POST Actions**:

| Action | Parameters | Description |
|--------|------------|-------------|
| `start` | `species` (int), `start_time` (optional) | Start incubation |
| `stop` | None | Stop incubation |
| `adjust_days` | `days` (int) | Adjust incubation progress |
| `set_day` | `day` (int) | Set specific incubation day |

## 4. Species Configuration

### 4.1 Species Enumeration
```c
enum BirdSpecies {
    BIRD_QUAIL = 0,      // 17-18 days, 37.8°C
    BIRD_CHICKEN = 1,    // 21 days, 37.8°C  
    BIRD_DUCK = 2,       // 28 days, 37.5°C
    BIRD_GOOSE = 3,      // 30-32 days, 37.5°C
    BIRD_PEACOCK = 4,    // 28-30 days, 37.5°C
    BIRD_TURKEY = 5,     // 28 days, 37.5°C
    BIRD_PHEASANT = 6,   // 24-25 days, 37.5°C
    BIRD_GUINEA_FOWL = 7,// 26-28 days, 37.5°C
    BIRD_CUSTOM = 8      // User-defined
};
```

### 4.2 Default Parameters
| Parameter | Range | Default | Units |
|-----------|-------|---------|-------|
| Temperature | 35.0 - 42.0 | 38.0 | °C |
| Humidity | 40.0 - 80.0 | 55.0 | % |
| Turn Interval | 3600 - 86400 | 28800 | seconds |
| Incubation Days | 1 - 45 | 21 | days |

## 5. Network Integration

### 5.1 Hostname Resolution
- **Static Hostname**: `incubator-esp32c`
- **mDNS**: `incubator-esp32c.local` (if supported)
- **IP Discovery**: DHCP with static lease recommended

### 5.2 Network Scanning
```bash
# Using nmap
nmap -p 80 --open 192.168.1.0/24

# Using arp-scan
sudo arp-scan --localnet | grep ESP32

# Using ping with hostname
ping incubator-esp32c.local
```

### 5.3 Connection Examples

**Python**:
```python
import requests
import json

base_url = "http://incubator-esp32c.local"

# Get status
response = requests.get(f"{base_url}/api/status")
status = response.json()

# Start incubation
payload = {"command": "start_incubation", "species": 1}
response = requests.post(f"{base_url}/api/command", json=payload)
```

**JavaScript**:
```javascript
async function getIncubatorStatus() {
    const response = await fetch('http://incubator-esp32c.local/api/status');
    return await response.json();
}

async function turnEggsNow() {
    const response = await fetch('http://incubator-esp32c.local/api/command', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({command: 'turn_now'})
    });
    return await response.json();
}
```

**cURL**:
```bash
# Get status
curl http://incubator-esp32c.local/api/status

# Turn eggs now
curl -X POST http://incubator-esp32c.local/api/command \
  -H "Content-Type: application/json" \
  -d '{"command":"turn_now"}'
```

## 6. Error Handling

### 6.1 HTTP Status Codes
- `200 OK`: Request successful
- `400 Bad Request`: Invalid parameters
- `404 Not Found`: Endpoint doesn't exist
- `405 Method Not Allowed`: Wrong HTTP method
- `500 Internal Server Error`: System error

### 6.2 Error Response Format
```json
{
  "success": false,
  "message": "Error description",
  "code": "ERROR_CODE"  // Optional
}
```

### 6.3 Common Error Codes
- `INVALID_JSON`: Malformed JSON in request
- `MISSING_PARAMETER`: Required parameter missing
- `INVALID_SPECIES`: Species value out of range
- `INCUBATION_ACTIVE`: Incubation already running
- `NO_INCUBATION`: No active incubation session

## 7. Performance Characteristics

### 7.1 Response Times
- **Status endpoint**: < 50ms
- **Command execution**: < 100ms (except restart)
- **Web interface load**: < 500ms
- **Data update interval**: 1 second (real-time)

### 7.2 Resource Usage
- **Memory footprint**: ~30KB (web server)
- **CPU usage**: < 5% (idle), < 20% (active)
- **Network bandwidth**: ~1KB/s (status updates)

### 7.3 Concurrent Connections
- **Maximum clients**: 5 concurrent connections
- **Request queue**: 10 pending requests
- **Timeout**: 30 seconds per request

## 8. Security Considerations

### 8.1 Assumptions
1. Network is trusted (home/private network)
2. Physical access is controlled
3. No sensitive data transmitted

### 8.2 Limitations
- No encryption (HTTP only)
- No authentication
- No rate limiting
- No input validation beyond JSON parsing

### 8.3 Mitigations
- Local network only operation
- Factory reset capability
- Command validation
- Safe default values

## 9. Integration Guidelines

### 9.1 Best Practices
1. **Polling Interval**: Minimum 5 seconds for status updates
2. **Error Handling**: Implement retry logic with exponential backoff
3. **Connection Management**: Close connections promptly
4. **Data Validation**: Verify response schema before use

### 9.2 Monitoring Recommendations
- Monitor `/api/status` every 30 seconds
- Alert on temperature/humidity out of range
- Log all command executions
- Track system uptime and reboots

### 9.3 Failure Scenarios
1. **Device offline**: Implement ping/heartbeat monitoring
2. **Sensor failure**: Check for stale temperature data
3. **Network issues**: Implement fallback to AP mode detection
4. **Power loss**: Consider UPS for critical applications

## 10. Future Enhancements

### 10.1 Planned Features
- HTTPS support with self-signed certificates
- Basic authentication
- WebSocket for real-time updates
- Historical data logging
- Remote firmware updates

### 10.2 API Extensions
- Bulk command execution
- Scheduled operations
- Alert configuration
- Data export endpoints

## 11. Support and Maintenance

### 11.1 Diagnostics
- System logs via serial interface
- Web-based diagnostic page
- Network connectivity tests
- Sensor calibration tools

### 11.2 Recovery Procedures
1. **Factory Reset**: Hold button during boot
2. **AP Mode**: Connect to "Incubator-Config" WiFi
3. **Serial Recovery**: USB connection for advanced debugging

---

**Document Version**: 1.0  
**Last Updated**: 2026-04-13  
**API Version**: 2.0.0  
**Contact**: System Administrator