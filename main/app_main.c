///@file app_main.c
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///@brief Starting point of project
///@version 1.0
///@date 2025-02-13
///
///@copyright Copyright (c) 2025
///

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../components/library/include/param_manager.h"
#include "cmd_nvs.h"
#include "cmd_system.h"
#include "esp_console.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs_dev.h"
#include "esp_vfs_fat.h"
#include "include/ascii_art.h"
#include "include/commands_registration.h"
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
    ESP_LOGI(TAG, "Starting up...\n");

    PrintAsciiArt();

    // Initialize NVS
    Param_ManagerInit();
    // Init wifi
    Wifi_InitSta();

    const char *ssid = Param_GetSsid(NULL);
    char *default_ssid = DEFAULT_SSID;
    if (strcmp(ssid, default_ssid) != 0) {
        printf("ssid: (%s) -> (%s): ", ssid, default_ssid);
        esp_err_t err = Param_SetSsid(default_ssid, strlen(default_ssid));
        printf("%s\n", (err != ESP_OK) ? "Failed" : "Done");
    }

    const char *password = Param_GetPassword(NULL);
    char *default_password = DEFAULT_PASS;
    if (strcmp(password, default_password) != 0) {
        printf("password: (%s) -> (%s): ", password, default_password);
        esp_err_t err = Param_SetPassword(default_password, strlen(default_password));
        printf("%s\n", (err != ESP_OK) ? "Failed" : "Done");
    }

    /////////////////////////////////////////////////

    printf("Brightness upon wake: %d\n", Param_GetBrightness());

    // Set parameter with type safety
    int new_brightness = Param_GetBrightness() + 10;
    Param_SetBrightness(new_brightness);

    printf("Brightness+10: %d\n", Param_GetBrightness());

    size_t num_elements;
    const int *my_int_arr2 = Param_GetMyArray(&num_elements);

    printf("Array elements: ");
    for (int i = 0; i < num_elements; i++) {
        printf("%d ", my_int_arr2[i]);
    }
    printf("\n");

    int local_copy[4];
    ESP_ERROR_CHECK(Param_CopyMyArray(local_copy, sizeof(local_copy)));

    for (int i = 0; i < 4; i++) {
        local_copy[i] += 5;
    }

    printf("update elements: ");
    for (int i = 0; i < num_elements; i++) {
        printf("%d ", local_copy[i]);
    }
    printf("\n");

    ESP_ERROR_CHECK(Param_SetMyArray(local_copy, num_elements));

    /////////////////////////////////////////////////
    printf("\n\n\n");
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = "PlantProtector>";
    repl_config.max_cmdline_length = 150;

    /* Register commands */
    printf("Registering commands...");
    esp_console_register_help_command();
    register_system();
    register_nvs();
    CMD_CommandsInit();
    printf("Done\n");

    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

    ESP_ERROR_CHECK(esp_console_start_repl(repl));

    while (!ssid || !password || strlen(ssid) == 0) {
        ESP_LOGI(TAG, "Waiting for valid SSID and password...");
        vTaskDelay(pdMS_TO_TICKS(1000));
        ssid = Param_GetSsid(NULL);
        password = Param_GetPassword(NULL);
    }

    Wifi_TryConnect();
}