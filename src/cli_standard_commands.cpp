#include "cli_standard_commands.h"
#include <Arduino.h>

// Global state variables (outside namespace)
static GenericCLI* g_cli = nullptr;
static bool g_exitRequested = false;

namespace CLIStandardCommands {
    
    // Helper function to set CLI reference
    void setCLIReference(GenericCLI& cli) {
        g_cli = &cli;
    }
    
    // Helper function to pad string to specified width
    String padString(const String& text, int width) {
        if (text.length() >= width) return text;
        
        String result = text;
        int padding = width - text.length();
        for (int i = 0; i < padding; i++) {
            result += " ";
        }
        return result;
    }
    
    // ========================================================================
    // COMMAND HANDLERS (FORWARD DECLARATIONS)
    // ========================================================================
    
    // Forward declarations for handlers
    void handleExit(const CLIArgs& args);
    void handleClear(const CLIArgs& args);
    void handleReboot(const CLIArgs& args);
    void handleStatus(const CLIArgs& args);
    void handleColors(const CLIArgs& args);
    void handleHistory(const CLIArgs& args);
    
    // ========================================================================
    // COMMAND REGISTRATION FUNCTIONS
    // ========================================================================
    
    void registerExitCommand(GenericCLI& cli) {
        setCLIReference(cli);
        cli.registerCommand("exit", "Exit the CLI", "exit [--force]",
            [](const CLIArgs& args) { handleExit(args); }, "System");
    }
    
    void registerClearCommand(GenericCLI& cli) {
        setCLIReference(cli);
        cli.registerCommand("clear", "Clear screen", "clear",
            [](const CLIArgs& args) { handleClear(args); }, "System");
    }
    
    void registerRebootCommand(GenericCLI& cli) {
        setCLIReference(cli);
        cli.registerCommand("reboot", "Restart ESP32", "reboot [--force] [--delay=seconds]",
            [](const CLIArgs& args) { handleReboot(args); }, "System");
    }
    
    void registerStatusCommand(GenericCLI& cli) {
        setCLIReference(cli);
        cli.registerCommand("status", "Show system status", "status [--compact] [--json]",
            [](const CLIArgs& args) { handleStatus(args); }, "System");
    }
    
    void registerColorsCommand(GenericCLI& cli) {
        setCLIReference(cli);
        cli.registerCommand("colors", "Control ANSI colors", "colors <on|off|test>",
            [](const CLIArgs& args) { handleColors(args); }, "System");
    }
    
    void registerHistoryCommand(GenericCLI& cli) {
        setCLIReference(cli);
        cli.registerCommand("history", "Show command history", "history [clear] [--limit=n]",
            [](const CLIArgs& args) { handleHistory(args); }, "System");
    }
    
    void registerAllStandardCommands(GenericCLI& cli) {
        registerExitCommand(cli);
        registerClearCommand(cli);
        registerRebootCommand(cli);
        registerStatusCommand(cli);
        registerColorsCommand(cli);
        registerHistoryCommand(cli);
    }
    
    void registerBasicCommands(GenericCLI& cli) {
        registerExitCommand(cli);
        registerClearCommand(cli);
        registerColorsCommand(cli);
        registerHistoryCommand(cli);
    }
    
    // ========================================================================
    // COMMAND HANDLERS (IMPLEMENTATION)
    // ========================================================================
    
    void handleExit(const CLIArgs& args) {
        if (args.hasFlag("force")) {
            g_cli->printInfo("Force exit - goodbye!");
            g_exitRequested = true;
            return;
        }
        
        g_cli->printInfo("Are you sure you want to exit? (y/n)");
        
        // Wait for confirmation
        unsigned long timeout = millis() + 10000; // 10 second timeout
        String response = "";
        
        while (millis() < timeout) {
            if (Serial.available()) {
                char c = Serial.read();
                if (c == '\n' || c == '\r') {
                    break;
                } else if (c >= 32 && c <= 126) {
                    response += c;
                    Serial.print(c); // Echo the character
                }
            }
            delay(10);
        }
        Serial.println(); // New line after input
        
        response.toLowerCase();
        if (response == "y" || response == "yes") {
            g_cli->printSuccess("Goodbye!");
            g_exitRequested = true;
        } else if (response == "n" || response == "no" || response.isEmpty()) {
            g_cli->printInfo("Exit cancelled");
        } else {
            g_cli->printWarning("Invalid response - exit cancelled");
        }
    }
    
