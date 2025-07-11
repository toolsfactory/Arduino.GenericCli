# Generic CLI Basic Example

This example demonstrates how to use the GenericCLI library to create a powerful command-line interface for ESP32 microcontrollers. The example showcases various features of the library including custom commands, standard commands, and advanced CLI functionality.

## Features Demonstrated

### Core CLI Features
- ✅ **Command Registration** - Register custom commands with descriptions and usage info
- ✅ **Argument Parsing** - Support for positional arguments and flags (`--flag=value`)
- ✅ **Command History** - Navigate through command history using arrow keys
- ✅ **Input Line Editing** - Cursor movement, backspace, delete, home/end keys
- ✅ **ANSI Color Support** - Colored output for better user experience
- ✅ **Built-in Help System** - Automatic help generation for all commands

### Standard Commands
- `exit` - Exit the CLI with confirmation
- `clear` - Clear the terminal screen
- `reboot` - Restart the ESP32 with optional delay
- `status` - Show comprehensive system status
- `colors` - Control ANSI color output
- `history` - Display and manage command history

### Custom Commands
- `led` - Control the built-in LED (on/off/toggle/blink)
- `gpio` - Read from and write to GPIO pins
- `sysinfo` - Display detailed system information
- `wifi` - WiFi management (scan/connect/disconnect/status)
- `mem` - Memory usage analysis

## Hardware Requirements

- ESP32 development board (ESP32, ESP32-S2, ESP32-S3, ESP32-C3)
- Built-in LED or external LED connected to GPIO 2
- USB cable for programming and serial communication

## Software Requirements

- PlatformIO IDE or PlatformIO Core
- Serial terminal that supports ANSI escape codes (recommended):
  - **Windows**: PuTTY, Windows Terminal, VS Code Terminal
  - **macOS**: Terminal.app, iTerm2
  - **Linux**: gnome-terminal, xterm, konsole

## Installation and Setup

### 1. Clone or Copy the Example

```bash
# If you're using the GenericCLI as a git submodule or separate repo
cd your_project_directory
cp -r path/to/Arduino.GenericCli/examples/basic_cli_example .
cd basic_cli_example
```

### 2. Build and Upload

```bash
# Using PlatformIO CLI
pio run --target upload

# Or using PlatformIO IDE
# Just click the upload button
```

### 3. Open Serial Monitor

```bash
# Using PlatformIO CLI
pio device monitor

# Or use your preferred serial terminal at 115200 baud
```

## Usage Examples

Once uploaded and connected, you'll see a welcome message. Here are some example commands to try:

### Basic Commands
```bash
# Get help for all commands
help

# Get help for a specific command
help led

# Show command history
history

# Clear screen
clear
```

### LED Control
```bash
# Turn LED on
led on

# Turn LED off
led off

# Toggle LED state
led toggle

# Blink LED 5 times with 200ms delay
led blink --count=5 --delay=200
```

### GPIO Operations
```bash
# Read from GPIO pin 4 with pullup
gpio 4 read --pullup

# Write HIGH to GPIO pin 5
gpio 5 write high

# Write LOW to GPIO pin 5
gpio 5 write 0
```

### System Information
```bash
# Basic system info
sysinfo

# Detailed system info
sysinfo --verbose

# Memory information
mem

# Detailed memory info
mem --detailed

# System status (built-in command)
status --compact
```

### WiFi Management
```bash
# Scan for available networks
wifi scan

# Connect to a WiFi network
wifi connect "YourNetworkName" "YourPassword"

# Check WiFi status
wifi status

# Disconnect from WiFi
wifi disconnect
```

## Command Reference

### LED Command
```bash
led <action> [options]

Actions:
  on      - Turn LED on
  off     - Turn LED off  
  toggle  - Toggle LED state
  blink   - Blink LED multiple times

Options:
  --count=n    - Number of blinks (1-50, default: 3)
  --delay=ms   - Delay between blinks (50-5000ms, default: 500)

Examples:
  led on
  led blink --count=10 --delay=100
```

### GPIO Command
```bash
gpio <pin> <operation> [value] [options]

Operations:
  read   - Read digital value from pin
  write  - Write digital value to pin

Values (for write):
  0, low, off   - Write LOW
  1, high, on   - Write HIGH

Options:
  --pullup     - Enable internal pullup (read only)
  --pulldown   - Enable internal pulldown (read only)

Examples:
  gpio 4 read --pullup
  gpio 2 write high
  gpio 13 write 0
```

