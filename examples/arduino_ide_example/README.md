# Arduino IDE Example - Generic CLI

This example demonstrates how to use the GenericCLI library with Arduino IDE. It provides a comprehensive CLI interface that's perfect for learning, prototyping, and educational projects.

## 🚀 Features

### Core CLI Functionality
- ✅ **Interactive Command Line**: Full CLI with command history and editing
- ✅ **Built-in Commands**: help, clear, exit, history, status, reboot
- ✅ **Color Support**: Beautiful ANSI colored output
- ✅ **Command Categories**: Organized commands by functionality

### Hardware Control
- 🔹 **LED Control**: Multiple modes (on/off/toggle/blink/fade)
- 🔹 **GPIO Operations**: Read from and write to any GPIO pin
- 🔹 **Analog Reading**: Read analog values from ADC pins

### System Features
- 📊 **System Information**: Detailed ESP32 system data
- 📡 **WiFi Management**: Scan, connect, and manage WiFi connections
- 🌡️ **Sensor Monitoring**: Real-time sensor data simulation
- 💾 **Memory Monitoring**: Heap and flash memory usage

## 📋 Hardware Requirements

- **ESP32 Development Board** (any variant: ESP32, ESP32-S2, ESP32-S3, ESP32-C3)
- **Built-in LED** (typically connected to GPIO 2)
- **USB Cable** for programming and serial communication
- **Optional**: External sensors, LEDs, or other components for testing GPIO functions

## 🛠️ Arduino IDE Setup

### Step 1: Install ESP32 Board Support
1. Open Arduino IDE
2. Go to **File → Preferences**
3. Add this URL to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for "ESP32" and install the ESP32 board package

### Step 2: Install the GenericCLI Library

#### Option A: Manual Installation (Recommended)
1. Create a new folder in your Arduino libraries directory:
   ```
   Documents/Arduino/libraries/GenericCLI/
   ```

2. Copy the library files into this folder:
   ```
   GenericCLI/
   ├── src/
   │   ├── generic_cli.h
   │   ├── generic_cli.cpp
   │   ├── cli_standard_commands.h
   │   └── cli_standard_commands.cpp
   └── library.properties
   ```

3. Create a `library.properties` file with this content:
   ```ini
   name=GenericCLI
   version=1.0.0
   author=ToolsFactory
   maintainer=ToolsFactory
   sentence=A comprehensive CLI library for Arduino and ESP32
   paragraph=Provides command-line interface functionality with argument parsing, command history, ANSI colors, and extensible command system.
   category=Communication
   url=https://github.com/toolsfactory/Arduino.GenericCli
   architectures=esp32
   includes=generic_cli.h
   ```

#### Option B: Library Manager (If Available)
1. Go to **Sketch → Include Library → Manage Libraries**
2. Search for "GenericCLI"
3. Install the library

### Step 3: Load and Upload the Example
1. Copy the `arduino_ide_example.ino` file to a new Arduino sketch folder
2. Open the sketch in Arduino IDE
3. Select your ESP32 board: **Tools → Board → ESP32 Dev Module** (or your specific board)
4. Select the correct COM port: **Tools → Port**
5. Set upload speed: **Tools → Upload Speed → 921600**
6. Click **Upload** (or press Ctrl+U)

### Step 4: Open Serial Monitor
1. After upload, open **Tools → Serial Monitor**
2. Set baud rate to **115200**
3. Set line ending to **Both NL & CR** or **Newline**
4. You should see the welcome message and CLI prompt

## 🎯 Quick Start Commands

Once the sketch is uploaded and serial monitor is open, try these commands:

### Basic Commands
```bash
help                    # Show all available commands
help led               # Show help for specific command
clear                  # Clear the screen
status                 # Show system status
```

### LED Control
```bash
led on                 # Turn LED on
led off                # Turn LED off
led toggle             # Toggle LED state
led blink              # Blink 3 times (default)
led blink 5 200        # Blink 5 times with 200ms delay
led fade               # Fade in and out (if PWM capable)
```

### GPIO Operations
```bash
gpio 4 read            # Read from GPIO 4
gpio 5 write high      # Write HIGH to GPIO 5
gpio 13 write 0        # Write LOW to GPIO 13
gpio 36 read           # Read analog pin (will show both digital and analog values)
```

### System Information
```bash
info                   # Basic system information
info verbose           # Detailed system information with uptime, temperature, etc.
```

### WiFi Management
```bash
wifi scan              # Scan for available networks
wifi status            # Show current WiFi status
wifi connect "MyWiFi" "MyPassword"  # Connect to a network
wifi disconnect        # Disconnect from current network
```

### Sensor Monitoring
```bash
sensor                 # Show current sensor readings
sensor read            # Force fresh sensor reading
sensor start           # Enable periodic sensor updates
sensor stop            # Disable sensor updates
sensor auto 15         # Auto-print sensors for 15 seconds
```

## 📖 Command Reference

