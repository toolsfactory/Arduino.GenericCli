/**
 * Generic CLI Basic Example
 * 
 * This example demonstrates the basic usage of the GenericCLI library
 * with custom commands and standard commands. It showcases:
 * 
 * - Basic CLI setup and configuration
 * - Custom command registration
 * - Standard commands (exit, clear, reboot, status, etc.)
 * - LED control commands
 * - GPIO commands
 * - System information commands
 * - WiFi management (basic example)
 * 
 * Hardware Requirements:
 * - ESP32 development board
 * - Built-in LED (or external LED on GPIO 2)
 * - Serial terminal supporting ANSI escape codes (recommended)
 * 
 * Usage:
 * 1. Upload the code to your ESP32
 * 2. Open serial monitor at 115200 baud
 * 3. Use a terminal that supports ANSI colors (like PuTTY, Terminal, etc.)
 * 4. Type 'help' to see available commands
 * 
 * Author: GenericCLI Example
 * Version: 1.0
 */

#include <Arduino.h>
#include <WiFi.h>
#include "../../src/generic_cli.h"
#include "../../src/cli_standard_commands.h"

// Configuration
#define LED_PIN 2
#define BAUD_RATE 115200

// Global variables
GenericCLI cli;
bool ledState = false;

// ========================================================================
// CUSTOM COMMAND HANDLERS
// ========================================================================

/**
 * LED Control Command
 * Usage: led <on|off|toggle|blink> [--count=n] [--delay=ms]
 */
