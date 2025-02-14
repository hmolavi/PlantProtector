///@file commands_registration.h
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///
///@version 1.0
///@date 2025-02-13
///
///@copyright Copyright (c) 2025
///

#ifndef __COMMANDS_REGISTRATION_H__
#define __COMMANDS_REGISTRATION_H__

#include <stdio.h>
#include <stdlib.h>

#include "esp_console.h"

#define ADD_CMD(name, helptxt, hinttxt, function)      \
    char helptxt_##function[] = helptxt;               \
    char hinttxt_##function[] = hinttxt;               \
    const esp_console_cmd_t console_cmd_##function = { \
        .command = name,                               \
        .help = helptxt_##function,                    \
        .hint = hinttxt_##function,                    \
        .func = &function};                            \
    ESP_ERROR_CHECK(esp_console_cmd_register(&console_cmd_##function));

/// @brief Initializes all of the command-line-commands
int register_commands(void);

#endif  // __COMMANDS_REGISTRATION_H__
