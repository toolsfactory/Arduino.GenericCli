/**
 * Generic CLI Arduino IDE Example
 * 
 * This example demonstrates how to use the GenericCLI library in Arduino IDE.
 * It provides a simplified but feature-rich CLI interface suitable for
 * learning and prototyping.
 * 
 * Features included:
 * - Basic CLI setup with standard commands
 * - LED control with multiple modes
 * - GPIO pin operations
 * - System information display
 * - WiFi network management
 * - Real-time sensor simulation
 * - Memory usage monitoring
 * 
 * Hardware Requirements:
 * - ESP32 development board
 * - Built-in LED (typically GPIO 2)
 * - USB cable for serial communication
 * 
 * Arduino IDE Setup:
 * 1. Install ESP32 board package in Arduino IDE
 * 2. Copy the GenericCLI library files to your Arduino libraries folder:
 *    - Copy 'src/generic_cli.h' and 'src/generic_cli.cpp' 
 *    - Copy 'src/cli_standard_commands.h' and 'src/cli_standard_commands.cpp'
 *    Or create a library folder structure as described in the README
 * 3. Select your ESP32 board and COM port
 * 4. Upload this sketch
 * 5. Open Serial Monitor at 115200 baud
 * 
 * Usage:
 * - Type 'help' to see all available commands
 * - Use arrow keys for command history (in compatible terminals)
 * - Commands are case-insensitive
 * 
 * Author: GenericCLI Example
 * Version: 1.0
 * Date: July 2025
 */

#include <WiFi.h>

// Include the GenericCLI library files
// Make sure these files are in your Arduino libraries folder
#include "generic_cli.h"
#include "cli_standard_commands.h"

// Pin definitions
#define LED_PIN 2
#define ANALOG_PIN A0

// Global variables
GenericCLI cli;
bool ledState = false;
bool sensorMonitoringEnabled = false;
unsigned long lastSensorRead = 0;
const unsigned long SENSOR_INTERVAL = 2000; // 2 seconds

// Simulated sensor data
float temperature = 22.5;
float humidity = 55.0;
int lightLevel = 512;

// ========================================================================
// COMMAND HANDLERS
// ========================================================================

/**
 * LED Control Command
 * Usage: led <on|off|toggle|blink|fade> [count] [delay]
 */
void handleLedCommand(const CLIArgs& args) {
    if (args.empty()) {
        cli.printError("Usage: led <on|off|toggle|blink|fade> [count] [delay]");
        cli.printInfo("Examples:");
        cli.printInfo("  led on");
        cli.printInfo("  led blink 5 200");
        cli.printInfo("  led fade");
        return;
    }
    
    String action = args.getPositional(0);
    action.toLowerCase();
    
    if (action == "on") {
        digitalWrite(LED_PIN, HIGH);
        ledState = true;
        cli.printSuccess("LED turned ON");
        
    } else if (action == "off") {
        digitalWrite(LED_PIN, LOW);
        ledState = false;
        cli.printSuccess("LED turned OFF");
        
    } else if (action == "toggle") {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);
        cli.printSuccess("LED toggled " + String(ledState ? "ON" : "OFF"));
        
    } else if (action == "blink") {
        int count = 3;
        int delayMs = 500;
        
        if (args.size() > 1) {
            count = args.getPositional(1).toInt();
            if (count < 1 || count > 20) {
                cli.printError("Count must be between 1 and 20");
                return;
            }
        }
        
        if (args.size() > 2) {
            delayMs = args.getPositional(2).toInt();
            if (delayMs < 50 || delayMs > 2000) {
                cli.printError("Delay must be between 50 and 2000 ms");
                return;
            }
        }
        
        cli.printInfo("Blinking LED " + String(count) + " times with " + String(delayMs) + "ms delay");
        
        bool originalState = ledState;
        for (int i = 0; i < count; i++) {
            digitalWrite(LED_PIN, HIGH);
            delay(delayMs);
            digitalWrite(LED_PIN, LOW);
            delay(delayMs);
        }
        
        // Restore original state
        digitalWrite(LED_PIN, originalState ? HIGH : LOW);
        cli.printSuccess("Blink sequence completed");
        
    } else if (action == "fade") {
        cli.printInfo("Fading LED (if PWM capable pin)");
        
        // Fade in
        for (int i = 0; i <= 255; i += 5) {
            analogWrite(LED_PIN, i);
            delay(20);
        }
        
        // Fade out
        for (int i = 255; i >= 0; i -= 5) {
            analogWrite(LED_PIN, i);
            delay(20);
        }
        
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);
        cli.printSuccess("Fade effect completed");
        
    } else {
        cli.printError("Unknown LED action: " + action);
        cli.printInfo("Available actions: on, off, toggle, blink, fade");
    }
}

