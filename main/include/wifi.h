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
void Wifi_InitSta();

/// @brief Clears the failed attempts and retries to connect to wifi
void WIFI_TryConnect();

/// @brief Indicator of wifi connection, only set to true if internet ver
bool wifiConnected;

#endif  // __WIFI_H__