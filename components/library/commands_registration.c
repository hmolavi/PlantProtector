///@file commands_registration.c
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///@brief
///@version 1.0
///@date 2025-02-13
///
///@copyright Copyright (c) 2025
///

#include <stdio.h>
#include <stdlib.h>

#include "esp_console.h"
#include "esp_err.h"
#include "include/commands_registration.h"


int register_commands(void) {
  ADD_CMD("hello", "says hello", "", CmdHello)
  ADD_CMD("ssid", "Set new WiFi SSID", "<new_ssid>", CmdSsid)
  ADD_CMD("password", "Set new WiFi password", "<new_password>", cmd_password)
  return EXIT_SUCCESS;
}
