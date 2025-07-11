# Advanced Generic CLI Example

This advanced example demonstrates sophisticated usage of the GenericCLI library for creating professional IoT device interfaces. It showcases enterprise-level features including configuration management, data logging, JSON output, and comprehensive system management.

## Advanced Features Demonstrated

### 🔧 Configuration Management
- **Persistent Storage**: Configuration saved to EEPROM with checksums
- **Dynamic Updates**: Change settings at runtime
- **Validation**: Input validation with proper error handling
- **JSON Support**: Export/import configuration in JSON format

### 📊 Data Management
- **Sensor Simulation**: Realistic temperature, humidity, pressure, and light sensors
- **Data Logging**: Circular buffer for efficient memory usage
- **Export Capabilities**: JSON and CSV export formats
- **Real-time Updates**: Configurable sampling intervals

### 📝 System Logging
- **Structured Logging**: Level-based logging system
- **Log Rotation**: Automatic log rotation to prevent memory overflow
- **Multiple Outputs**: Console and structured log viewing

### ⚙️ Task Management
- **Task Scheduling**: Framework for managing background tasks
- **System Monitoring**: Built-in system health monitoring
- **Custom Tasks**: Extensible task creation system

## Hardware Requirements

- ESP32 development board with at least 4MB flash
- USB cable for programming and communication
- Optional: External sensors (the example uses simulated data)

## Software Requirements

- PlatformIO IDE or Core
- ArduinoJson library (automatically installed)
- Terminal with ANSI support for best experience

## Installation

1. **Copy the example:**
   ```bash
   cp -r path/to/Arduino.GenericCli/examples/advanced_cli_example .
   cd advanced_cli_example
   ```

2. **Build and upload:**
   ```bash
   pio run --target upload
   pio device monitor
   ```

## Quick Start Guide

### 1. Initial Configuration
```bash
# View current configuration
config

# Set device name
config set name "MyIoTDevice"

# Configure WiFi
config set ssid "YourWiFiNetwork"
config set password "YourPassword"
config set auto_connect true

# Set sensor update interval (5 seconds)
config set interval 5000

# Enable JSON output for API-like responses
config set json true
```

### 2. Sensor Data Management
```bash
# Start sensor data collection
sensor start

# View current sensor readings
sensor

# Export last 20 readings as JSON
sensor export json --count=20

# Export as CSV
sensor export csv --count=50

# Clear all sensor data
sensor clear
```

### 3. System Monitoring
```bash
# View system logs
log

# Add a custom log entry
log add "System maintenance started" --level=INFO

# View last 25 log entries
log --count=25

# Clear logs
log clear
```

## Advanced Command Reference

### Configuration Commands

#### View Configuration
```bash
config              # Show all settings in human-readable format
config --json       # Show configuration as JSON
```

#### Modify Settings
```bash
config set <key> <value>

# Available keys:
config set name "DeviceName"           # Device identifier
config set ssid "WiFiNetwork"         # WiFi network name
config set password "WiFiPassword"    # WiFi password
config set auto_connect true|false    # Auto-connect on boot
config set interval 1000-60000        # Sensor interval in ms
config set json true|false           # Default output format
config set log_level 0-4             # Logging verbosity
```

#### Reset Configuration
```bash
config reset        # Reset all settings to defaults
```

### Sensor Management Commands

#### Basic Operations
```bash
sensor              # Show current readings
sensor --json       # Show readings in JSON format
sensor start        # Start data collection
sensor stop         # Stop data collection
sensor clear        # Clear all collected data
```

#### Data Export
```bash
sensor export json [--count=N]    # Export as JSON (default: 10 readings)
sensor export csv [--count=N]     # Export as CSV
```

**JSON Export Example:**
```json
{
  "readings": [
    {
      "timestamp": 1234567890,
      "temperature": 23.45,
      "humidity": 65.2,
      "pressure": 1013.25,
      "light_level": 2048
    }
  ]
}
```

**CSV Export Example:**
```csv
timestamp,temperature,humidity,pressure,light_level
1234567890,23.45,65.2,1013.25,2048
1234567950,23.52,65.1,1013.30,2055
```

### Task Management Commands

#### Task Operations
```bash
task list           # Show all system tasks
task run sensor_test # Run a specific test task
```

**Sample Output:**
```
=== Active Tasks ===
1. Sensor Data Collection - Running
2. WiFi Monitor - Connected  
3. System Monitor - Running
```

### Logging Commands

#### View Logs
```bash
log                 # Show recent log entries
log --count=25      # Show last 25 entries
```

#### Manage Logs
```bash
log add "Custom message" --level=INFO    # Add custom log entry
log clear                                # Clear all logs
```

**Log Levels:**
- `ERROR` (1): Critical errors only
- `WARN` (2): Warnings and errors
- `INFO` (3): Informational messages (default)
- `DEBUG` (4): Detailed debugging information

## Configuration Details

### Device Configuration Structure
```cpp
struct DeviceConfig {
    char deviceName[32];        // Device identifier
    char wifiSSID[64];         // WiFi network name  
    char wifiPassword[64];     // WiFi password
    bool autoConnect;          // Auto-connect on boot
    uint16_t sensorInterval;   // Sampling interval (ms)
    bool jsonOutput;           // Default output format
    uint8_t logLevel;          // Logging verbosity (0-4)
};
```