    void handleClear(const CLIArgs& args) {
        // Clear screen using ANSI escape codes
        Serial.print("\033[2J\033[H");
        g_cli->printInfo("Screen cleared");
    }
    
    void handleReboot(const CLIArgs& args) {
        int delaySeconds = args.getFlag("delay", "3").toInt();
        if (delaySeconds < 1) delaySeconds = 1;
        if (delaySeconds > 30) delaySeconds = 30;
        
        if (args.hasFlag("force")) {
            g_cli->printWarning("Force reboot in " + String(delaySeconds) + " seconds...");
            delay(delaySeconds * 1000);
            ESP.restart();
        } else {
            g_cli->printInfo("System will reboot in " + String(delaySeconds) + " seconds");
            g_cli->printInfo("Use 'reboot --force' for immediate restart");
            
            for (int i = delaySeconds; i > 0; i--) {
                Serial.println("Rebooting in " + String(i) + "...");
                delay(1000);
            }
            ESP.restart();
        }
    }
    
    void handleStatus(const CLIArgs& args) {
        bool compact = args.hasFlag("compact");
        bool jsonFormat = args.hasFlag("json");
        unsigned long uptime = millis() / 1000;
        
        if (jsonFormat) {
            Serial.println("{");
            Serial.println("  \"device\": \"" + String(ESP.getChipModel()) + "\",");
            Serial.println("  \"uptime_seconds\": " + String(uptime) + ",");
            Serial.println("  \"free_heap\": " + String(ESP.getFreeHeap()) + ",");
            Serial.println("  \"total_heap\": " + String(ESP.getHeapSize()) + ",");
            Serial.println("  \"cpu_freq_mhz\": " + String(ESP.getCpuFreqMHz()) + ",");
            Serial.println("  \"flash_size\": " + String(ESP.getFlashChipSize()) + ",");
            Serial.println("  \"chip_revision\": " + String(ESP.getChipRevision()) + ",");
            Serial.println("  \"colors_enabled\": " + String(g_cli->getConfig().colorsEnabled ? "true" : "false"));
            Serial.println("}");
        } else if (compact) {
            String uptimeStr = "";
            unsigned long hours = uptime / 3600;
            unsigned long minutes = (uptime % 3600) / 60;
            if (hours > 0) {
                uptimeStr = String(hours) + "h" + String(minutes) + "m";
            } else {
                uptimeStr = String(minutes) + "m" + String(uptime % 60) + "s";
            }
            
            String memStr = "";
            size_t freeHeap = ESP.getFreeHeap();
            if (freeHeap < 1024) {
                memStr = String(freeHeap) + "B";
            } else if (freeHeap < 1024 * 1024) {
                memStr = String(freeHeap / 1024) + "KB";
            } else {
                memStr = String(freeHeap / (1024 * 1024)) + "MB";
            }
            
            Serial.println("Status: " + String(ESP.getChipModel()) + 
                         " | Up:" + uptimeStr + 
                         " | RAM:" + memStr + 
                         " | CPU:" + String(ESP.getCpuFreqMHz()) + "MHz");
        } else {
            Serial.println("\nSYSTEM STATUS");
            Serial.println("=============");
            
            String chipModel = String(ESP.getChipModel());
            Serial.println("Chip: " + chipModel);
            
            String cpuInfo = String(ESP.getCpuFreqMHz()) + " MHz";
            Serial.println("CPU: " + cpuInfo);
            
            // Format uptime
            String uptimeStr = "";
            unsigned long hours = uptime / 3600;
            unsigned long minutes = (uptime % 3600) / 60;
            unsigned long seconds = uptime % 60;
            if (hours > 0) {
                uptimeStr = String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";
            } else if (minutes > 0) {
                uptimeStr = String(minutes) + "m " + String(seconds) + "s";
            } else {
                uptimeStr = String(seconds) + "s";
            }
            Serial.println("Uptime: " + uptimeStr);
            
            // Format memory
            String freeHeapStr = "";
            size_t freeHeap = ESP.getFreeHeap();
            if (freeHeap < 1024) {
                freeHeapStr = String(freeHeap) + " B";
            } else if (freeHeap < 1024 * 1024) {
                freeHeapStr = String(freeHeap / 1024.0, 1) + " KB";
            } else {
                freeHeapStr = String(freeHeap / (1024.0 * 1024.0), 1) + " MB";
            }
            Serial.println("Free RAM: " + freeHeapStr);
            
            String totalHeapStr = "";
            size_t totalHeap = ESP.getHeapSize();
            if (totalHeap < 1024 * 1024) {
                totalHeapStr = String(totalHeap / 1024) + " KB";
            } else {
                totalHeapStr = String(totalHeap / (1024 * 1024)) + " MB";
            }
            Serial.println("Total RAM: " + totalHeapStr);
            
            String flashStr = String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB";
            Serial.println("Flash: " + flashStr);
            
            String colorsStr = g_cli->getConfig().colorsEnabled ? "ENABLED" : "DISABLED";
            Serial.println("Colors: " + colorsStr);
        }
    }
    
