///@file commands.c
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///@brief Location for all of the command handlers
///@version 1.0
///@date 2025-02-13
///
///@copyright Copyright (c) 2025
///

// #include "include/commands.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_console.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"

#define NETWORK_STORAGE_NAMESPACE "wifi_config"

static const char *TAG = "commands.c";

int CmdHello(int argc, char **argv)
{
    printf("HELLOOOO \n");
    return EXIT_SUCCESS;
}

int CmdSsid(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: %s <new_ssid>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *new_ssid = argv[1];
    size_t ssid_len = strlen(new_ssid);

    // Validate SSID length
    if (ssid_len >= 32) {
        printf("Error: SSID must be %d characters or less\n", 31);
        return EXIT_FAILURE;
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NETWORK_STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return EXIT_FAILURE;
    }

    // Save SSID
    err = nvs_set_str(nvs_handle, "ssid", new_ssid);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save SSID!");
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit changes in NVS!");
    }

    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Wi-Fi ssid saved to NVS");
    return EXIT_SUCCESS;
}

int CmdPassword(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: %s <new_password>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *new_password = argv[1];
    size_t pass_len = strlen(new_password);

    if (pass_len >= 32) {
        printf("Error: Password must be %d characters or less\n", 31);
        return EXIT_FAILURE;
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NETWORK_STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return EXIT_FAILURE;
    }

    // Save password
    err = nvs_set_str(nvs_handle, "password", new_password);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save Password!");
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit changes in NVS!");
    }

    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Wi-Fi password saved to NVS");
    return EXIT_SUCCESS;
}

int CmdReboot(int argc, char **argv)
{
    esp_restart();
    // If board cant reboot means command failed
    return EXIT_FAILURE;
}