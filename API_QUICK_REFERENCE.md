# Incubator API - Quick Reference Card

## 📍 Device Information
- **Hostname**: `incubator-esp32c`
- **Default IP**: DHCP assigned (check router)
- **Web Interface**: `http://incubator-esp32c.local` or `http://<IP>`
- **Port**: 80

## 🔍 Network Discovery
```bash
# Find device on network
nmap -sn 192.168.1.0/24 | grep -i incubator
ping incubator-esp32c.local
```

## 📊 Status & Monitoring

### GET `/api/status`
**Real-time system status**
```bash
curl http://incubator-esp32c.local/api/status
```
**Key Fields:**
- `temperature`, `humidity`: Current readings
- `target_temperature`, `target_humidity`: Set points
- `time_until_next_turn`: Seconds remaining
- `incubation_active`: true/false
- `ip_address`, `hostname`: Network info

### GET `/api/system`
**System information**
```bash
curl http://incubator-esp32c.local/api/system
```

## ⚙️ Configuration

### GET `/api/config`
**View current configuration**
```bash
curl http://incubator-esp32c.local/api/config
```

### POST `/api/config`
**Update configuration**
```bash
curl -X POST http://incubator-esp32c.local/api/config \
  -H "Content-Type: application/json" \
  -d '{"target_temp": 37.8, "turn_interval": 14400}'
```

## 🎛️ Commands

### POST `/api/command`
**Execute control commands**

| Command | Parameters | Description |
|---------|------------|-------------|
| `turn_now` | None | Turn eggs immediately |
| `reset_timer` | None | Reset turn timer |
| `restart` | None | Restart system |
| `factory_reset` | None | Clear all settings |
| `start_incubation` | `species` (0-8) | Start incubation |
| `stop_incubation` | None | Stop incubation |

**Example:**
```bash
# Turn eggs now
curl -X POST http://incubator-esp32c.local/api/command \
  -H "Content-Type: application/json" \
  -d '{"command":"turn_now"}'

# Start chicken incubation
curl -X POST http://incubator-esp32c.local/api/command \
  -H "Content-Type: application/json" \
  -d '{"command":"start_incubation","species":1}'
```

## 🐣 Incubation Management

### GET `/api/incubation`
**Get incubation status**
```bash
curl http://incubator-esp32c.local/api/incubation
```

### POST `/api/incubation`
**Manage incubation sessions**

| Action | Parameters | Use Case |
|--------|------------|----------|
| `start` | `species` (0-8), `start_time` (optional) | Start new session |
| `stop` | None | Stop current session |
| `adjust_days` | `days` (int) | Adjust progress (± days) |
| `set_day` | `day` (int) | Set specific day |

**Examples:**
```bash
# Start duck incubation (species 2)
curl -X POST http://incubator-esp32c.local/api/incubation \
  -H "Content-Type: application/json" \
  -d '{"action":"start","species":2}'

# Add 3 days to current progress
curl -X POST http://incubator-esp32c.local/api/incubation \
  -H "Content-Type: application/json" \
  -d '{"action":"adjust_days","days":3}'

# Set to day 10 (for transferred eggs)
curl -X POST http://incubator-esp32c.local/api/incubation \
  -H "Content-Type: application/json" \
  -d '{"action":"set_day","day":10}'
```

## 🕐 Time Information

### GET `/api/time`
**Get current time**
```bash
curl http://incubator-esp32c.local/api/time
```

## 🐦 Species Reference

| ID | Species | Days | Temp (°C) |
|----|---------|------|-----------|
| 0 | Quail | 17-18 | 37.8 |
| 1 | Chicken | 21 | 37.8 |
| 2 | Duck | 28 | 37.5 |
| 3 | Goose | 30-32 | 37.5 |
| 4 | Peacock | 28-30 | 37.5 |
| 5 | Turkey | 28 | 37.5 |
| 6 | Pheasant | 24-25 | 37.5 |
| 7 | Guinea Fowl | 26-28 | 37.5 |
| 8 | Custom | User-defined | User-defined |

## 📱 Web Dashboard
Access full web interface at: `http://incubator-esp32c.local`

**Features:**
- Real-time graphs
- One-click controls
- Species selection
- Day adjustment tools
- Network information

## ⚠️ Error Handling

**All endpoints return JSON:**
```json
{
  "success": true|false,
  "message": "Description"
}
```

**HTTP Status Codes:**
- `200`: Success
- `400`: Bad request (check parameters)
- `404`: Endpoint not found
- `405`: Wrong HTTP method
- `500`: System error

## 🔧 Troubleshooting

1. **Device not found?**
   - Check power and WiFi LED
   - Use network scanner: `nmap -sn 192.168.1.0/24`
   - Try AP mode: Connect to "Incubator-Config" WiFi

2. **API not responding?**
   - Check web interface first: `http://<IP>`
   - Verify device is on same network
   - Try restart: `{"command":"restart"}`

3. **Time wrong?**
   - Device needs internet for NTP
   - Allow 5 minutes after WiFi connect
   - Check `/api/time` for sync status

## 📞 Quick Examples

**Python:**
```python
import requests
base = "http://incubator-esp32c.local"
status = requests.get(f"{base}/api/status").json()
print(f"Temp: {status['temperature']}°C")
```

**JavaScript:**
```javascript
fetch('http://incubator-esp32c.local/api/status')
  .then(r => r.json())
  .then(data => console.log(data));
```

**Home Assistant (RESTful Sensor):**
```yaml
sensor:
  - platform: rest
    name: Incubator Temperature
    resource: http://incubator-esp32c.local/api/status
    value_template: "{{ value_json.temperature }}"
    unit_of_measurement: "°C"
```

---

**Version**: API v2.0.0  
**Hostname**: incubator-esp32c  
**Default Port**: 80  
**Format**: JSON  
**Updated**: 2026-04-13