### LED Command
```bash
led <action> [parameters]

Actions:
  on           Turn LED on
  off          Turn LED off
  toggle       Toggle current state
  blink        Blink LED multiple times
  fade         Fade in and out (PWM pins only)

Parameters (for blink):
  count        Number of blinks (1-20, default: 3)
  delay        Delay between blinks in ms (50-2000, default: 500)

Examples:
  led on
  led blink 10 100
  led fade
```

### GPIO Command
```bash
gpio <pin> <operation> [value]

Pin: GPIO pin number (0-39 for ESP32)
Operations:
  read         Read digital value from pin
  write        Write digital value to pin

Values (for write):
  0, low, off  Write LOW (0V)
  1, high, on  Write HIGH (3.3V)

Examples:
  gpio 4 read
  gpio 2 write high
  gpio 13 write 0
```

### Info Command
```bash
info [verbose]

Shows system information including:
- Chip model and revision
- CPU frequency and cores
- Memory usage (heap, flash)
- Uptime (verbose mode)
- CPU temperature (verbose mode, if supported)
- SDK version (verbose mode)

Examples:
  info
  info verbose
```

### WiFi Command
```bash
wifi <action> [parameters]

Actions:
  scan                    Scan for available networks
  status                  Show current connection status
  connect <ssid> [pass]   Connect to network
  disconnect              Disconnect from current network

Examples:
  wifi scan
  wifi connect "MyNetwork" "MyPassword"
  wifi connect "OpenNetwork"
  wifi status
  wifi disconnect
```

### Sensor Command
```bash
sensor [action] [parameters]

Actions:
  (none)       Show current readings
  read         Force immediate reading
  start        Enable automatic monitoring
  stop         Disable automatic monitoring
  auto [sec]   Auto-print for specified seconds (5-60)

Examples:
  sensor
  sensor read
  sensor start
  sensor auto 20
```

## 🔧 Customization

### Adding Your Own Commands

You can easily add custom commands by following this pattern:

```cpp
void handleMyCommand(const CLIArgs& args) {
    // Validate arguments
    if (args.empty()) {
        cli.printError("Usage: mycommand <parameter>");
        return;
    }
    
    // Get arguments
    String param = args.getPositional(0);
    
    // Your command logic here
    cli.printSuccess("Command executed with: " + param);
}

// Register in setup()
cli.registerCommand("mycommand", "Description of my command", 
                   "mycommand <parameter>", 
                   handleMyCommand, "Custom");
```

### Modifying CLI Configuration

```cpp
CLIConfig config;
config.prompt = "my_device";           // Change prompt
config.welcomeMessage = "Welcome!";    // Custom welcome message
config.colorsEnabled = false;          // Disable colors
config.historySize = 10;               // Reduce history size
cli.setConfig(config);
```

### Adding Real Sensors

Replace the simulated sensor data with real sensor readings:

```cpp
#include <DHT.h>
DHT dht(DHT_PIN, DHT22);

void updateSensorData() {
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    lightLevel = analogRead(LDR_PIN);
    
    // Validate readings
    if (isnan(temperature)) temperature = -999;
    if (isnan(humidity)) humidity = -999;
}
```

## 🐛 Troubleshooting

### Common Issues

**CLI not responding:**
- Check baud rate is set to 115200
- Verify correct COM port is selected
- Try different terminal software (PuTTY, Tera Term)
- Reset the ESP32 and check for startup messages

**Commands not recognized:**
- Check spelling (commands are case-insensitive)
- Use `help` to see all available commands
- Make sure you press Enter after typing

**Colors not working:**
- Some serial monitors don't support ANSI colors
- Try PuTTY or other terminal emulators
- Use `colors off` command to disable colors

**Upload fails:**
- Hold BOOT button while clicking upload
- Check board and port selection
- Try lower upload speed (460800 or 115200)

**WiFi issues:**
- Make sure WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
- Check password spelling
- Ensure network allows new devices

### Debug Information

```bash
# Check system status
status

# View memory usage
info verbose

# Test basic functionality
led blink 1
gpio 2 read
```

## 📈 Performance Notes

- **Memory Usage**: ~50KB flash, ~8KB RAM
- **Command Processing**: ~1-2ms per command
- **Sensor Updates**: Every 2 seconds (configurable)
- **History Buffer**: Stores last 15 commands (configurable)

## 🔄 Advanced Features

### Command History
- Use ↑ and ↓ arrow keys to navigate through previous commands
- History persists until device reset
- Configurable history size

### Argument Parsing
The CLI supports sophisticated argument parsing:
```bash
command arg1 arg2           # Positional arguments
command --flag=value        # Named flags with values
command --boolean-flag      # Boolean flags
```

### Error Handling
All commands include comprehensive error handling:
- Input validation
- Range checking
- Clear error messages
- Usage examples

## 📄 License

This example is provided under the same license as the GenericCLI library.

## 🤝 Contributing

Feel free to improve this example by:
- Adding more sensor types
- Implementing additional protocols
- Enhancing error handling
- Adding more hardware control features

## 📚 Related Examples

- [Basic PlatformIO Example](../basic_cli_example/) - PlatformIO version with similar features
- [Advanced PlatformIO Example](../advanced_cli_example/) - Advanced features with persistent storage and JSON export
