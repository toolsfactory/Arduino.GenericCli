#include "generic_cli.h"
#include <algorithm>

// Constructor
GenericCLI::GenericCLI() : 
    historyIndex(-1), 
    inHistoryMode(false),
    cursorPos(0),
    isRunning(false) {
    
    // Register built-in commands
    registerCommand("help", "Show available commands", "help [command]", 
        [this](const CLIArgs& args) { handleHelpCommand(args); }, "Built-in");
    
    registerCommand("history", "Show command history", "history [clear]",
        [this](const CLIArgs& args) { handleHistoryCommand(args); }, "Built-in");
    
    registerCommand("clear", "Clear screen", "clear",
        [this](const CLIArgs& args) { handleClearCommand(args); }, "Built-in");
    
    registerCommand("exit", "Exit CLI", "exit",
        [this](const CLIArgs& args) { handleExitCommand(args); }, "Built-in");
}

GenericCLI::GenericCLI(const CLIConfig& cfg) : GenericCLI() {
    setConfig(cfg);
}

GenericCLI::~GenericCLI() {
    // Cleanup
}

// Configuration methods
void GenericCLI::setConfig(const CLIConfig& cfg) {
    config = cfg;
    
    // Adjust history size if needed
    while (commandHistory.size() > config.historySize) {
        commandHistory.pop_front();
    }
}

void GenericCLI::setPrompt(const String& prompt) {
    config.prompt = prompt;
}

void GenericCLI::setWelcomeMessage(const String& message) {
    config.welcomeMessage = message;
}

void GenericCLI::setColorsEnabled(bool enabled) {
    config.colorsEnabled = enabled;
}

void GenericCLI::setEchoEnabled(bool enabled) {
    config.echoEnabled = enabled;
}

void GenericCLI::setHistorySize(size_t size) {
    config.historySize = size;
    while (commandHistory.size() > config.historySize) {
        commandHistory.pop_front();
    }
}

// Command registration
bool GenericCLI::registerCommand(const String& name, const String& description, 
                                const String& usage, CommandCallback callback, 
                                const String& category) {
    // Check if command already exists
    if (findCommand(name) != nullptr) {
        Serial.printf("[%s] Warning: Command '%s' already exists, overwriting\n", 
                     config.logTag.c_str(), name.c_str());
        unregisterCommand(name);
    }
    
    CLICommand cmd(name, description, usage, callback, false, category);
    commands.push_back(cmd);
    return true;
}

bool GenericCLI::registerCommand(const CLICommand& command) {
    if (findCommand(command.name) != nullptr) {
        Serial.printf("[%s] Warning: Command '%s' already exists, overwriting\n", 
                     config.logTag.c_str(), command.name.c_str());
        unregisterCommand(command.name);
    }
    
    commands.push_back(command);
    return true;
}

bool GenericCLI::unregisterCommand(const String& name) {
    auto it = std::remove_if(commands.begin(), commands.end(),
        [&](const CLICommand& cmd) { 
            return config.caseSensitive ? cmd.name.equals(name) : cmd.name.equalsIgnoreCase(name);
        });
    
    if (it != commands.end()) {
        commands.erase(it, commands.end());
        return true;
    }
    return false;
}

void GenericCLI::clearCommands() {
    commands.clear();
}

// Core functionality
void GenericCLI::begin() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    
    isRunning = true;
    
    if (config.colorsEnabled) {
        // Enable ANSI sequences
        Serial.print("\033[?25h"); // Show cursor
    }
    
    printWelcome();
    printPrompt();
}

