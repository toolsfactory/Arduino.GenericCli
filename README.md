# GenericCLI - Advanced Command Line Interface for Arduino & ESP32

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/toolsfactory/Arduino.GenericCli/releases)
[![Platform](https://img.shields.io/badge/platform-ESP32%20%7C%20ESP8266-lightgrey.svg)](https://github.com/toolsfactory/Arduino.GenericCli)
[![Framework](https://img.shields.io/badge/framework-Arduino%20%7C%20ESP--IDF-orange.svg)](https://github.com/toolsfactory/Arduino.GenericCli)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

A comprehensive command-line interface library for Arduino and ESP32 microcontrollers that brings professional CLI capabilities to embedded systems. Perfect for IoT devices, debugging, configuration management, and interactive device control.

## ✨ Key Features

### 🚀 **Core CLI Functionality**
- **Command Registration**: Easy-to-use command system with categories
- **Argument Parsing**: Support for positional arguments and flags (`--flag=value`)
- **Command History**: Navigate through previous commands with arrow keys
- **Input Line Editing**: Full cursor control, backspace, delete, home/end keys
- **ANSI Color Support**: Beautiful colored output with icons and themes
- **Built-in Help System**: Automatic help generation with usage examples

### 🎯 **Advanced Features**
- **Standard Commands**: Pre-built commands (help, exit, clear, reboot, status)
- **Non-blocking Operation**: Fully asynchronous command processing
- **Memory Efficient**: Optimized for microcontroller environments
- **Extensible Design**: Easy to add custom commands and features
- **Cross-platform**: Works with Arduino IDE and PlatformIO

### 🛠️ **Professional Tools**
- **Configuration Management**: Persistent settings with EEPROM
- **Data Logging**: Structured logging with multiple levels
- **Export Capabilities**: JSON and CSV data export
- **Task Management**: Background task scheduling framework
- **Input Validation**: Comprehensive error handling and validation

## 📱 Quick Preview

```bash
esp32❯ help
Available Commands:
══════════════════════════════════════════════════════════════════════

Hardware Commands:
  led <on|off|toggle|blink|fade> [--count=n] [--delay=ms]
      Control built-in LED with various modes
  
  gpio <pin> <read|write> [value] [--pullup] [--pulldown]  
      GPIO pin operations with validation

System Commands:
  info [--verbose]           Show detailed system information
  status [--compact]         Display system status and health
  mem [--detailed]          Memory usage analysis
  reboot [--force]          Restart the device

Network Commands:
  wifi <scan|connect|disconnect|status> [ssid] [password]
      WiFi network management

esp32❯ led blink --count=5 --delay=200
✓ Blinking LED 5 times with 200ms delay
✓ Blink sequence completed

esp32❯ wifi scan
ℹ Scanning for WiFi networks...
✓ Found 8 networks:
  1. HomeNetwork-5G        -45dBm  Ch 36  WPA2
  2. OfficeWiFi           -67dBm  Ch 6   WPA2
  3. IoT-Devices          -72dBm  Ch 11  WPA2
```

## 🚀 Getting Started

### Arduino IDE Installation

1. **Download the library files** and place them in your Arduino libraries folder:
   ```
   Documents/Arduino/libraries/GenericCLI/
   ├── src/
   │   ├── generic_cli.h
   │   ├── generic_cli.cpp
   │   ├── cli_standard_commands.h
   │   └── cli_standard_commands.cpp
   └── library.properties
   ```

2. **Copy the Arduino IDE example**:
   - Open `examples/arduino_ide_example/arduino_ide_example.ino`
   - Upload to your ESP32
   - Open Serial Monitor at 115200 baud

### PlatformIO Installation

1. **Add to your `platformio.ini`**:
   ```ini
   [env:esp32dev]
   platform = espressif32
   board = esp32dev
   framework = arduino
   lib_deps = 
       https://github.com/toolsfactory/Arduino.GenericCli.git#v1.0.0
   monitor_speed = 115200
   
   ; Monitor configuration for proper ANSI color support
   monitor_filters = 
       esp32_exception_decoder
   monitor_echo = yes
   monitor_eol = LF
   monitor_raw = yes
   monitor_rich = true
   ```

   **Important**: The monitor configuration is crucial for proper ANSI color support:
   - `monitor_rich = true` - Enables rich text and enhanced ANSI color processing
   - `esp32_exception_decoder` - Decodes ESP32 stack traces (optional but recommended)
   - `monitor_raw = yes` - Preserves ANSI escape sequences
   - `monitor_echo = yes` - Shows typed characters for better CLI experience

2. **Use in your code**:
   ```cpp
   #include <generic_cli.h>
   #include <cli_standard_commands.h>
   
   GenericCLI cli;
   
   void setup() {
       cli.begin();
       CLIStandardCommands::registerAllStandardCommands(cli);
   }
   
   void loop() {
       cli.update();
   }
   ```

## 📚 Examples

We provide three comprehensive examples to get you started:

### 🎯 [Arduino IDE Example](examples/arduino_ide_example/)
**Perfect for Arduino IDE users and beginners**
- Single `.ino` file for easy setup
- LED control, GPIO operations, WiFi management
- System monitoring and sensor simulation
- Comprehensive documentation

### 🔧 [Basic PlatformIO Example](examples/basic_cli_example/)
**Great for learning and simple projects**
- PlatformIO project structure
- Hardware control and system information
- Network management capabilities
- Memory monitoring

### 🏢 [Advanced PlatformIO Example](examples/advanced_cli_example/)
**Production-ready implementation**
- Persistent configuration with EEPROM
- Data logging and export (JSON/CSV)
- Task management framework
- Enterprise-level features

## 💡 Usage Examples

### Basic Command Registration

```cpp
#include <generic_cli.h>

GenericCLI cli;

void handleLedCommand(const CLIArgs& args) {
    if (args.empty()) {
        cli.printError("Usage: led <on|off|toggle>");
        return;
    }
    
    String action = args.getPositional(0);
    if (action == "on") {
        digitalWrite(LED_PIN, HIGH);
        cli.printSuccess("LED turned ON");
    } else if (action == "off") {
        digitalWrite(LED_PIN, LOW);
        cli.printSuccess("LED turned OFF");
    }
}

void setup() {
    cli.begin();
    
    // Register custom command
    cli.registerCommand("led", "Control LED", "led <on|off|toggle>", 
                       handleLedCommand, "Hardware");
}

void loop() {
    cli.update();
}
```

### Advanced Command with Flags

```cpp
void handleSensorCommand(const CLIArgs& args) {
    // Get positional arguments
    String action = args.getPositional(0, "read");
    
    // Get optional flags
    bool verbose = args.hasFlag("verbose");
    int samples = args.getFlag("samples", "1").toInt();
    String format = args.getFlag("format", "text");
    
    // Command logic here...
    if (format == "json") {
        Serial.println("{\"temperature\": 23.5, \"humidity\": 65.2}");
    } else {
        cli.printInfo("Temperature: 23.5°C, Humidity: 65.2%");
    }
}

// Usage: sensor read --verbose --samples=10 --format=json
```

### Configuration Management

```cpp
// Save settings to EEPROM
void handleConfigCommand(const CLIArgs& args) {
    if (args.getPositional(0) == "set") {
        String key = args.getPositional(1);
        String value = args.getPositional(2);
        
        if (key == "wifi_ssid") {
            strcpy(config.wifiSSID, value.c_str());
            saveConfiguration();
            cli.printSuccess("WiFi SSID updated");
        }
    }
}
```

## 🎨 Customization

### CLI Configuration

```cpp
CLIConfig config;
config.prompt = "my_device";                    // Custom prompt
config.welcomeMessage = "Welcome to My Device!"; // Custom welcome
config.colorsEnabled = true;                     // Enable colors
config.historySize = 20;                         // Command history size
config.caseSensitive = false;                    // Case sensitivity

cli.setConfig(config);
```

### Custom Color Themes

```cpp
// Use built-in color constants
cli.print("Success!", MessageType::SUCCESS);     // Green with ✓
cli.print("Error!", MessageType::ERROR);         // Red with ✗
cli.print("Warning!", MessageType::WARNING);     // Yellow with ⚠
cli.print("Info", MessageType::INFO);            // Cyan with ℹ

// Custom colors
Serial.print(ANSIColors::CBRIGHT_BLUE);
Serial.print("Custom colored text");
Serial.print(ANSIColors::CRESET);
```

## 🔧 API Reference

### Core Classes

#### `GenericCLI`
Main CLI class for command management and user interaction.

**Key Methods:**
- `begin()` - Initialize the CLI
- `update()` - Process user input (call in loop)
- `registerCommand(name, desc, usage, callback, category)` - Add commands
- `executeCommand(commandLine)` - Execute command programmatically
- `print*(message)` - Output functions with color support

#### `CLIArgs`
Container for parsed command arguments.

**Key Methods:**
- `getPositional(index, defaultValue)` - Get positional argument
- `getFlag(name, defaultValue)` - Get flag value
- `hasFlag(name)` - Check if flag exists
- `size()` - Number of positional arguments

#### `CLIStandardCommands`
Pre-built standard commands for common functionality.

**Available Commands:**
- `help` - Show command help
- `exit` - Exit CLI with confirmation
- `clear` - Clear terminal screen
- `reboot` - Restart device
- `status` - System status information
- `history` - Command history management

### Command Categories

Commands are automatically organized into categories:
- **Hardware** - GPIO, LED, sensor control
- **System** - Status, memory, configuration
- **Network** - WiFi, connectivity
- **Built-in** - Help, history, exit
- **Custom** - Your application-specific commands

## 🚀 Performance

### Memory Usage
- **Flash**: ~40KB for core library
- **RAM**: ~8KB for CLI state and buffers
- **Optional**: Additional memory for advanced features

### Optimizations
- Non-blocking input processing
- Efficient command lookup
- Minimal memory allocations
- Configurable buffer sizes

## 🔍 Troubleshooting

### Common Issues

**Commands not working:**
- Check baud rate (115200)
- Verify command spelling
- Use `help` to see available commands

**Colors not displaying:**
- Use terminal with ANSI support (PuTTY, VS Code Terminal)
- For PlatformIO: Enable rich text mode in `platformio.ini`:
  ```ini
  monitor_rich = true
  monitor_raw = yes
  ```
- Try `colors off` to disable
- Check terminal emulation settings

**Arrow keys not working:**
- Use proper terminal emulator
- Avoid Arduino IDE Serial Monitor for history navigation
- Try PuTTY or system terminal

### Debug Information

```bash
# Check system status
status --verbose

# View memory usage
mem --detailed

# Test basic functionality
help
led on
gpio 2 read
```

## 🤝 Contributing

We welcome contributions! Please consider:

- **Bug Reports**: Use GitHub issues with detailed descriptions
- **Feature Requests**: Describe use cases and expected behavior  
- **Documentation**: Improve examples and guides
- **Code**: Follow existing style and add tests

### Development Setup

```bash
git clone https://github.com/toolsfactory/Arduino.GenericCli.git
cd Arduino.GenericCli
# Open in PlatformIO or Arduino IDE
```

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🔗 Links

- **GitHub Repository**: https://github.com/toolsfactory/Arduino.GenericCli
- **Documentation**: [examples/README.md](examples/README.md)
- **Arduino IDE Example**: [examples/arduino_ide_example/](examples/arduino_ide_example/)
- **PlatformIO Examples**: [examples/](examples/)
- **Issues & Support**: https://github.com/toolsfactory/Arduino.GenericCli/issues

## 🏆 Why Choose GenericCLI?

### ✅ **Proven & Reliable**
- Tested on multiple ESP32 variants
- Memory-efficient implementation
- Production-ready error handling

### ✅ **Developer Friendly**
- Extensive documentation and examples
- Intuitive API design
- Compatible with both Arduino IDE and PlatformIO

### ✅ **Feature Rich**
- Professional CLI features out of the box
- Extensible architecture for custom needs
- Beautiful ANSI color support

---

**Ready to add professional CLI capabilities to your ESP32 project?** 🚀

Choose your starting point:
- **[Arduino IDE Example](examples/arduino_ide_example/)** - Single file, easy setup
- **[Basic PlatformIO](examples/basic_cli_example/)** - Learning and prototyping  
- **[Advanced PlatformIO](examples/advanced_cli_example/)** - Production features

*Built with ❤️ by [ToolsFactory](https://github.com/toolsfactory)*
