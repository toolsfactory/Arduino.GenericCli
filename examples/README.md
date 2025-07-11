# Generic CLI Examples

This directory contains comprehensive examples demonstrating how to use the GenericCLI library for Arduino/ESP32 projects. The examples progress from basic usage to advanced enterprise-level implementations.

## 📚 Available Examples

### 1. [Arduino IDE Example](./arduino_ide_example/)
**Perfect for Arduino IDE users and beginners**

- 🎯 **Arduino IDE Compatible**: Single .ino file, easy to use
- ✅ Basic CLI setup and configuration
- ✅ LED control with multiple modes (blink, fade, toggle)
- ✅ GPIO operations (read/write with validation)
- ✅ System information and monitoring
- ✅ WiFi management and scanning
- ✅ Real-time sensor simulation
- 📚 Comprehensive documentation and examples

**Best for:**
- Arduino IDE users
- Educational projects and learning
- Quick prototyping
- Students and beginners

### 2. [Basic CLI Example](./basic_cli_example/)
**PlatformIO version for development workflows**

- ✅ Basic CLI setup and configuration
- ✅ Custom command registration
- ✅ Standard commands (exit, clear, reboot, status)
- ✅ Hardware control (LED, GPIO)
- ✅ System information display
- ✅ WiFi management
- ✅ Memory monitoring

**Best for:**
- PlatformIO users
- Learning the library basics
- Simple IoT devices
- Prototyping and development

### 3. [Advanced CLI Example](./advanced_cli_example/)
**Professional-grade implementation for production systems**

- 🔧 Persistent configuration management (EEPROM)
- 📊 Advanced data logging and export (JSON/CSV)
- 📝 Structured logging with levels
- ⚙️ Task management framework
- 🎯 Input validation and error handling
- 🔄 Data export capabilities
- 🎨 Custom CLI themes and formatting

**Best for:**
- Production IoT devices
- Data collection systems
- Enterprise applications
- Complex device management

## 🚀 Quick Start

### Choose Your Starting Point

**Using Arduino IDE?** → Start with [Arduino IDE Example](./arduino_ide_example/)

**Using PlatformIO and new to CLI?** → Try [Basic CLI Example](./basic_cli_example/)

**Building production systems?** → Jump to [Advanced CLI Example](./advanced_cli_example/)

**Need specific features?** → See feature comparison below

## 📋 Feature Comparison

| Feature | Arduino IDE | Basic PlatformIO | Advanced PlatformIO |
|---------|-------------|------------------|---------------------|
| **Development Environment** | | | |
| Arduino IDE compatible | ✅ | ⚪ | ⚪ |
| PlatformIO compatible | ⚪ | ✅ | ✅ |
| Single file (.ino) | ✅ | ⚪ | ⚪ |
| **Core CLI** | | | |
| Command registration | ✅ | ✅ | ✅ |
| Argument parsing | ✅ | ✅ | ✅ |
| Command history | ✅ | ✅ | ✅ |
| ANSI colors | ✅ | ✅ | ✅ |
| **Hardware Control** | | | |
| LED control | ✅ | ✅ | ⚪ |
| GPIO operations | ✅ | ✅ | ⚪ |
| Sensor simulation | ✅ | ⚪ | ✅ |
| **System Management** | | | |
| System info | ✅ | ✅ | ✅ |
| Memory monitoring | ✅ | ✅ | ✅ |
| WiFi management | ✅ | ✅ | ⚪ |
| **Advanced Features** | | | |
| Configuration persistence | ⚪ | ⚪ | ✅ |
| JSON output | ⚪ | ⚪ | ✅ |
| Data logging | ⚪ | ⚪ | ✅ |
| Data export (CSV/JSON) | ⚪ | ⚪ | ✅ |
| Task management | ⚪ | ⚪ | ✅ |
| Structured logging | ⚪ | ⚪ | ✅ |
| Input validation | ⚪ | ⚪ | ✅ |
| Custom themes | ⚪ | ⚪ | ✅ |

Legend: ✅ Implemented, ⚪ Not included

## 🛠️ Common Setup Instructions

### Prerequisites
```bash
# Install PlatformIO (if not already installed)
pip install platformio

# Or install via VS Code extension
```

### Building Any Example
```bash
# Navigate to example directory
cd basic_cli_example  # or advanced_cli_example

# Build and upload
pio run --target upload

# Open serial monitor
pio device monitor
```

### Hardware Setup
Both examples work with standard ESP32 development boards:
- ESP32 DevKit V1
- ESP32-WROOM-32
- ESP32-S2/S3 boards
- ESP32-C3 boards

**Connections:**
- Built-in LED (usually GPIO 2)
- USB cable for programming and serial communication
- No additional hardware required for basic functionality

## 🎯 Learning Path

### Beginner Path
1. **Start with Basic Example**
   - Upload and run the basic example
   - Try the built-in commands (`help`, `status`, `led on`)
   - Experiment with GPIO commands
   - Test WiFi connectivity