void handleLedCommand(const CLIArgs& args) {
    if (args.empty()) {
        cli.printError("Usage: led <on|off|toggle|blink> [--count=n] [--delay=ms]");
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
        int count = args.getFlag("count", "3").toInt();
        int delayMs = args.getFlag("delay", "500").toInt();
        
        if (count < 1 || count > 50) {
            cli.printError("Count must be between 1 and 50");
            return;
        }
        
        if (delayMs < 50 || delayMs > 5000) {
            cli.printError("Delay must be between 50 and 5000 ms");
            return;
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
        
    } else {
        cli.printError("Unknown LED action: " + action);
        cli.printInfo("Available actions: on, off, toggle, blink");
    }
}

/**
 * GPIO Command
 * Usage: gpio <pin> <read|write> [value] [--pullup] [--pulldown]
 */
void handleGpioCommand(const CLIArgs& args) {
    if (args.size() < 2) {
        cli.printError("Usage: gpio <pin> <read|write> [value] [--pullup] [--pulldown]");
        return;
    }
    
    int pin = args.getPositional(0).toInt();
    String operation = args.getPositional(1);
    operation.toLowerCase();
    
    // Validate pin number
    if (pin < 0 || pin > 39) {
        cli.printError("Invalid pin number. ESP32 has pins 0-39");
        return;
    }
    
    // Check for restricted pins
    if (pin == 6 || pin == 7 || pin == 8 || pin == 9 || pin == 10 || pin == 11) {
        cli.printWarning("Pin " + String(pin) + " is used for flash memory - use with caution");
    }
    
    if (operation == "read") {
        // Configure pin mode
        if (args.hasFlag("pullup")) {
            pinMode(pin, INPUT_PULLUP);
            cli.printInfo("Pin " + String(pin) + " configured as INPUT_PULLUP");
        } else if (args.hasFlag("pulldown")) {
            pinMode(pin, INPUT_PULLDOWN);
            cli.printInfo("Pin " + String(pin) + " configured as INPUT_PULLDOWN");
        } else {
            pinMode(pin, INPUT);
            cli.printInfo("Pin " + String(pin) + " configured as INPUT");
        }
        
        int value = digitalRead(pin);
        cli.printSuccess("GPIO" + String(pin) + " = " + String(value) + " (" + 
                        String(value ? "HIGH" : "LOW") + ")");
                        
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
 * Usage: sysinfo [--verbose]
 */
void handleSysInfoCommand(const CLIArgs& args) {
    bool verbose = args.hasFlag("verbose");
    
    cli.printInfo("=== ESP32 System Information ===");
    
    // Basic chip information
    Serial.println("Chip Model: " + String(ESP.getChipModel()));
    Serial.println("Chip Revision: " + String(ESP.getChipRevision()));
    Serial.println("CPU Cores: " + String(ESP.getChipCores()));
    Serial.println("CPU Frequency: " + String(ESP.getCpuFreqMHz()) + " MHz");
    
    // Memory information
    size_t totalHeap = ESP.getHeapSize();
    size_t freeHeap = ESP.getFreeHeap();
    size_t usedHeap = totalHeap - freeHeap;
    
    Serial.println("Total Heap: " + String(totalHeap) + " bytes (" + 
                  String(totalHeap / 1024) + " KB)");
    Serial.println("Free Heap: " + String(freeHeap) + " bytes (" + 
                  String(freeHeap / 1024) + " KB)");
    Serial.println("Used Heap: " + String(usedHeap) + " bytes (" + 
                  String(usedHeap / 1024) + " KB)");
    Serial.println("Heap Usage: " + String((usedHeap * 100) / totalHeap) + "%");
    
    // Flash information
    Serial.println("Flash Size: " + String(ESP.getFlashChipSize()) + " bytes (" + 
                  String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB)");
    
    if (verbose) {
        Serial.println();
        cli.printInfo("=== Detailed Information ===");
        
        // Uptime
        unsigned long uptime = millis() / 1000;
        unsigned long days = uptime / 86400;
        unsigned long hours = (uptime % 86400) / 3600;
        unsigned long minutes = (uptime % 3600) / 60;
        unsigned long seconds = uptime % 60;
        
        Serial.println("Uptime: " + String(days) + "d " + String(hours) + "h " + 
                      String(minutes) + "m " + String(seconds) + "s");
        
        // SDK version
        Serial.println("ESP-IDF Version: " + String(ESP.getSdkVersion()));
        
        // WiFi MAC address
        Serial.println("WiFi MAC: " + WiFi.macAddress());
        
        // Reset reason
        Serial.println("Reset Reason: " + String(esp_reset_reason()));
        
        // Temperature (if available)
        #ifdef SOC_TEMP_SENSOR_SUPPORTED
        Serial.println("Internal Temperature: " + String(temperatureRead()) + "°C");
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
        int n = WiFi.scanNetworks();
        
        if (n == 0) {
            cli.printWarning("No networks found");
        } else {
            cli.printSuccess("Found " + String(n) + " networks:");
            Serial.println();
            Serial.println("  #  SSID                         RSSI  Ch  Encryption");
            Serial.println("  ─  ────────────────────────────  ────  ──  ──────────");
            
            for (int i = 0; i < n; i++) {
                String ssid = WiFi.SSID(i);
                if (ssid.length() > 30) {
                    ssid = ssid.substring(0, 27) + "...";
                }
                
                String encryption;
                switch (WiFi.encryptionType(i)) {
                    case WIFI_AUTH_OPEN: encryption = "Open"; break;
                    case WIFI_AUTH_WEP: encryption = "WEP"; break;
                    case WIFI_AUTH_WPA_PSK: encryption = "WPA"; break;
                    case WIFI_AUTH_WPA2_PSK: encryption = "WPA2"; break;
                    case WIFI_AUTH_WPA_WPA2_PSK: encryption = "WPA/WPA2"; break;
                    case WIFI_AUTH_WPA2_ENTERPRISE: encryption = "WPA2-ENT"; break;
                    default: encryption = "Unknown"; break;
                }
                
                Serial.printf("%3d  %-30s  %4d  %2d  %s\n", 
                    i + 1, ssid.c_str(), WiFi.RSSI(i), WiFi.channel(i), encryption.c_str());
            }
        }
        
    } else if (action == "connect") {
        if (args.size() < 2) {
            cli.printError("Usage: wifi connect <ssid> [password]");
            return;
        }
        
        String ssid = args.getPositional(1);
        String password = args.getPositional(2);
        
        cli.printInfo("Connecting to: " + ssid);
        
        if (password.isEmpty()) {
            WiFi.begin(ssid.c_str());
        } else {
            WiFi.begin(ssid.c_str(), password.c_str());
        }
        
        // Wait for connection with timeout
        unsigned long startTime = millis();
        const unsigned long timeout = 15000; // 15 seconds
        
        while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
            delay(500);
            Serial.print(".");
        }
        Serial.println();
        
        if (WiFi.status() == WL_CONNECTED) {
            cli.printSuccess("Connected to " + ssid);
            Serial.println("IP Address: " + WiFi.localIP().toString());
            Serial.println("Signal Strength: " + String(WiFi.RSSI()) + " dBm");
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
            Serial.println("IP Address: " + WiFi.localIP().toString());
            Serial.println("Gateway: " + WiFi.gatewayIP().toString());
            Serial.println("DNS: " + WiFi.dnsIP().toString());
            Serial.println("Signal Strength: " + String(WiFi.RSSI()) + " dBm");
            Serial.println("Channel: " + String(WiFi.channel()));
            Serial.println("MAC Address: " + WiFi.macAddress());
        } else {
            cli.printWarning("WiFi Status: Disconnected");
        }
        
    } else {
        cli.printError("Unknown WiFi action: " + action);
        cli.printInfo("Available actions: scan, connect, disconnect, status");
    }
}

/**
 * Memory Command - Display and analyze memory usage
 * Usage: mem [--detailed]
 */
void handleMemoryCommand(const CLIArgs& args) {
    bool detailed = args.hasFlag("detailed");
    
    cli.printInfo("=== Memory Information ===");
    
    // Heap memory
    size_t totalHeap = ESP.getHeapSize();
    size_t freeHeap = ESP.getFreeHeap();
    size_t usedHeap = totalHeap - freeHeap;
    size_t minFreeHeap = ESP.getMinFreeHeap();
    size_t maxAllocHeap = ESP.getMaxAllocHeap();
    
    Serial.println("Heap Memory:");
    Serial.println("  Total: " + String(totalHeap) + " bytes (" + String(totalHeap / 1024) + " KB)");
    Serial.println("  Free:  " + String(freeHeap) + " bytes (" + String(freeHeap / 1024) + " KB)");
    Serial.println("  Used:  " + String(usedHeap) + " bytes (" + String(usedHeap / 1024) + " KB)");
    Serial.println("  Usage: " + String((usedHeap * 100) / totalHeap) + "%");
    
    if (detailed) {
        Serial.println("  Min Free: " + String(minFreeHeap) + " bytes (" + String(minFreeHeap / 1024) + " KB)");
        Serial.println("  Max Alloc: " + String(maxAllocHeap) + " bytes (" + String(maxAllocHeap / 1024) + " KB)");
        
        // PSRAM information (if available)
        if (ESP.getPsramSize() > 0) {
            Serial.println();
            Serial.println("PSRAM Memory:");
            Serial.println("  Total: " + String(ESP.getPsramSize()) + " bytes (" + String(ESP.getPsramSize() / 1024) + " KB)");
            Serial.println("  Free:  " + String(ESP.getFreePsram()) + " bytes (" + String(ESP.getFreePsram() / 1024) + " KB)");
        }
        
        // Flash information
        Serial.println();
        Serial.println("Flash Memory:");
        Serial.println("  Size: " + String(ESP.getFlashChipSize()) + " bytes (" + String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB)");
        Serial.println("  Speed: " + String(ESP.getFlashChipSpeed() / 1000000) + " MHz");
    }
}

// ========================================================================
// SETUP AND MAIN LOOP
// ========================================================================

void setup() {
    // Initialize hardware
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Configure CLI
    CLIConfig config;
    config.prompt = "esp32";
    config.welcomeMessage = "\n🚀 ESP32 Generic CLI Demo v1.0\n" 
                           "Type 'help' to see available commands\n"
                           "Use arrow keys for command history navigation\n";
    config.colorsEnabled = true;
    config.echoEnabled = true;
    config.historySize = 20;
    
    cli.setConfig(config);
    
    // Register standard commands (exit, clear, reboot, status, colors, history)
    CLIStandardCommands::registerAllStandardCommands(cli);
    
    // Register custom commands
    cli.registerCommand("led", "Control built-in LED", 
                       "led <on|off|toggle|blink> [--count=n] [--delay=ms]", 
                       handleLedCommand, "Hardware");
    
    cli.registerCommand("gpio", "GPIO pin control", 
                       "gpio <pin> <read|write> [value] [--pullup] [--pulldown]", 
                       handleGpioCommand, "Hardware");
    
    cli.registerCommand("sysinfo", "Show system information", 
                       "sysinfo [--verbose]", 
                       handleSysInfoCommand, "System");
    
    cli.registerCommand("wifi", "WiFi management", 
                       "wifi <scan|connect|disconnect|status> [ssid] [password]", 
                       handleWiFiCommand, "Network");
    
    cli.registerCommand("mem", "Memory information", 
                       "mem [--detailed]", 
                       handleMemoryCommand, "System");
    
    // Start CLI
    cli.begin();
    
    // Initialize WiFi in station mode
    WiFi.mode(WIFI_STA);
}

void loop() {
    // Update CLI (handle user input)
    cli.update();
    
    // Check if user requested to exit
    if (CLIStandardCommands::isExitRequested()) {
        cli.printInfo("CLI stopped. Entering main application loop...");
        
        // Reset the exit flag if you want to restart CLI later
        CLIStandardCommands::resetExitFlag();
        
        // Your main application code can go here
        while (true) {
            // Simple LED blink to show the system is still running
            digitalWrite(LED_PIN, HIGH);
            delay(1000);
            digitalWrite(LED_PIN, LOW);
            delay(1000);
            
            // You could add a way to restart the CLI here
            // For example, check for a specific condition or input
        }
    }
    
    // Small delay to prevent watchdog issues
    delay(1);
}
