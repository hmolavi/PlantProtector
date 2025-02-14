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

#define NETWORK_STORAGE_NAMESPACE "wifi_config"

bool wifiInitialized;
bool wifiInternetConnected;

int save_wifi_credentials(const char *ssid, const char *password);
int load_wifi_credentials(char *ssid, size_t ssid_size, char *password, size_t pass_size);
void wifi_init_sta(const char *ssid, const char *password);

#endif  // __WIFI_H__