    void handleColors(const CLIArgs& args) {
        if (args.empty()) {
            String status = g_cli->getConfig().colorsEnabled ? "ENABLED" : "DISABLED";
            g_cli->println("Colors currently: " + status);
            g_cli->printInfo("Usage: colors <on|off|test>");
            return;
        }
        
        String action = args.getPositional(0);
        action.toLowerCase();
        
        if (action == "on") {
            CLIConfig config = g_cli->getConfig();
            config.colorsEnabled = true;
            g_cli->setConfig(config);
            g_cli->printSuccess("ANSI colors enabled! 🎨");
            
        } else if (action == "off") {
            CLIConfig config = g_cli->getConfig();
            config.colorsEnabled = false;
            g_cli->setConfig(config);
            Serial.println("SUCCESS: ANSI colors disabled");
            
        } else if (action == "test") {
            Serial.println("\nANSI COLOR TEST");
            Serial.println("===============");
            Serial.println();
            Serial.println("Basic Colors:");
            Serial.println("\033[31m■ Red\033[0m \033[32m■ Green\033[0m \033[33m■ Yellow\033[0m \033[34m■ Blue\033[0m \033[35m■ Magenta\033[0m \033[36m■ Cyan\033[0m");
            Serial.println();
            Serial.println("Icons and Symbols:");
            Serial.println("\033[32m✓ Success\033[0m \033[31m✗ Error\033[0m \033[33m⚠ Warning\033[0m \033[36mℹ Info\033[0m");
            Serial.println("→ ← ↑ ↓ • ★ ▲ ◆ ■ □ ▓ ░");
            Serial.println();
            Serial.println("Results:");
            Serial.println("✓ If you see colored squares: type 'colors on'");
            Serial.println("✗ If you see codes like [31m: ANSI not supported");
            Serial.println("⚠ If mixed results: limited terminal support");
            Serial.println();
            
        } else {
            g_cli->printError("Invalid option. Use: on, off, or test");
        }
    }
    
    void handleHistory(const CLIArgs& args) {
        if (args.hasFlag("clear") || args.getPositional(0).equalsIgnoreCase("clear")) {
            g_cli->clearHistory();
            g_cli->printSuccess("Command history cleared");
            return;
        }
        
        std::vector<String> history = g_cli->getHistory();
        if (history.empty()) {
            g_cli->printInfo("No commands in history");
            return;
        }
        
        int limit = args.getFlag("limit", "20").toInt();
        if (limit <= 0) limit = history.size();
        if (limit > (int)history.size()) limit = history.size();
        
        Serial.println();
        if (g_cli->getConfig().colorsEnabled) {
            Serial.println("\033[97mCommand History:\033[0m");
        } else {
            Serial.println("Command History:");
        }
        Serial.println("================");
        
        int start = max(0, (int)history.size() - limit);
        for (int i = start; i < (int)history.size(); i++) {
            if (g_cli->getConfig().colorsEnabled) {
                Serial.println("\033[90m" + String(i + 1, DEC) + ".\033[0m " + history[i]);
            } else {
                Serial.println(String(i + 1) + ". " + history[i]);
            }
        }
        
        Serial.println();
        g_cli->printInfo("Showing last " + String(limit) + " of " + String(history.size()) + " commands");
        g_cli->printInfo("Use 'run <number>' to execute a command from history");
    }
    
} // End namespace CLIStandardCommands

// ========================================================================
// PUBLIC UTILITY FUNCTIONS (outside namespace for global access)
// ========================================================================

bool CLIStandardCommands::isExitRequested() {
    return g_exitRequested;
}

void CLIStandardCommands::resetExitFlag() {
    g_exitRequested = false;
}