void GenericCLI::update() {
    if (!isRunning) {
        return;
    }
    
    while (Serial.available()) {
        char c = Serial.read();
        
        // Handle special sequences (ANSI escape codes)
        if (c == '\033') { // ESC
            if (Serial.available() >= 2) {
                char seq1 = Serial.read();
                char seq2 = Serial.read();
                if (seq1 == '[') {
                    switch (seq2) {
                        case 'A': processArrowUp(); break;
                        case 'B': processArrowDown(); break;
                        case 'C': // Right arrow - move cursor right
                            if (cursorPos < inputBuffer.length()) {
                                Serial.print("\033[C");
                                cursorPos++;
                            }
                            break;
                        case 'D': // Left arrow - move cursor left
                            if (cursorPos > 0) {
                                Serial.print("\033[D");
                                cursorPos--;
                            }
                            break;
                        case 'H': processHome(); break;
                        case 'F': processEnd(); break;
                        case '3': // Delete key sequence
                            if (Serial.available() && Serial.read() == '~') {
                                processDelete();
                            }
                            break;
                    }
                }
            }
            continue;
        }
        
        // Handle regular characters
        if (c == '\n' || c == '\r') {
            Serial.println();
            if (!inputBuffer.isEmpty()) {
                executeCommand(inputBuffer);
                addToHistory(inputBuffer);
                inputBuffer = "";
                cursorPos = 0;
            }
            exitHistoryMode();
            
            // Only print prompt if CLI is still running
            if (isRunning) {
                printPrompt();
            }
        } else if (c == '\b' || c == 127) { // Backspace
            processBackspace();
        } else if (c >= 32 && c <= 126) { // Printable characters
            if (cursorPos == inputBuffer.length()) {
                // Append to end
                inputBuffer += c;
                if (config.echoEnabled) {
                    Serial.print(c);
                }
            } else {
                // Insert at cursor position
                inputBuffer = inputBuffer.substring(0, cursorPos) + c + 
                             inputBuffer.substring(cursorPos);
                if (config.echoEnabled) {
                    redrawInputLine();
                }
            }
            cursorPos++;
            exitHistoryMode();
        }
    }
}

void GenericCLI::executeCommand(const String& commandLine) {
    if (commandLine.isEmpty()) {
        return;
    }
    
    CLIArgs args = parseArguments(commandLine);
    if (args.empty()) {
        return;
    }
    
    String commandName = args.positional[0];
    if (!config.caseSensitive) {
        commandName.toLowerCase();
    }
    
    // Remove command name from positional args
    args.positional.erase(args.positional.begin());
    
    // Find and execute command
    CLICommand* cmd = findCommand(commandName);
    if (cmd != nullptr) {
        try {
            cmd->callback(args);
        } catch (const std::exception& e) {
            printError("Command execution failed: " + String(e.what()));
        } catch (...) {
            printError("Unknown error occurred during command execution");
        }
    } else {
        printError("Unknown command: '" + commandName + "'. Type 'help' for available commands.");
    }
}

// Argument parsing
CLIArgs GenericCLI::parseArguments(const String& input) {
    CLIArgs args;
    String current = "";
    bool inQuotes = false;
    bool inFlag = false;
    String flagName = "";
    
    for (size_t i = 0; i < input.length(); i++) {
        char c = input[i];
        
        if (c == '"' && !inFlag) {
            inQuotes = !inQuotes;
        } else if (c == ' ' && !inQuotes) {
            if (!current.isEmpty()) {
                if (inFlag) {
                    args.flags[flagName] = current;
                    inFlag = false;
                    flagName = "";
                } else {
                    args.positional.push_back(current);
                }
                current = "";
            }
        } else if (c == '-' && !inQuotes && current.isEmpty() && i + 1 < input.length() && input[i + 1] == '-') {
            // Long flag (--flag=value or --flag)
            i++; // Skip second dash
            size_t eqPos = input.indexOf('=', i + 1);
            if (eqPos != -1 && eqPos < input.indexOf(' ', i + 1)) {
                // --flag=value format
                flagName = input.substring(i + 1, eqPos);
                i = eqPos;
                inFlag = true;
            } else {
                // --flag format (boolean flag)
                size_t spacePos = input.indexOf(' ', i + 1);
                if (spacePos == -1) spacePos = input.length();
                flagName = input.substring(i + 1, spacePos);
                args.flags[flagName] = "true";
                i = spacePos - 1;
            }
        } else if (c == '=' && inFlag) {
            // Continue parsing flag value
        } else {
            current += c;
        }
    }
    
    // Handle remaining content
    if (!current.isEmpty()) {
        if (inFlag) {
            args.flags[flagName] = current;
        } else {
            args.positional.push_back(current);
        }
    }
    
    return args;
}

// History management
void GenericCLI::addToHistory(const String& command) {
    if (command.isEmpty()) return;
    
    // Remove duplicate if it's the last command
    if (!commandHistory.empty() && commandHistory.back().equals(command)) {
        return;
    }
    
    commandHistory.push_back(command);
    
    // Maintain history size limit
    while (commandHistory.size() > config.historySize) {
        commandHistory.pop_front();
    }
}

void GenericCLI::enterHistoryMode() {
    if (!inHistoryMode) {
        savedInput = inputBuffer;
        inHistoryMode = true;
        historyIndex = commandHistory.size();
    }
}

void GenericCLI::exitHistoryMode() {
    if (inHistoryMode) {
        inHistoryMode = false;
        historyIndex = -1;
    }
}

