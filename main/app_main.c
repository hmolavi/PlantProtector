///@file app_main.c
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///@brief Starting point of project
///@version 1.0
///@date 2025-02-13
///
///@copyright Copyright (c) 2025
///

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../components/library/include/param_manager.h"
#include "ascii_art.c"
#include "cmd_nvs.h"
#include "cmd_system.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs_dev.h"
#include "esp_vfs_fat.h"
#include "include/commands_registration.h"
#include "include/internet_check.h"
#include "include/parameters.h"
#include "include/wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "private.h"  // Holds the wifi ssid and password

static const char *TAG = "app_main.c";

// static void initialize_nvs(void)
// {
//     esp_err_t err = nvs_flash_init();
//     if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
//         err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         err = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(err);
// }

void app_main(void)
{
    ESP_LOGI(TAG, "Starting up...");

    PrintAsciiArt();

    // Initialize NVS
    // initialize_nvs();
    param_manager_init();

    // Read parameter with code completion
    printf("Brightness upon wake: %d\n",  Param_GetBrightness());

    // Set parameter with type safety
    int new_brightness = Param_GetBrightness() + 10;
    Param_SetBrightness(new_brightness);

    printf("Brightness+10: %d\n", Param_GetBrightness());

    char *ssid_p = Param_GetSsid();
    if (ssid_p != NULL) {
        printf("Ssid upon wake: %s\n", ssid_p);

        // Add one char to the ssid and set the value
        size_t ssid_len = strlen(ssid_p);
        if (ssid_len < 31) { // Ensure there is space for one more character and null terminator
            ssid_p[ssid_len] = 'X'; // Add 'X' to the end of ssid
            ssid_p[ssid_len + 1] = '\0'; // Null terminate the string
            Param_SetSsid(ssid_p); // Set the new ssid value
            printf("Ssid+X: %s\n", Param_GetSsid());
        } else {
            printf("SSID length is too long to add a character\n");
        }
    } else {
        printf("Failed to get ssid upon wake\n");
    }


    /////////////////////////////////////////////////
    printf("---CMD SHOULD BE INTERACTIVE FROM HERE\n");
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = "PlantProtector>";
    repl_config.max_cmdline_length = 150;

    /* Register commands */
    printf("Registering commands...");
    esp_console_register_help_command();
    register_system();
    register_nvs();
    register_commands();
    printf("Done\n");

    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

    ESP_ERROR_CHECK(esp_console_start_repl(repl));

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    char ssid[32] = {0};
    char password[64] = {0};

    int res = load_wifi_credentials(ssid, sizeof(ssid), password, sizeof(password));
    if (res == EXIT_SUCCESS) {
        printf("LOADED SSID AND PASS from NVS!!\n");
    }
    else {
        printf("Should use defaults now...");
        strncpy(ssid, DEFAULT_SSID, sizeof(ssid));
        strncpy(password, DEFAULT_PASS, sizeof(password));
        printf("Done\n");
    }

    wifi_init_sta(ssid, password);
    check_internet_connection();
}