///@file app_main.c
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///@brief Starting point of project
///@version 1.0
///@date 2025-02-13
///
///@copyright Copyright (c) 2025
///

/*
    -*-C-*-
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Components */
#include "adc_manager.h"
#include "esp32_arduino_comm.h"
#include "gpio_manager.h"
#include "param_manager.h"

/* Other tings */
#include "ascii_art.h"
#include "commands_registration.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "private.h"  // Holds the wifi ssid and password
#include "thermistor.h"
#include "wifi.h"

static const char *TAG = "app_main.c";

void wifi_task(void *arg)
{
    // Initialize wifi
    Wifi_InitSta();

    const char *ssid = Param_GetSsid(NULL);
    const char *password = Param_GetPassword(NULL);

    while (!ssid || !password || strlen(ssid) == 0) {
        ESP_LOGI(TAG, "Waiting for valid SSID and password...");
        vTaskDelay(pdMS_TO_TICKS(1000));
        ssid = Param_GetSsid(NULL);
        password = Param_GetPassword(NULL);
    }

    Wifi_TryConnect();

    while (1) { /* Weeeeeee */
        esp_err_t ret;

        ret = ADC_Update();
        if (ret == ESP_OK) {
            Thermistor_Print();
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting up...\n");

    PrintAsciiArt();

    /* Initialize Components */
    esp_err_t ret;

    ret = ParamManager_Init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init Parameters");
        return;
    }
    ret = GPIO_Init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init GPIO");
        return;
    }
    ret = ADC_Init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init ADC");
        return;
    }

    CommError_t a = Comm_ExecuteCommand(COMM_RTC_Read, NULL);
    if (a) {
        printf("code is buns");
    }

    /* Update wifi name and password */
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

#ifdef PARAM_TESTING
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
#endif

    /////////////////////////////////////////////////
    printf("\n\n\n");
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = "PlantProtector>";
    repl_config.max_cmdline_length = 150;

    /* Register commands */
    printf("Registering commands...");
    esp_console_register_help_command();
    CMD_CommandsInit();
    printf("Done\n");

    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

    ESP_ERROR_CHECK(esp_console_start_repl(repl));

    // Create WiFi Task (Core 0)
    xTaskCreatePinnedToCore(&wifi_task, "wifi_task", 4096, NULL, 5, NULL, 0);

    // Note: ESP32-S3 uses APP CPU (Core 0) and PRO CPU (Core 1)
    // Adjust core affinity based on your specific requirements
}