// Arrow key processing
void GenericCLI::processArrowUp() {
    if (commandHistory.empty()) return;
    
    enterHistoryMode();
    
    if (historyIndex > 0) {
        historyIndex--;
        clearInputLine();
        inputBuffer = commandHistory[historyIndex];
        cursorPos = inputBuffer.length();
        if (config.echoEnabled) {
            Serial.print(inputBuffer);
        }
    }
}

void GenericCLI::processArrowDown() {
    if (!inHistoryMode) return;
    
    if (historyIndex < (int)commandHistory.size() - 1) {
        historyIndex++;
        clearInputLine();
        inputBuffer = commandHistory[historyIndex];
        cursorPos = inputBuffer.length();
        if (config.echoEnabled) {
            Serial.print(inputBuffer);
        }
    } else {
        // Restore saved input
        clearInputLine();
        inputBuffer = savedInput;
        cursorPos = inputBuffer.length();
        if (config.echoEnabled) {
            Serial.print(inputBuffer);
        }
        exitHistoryMode();
    }
}

void GenericCLI::processBackspace() {
    if (cursorPos > 0 && !inputBuffer.isEmpty()) {
        inputBuffer.remove(cursorPos - 1, 1);
        cursorPos--;
        if (config.echoEnabled) {
            if (cursorPos == inputBuffer.length()) {
                Serial.print("\b \b");
            } else {
                redrawInputLine();
            }
        }
        exitHistoryMode();
    }
}

void GenericCLI::processDelete() {
    if (cursorPos < inputBuffer.length()) {
        inputBuffer.remove(cursorPos, 1);
        if (config.echoEnabled) {
            redrawInputLine();
        }
        exitHistoryMode();
    }
}

void GenericCLI::processHome() {
    if (cursorPos > 0 && config.echoEnabled) {
        Serial.printf("\033[%dD", cursorPos);
        cursorPos = 0;
    }
}

void GenericCLI::processEnd() {
    if (cursorPos < inputBuffer.length() && config.echoEnabled) {
        Serial.printf("\033[%dC", inputBuffer.length() - cursorPos);
        cursorPos = inputBuffer.length();
    }
}

// Display functions
void GenericCLI::redrawInputLine() {
    if (!config.echoEnabled) return;
    
    // Save cursor position
    Serial.printf("\033[%dD", cursorPos); // Move to beginning
    Serial.print("\033[K"); // Clear to end of line
    Serial.print(inputBuffer); // Print entire buffer
    
    // Move cursor to correct position
    if (cursorPos < inputBuffer.length()) {
        Serial.printf("\033[%dD", inputBuffer.length() - cursorPos);
    }
}

void GenericCLI::clearInputLine() {
    if (!config.echoEnabled) return;
    
    Serial.printf("\033[2K\033[G");
    printPrompt();
//    Serial.printf("\033[%dD", cursorPos); // Move to beginning
//    Serial.print("\033[K"); // Clear to end of line
}

// Output functions
void GenericCLI::print(const String& message, MessageType type) {
    Serial.print(formatMessage(type, message));
}

void GenericCLI::println(const String& message, MessageType type) {
    Serial.println(formatMessage(type, message));
}

void GenericCLI::printSuccess(const String& message) {
    println(message, MessageType::SUCCESS);
}

void GenericCLI::printError(const String& message) {
    println(message, MessageType::ERROR);
}

void GenericCLI::printWarning(const String& message) {
    println(message, MessageType::WARNING);
}

void GenericCLI::printInfo(const String& message) {
    println(message, MessageType::INFO);
}

void GenericCLI::printWelcome() {
    if (!config.welcomeMessage.isEmpty()) {
        if (config.colorsEnabled) {
            Serial.print(ANSIColors::CBRIGHT_CYAN);
            Serial.print(ANSIIcons::INFO);
            Serial.print(" ");
        }
        Serial.print(config.welcomeMessage);
        if (config.colorsEnabled) {
            Serial.print(ANSIColors::CRESET);
        }
        Serial.println();
        println("Type 'help' to see available commands.", MessageType::INFO);
        Serial.println();
    }
}

void GenericCLI::printPrompt() {
    if (config.colorsEnabled) {
        // Simplified prompt for better compatibility
        Serial.print(ANSIColors::CBRIGHT_CYAN);
        Serial.print(config.prompt);
        Serial.print(ANSIColors::CCYAN);
        Serial.print(" > ");
        Serial.print(ANSIColors::CRESET);
    } else {
        Serial.print(config.prompt + " > ");
    }
}

