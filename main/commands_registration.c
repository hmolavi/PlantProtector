///@file commands_registration.c
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///@brief Contains implementation for registering commands
///@version 1.0
///@date 2025-02-13
///
///@copyright Copyright (c) 2025
///

#include "include/commands_registration.h"

#include <stdio.h>
#include <stdlib.h>

#include "commands.c"
#include "esp_console.h"
#include "esp_err.h"

// TODO: Fix the helpers printing
#define COMMANDS_LIST                                                       \
    CMD("hello", "says hello", "", CmdHello)                                    \
    CMD("ssid", "Set new WiFi SSID", "<new_ssid>", CmdSsid)                 \
    CMD("password", "Set new WiFi password", "<new_password>", CmdPassword) \
    CMD("reset", "Reboot the board", "", CmdReboot)

int register_commands(void)
{
    COMMANDS_LIST
    return EXIT_SUCCESS;
}