/**
 * GPIO Command - Read from or write to GPIO pins
 * Usage: gpio <pin> <read|write> [value]
 */
void handleGpioCommand(const CLIArgs& args) {
    if (args.size() < 2) {
        cli.printError("Usage: gpio <pin> <read|write> [value]");
        cli.printInfo("Examples:");
        cli.printInfo("  gpio 4 read");
        cli.printInfo("  gpio 5 write high");
        cli.printInfo("  gpio 13 write 0");
        return;
    }
    
    int pin = args.getPositional(0).toInt();
    String operation = args.getPositional(1);
    operation.toLowerCase();
    
    // Basic pin validation
    if (pin < 0 || pin > 39) {
        cli.printError("Invalid pin number. ESP32 has pins 0-39");
        return;
    }
    
    // Warn about flash pins
    if (pin >= 6 && pin <= 11) {
        cli.printWarning("Pin " + String(pin) + " is connected to flash memory - use with caution");
    }
    
    if (operation == "read") {
        pinMode(pin, INPUT);
        int value = digitalRead(pin);
        cli.printSuccess("GPIO" + String(pin) + " = " + String(value) + " (" + 
                        String(value ? "HIGH" : "LOW") + ")");
                        
        // Also show analog value if applicable
        if (pin >= 32 && pin <= 39) {
            int analogValue = analogRead(pin);
            Serial.println("Analog value: " + String(analogValue) + " (0-4095)");
        }
        
    } else if (operation == "write") {
        if (args.size() < 3) {
            cli.printError("Usage: gpio <pin> write <0|1|high|low>");
            return;
        }
        
        String valueStr = args.getPositional(2);
        valueStr.toLowerCase();
        int value;
        
        if (valueStr == "1" || valueStr == "high" || valueStr == "on") {
            value = HIGH;
        } else if (valueStr == "0" || valueStr == "low" || valueStr == "off") {
            value = LOW;
        } else {
            cli.printError("Invalid value. Use: 0, 1, high, low, on, off");
            return;
        }
        
        pinMode(pin, OUTPUT);
        digitalWrite(pin, value);
        cli.printSuccess("GPIO" + String(pin) + " set to " + String(value) + 
                        " (" + String(value ? "HIGH" : "LOW") + ")");
                        
    } else {
        cli.printError("Unknown GPIO operation: " + operation);
        cli.printInfo("Available operations: read, write");
    }
}

/**
 * System Information Command
 * Usage: info [verbose]
 */
