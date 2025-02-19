///@file wifi.h
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///
///@version 1.0
///@date 2025-02-13
///
///@copyright Copyright (c) 2025
///

#ifndef __WIFI_H__
#define __WIFI_H__

#include <stdbool.h>
#include <stddef.h>

/// @brief Initialise the wifi settings and try to connect to wifi
void Wifi_InitSta(void);

/// @brief Clears the failed attempts and retries to connect to wifi
void Wifi_TryConnect(void);

/// @brief Wifi connection flag
extern bool g_wifi_connected;

/// @brief Wifi internet connection flag
extern bool g_wifi_internet_connected;

#endif  // __WIFI_H__