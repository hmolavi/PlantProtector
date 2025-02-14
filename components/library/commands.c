///@file commands.c
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///@brief Location for all of the command handlers
///@version 1.0
///@date 2025-02-13
///
///@copyright Copyright (c) 2025
///

#include <stdio.h>
#include <stdlib.h>
#include "include/commands.h"
#include "esp_console.h"

#define NETWORK_STORAGE_NAMESPACE "wifi_config"

int CmdHello(int argc, char **argv) {
    printf("HELLOOOO \n");
    return EXIT_SUCCESS;
  }
  
  int CmdSsid(int argc, char **argv) {
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
    esp_err_t err =
        nvs_open(NETWORK_STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
      return EXIT_FAILURE;
    }
  
    // Save SSID
    err = nvs_set_str(nvs_handle, "ssid", ssid);
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
  
  int cmd_password(int argc, char **argv) {
    if (argc != 2) {
      printf("Usage: %s <new_password>\n", argv[0]);
      return 1;
    }
  
    const char *new_password = argv[1];
    size_t pass_len = strlen(new_password);
  
    // Save with existing SSID
  
    nvs_handle_t nvs_handle;
    esp_err_t err =
        nvs_open(NETWORK_STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
      return EXIT_FAILURE;
    }
  
    // Save password
    err = nvs_set_str(nvs_handle, "password", password);
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
  
  