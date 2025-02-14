///@file commands.h
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///
///@version 1.0
///@date 2025-02-13
///
///@copyright Copyright (c) 2025
///

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include <stdio.h>
#include <stdlib.h>

#include "esp_console.h"

int CmdHello(int argc, char **argv);
int CmdSsid(int argc, char **argv);
int CmdPassword(int argc, char **argv);

#endif  // __COMMANDS_REGISTRATION_H__
