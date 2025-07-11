/**
 * Advanced Generic CLI Example
 * 
 * This example demonstrates advanced features of the GenericCLI library:
 * - JSON output for commands
 * - Configuration management with EEPROM
 * - Advanced argument validation
 * - Sensor data simulation and logging
 * - Task scheduling and management
 * - Data export capabilities
 * - Custom CLI themes and formatting
 * 
 * This example shows how to build a professional IoT device interface
 * with comprehensive CLI management capabilities.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include "../../src/generic_cli.h"
#include "../../src/cli_standard_commands.h"

// Configuration
#define EEPROM_SIZE 512
#define CONFIG_VERSION 1
#define MAX_SENSOR_READINGS 100

// Global objects
GenericCLI cli;

// Simulated sensor data
struct SensorReading {
    unsigned long timestamp;
    float temperature;
    float humidity;
    float pressure;
    uint16_t lightLevel;
};

// Configuration structure
struct DeviceConfig {
    uint8_t version;
    char deviceName[32];
    char wifiSSID[64];
    char wifiPassword[64];
    bool autoConnect;
    uint16_t sensorInterval;
    bool jsonOutput;
    uint8_t logLevel;
    char checksum;
};

// Global variables
DeviceConfig config;
SensorReading sensorData[MAX_SENSOR_READINGS];
int sensorDataIndex = 0;
unsigned long lastSensorReading = 0;
bool dataLoggingEnabled = false;

// ========================================================================
// CONFIGURATION MANAGEMENT
// ========================================================================

uint8_t calculateChecksum(const DeviceConfig& cfg) {
    uint8_t sum = 0;
    const uint8_t* data = (const uint8_t*)&cfg;
    for (size_t i = 0; i < sizeof(DeviceConfig) - 1; i++) {
        sum += data[i];
    }
    return ~sum + 1; // Two's complement
}

void loadConfiguration() {
    EEPROM.readBytes(0, &config, sizeof(DeviceConfig));
    
    if (config.version != CONFIG_VERSION || 
        config.checksum != calculateChecksum(config)) {
        // Invalid config, load defaults
        cli.printWarning("Invalid configuration found, loading defaults");
        
        config.version = CONFIG_VERSION;
        strcpy(config.deviceName, "ESP32-CLI-Device");
        strcpy(config.wifiSSID, "");
        strcpy(config.wifiPassword, "");
        config.autoConnect = false;
        config.sensorInterval = 5000; // 5 seconds
        config.jsonOutput = false;
        config.logLevel = 2; // INFO level
        config.checksum = calculateChecksum(config);
        
        saveConfiguration();
    }
}

void saveConfiguration() {
    config.checksum = calculateChecksum(config);
    EEPROM.writeBytes(0, &config, sizeof(DeviceConfig));
    EEPROM.commit();
}

// ========================================================================
// SENSOR SIMULATION
// ========================================================================

void updateSensorData() {
    if (millis() - lastSensorReading >= config.sensorInterval && dataLoggingEnabled) {
        SensorReading& reading = sensorData[sensorDataIndex];
        
        // Simulate realistic sensor data
        reading.timestamp = millis();
        reading.temperature = 20.0 + (random(-50, 150) / 10.0); // 15-35°C
        reading.humidity = 45.0 + (random(-200, 300) / 10.0);   // 25-75%
        reading.pressure = 1013.25 + (random(-100, 100) / 10.0); // ±10 hPa
        reading.lightLevel = random(0, 4096); // 12-bit ADC value
        
        sensorDataIndex = (sensorDataIndex + 1) % MAX_SENSOR_READINGS;
        lastSensorReading = millis();
    }
}

// ========================================================================
// ADVANCED COMMAND HANDLERS
// ========================================================================

void handleConfigCommand(const CLIArgs& args) {
    if (args.empty()) {
        // Show current configuration
        if (config.jsonOutput || args.hasFlag("json")) {
            DynamicJsonDocument doc(1024);
            doc["device_name"] = config.deviceName;
            doc["wifi_ssid"] = config.wifiSSID;
            doc["auto_connect"] = config.autoConnect;
            doc["sensor_interval"] = config.sensorInterval;
            doc["json_output"] = config.jsonOutput;
            doc["log_level"] = config.logLevel;
            
            String output;
            serializeJson(doc, output);
            Serial.println(output);
        } else {
            cli.printInfo("=== Device Configuration ===");
            Serial.println("Device Name: " + String(config.deviceName));
            Serial.println("WiFi SSID: " + String(config.wifiSSID));
            Serial.println("Auto Connect: " + String(config.autoConnect ? "Yes" : "No"));
            Serial.println("Sensor Interval: " + String(config.sensorInterval) + "ms");
            Serial.println("JSON Output: " + String(config.jsonOutput ? "Yes" : "No"));
            Serial.println("Log Level: " + String(config.logLevel));
        }
        return;
    }
    
    String action = args.getPositional(0);
    action.toLowerCase();
    
    if (action == "set") {
        if (args.size() < 3) {
            cli.printError("Usage: config set <key> <value>");
            return;
        }
        
        String key = args.getPositional(1);
        String value = args.getPositional(2);
        key.toLowerCase();
        
        bool changed = false;
        
        if (key == "name" || key == "device_name") {
            if (value.length() < 32) {
                strcpy(config.deviceName, value.c_str());
                changed = true;
            } else {
                cli.printError("Device name too long (max 31 characters)");
                return;
            }
        } else if (key == "wifi_ssid" || key == "ssid") {
            if (value.length() < 64) {
                strcpy(config.wifiSSID, value.c_str());
                changed = true;
            } else {
                cli.printError("SSID too long (max 63 characters)");
                return;
            }
        } else if (key == "wifi_password" || key == "password") {
            if (value.length() < 64) {
                strcpy(config.wifiPassword, value.c_str());
                changed = true;
            } else {
                cli.printError("Password too long (max 63 characters)");
                return;
            }
        } else if (key == "auto_connect") {
            value.toLowerCase();
            if (value == "true" || value == "1" || value == "yes") {
                config.autoConnect = true;
                changed = true;
            } else if (value == "false" || value == "0" || value == "no") {
                config.autoConnect = false;
                changed = true;
            } else {
                cli.printError("Invalid boolean value. Use: true/false, 1/0, yes/no");
                return;
            }
        } else if (key == "sensor_interval" || key == "interval") {
            int interval = value.toInt();
            if (interval >= 1000 && interval <= 60000) {
                config.sensorInterval = interval;
                changed = true;
            } else {
                cli.printError("Interval must be between 1000 and 60000 ms");
                return;
            }
        } else if (key == "json_output" || key == "json") {
            value.toLowerCase();
            if (value == "true" || value == "1" || value == "yes") {
                config.jsonOutput = true;
                changed = true;
            } else if (value == "false" || value == "0" || value == "no") {
                config.jsonOutput = false;
                changed = true;
            } else {
                cli.printError("Invalid boolean value");
                return;
            }
        } else if (key == "log_level" || key == "loglevel") {
            int level = value.toInt();
            if (level >= 0 && level <= 4) {
                config.logLevel = level;
                changed = true;
            } else {
                cli.printError("Log level must be 0-4 (0=None, 1=Error, 2=Warn, 3=Info, 4=Debug)");
                return;
            }
        } else {
            cli.printError("Unknown configuration key: " + key);
            cli.printInfo("Available keys: name, ssid, password, auto_connect, interval, json, log_level");
            return;
        }
        
        if (changed) {
            saveConfiguration();
            cli.printSuccess("Configuration updated: " + key + " = " + value);
        }
        
    } else if (action == "reset") {
        // Reset to defaults
        config.version = CONFIG_VERSION;
        strcpy(config.deviceName, "ESP32-CLI-Device");
        strcpy(config.wifiSSID, "");
        strcpy(config.wifiPassword, "");
        config.autoConnect = false;
        config.sensorInterval = 5000;
        config.jsonOutput = false;
        config.logLevel = 2;
        
        saveConfiguration();
        cli.printSuccess("Configuration reset to defaults");
        
    } else {
        cli.printError("Unknown config action: " + action);
        cli.printInfo("Available actions: set, reset, or no action to view current config");
    }
}

void handleSensorCommand(const CLIArgs& args) {
    if (args.empty()) {
        // Show current sensor data
        if (sensorDataIndex == 0 && sensorData[0].timestamp == 0) {
            cli.printWarning("No sensor data available");
            return;
        }
        
        // Get the most recent reading
        int lastIndex = (sensorDataIndex - 1 + MAX_SENSOR_READINGS) % MAX_SENSOR_READINGS;
        const SensorReading& reading = sensorData[lastIndex];
        
        if (config.jsonOutput || args.hasFlag("json")) {
            DynamicJsonDocument doc(512);
            doc["timestamp"] = reading.timestamp;
            doc["temperature"] = reading.temperature;
            doc["humidity"] = reading.humidity;
            doc["pressure"] = reading.pressure;
            doc["light_level"] = reading.lightLevel;
            doc["logging_enabled"] = dataLoggingEnabled;
            
            String output;
            serializeJson(doc, output);
            Serial.println(output);
        } else {
            cli.printInfo("=== Current Sensor Data ===");
            Serial.println("Timestamp: " + String(reading.timestamp) + "ms");
            Serial.println("Temperature: " + String(reading.temperature, 2) + "°C");
            Serial.println("Humidity: " + String(reading.humidity, 1) + "%");
            Serial.println("Pressure: " + String(reading.pressure, 2) + " hPa");
            Serial.println("Light Level: " + String(reading.lightLevel) + " (0-4095)");
            Serial.println("Logging: " + String(dataLoggingEnabled ? "Enabled" : "Disabled"));
        }
        return;
    }
    
    String action = args.getPositional(0);
    action.toLowerCase();
    
    if (action == "start") {
        dataLoggingEnabled = true;
        cli.printSuccess("Sensor logging started");
        
    } else if (action == "stop") {
        dataLoggingEnabled = false;
        cli.printSuccess("Sensor logging stopped");
        
    } else if (action == "clear") {
        // Clear sensor data
        memset(sensorData, 0, sizeof(sensorData));
        sensorDataIndex = 0;
        cli.printSuccess("Sensor data cleared");
        
    } else if (action == "export") {
        // Export sensor data
        String format = args.getPositional(1, "json");
        int count = args.getFlag("count", "10").toInt();
        
        if (count > MAX_SENSOR_READINGS) count = MAX_SENSOR_READINGS;
        if (count < 1) count = 1;
        
        format.toLowerCase();
        
        if (format == "json") {
            DynamicJsonDocument doc(8192);
            JsonArray readings = doc.createNestedArray("readings");
            
            for (int i = 0; i < count; i++) {
                int idx = (sensorDataIndex - count + i + MAX_SENSOR_READINGS) % MAX_SENSOR_READINGS;
                if (sensorData[idx].timestamp == 0) continue;
                
                JsonObject reading = readings.createNestedObject();
                reading["timestamp"] = sensorData[idx].timestamp;
                reading["temperature"] = sensorData[idx].temperature;
                reading["humidity"] = sensorData[idx].humidity;
                reading["pressure"] = sensorData[idx].pressure;
                reading["light_level"] = sensorData[idx].lightLevel;
            }
            
            String output;
            serializeJson(doc, output);
            Serial.println(output);
            
        } else if (format == "csv") {
            Serial.println("timestamp,temperature,humidity,pressure,light_level");
            for (int i = 0; i < count; i++) {
                int idx = (sensorDataIndex - count + i + MAX_SENSOR_READINGS) % MAX_SENSOR_READINGS;
                if (sensorData[idx].timestamp == 0) continue;
                
                Serial.println(String(sensorData[idx].timestamp) + "," +
                             String(sensorData[idx].temperature, 2) + "," +
                             String(sensorData[idx].humidity, 1) + "," +
                             String(sensorData[idx].pressure, 2) + "," +
                             String(sensorData[idx].lightLevel));
            }
        } else {
            cli.printError("Unknown export format: " + format);
            cli.printInfo("Available formats: json, csv");
        }
        
    } else {
        cli.printError("Unknown sensor action: " + action);
        cli.printInfo("Available actions: start, stop, clear, export");
    }
}

void handleTaskCommand(const CLIArgs& args) {
    if (args.empty()) {
        cli.printError("Usage: task <list|create|delete|run> [parameters]");
        return;
    }
    
    String action = args.getPositional(0);
    action.toLowerCase();
    
    if (action == "list") {
        cli.printInfo("=== Active Tasks ===");
        Serial.println("1. Sensor Data Collection - " + String(dataLoggingEnabled ? "Running" : "Stopped"));
        Serial.println("2. WiFi Monitor - " + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected"));
        Serial.println("3. System Monitor - Running");
        
    } else if (action == "create") {
        cli.printInfo("Task creation not implemented in this demo");
        cli.printInfo("This would allow creating custom scheduled tasks");
        
    } else if (action == "delete") {
        cli.printInfo("Task deletion not implemented in this demo");
        
    } else if (action == "run") {
        String taskName = args.getPositional(1);
        if (taskName == "sensor_test") {
            cli.printInfo("Running sensor test task...");
            for (int i = 0; i < 5; i++) {
                updateSensorData();
                delay(1000);
                Serial.println("Test reading " + String(i + 1) + " completed");
            }
            cli.printSuccess("Sensor test completed");
        } else {
            cli.printError("Unknown task: " + taskName);
        }
        
    } else {
        cli.printError("Unknown task action: " + action);
    }
}

void handleLogCommand(const CLIArgs& args) {
    static String logBuffer[50];
    static int logIndex = 0;
    static bool logInitialized = false;
    
    if (!logInitialized) {
        // Initialize with some sample log entries
        logBuffer[logIndex++] = "[INFO] System started";
        logBuffer[logIndex++] = "[INFO] Configuration loaded";
        logBuffer[logIndex++] = "[WARN] WiFi not configured";
        logInitialized = true;
    }
    
    if (args.empty()) {
        // Show recent logs
        int count = 10;
        if (args.hasFlag("count")) {
            count = args.getFlag("count").toInt();
            if (count > 50) count = 50;
            if (count < 1) count = 1;
        }
        
        cli.printInfo("=== Recent Log Entries ===");
        int startIdx = max(0, logIndex - count);
        for (int i = startIdx; i < logIndex; i++) {
            Serial.println(logBuffer[i]);
        }
        return;
    }
    
    String action = args.getPositional(0);
    action.toLowerCase();
    
    if (action == "clear") {
        logIndex = 0;
        cli.printSuccess("Log cleared");
        
    } else if (action == "add") {
        if (args.size() < 2) {
            cli.printError("Usage: log add <message>");
            return;
        }
        
        String level = args.getFlag("level", "INFO");
        level.toUpperCase();
        String message = args.getPositional(1);
        
        if (logIndex >= 50) {
            // Rotate logs
            for (int i = 0; i < 49; i++) {
                logBuffer[i] = logBuffer[i + 1];
            }
            logIndex = 49;
        }
        
        logBuffer[logIndex++] = "[" + level + "] " + message;
        cli.printSuccess("Log entry added");
        
    } else {
        cli.printError("Unknown log action: " + action);
        cli.printInfo("Available actions: clear, add, or no action to view logs");
    }
}

// ========================================================================
// SETUP AND MAIN LOOP
// ========================================================================

void setup() {
    // Initialize EEPROM
    EEPROM.begin(EEPROM_SIZE);
    
    // Load configuration
    loadConfiguration();
    
    // Configure CLI with custom theme
    CLIConfig cliConfig;
    cliConfig.prompt = config.deviceName;
    cliConfig.welcomeMessage = String("\n") +
        "🔧 " + String(config.deviceName) + " - Advanced CLI v2.0\n" +
        "═══════════════════════════════════════════════\n" +
        "🚀 Type 'help' for commands\n" +
        "⚙️  Type 'config' to view/modify settings\n" +
        "📊 Type 'sensor' to manage sensor data\n" +
        "📝 Type 'log' to view system logs\n";
    cliConfig.colorsEnabled = true;
    cliConfig.echoEnabled = true;
    cliConfig.historySize = 30;
    
    cli.setConfig(cliConfig);
    
    // Register standard commands
    CLIStandardCommands::registerAllStandardCommands(cli);
    
    // Register advanced commands
    cli.registerCommand("config", "Manage device configuration", 
                       "config [set <key> <value>] [reset] [--json]", 
                       handleConfigCommand, "Configuration");
    
    cli.registerCommand("sensor", "Sensor data management", 
                       "sensor [start|stop|clear|export] [--json] [--count=n]", 
                       handleSensorCommand, "Data");
    
    cli.registerCommand("task", "Task management", 
                       "task <list|create|delete|run> [parameters]", 
                       handleTaskCommand, "System");
    
    cli.registerCommand("log", "System log management", 
                       "log [clear|add <message>] [--level=LEVEL] [--count=n]", 
                       handleLogCommand, "System");
    
    // Start CLI
    cli.begin();
    
    // Auto-connect to WiFi if configured
    if (config.autoConnect && strlen(config.wifiSSID) > 0) {
        cli.printInfo("Auto-connecting to WiFi: " + String(config.wifiSSID));
        WiFi.begin(config.wifiSSID, config.wifiPassword);
    }
    
    // Initialize random seed for sensor simulation
    randomSeed(analogRead(0));
}

void loop() {
    // Update CLI
    cli.update();
    
    // Update sensor data
    updateSensorData();
    
    // Handle exit
    if (CLIStandardCommands::isExitRequested()) {
        cli.printInfo("CLI stopped. System entering maintenance mode...");
        CLIStandardCommands::resetExitFlag();
        
        while (true) {
            // Maintenance mode - basic functionality only
            updateSensorData();
            delay(1000);
        }
    }
    
    delay(1);
}