### WiFi Command
```bash
wifi <action> [parameters]

Actions:
  scan         - Scan for available networks
  connect      - Connect to a network
  disconnect   - Disconnect from current network
  status       - Show connection status

Examples:
  wifi scan
  wifi connect "MyNetwork" "MyPassword"
  wifi connect "OpenNetwork"
  wifi status
```

### System Info Commands
```bash
# System information
sysinfo [--verbose]

# Memory information  
mem [--detailed]

# Built-in status command
status [--compact] [--json]
```

## Advanced Features

### Command History
- Use ↑ and ↓ arrow keys to navigate through command history
- History is automatically saved and limited to configured size
- Use `history` command to view all previous commands
- Use `history clear` to clear the history

### Input Line Editing
- **Arrow Keys**: Navigate through history (↑/↓) and cursor position (←/→)
- **Home/End**: Jump to beginning/end of line
- **Backspace**: Delete character before cursor
- **Delete**: Delete character at cursor
- **Insert Mode**: Type anywhere in the line

### Color Support
- Automatic color coding for different message types:
  - 🟢 **Success**: Green text with ✓ icon
  - 🔴 **Error**: Red text with ✗ icon  
  - 🟡 **Warning**: Yellow text with ⚠ icon
  - 🔵 **Info**: Cyan text with ℹ icon
- Use `colors off` to disable colors, `colors on` to enable
- Use `colors test` to see all available colors

### Flag and Argument Parsing
The CLI supports sophisticated argument parsing:

```bash
# Positional arguments
command arg1 arg2 arg3

# Named flags with values
command --flag=value --otherflag=123

# Boolean flags
command --verbose --force

# Mixed usage
command arg1 --flag=value arg2 --boolean-flag
```

## Customization

### Adding Your Own Commands

```cpp
// Register a simple command
cli.registerCommand("mycommand", "Description of my command", 
                   "mycommand <arg1> [--flag]", 
                   [](const CLIArgs& args) {
                       // Your command implementation
                       Serial.println("Hello from my command!");
                   }, "Custom");

// Register a more complex command
cli.registerCommand("complex", "A complex command example",
                   "complex <required_arg> [optional_arg] [--flag=value]",
                   handleComplexCommand, "Custom");

void handleComplexCommand(const CLIArgs& args) {
    // Check argument count
    if (args.empty()) {
        cli.printError("Missing required argument");
        return;
    }
    
    // Get positional arguments
    String requiredArg = args.getPositional(0);
    String optionalArg = args.getPositional(1, "default_value");
    
    // Get flags
    String flagValue = args.getFlag("flag", "default");
    bool boolFlag = args.hasFlag("verbose");
    
    // Your logic here
    cli.printSuccess("Command executed successfully!");
}
```

### Configuring the CLI

```cpp
CLIConfig config;
config.prompt = "my_device";                    // Custom prompt
config.welcomeMessage = "Welcome to My Device"; // Custom welcome
config.colorsEnabled = true;                    // Enable/disable colors
config.echoEnabled = true;                      // Enable/disable echo
config.historySize = 50;                        // Command history size
config.caseSensitive = false;                   // Case sensitivity

cli.setConfig(config);
```

## Troubleshooting

### Common Issues

1. **No output in serial monitor**
   - Check baud rate is set to 115200
   - Ensure USB cable supports data transfer
   - Try different serial terminal software

2. **Colors not working**
   - Use a terminal that supports ANSI escape codes
   - Try `colors test` command to verify support
   - Use `colors off` to disable if problematic

3. **Commands not recognized**
   - Check spelling and case sensitivity
   - Use `help` to see all available commands
   - Ensure command was registered properly

4. **Arrow keys not working**
   - Use a proper terminal emulator (not Arduino IDE serial monitor)
   - Try PuTTY, VS Code terminal, or system terminal

### Performance Notes

- The CLI is designed to be non-blocking
- Commands should avoid long delays
- For long-running operations, consider breaking them into smaller chunks
- The CLI uses minimal memory and CPU resources

## License

This example is provided under the same license as the GenericCLI library.

## Support

For issues, questions, or contributions, please refer to the main GenericCLI repository.
