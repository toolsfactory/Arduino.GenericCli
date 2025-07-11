#ifndef CLI_STANDARD_COMMANDS_H
#define CLI_STANDARD_COMMANDS_H

#include "generic_cli.h"

/**
 * Standard CLI Commands
 * 
 * This file provides commonly used CLI commands that can be easily
 * registered with any GenericCLI instance. These commands provide
 * basic functionality like help, history, system control, etc.
 * 
 * Usage:
 *   GenericCLI cli;
 *   CLIStandardCommands::registerAllStandardCommands(cli);
 *   cli.begin();
 */

namespace CLIStandardCommands {
    
    // ========================================================================
    // COMMAND REGISTRATION FUNCTIONS
    // ========================================================================
    
    // Individual command registration
    void registerExitCommand(GenericCLI& cli);
    void registerClearCommand(GenericCLI& cli);
    void registerRebootCommand(GenericCLI& cli);
    void registerStatusCommand(GenericCLI& cli);
    void registerColorsCommand(GenericCLI& cli);
    void registerHistoryCommand(GenericCLI& cli);
    
    // Convenience registration functions
    void registerAllStandardCommands(GenericCLI& cli);
    void registerBasicCommands(GenericCLI& cli);
    
    // ========================================================================
    // EXIT CONTROL
    // ========================================================================
    
    // Check if user requested to exit the CLI
    bool isExitRequested();
    
    // Reset the exit flag (if you want to continue after exit was requested)
    void resetExitFlag();
}

#endif // CLI_STANDARD_COMMANDS_H