# Incubator ESP32C Roll Turn - API Documentation

## Overview

This document provides comprehensive API documentation for the ESP32-based egg incubator controller. The system provides a RESTful API over HTTP for monitoring and controlling the incubator.

## Base URL

```
http://<incubator-ip-address>/
```

**Note:** The device hostname is `incubator-esp32c` for easy network discovery.

## API Endpoints

### 1. System Status
**Endpoint:** `GET /api/status`

**Description:** Get current system status including temperature, humidity, and operational state.

**Response:**
```json
{
  "temperature": 37.8,
  "humidity": 55.2,
  "target_temperature": 38.0,
  "target_humidity": 55.0,
  "heater_on": true,
  "fan_on": true,
  "light_on": false,
  "egg_turner_active": false,
  "time_until_next_turn": 28740,
  "incubation_active": true,
  "wifi_connected": true,
  "ip_address": "192.168.1.100",
  "hostname": "incubator-esp32c",
  "uptime": 86400
}
```

### 2. System Configuration
**Endpoint:** `GET /api/config`

**Description:** Get current system configuration.

**Response:**
```json
{
  "target_temp": 38.0,
  "turn_interval": 28800,
  "heater_hysteresis": 0.3,
  "humidity_hysteresis": 2.0,
  "max_heater_on_time": 300,
  "min_fan_off_time": 60
}
```

**Endpoint:** `POST /api/config`

**Description:** Update system configuration.

**Request Body:**
```json
{
  "target_temp": 37.8,
  "turn_interval": 14400
}
```

**Response:**
```json
{
  "success": true,
  "message": "Configuration updated"
}
```

### 3. System Commands
**Endpoint:** `POST /api/command`

**Description:** Execute system commands.

**Available Commands:**

#### a) Turn Eggs Now
```json
{
  "command": "turn_now"
}
```
**Description:** Immediately triggers the egg turner.

#### b) Reset Timer
```json
{
  "command": "reset_timer"
}
```
**Description:** Resets the egg turner timer to start counting from zero.

#### c) Restart System
```json
{
  "command": "restart"
}
```
**Description:** Restarts the ESP32 microcontroller.

#### d) Factory Reset
```json
{
  "command": "factory_reset"
}
```
**Description:** Clears all WiFi credentials and resets to factory defaults.

#### e) Start Incubation
```json
{
  "command": "start_incubation",
  "species": 1
}
```
**Description:** Starts incubation with specified bird species.

**Species Codes:**
- `0` - Quail
- `1` - Chicken
- `2` - Duck
- `3` - Goose
- `4` - Peacock
- `5` - Turkey
- `6` - Pheasant
- `7` - Guinea Fowl
- `8` - Custom

#### f) Stop Incubation
```json
{
  "command": "stop_incubation"
}
```
**Description:** Stops the current incubation session and restores default settings.

**Response for all commands:**
```json
{
  "success": true,
  "message": "Command executed"
}
```

### 4. Time Information
**Endpoint:** `GET /api/time`

**Description:** Get current time and synchronization status.

**Response:**
```json
{
  "current_time": "14:30:45",
  "current_date": "2026-04-13",
  "time_synced": true,
  "timestamp": 1744554645
}
```

### 5. System Information
**Endpoint:** `GET /api/system`

**Description:** Get detailed system information.

**Response:**
```json
{
  "firmware_version": "2.0.0",
  "chip_model": "ESP32-C3",
  "chip_revision": 3,
  "cpu_freq_mhz": 160,
  "free_heap": 123456,
  "min_free_heap": 120000,
  "max_alloc_heap": 130000,
  "psram_size": 0,
  "flash_size": 4194304,
  "sdk_version": "v5.1.2",
  "compiled_date": __DATE__,
  "compiled_time": __TIME__
}
```

### 6. Incubation Management
**Endpoint:** `GET /api/incubation`

**Description:** Get current incubation status.