void handleInfoCommand(const CLIArgs& args) {
    bool verbose = (args.size() > 0 && args.getPositional(0) == "verbose");
    
    cli.printInfo("=== ESP32 System Information ===");
    
    // Basic information
    Serial.println("Chip: " + String(ESP.getChipModel()));
    Serial.println("Cores: " + String(ESP.getChipCores()));
    Serial.println("Frequency: " + String(ESP.getCpuFreqMHz()) + " MHz");
    Serial.println("Revision: " + String(ESP.getChipRevision()));
    
    // Memory information
    Serial.println("Free Heap: " + String(ESP.getFreeHeap() / 1024) + " KB");
    Serial.println("Total Heap: " + String(ESP.getHeapSize() / 1024) + " KB");
    Serial.println("Flash Size: " + String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB");
    
    if (verbose) {
        Serial.println();
        cli.printInfo("=== Detailed Information ===");
        
        // Uptime calculation
        unsigned long uptime = millis() / 1000;
        unsigned long hours = uptime / 3600;
        unsigned long minutes = (uptime % 3600) / 60;
        unsigned long seconds = uptime % 60;
        
        Serial.println("Uptime: " + String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s");
        Serial.println("SDK Version: " + String(ESP.getSdkVersion()));
        Serial.println("WiFi MAC: " + WiFi.macAddress());
        
        // Temperature reading (if supported)
        #ifdef CONFIG_IDF_TARGET_ESP32
        float temp = (temprature_sens_read() - 32) / 1.8;
        Serial.println("CPU Temperature: " + String(temp, 1) + "°C");
        #endif
    }
}

/**
 * WiFi Management Command
 * Usage: wifi <scan|connect|disconnect|status> [ssid] [password]
 */
void handleWiFiCommand(const CLIArgs& args) {
    if (args.empty()) {
        cli.printError("Usage: wifi <scan|connect|disconnect|status> [ssid] [password]");
        return;
    }
    
    String action = args.getPositional(0);
    action.toLowerCase();
    
    if (action == "scan") {
        cli.printInfo("Scanning for WiFi networks...");
        WiFi.mode(WIFI_STA);
        int n = WiFi.scanNetworks();
        
        if (n == 0) {
            cli.printWarning("No networks found");
        } else {
            cli.printSuccess("Found " + String(n) + " networks:");
            Serial.println();
            
            for (int i = 0; i < n; i++) {
                String security = "";
                switch (WiFi.encryptionType(i)) {
                    case WIFI_AUTH_OPEN: security = "Open"; break;
                    case WIFI_AUTH_WEP: security = "WEP"; break;
                    case WIFI_AUTH_WPA_PSK: security = "WPA"; break;
                    case WIFI_AUTH_WPA2_PSK: security = "WPA2"; break;
                    case WIFI_AUTH_WPA_WPA2_PSK: security = "WPA/WPA2"; break;
                    default: security = "Unknown"; break;
                }
                
                Serial.println("  " + String(i + 1) + ". " + WiFi.SSID(i) + 
                             " (RSSI: " + String(WiFi.RSSI(i)) + "dBm, " + security + ")");
            }
        }
        
    } else if (action == "connect") {
        if (args.size() < 2) {
            cli.printError("Usage: wifi connect <ssid> [password]");
            return;
        }
        
        String ssid = args.getPositional(1);
        String password = "";
        if (args.size() > 2) {
            password = args.getPositional(2);
        }
        
        cli.printInfo("Connecting to: " + ssid);
        WiFi.mode(WIFI_STA);
        
        if (password.length() > 0) {
            WiFi.begin(ssid.c_str(), password.c_str());
        } else {
            WiFi.begin(ssid.c_str());
        }
        
        // Wait for connection with timeout
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        Serial.println();
        
        if (WiFi.status() == WL_CONNECTED) {
            cli.printSuccess("Connected to " + ssid);
            Serial.println("IP Address: " + WiFi.localIP().toString());
        } else {
            cli.printError("Failed to connect to " + ssid);
        }
        
    } else if (action == "disconnect") {
        WiFi.disconnect();
        cli.printInfo("Disconnected from WiFi");
        
    } else if (action == "status") {
        if (WiFi.status() == WL_CONNECTED) {
            cli.printSuccess("WiFi Status: Connected");
            Serial.println("SSID: " + WiFi.SSID());
            Serial.println("IP: " + WiFi.localIP().toString());
            Serial.println("RSSI: " + String(WiFi.RSSI()) + " dBm");
        } else {
            cli.printWarning("WiFi Status: Disconnected");
        }
        
    } else {
        cli.printError("Unknown WiFi action: " + action);
        cli.printInfo("Available actions: scan, connect, disconnect, status");
    }
}

/**
 * Sensor Monitoring Command
 * Usage: sensor <start|stop|read|auto> [interval]
 */
void handleSensorCommand(const CLIArgs& args) {
    if (args.empty()) {
        // Show current sensor values
        cli.printInfo("=== Current Sensor Readings ===");
        Serial.println("Temperature: " + String(temperature, 1) + "°C");
        Serial.println("Humidity: " + String(humidity, 1) + "%");
        Serial.println("Light Level: " + String(lightLevel) + " (0-1023)");
        Serial.println("Monitoring: " + String(sensorMonitoringEnabled ? "Enabled" : "Disabled"));
        return;
    }
    
    String action = args.getPositional(0);
    action.toLowerCase();
    
    if (action == "start") {
        sensorMonitoringEnabled = true;
        cli.printSuccess("Sensor monitoring started");
        
    } else if (action == "stop") {
        sensorMonitoringEnabled = false;
        cli.printSuccess("Sensor monitoring stopped");
        
    } else if (action == "read") {
        // Force immediate sensor reading
        updateSensorData();
        cli.printInfo("=== Fresh Sensor Reading ===");
        Serial.println("Temperature: " + String(temperature, 1) + "°C");
        Serial.println("Humidity: " + String(humidity, 1) + "%");
        Serial.println("Light Level: " + String(lightLevel));
        
    } else if (action == "auto") {
        // Auto-print sensor values every few seconds
        int duration = 10; // seconds
        if (args.size() > 1) {
            duration = args.getPositional(1).toInt();
            if (duration < 5 || duration > 60) {
                cli.printError("Duration must be between 5 and 60 seconds");
                return;
            }
        }
        
        cli.printInfo("Auto-reading sensors for " + String(duration) + " seconds...");
        cli.printInfo("Press Ctrl+C or send any character to stop");
        
        unsigned long startTime = millis();
        unsigned long nextReading = millis();
        
        while ((millis() - startTime) < (duration * 1000UL)) {
            if (millis() >= nextReading) {
                updateSensorData();
                Serial.println("T:" + String(temperature, 1) + "°C  H:" + 
                             String(humidity, 1) + "%  L:" + String(lightLevel));
                nextReading = millis() + 1000; // Update every second
            }
            
            // Check for user input to stop
            if (Serial.available()) {
                Serial.read(); // Clear input
                cli.printInfo("Auto-reading stopped by user");
                return;
            }
            
            delay(100);
        }
        
        cli.printSuccess("Auto-reading completed");
        
    } else {
        cli.printError("Unknown sensor action: " + action);
        cli.printInfo("Available actions: start, stop, read, auto");
    }
}

// ========================================================================
// HELPER FUNCTIONS
// ========================================================================

void updateSensorData() {
    // Simulate realistic sensor data with some variation
    temperature += (random(-20, 21) / 100.0); // ±0.2°C variation
    if (temperature < 15.0) temperature = 15.0;
    if (temperature > 35.0) temperature = 35.0;
    
    humidity += (random(-50, 51) / 100.0); // ±0.5% variation
    if (humidity < 20.0) humidity = 20.0;
    if (humidity > 80.0) humidity = 80.0;
    
    lightLevel = analogRead(ANALOG_PIN); // Read actual analog value
    
    lastSensorRead = millis();
}

void printSensorUpdate() {
    if (sensorMonitoringEnabled) {
        Serial.println("[SENSOR] T:" + String(temperature, 1) + "°C  H:" + 
                      String(humidity, 1) + "%  L:" + String(lightLevel));
    }
}

// ========================================================================
// SETUP FUNCTION
// ========================================================================

void setup() {
    // Initialize hardware
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Initialize random seed
    randomSeed(analogRead(A0));
    
    // Configure CLI
    CLIConfig config;
    config.prompt = "esp32";
    config.welcomeMessage = "\n🚀 ESP32 Generic CLI - Arduino IDE Example\n"
                           "==========================================\n"
                           "Welcome to the GenericCLI demonstration!\n\n"
                           "📋 Type 'help' to see all available commands\n"
                           "💡 Try 'led blink 3' to test the LED\n"
                           "📡 Use 'wifi scan' to find networks\n"
                           "📊 Type 'sensor auto 10' for live sensor data\n"
                           "ℹ️  Use 'info verbose' for detailed system info\n";
    config.colorsEnabled = true;
    config.echoEnabled = true;
    config.historySize = 15;
    
    cli.setConfig(config);
    
    // Register standard commands
    CLIStandardCommands::registerAllStandardCommands(cli);
    
    // Register custom commands
    cli.registerCommand("led", "Control built-in LED", 
                       "led <on|off|toggle|blink|fade> [count] [delay]", 
                       handleLedCommand, "Hardware");
    
    cli.registerCommand("gpio", "GPIO pin operations", 
                       "gpio <pin> <read|write> [value]", 
                       handleGpioCommand, "Hardware");
    
    cli.registerCommand("info", "Show system information", 
                       "info [verbose]", 
                       handleInfoCommand, "System");
    
    cli.registerCommand("wifi", "WiFi management", 
                       "wifi <scan|connect|disconnect|status> [ssid] [password]", 
                       handleWiFiCommand, "Network");
    
    cli.registerCommand("sensor", "Sensor monitoring", 
                       "sensor [start|stop|read|auto] [duration]", 
                       handleSensorCommand, "Sensors");
    
    // Start the CLI
    cli.begin();
    
    // Initialize WiFi in station mode
    WiFi.mode(WIFI_STA);
    
    // Initial sensor reading
    updateSensorData();
}

// ========================================================================
// MAIN LOOP
// ========================================================================

void loop() {
    // Update CLI (process user input)
    cli.update();
    
    // Update sensor data periodically
    if (millis() - lastSensorRead >= SENSOR_INTERVAL) {
        updateSensorData();
        
        // Print sensor update if monitoring is enabled
        if (sensorMonitoringEnabled) {
            printSensorUpdate();
        }
    }
    
    // Handle CLI exit
    if (CLIStandardCommands::isExitRequested()) {
        cli.printInfo("CLI exited. Entering simple mode...");
        CLIStandardCommands::resetExitFlag();
        
        // Simple mode - just blink LED to show system is alive
        while (true) {
            digitalWrite(LED_PIN, HIGH);
            delay(1000);
            digitalWrite(LED_PIN, LOW);
            delay(1000);
            
            // Could add code here to restart CLI on specific condition
            // For example, check for specific serial input
        }
    }
    
    // Small delay to prevent watchdog issues
    delay(1);
}
