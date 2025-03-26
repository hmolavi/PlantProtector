///@file commands_registration.h
///@version 1.0
///@date 2025-02-13
///@copyright Copyright (c) 2025

#ifndef __COMMANDS_REGISTRATION_H__
#define __COMMANDS_REGISTRATION_H__

#include <stdio.h>
#include <stdlib.h>

#include "esp_console.h"

#define CMD(name, helptxt, hinttxt, function)          \
    char helptxt_##function[] = helptxt;               \
    char hinttxt_##function[] = hinttxt;               \
    const esp_console_cmd_t console_cmd_##function = { \
        .command = name,                               \
        .help = helptxt_##function,                    \
        .hint = hinttxt_##function,                    \
        .func = &function};                            \
    ESP_ERROR_CHECK(esp_console_cmd_register(&console_cmd_##function));

/// @brief Initializes all of the command-line-commands
int CMD_CommandsInit(void);

#endif  // __COMMANDS_REGISTRATION_H__

/*

#define CMD_WITHOUT_HINT(name, helptxt, function)      \
    char helptxt_##function[] = helptxt;               \
    const esp_console_cmd_t console_cmd_##function = { \
        .command = name,                               \
        .help = helptxt_##function,                    \
        .hint = NULL,                                  \
        .func = &function};                            \
    ESP_ERROR_CHECK(esp_console_cmd_register(&console_cmd_##function));

#define CMD_IMPL(_1, _2, _3, NAME, ...) NAME
#define CMD(...) CMD_IMPL(__VA_ARGS__, CMD_WITH_HINT, CMD_WITHOUT_HINT)(__VA_ARGS__)

char helptxt_Cmd_Art[] = "print Plant Protector ascii art";
char hinttxt_Cmd_Art[] = "";
const esp_console_cmd_t console_cmd_Cmd_Art = {.command = "art", .help = helptxt_Cmd_Art, .hint = hinttxt_Cmd_Art, .func = &Cmd_Art};
ESP_ERROR_CHECK(esp_console_cmd_register(&console_cmd_Cmd_Art));

*/