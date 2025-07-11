#ifndef GENERIC_CLI_H
#define GENERIC_CLI_H

#include <Arduino.h>
#include <vector>
#include <functional>
#include <deque>
#include <map>

// ANSI Color Codes
namespace ANSIColors {
    const char* const CRESET = "\033[0m";
    const char* const CRED = "\033[31m";
    const char* const CGREEN = "\033[32m";
    const char* const CYELLOW = "\033[33m";
    const char* const CBLUE = "\033[34m";
    const char* const CMAGENTA = "\033[35m";
    const char* const CCYAN = "\033[36m";
    const char* const CWHITE = "\033[37m";
    const char* const CBRIGHT_RED = "\033[91m";
    const char* const CBRIGHT_GREEN = "\033[92m";
    const char* const CBRIGHT_YELLOW = "\033[93m";
    const char* const CBRIGHT_BLUE = "\033[94m";
    const char* const CBRIGHT_MAGENTA = "\033[95m";
    const char* const CBRIGHT_CYAN = "\033[96m";
    const char* const CBRIGHT_WHITE = "\033[97m";
    const char* const CBRIGHT_BLACK = "\033[90m";
    
    // Background colors
    const char* const BG_RED = "\033[41m";
    const char* const BG_GREEN = "\033[42m";
    const char* const BG_YELLOW = "\033[43m";
    const char* const BG_BLUE = "\033[44m";
}

// ANSI Icons/Symbols
namespace ANSIIcons {
    const char* const SUCCESS = "✓";
    const char* const ERROR = "✗";
    const char* const WARNING = "⚠";
    const char* const INFO = "ℹ";
    const char* const ARROW_RIGHT = "→";
    const char* const BULLET = "•";
    const char* const PROMPT = "❯";
}

// Message types for colored output
enum class MessageType {
    SUCCESS,
    ERROR, 
    WARNING,
    INFO,
    NORMAL
};

// Command argument structure
struct CLIArgs {
    std::vector<String> positional;
    std::map<String, String> flags;
    
    bool hasFlag(const String& flag) const {
        return flags.find(flag) != flags.end();
    }
    
    String getFlag(const String& flag, const String& defaultValue = "") const {
        auto it = flags.find(flag);
        return (it != flags.end()) ? it->second : defaultValue;
    }
    
    String getPositional(size_t index, const String& defaultValue = "") const {
        return (index < positional.size()) ? positional[index] : defaultValue;
    }
    
    size_t size() const { return positional.size(); }
    bool empty() const { return positional.empty(); }
};

// Command callback function type
using CommandCallback = std::function<void(const CLIArgs&)>;

// Command structure
struct CLICommand {
    String name;
    String description;
    String usage;
    CommandCallback callback;
    bool hidden;
    String category;
    
    CLICommand() : hidden(false), category("General") {}
    
    CLICommand(const String& n, const String& desc, const String& use, 
               CommandCallback cb, bool hide = false, const String& cat = "General") :
        name(n), description(desc), usage(use), callback(cb), hidden(hide), category(cat) {}
};

// CLI Configuration
struct CLIConfig {
    String prompt;
    String welcomeMessage;
    bool echoEnabled;
    bool colorsEnabled;
    size_t historySize;
    bool caseSensitive;
    String logTag;
    
    CLIConfig() : 
        prompt("cli"), 
        welcomeMessage("Generic CLI Ready"),
        echoEnabled(true),
        colorsEnabled(true), 
        historySize(50),
        caseSensitive(false),
        logTag("CLI") {}
};

class GenericCLI {
private:
    // Configuration
    CLIConfig config;
    
    // Commands
    std::vector<CLICommand> commands;
    
    // Input handling
    String inputBuffer;
    std::deque<String> commandHistory;
    int historyIndex;
    bool inHistoryMode;
    String savedInput;
    
    // Terminal state
    size_t cursorPos;
    bool isRunning;
    
    // Internal command handlers
    void handleHelpCommand(const CLIArgs& args);
    void handleHistoryCommand(const CLIArgs& args);
    void handleClearCommand(const CLIArgs& args);
    void handleExitCommand(const CLIArgs& args);
    
    // Input processing
    CLIArgs parseArguments(const String& input);
    void processSpecialKey(char c);
    void processArrowUp();
    void processArrowDown();
    void processBackspace();
    void processDelete();
    void processHome();
    void processEnd();
    
    // History management
    void addToHistory(const String& command);
    void enterHistoryMode();
    void exitHistoryMode();
    
    // Display functions
    void redrawInputLine();
    void clearInputLine();
    void moveCursor(int delta);
    
    // Utility functions
    String colorize(const String& text, const char* color) const;
    String formatMessage(MessageType type, const String& message) const;
    CLICommand* findCommand(const String& name);
    
    // Internal utility to stop CLI
    void stopCLI();
    
public:
    GenericCLI();
    GenericCLI(const CLIConfig& cfg);
    ~GenericCLI();
    
    // Configuration
    void setConfig(const CLIConfig& cfg);
    CLIConfig getConfig() const { return config; }
    void setPrompt(const String& prompt);
    void setWelcomeMessage(const String& message);
    void setColorsEnabled(bool enabled);
    void setEchoEnabled(bool enabled);
    void setHistorySize(size_t size);
    
    // Command registration
    bool registerCommand(const String& name, const String& description, 
                        const String& usage, CommandCallback callback, 
                        const String& category = "General");
    bool registerCommand(const CLICommand& command);
    bool unregisterCommand(const String& name);
    void clearCommands();
    
    // Core functionality
    void begin();
    void update();
    void executeCommand(const String& commandLine);
    void stop(); // Stop the CLI
    bool running() const; // Check if CLI is running
    
    // Output functions
    void print(const String& message, MessageType type = MessageType::NORMAL);
    void println(const String& message = "", MessageType type = MessageType::NORMAL);
    void printSuccess(const String& message);
    void printError(const String& message);
    void printWarning(const String& message);
    void printInfo(const String& message);
    
    // Display functions
    void printWelcome();
    void printPrompt();
    void printHelp(const String& commandName = "");
    void printCommandList();
    void clearScreen();
    
    // Utility
    size_t getCommandCount() const { return commands.size(); }
    std::vector<String> getCommandNames() const;
    bool hasCommand(const String& name) const;
    
    // History access
    std::vector<String> getHistory() const;
    void clearHistory();
};

// Helper macros for command registration
#define CLI_COMMAND(cli, name, desc, usage, category) \
    cli.registerCommand(name, desc, usage, [&](const CLIArgs& args)

#define CLI_SIMPLE_COMMAND(cli, name, desc) \
    cli.registerCommand(name, desc, name, [&](const CLIArgs& args)

// Example usage helper
namespace CLIHelpers {
    // Create a basic CLI with common commands
    GenericCLI createBasicCLI(const String& prompt = "cli", bool withBuiltins = true);
    
    // Argument parsing helpers
    bool validateArgCount(const CLIArgs& args, size_t min, size_t max = SIZE_MAX);
    bool validateFlags(const CLIArgs& args, const std::vector<String>& requiredFlags);
}

#endif // GENERIC_CLI_H