2. **Understand the Code**
   - Examine command registration in `setup()`
   - Study command handler functions
   - Learn argument parsing with `CLIArgs`

3. **Add Custom Commands**
   - Create simple commands that print messages
   - Add commands with arguments
   - Implement flag handling

### Intermediate Path
1. **Advanced Example Exploration**
   - Run the advanced example
   - Explore configuration management
   - Test data logging and export
   - Understand persistent storage

2. **Customization**
   - Modify configuration parameters
   - Add custom sensor types
   - Implement new export formats
   - Create custom validation rules

3. **Integration**
   - Connect real sensors
   - Implement network APIs
   - Add authentication
   - Create monitoring dashboards

### Advanced Path
1. **Production Deployment**
   - Security hardening
   - Performance optimization
   - Error handling and recovery
   - Automated testing

2. **Custom Features**
   - Protocol implementations
   - Advanced data processing
   - Custom CLI extensions
   - Integration with cloud services

## 🔧 Common Customizations

### Adding New Commands
```cpp
// Simple command without arguments
cli.registerCommand("hello", "Say hello", "hello", 
    [](const CLIArgs& args) {
        Serial.println("Hello, World!");
    }, "Demo");

// Command with arguments and flags
cli.registerCommand("calc", "Simple calculator", "calc <num1> <op> <num2> [--format=decimal|hex]",
    [](const CLIArgs& args) {
        if (args.size() < 3) {
            cli.printError("Usage: calc <num1> <op> <num2>");
            return;
        }
        
        float a = args.getPositional(0).toFloat();
        String op = args.getPositional(1);
        float b = args.getPositional(2).toFloat();
        float result = 0;
        
        if (op == "+") result = a + b;
        else if (op == "-") result = a - b;
        else if (op == "*") result = a * b;
        else if (op == "/") result = b != 0 ? a / b : 0;
        
        String format = args.getFlag("format", "decimal");
        if (format == "hex") {
            Serial.println("Result: 0x" + String((int)result, HEX));
        } else {
            Serial.println("Result: " + String(result));
        }
    }, "Utilities");
```

### Custom Configuration
```cpp
CLIConfig config;
config.prompt = "my_device";
config.welcomeMessage = "Welcome to My Device CLI!";
config.colorsEnabled = true;
config.historySize = 25;
cli.setConfig(config);
```

### Error Handling
```cpp
void handleSafeCommand(const CLIArgs& args) {
    try {
        // Validate inputs
        if (args.empty()) {
            cli.printError("No arguments provided");
            return;
        }
        
        // Perform operation
        // ...
        
        cli.printSuccess("Operation completed successfully");
    } catch (const std::exception& e) {
        cli.printError("Error: " + String(e.what()));
    }
}
```

## 🐛 Troubleshooting

### Build Issues
```bash
# Clean and rebuild
pio run --target clean
pio run

# Update libraries
pio lib update

# Check dependencies
pio lib list
```

### Runtime Issues
```bash
# Check serial configuration
# Baud rate: 115200
# Line ending: LF or CRLF
# Echo: Enabled

# Try different terminal software:
# - PuTTY (Windows)
# - Terminal.app (macOS) 
# - gnome-terminal (Linux)
```

### Common Problems

**Commands not recognized:**
- Check spelling and case sensitivity
- Use `help` to list available commands
- Verify command registration in code

**Colors not working:**
- Use terminal that supports ANSI escape codes
- Try `colors test` command
- Disable with `colors off` if problematic

**History not working:**
- Use proper terminal emulator (not Arduino IDE serial monitor)
- Check that arrow key sequences are supported
- Try different terminal software

## 📖 Additional Resources

### Documentation
- [GenericCLI Library Documentation](../README.md)
- [Standard Commands Reference](../src/cli_standard_commands.h)
- [API Reference](../src/generic_cli.h)

### Community Examples
- IoT sensor networks
- Home automation controllers
- Industrial monitoring systems
- Educational robotics platforms

### Best Practices
1. **Command Design**
   - Use clear, descriptive command names
   - Provide comprehensive help text
   - Implement proper argument validation
   - Handle errors gracefully

2. **Performance**
   - Keep command handlers lightweight
   - Use non-blocking operations
   - Monitor memory usage
   - Optimize for your specific use case

3. **User Experience**
   - Provide clear feedback messages
   - Use consistent command syntax
   - Include usage examples in help
   - Support both human and machine-readable output

## 🤝 Contributing

We welcome contributions to the example collection! Consider adding:

- **Domain-specific examples** (industrial, agricultural, smart home)
- **Protocol implementations** (MQTT, HTTP, Modbus)
- **Integration examples** (cloud services, databases)
- **Advanced features** (scripting, automation, scheduling)

## 📄 License

These examples are provided under the same license as the GenericCLI library.