void GenericCLI::clearScreen() {
    Serial.print("\033[2J\033[H");
}

// Built-in command handlers
void GenericCLI::handleHelpCommand(const CLIArgs& args) {
    if (args.empty()) {
        printCommandList();
    } else {
        String commandName = args.getPositional(0);
        CLICommand* cmd = findCommand(commandName);
        if (cmd != nullptr) {
            Serial.println();
            if (config.colorsEnabled) {
                Serial.print(ANSIColors::CBRIGHT_WHITE);
                Serial.print("Command: ");
                Serial.print(ANSIColors::CBRIGHT_CYAN);
                Serial.println(cmd->name);
                Serial.print(ANSIColors::CBRIGHT_WHITE);
                Serial.print("Category: ");
                Serial.print(ANSIColors::CYELLOW);
                Serial.println(cmd->category);
                Serial.print(ANSIColors::CBRIGHT_WHITE);
                Serial.print("Description: ");
                Serial.print(ANSIColors::CRESET);
                Serial.println(cmd->description);
                Serial.print(ANSIColors::CBRIGHT_WHITE);
                Serial.print("Usage: ");
                Serial.print(ANSIColors::CGREEN);
                Serial.println(cmd->usage);
                Serial.print(ANSIColors::CRESET);
            } else {
                Serial.println("Command: " + cmd->name);
                Serial.println("Category: " + cmd->category);
                Serial.println("Description: " + cmd->description);
                Serial.println("Usage: " + cmd->usage);
            }
        } else {
            printError("Command not found: " + commandName);
        }
    }
}

void GenericCLI::handleHistoryCommand(const CLIArgs& args) {
    if (args.hasFlag("clear") || args.getPositional(0).equalsIgnoreCase("clear")) {
        clearHistory();
        printSuccess("Command history cleared");
        return;
    }
    
    if (commandHistory.empty()) {
        printInfo("No commands in history");
        return;
    }
    
    Serial.println();
    if (config.colorsEnabled) {
        Serial.print(ANSIColors::CBRIGHT_WHITE);
        Serial.println("Command History:");
        Serial.print(ANSIColors::CRESET);
    } else {
        Serial.println("Command History:");
    }
    Serial.println("===============");
    
    for (size_t i = 0; i < commandHistory.size(); i++) {
        if (config.colorsEnabled) {
            Serial.printf("%s%3d%s %s%s%s %s\n",
                         ANSIColors::CBRIGHT_BLACK, i + 1, ANSIColors::CRESET,
                         ANSIColors::CCYAN, ANSIIcons::ARROW_RIGHT, ANSIColors::CRESET,
                         commandHistory[i].c_str());
        } else {
            Serial.printf("%3d > %s\n", i + 1, commandHistory[i].c_str());
        }
    }
    Serial.println();
}

void GenericCLI::handleClearCommand(const CLIArgs& args) {
    clearScreen();
    printInfo("Screen cleared");
}

void GenericCLI::handleExitCommand(const CLIArgs& args) {
    printInfo("Goodbye!");
    stopCLI();
}

void GenericCLI::printCommandList() {
    Serial.println();
    
    // Group commands by category
    std::map<String, std::vector<CLICommand*>> categorized;
    for (auto& cmd : commands) {
        if (!cmd.hidden) {
            categorized[cmd.category].push_back(&cmd);
        }
    }
    
    if (config.colorsEnabled) {
        Serial.println("\033[97mAvailable Commands:\033[0m");
    } else {
        Serial.println("Available Commands:");
    }
    Serial.println("==================");
    
    for (const auto& category : categorized) {
        Serial.println();
        if (config.colorsEnabled) {
            Serial.println("\033[33m• " + category.first + "\033[0m");
        } else {
            Serial.println("• " + category.first);
        }
        
        for (const auto& cmd : category.second) {
            if (config.colorsEnabled) {
                Serial.println("  \033[36m" + cmd->name + "\033[0m - " + cmd->description);
            } else {
                Serial.println("  " + cmd->name + " - " + cmd->description);
            }
        }
    }
    
    Serial.println();
    if (config.colorsEnabled) {
        Serial.println("\033[36mℹ\033[0m Use 'help <command>' for detailed usage information");
    } else {
        Serial.println("INFO: Use 'help <command>' for detailed usage information");
    }
}

// Utility functions
String GenericCLI::colorize(const String& text, const char* color) const {
    if (!config.colorsEnabled) {
        return text;
    }
    return String(color) + text + String(ANSIColors::CRESET);
}