**Response:**
```json
{
  "session_active": true,
  "species": "Chicken",
  "elapsed_days": 7,
  "remaining_days": 14,
  "is_candling_day": true,
  "is_lockdown_day": false,
  "is_hatching_day": false,
  "time_remaining": "14 days, 3 hours",
  "target_temp": 37.8,
  "target_humidity": 55.0,
  "incubation_days": 21,
  "candling_day": 7,
  "lockdown_day": 18,
  "turn_interval": 14400
}
```

**Endpoint:** `POST /api/incubation`

**Description:** Manage incubation sessions.

#### a) Start Incubation
```json
{
  "action": "start",
  "species": 1,
  "start_time": 1744554645
}
```
**Note:** `start_time` is optional (Unix timestamp). If not provided, uses current time.

#### b) Stop Incubation
```json
{
  "action": "stop"
}
```

#### c) Adjust Incubation Days
```json
{
  "action": "adjust_days",
  "days": 3
}
```
**Description:** Add or subtract days from current incubation progress. Useful for eggs transferred from another incubator.

#### d) Set Specific Day
```json
{
  "action": "set_day",
  "day": 10
}
```
**Description:** Directly set incubation to a specific day number.

**Response:**
```json
{
  "success": true,
  "message": "Incubation session started"
}
```

## Web Interface

### Main Dashboard
**Endpoint:** `GET /`

**Description:** Web-based control interface with real-time monitoring.

**Features:**
- Real-time temperature and humidity graphs
- Incubation status and controls
- Egg turner controls
- System configuration
- Network information with hostname display

## Network Discovery

The device uses the hostname `incubator-esp32c` for easy network discovery. You can find it using:

```bash
# Using nmap
nmap -sn 192.168.1.0/24 | grep incubator

# Using ping
ping incubator-esp32c.local
```

## Error Handling

All API endpoints return appropriate HTTP status codes:

- `200` - Success
- `400` - Bad Request (invalid parameters)
- `404` - Not Found
- `405` - Method Not Allowed
- `500` - Internal Server Error

Error responses include a JSON object with error details:
```json
{
  "success": false,
  "message": "Error description"
}
```

## OLED Display Information

The OLED display cycles between two pieces of information:

1. **Remaining time until next egg turn** (in HH:MM:SS format)
2. **IP address** of the device (e.g., "192.168.1.100")

The display cycles every 5 seconds.

## Species-Specific Incubation Parameters

Each bird species has predefined incubation parameters:

| Species | Days | Temp (°C) | Humidity (%) | Candling Day | Lockdown Day | Turn Interval (hours) |
|---------|------|-----------|--------------|--------------|--------------|----------------------|
| Quail | 17-18 | 37.8 | 55-60 | 7 | 14 | 4 |
| Chicken | 21 | 37.8 | 55-60 | 7 | 18 | 4 |
| Duck | 28 | 37.5 | 55-65 | 7 | 25 | 4 |
| Goose | 30-32 | 37.5 | 55-65 | 7 | 25 | 4 |
| Turkey | 28 | 37.5 | 55-60 | 7 | 25 | 4 |
| Custom | User-defined | User-defined | User-defined | User-defined | User-defined | User-defined |

## Security Notes

1. The web interface is accessible only on the local network
2. No authentication is implemented (suitable for home/private networks)
3. Factory reset clears all stored credentials
4. The device creates a WiFi access point (`Incubator-Config`) for initial setup

## Troubleshooting

### Common Issues:

1. **Device not found on network**
   - Check if WiFi is connected (LED indicators)
   - Verify hostname: `incubator-esp32c`
   - Use network scanner tools

2. **API returns 404**
   - Ensure device is powered on
   - Check IP address in web browser first
   - Verify endpoint URLs are correct

3. **Time not synchronized**
   - Device needs internet connection for NTP
   - Check WiFi connectivity
   - Allow up to 5 minutes for time sync

## Version History

- **v2.0.0** (Current): Added hostname support, adjustable incubation days, OLED IP cycling
- **v1.0.0**: Initial release with basic incubation controls

## Support

For issues or questions, refer to the project documentation or contact the development team.