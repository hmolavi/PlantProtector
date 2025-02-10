/** wifi.h
 *
 */

#ifndef __WIFI_H__
#define __WIFI_H__

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <stddef.h>

bool wifiInitialized;
bool wifiInternetConnected;

int save_wifi_credentials(const char *ssid, const char *password);
int load_wifi_credentials(char *ssid, size_t ssid_size, char *password, size_t pass_size);
void wifi_init_sta(const char *ssid, const char *password);

#endif // __WIFI_H__