String GenericCLI::formatMessage(MessageType type, const String& message) const {
    if (!config.colorsEnabled) {
        switch (type) {
            case MessageType::SUCCESS: return "SUCCESS: " + message;
            case MessageType::ERROR: return "ERROR: " + message;
            case MessageType::WARNING: return "WARNING: " + message;
            case MessageType::INFO: return "INFO: " + message;
            default: return message;
        }
    }
    
    // Simplified ANSI codes for better Windows compatibility
    switch (type) {
        case MessageType::SUCCESS:
            return String("\033[32m") + ANSIIcons::SUCCESS + " " + message + "\033[0m";
        case MessageType::ERROR:
            return String("\033[31m") + ANSIIcons::ERROR + " " + message + "\033[0m";
        case MessageType::WARNING:
            return String("\033[33m") + ANSIIcons::WARNING + " " + message + "\033[0m";
        case MessageType::INFO:
            return String("\033[36m") + ANSIIcons::INFO + " " + message + "\033[0m";
        default:
            return message;
    }
}

CLICommand* GenericCLI::findCommand(const String& name) {
    for (auto& cmd : commands) {
        if (config.caseSensitive ? cmd.name.equals(name) : cmd.name.equalsIgnoreCase(name)) {
            return &cmd;
        }
    }
    return nullptr;
}

// Public utility methods
std::vector<String> GenericCLI::getCommandNames() const {
    std::vector<String> names;
    for (const auto& cmd : commands) {
        if (!cmd.hidden) {
            names.push_back(cmd.name);
        }
    }
    return names;
}

bool GenericCLI::hasCommand(const String& name) const {
    return const_cast<GenericCLI*>(this)->findCommand(name) != nullptr;
}

std::vector<String> GenericCLI::getHistory() const {
    std::vector<String> history;
    for (const auto& cmd : commandHistory) {
        history.push_back(cmd);
    }
    return history;
}

void GenericCLI::clearHistory() {
    commandHistory.clear();
    historyIndex = -1;
    inHistoryMode = false;
}

// Private method to stop CLI
void GenericCLI::stopCLI() {
    isRunning = false;
}

// Helper functions implementation
namespace CLIHelpers {
    GenericCLI createBasicCLI(const String& prompt, bool withBuiltins) {
        CLIConfig config;
        config.prompt = prompt;
        config.welcomeMessage = "CLI System Ready";
        config.colorsEnabled = true;
        config.echoEnabled = true;
        config.historySize = 50;
        config.caseSensitive = false;
        
        GenericCLI newCli(config);
        
        if (withBuiltins) {
            // Additional built-in commands beyond the defaults
            newCli.registerCommand("version", "Show version information", "version",
                [](const CLIArgs& args) {
                    Serial.println("Generic CLI Library v1.0.0");
                }, "System");
            
            newCli.registerCommand("uptime", "Show system uptime", "uptime",
                [](const CLIArgs& args) {
                    unsigned long uptime = millis() / 1000;
                    unsigned long days = uptime / 86400;
                    unsigned long hours = (uptime % 86400) / 3600;
                    unsigned long minutes = (uptime % 3600) / 60;
                    unsigned long seconds = uptime % 60;
                    
                    Serial.printf("Uptime: %lu days, %02lu:%02lu:%02lu\n", 
                                 days, hours, minutes, seconds);
                }, "System");
            
            newCli.registerCommand("memory", "Show memory information", "memory",
                [](const CLIArgs& args) {
                    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
                    Serial.printf("Heap Size: %d bytes\n", ESP.getHeapSize());
                    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
                    Serial.printf("PSRAM Size: %d bytes\n", ESP.getPsramSize());
                }, "System");
        }
        
        return newCli;
    }
    
    bool validateArgCount(const CLIArgs& args, size_t min, size_t max) {
        size_t count = args.size();
        if (count < min) {
            Serial.printf("Error: Too few arguments. Expected at least %d, got %d\n", 
                         min, count);
            return false;
        }
        if (max != SIZE_MAX && count > max) {
            Serial.printf("Error: Too many arguments. Expected at most %d, got %d\n", 
                         max, count);
            return false;
        }
        return true;
    }
    
    bool validateFlags(const CLIArgs& args, const std::vector<String>& requiredFlags) {
        for (const String& flag : requiredFlags) {
            if (!args.hasFlag(flag)) {
                Serial.printf("Error: Required flag --%s is missing\n", flag.c_str());
                return false;
            }
        }
        return true;
    }
}