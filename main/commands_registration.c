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
#define COMMANDS_LIST                                                        \
    CMD("art", "print Plant Protector ascii art", "", Cmd_Art)               \
    CMD("ssid", "Set new WiFi SSID", "<new_ssid>", Cmd_Ssid)                 \
    CMD("password", "Set new WiFi password", "<new_password>", Cmd_Password) \
    CMD("reset", "Reboot the board", "", Cmd_Reboot)                         \
    CMD("connect", "Attempt to connect to wifi", "", Cmd_Connect)            \
    CMD("save", "Save the dirty parameters", "", Cmd_Save)

int CMD_CommandsInit(void)
{
    COMMANDS_LIST
    return ESP_OK;
}