### Persistent Storage
- Configuration automatically saved to EEPROM
- Checksum validation prevents corruption
- Automatic fallback to defaults if invalid
- Survives power cycles and resets

### Memory Management
- Efficient circular buffer for sensor data
- Automatic log rotation (50 entries max)
- Fixed memory allocation prevents fragmentation
- Configurable data retention limits

## Integration Examples

### REST API Integration
The JSON output format makes it easy to integrate with web APIs:

```bash
# Get sensor data for API
sensor --json

# Response can be directly forwarded to REST endpoints
```

### Configuration via Serial
```bash
# Batch configuration script
config set name "ProductionDevice01"
config set ssid "ProductionWiFi" 
config set password "SecurePassword123"
config set auto_connect true
config set interval 10000
config set json true
reboot
```

### Data Export for Analysis
```bash
# Export 100 readings for data analysis
sensor export csv --count=100 > sensor_data.csv

# Export configuration
config --json > device_config.json
```

## Customization Guide

### Adding Custom Sensors
```cpp
void handleCustomSensorCommand(const CLIArgs& args) {
    // Read from actual hardware sensors
    float temperature = readTemperatureSensor();
    float humidity = readHumiditySensor();
    
    if (config.jsonOutput || args.hasFlag("json")) {
        DynamicJsonDocument doc(256);
        doc["temperature"] = temperature;
        doc["humidity"] = humidity;
        
        String output;
        serializeJson(doc, output);
        Serial.println(output);
    } else {
        cli.printInfo("Custom Sensor Readings:");
        Serial.println("Temperature: " + String(temperature) + "°C");
        Serial.println("Humidity: " + String(humidity) + "%");
    }
}

// Register in setup()
cli.registerCommand("mysensor", "Read custom sensors", 
                   "mysensor [--json]", 
                   handleCustomSensorCommand, "Hardware");
```

### Adding Configuration Parameters
```cpp
// Add to DeviceConfig struct
struct DeviceConfig {
    // ...existing fields...
    uint16_t customTimeout;
    bool customFeatureEnabled;
};

// Handle in config command
if (key == "timeout") {
    int timeout = value.toInt();
    if (timeout >= 1000 && timeout <= 30000) {
        config.customTimeout = timeout;
        changed = true;
    }
}
```

### Custom Data Export Formats
```cpp
else if (format == "xml") {
    Serial.println("<?xml version=\"1.0\"?>");
    Serial.println("<sensor_data>");
    
    for (int i = 0; i < count; i++) {
        int idx = (sensorDataIndex - count + i + MAX_SENSOR_READINGS) % MAX_SENSOR_READINGS;
        if (sensorData[idx].timestamp == 0) continue;
        
        Serial.println("  <reading>");
        Serial.println("    <timestamp>" + String(sensorData[idx].timestamp) + "</timestamp>");
        Serial.println("    <temperature>" + String(sensorData[idx].temperature) + "</temperature>");
        Serial.println("  </reading>");
    }
    
    Serial.println("</sensor_data>");
}
```

## Performance Considerations

### Memory Usage
- **EEPROM**: ~512 bytes for configuration
- **RAM**: ~4KB for sensor data buffer
- **Flash**: ~50KB additional for JSON library

### Processing Overhead
- CLI processing: ~1ms per update cycle
- Sensor simulation: ~0.1ms per reading
- JSON serialization: ~10ms for large exports

### Optimization Tips
1. **Adjust Buffer Sizes**: Reduce `MAX_SENSOR_READINGS` for memory-constrained devices
2. **Optimize JSON**: Use StaticJsonDocument for fixed-size responses
3. **Reduce Logging**: Lower log level in production
4. **Batch Operations**: Export data in chunks for large datasets

## Troubleshooting

### Common Issues

**Configuration Not Persisting:**
- Check EEPROM initialization
- Verify sufficient flash memory
- Ensure proper checksum calculation

**JSON Output Malformed:**
- Increase ArduinoJson buffer size
- Check for special characters in strings
- Validate JSON with online tools

**Memory Issues:**
- Reduce sensor buffer size
- Lower log retention count
- Monitor heap usage with `status` command

**Performance Issues:**
- Increase sensor interval
- Reduce JSON output frequency
- Optimize command handlers

### Debug Commands
```bash
# Check system status
status --verbose

# Monitor memory usage  
mem --detailed

# View system information
sysinfo --verbose

# Test sensor functionality
task run sensor_test
```

## Production Deployment

### Security Considerations
- Change default device names
- Use strong WiFi passwords
- Implement access control for sensitive commands
- Consider encrypted storage for passwords

### Monitoring and Maintenance
```bash
# Regular health checks
status
mem
log --count=10

# Configuration backup
config --json > backup.json

# Sensor data export for analysis
sensor export json --count=1000 > daily_data.json
```

### Automation Scripts
```bash
#!/bin/bash
# Daily maintenance script
echo "config" | nc device_ip 23 > config_backup.json
echo "sensor export csv --count=1440" | nc device_ip 23 > daily_sensors.csv
echo "log --count=100" | nc device_ip 23 > system_logs.txt
```

## License

This example is provided under the same license as the GenericCLI library.
