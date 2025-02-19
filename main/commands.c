///@file commands.c
///@brief Location for all of the command handlers
///@version 1.0
///@date 2025-02-13
///

// #include "include/commands.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../components/library/include/param_manager.h"
#include "esp_console.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "include/ascii_art.h"
#include "include/wifi.h"

esp_err_t Cmd_Art(int argc, char **argv)
{
    PrintAsciiArt();
    return ESP_OK;
}

esp_err_t Cmd_Ssid(int argc, char **argv)
{
    if (argc != 2) {
        printf("\n\nUsage: %s <new_ssid>\n", argv[0]);
        return ESP_ERR_INVALID_ARG;
    }

    const char *new_ssid = argv[1];
    size_t new_ssid_len = strlen(new_ssid);

    size_t max_ssid_len;
    const char *cur_ssid = Param_GetSsid(&max_ssid_len);

    // Validate ssid length
    if (new_ssid_len >= max_ssid_len) {
        printf("Error: SSID must be %d characters or less\n", max_ssid_len);
        return ESP_ERR_INVALID_SIZE;
    }

    printf("\nssid: (%s) -> (%s) ...", cur_ssid, new_ssid);
    esp_err_t err = Param_SetSsid(new_ssid, new_ssid_len);
    if (err != ESP_OK) {
        printf("Error: Failed with error code (%s)\n", esp_err_to_name(err));
        return err;
    }
    else {
        printf("Done");
    }

    return ESP_OK;
}

esp_err_t Cmd_Password(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: %s <new_password>\n", argv[0]);
        return ESP_ERR_INVALID_ARG;
    }

    const char *new_password = argv[1];
    size_t new_password_len = strlen(new_password);

    size_t max_password_len;
    const char *cur_password = Param_GetPassword(&max_password_len);

    // Validate password length
    if (new_password_len >= max_password_len) {
        printf("Error: Password must be %d characters or less\n", max_password_len);
        return ESP_ERR_INVALID_SIZE;
    }

    printf("password: (%s) -> (%s) ...", cur_password, new_password);
    esp_err_t err = Param_SetPassword(new_password, new_password_len);
    if (err != ESP_OK) {
        printf("Error: Failed with error code (%s)\n", esp_err_to_name(err));
        return err;
    }
    else {
        printf("Done");
    }

    return ESP_OK;
}

esp_err_t Cmd_Reboot(int argc, char **argv)
{
    esp_restart();
    // If board can't reboot means command failed
    return ESP_FAIL;
}

esp_err_t Cmd_Connect(int argc, char **argv)
{
    Wifi_TryConnect();
    return ESP_OK;
}

esp_err_t Cmd_Save(int argc, char **argv)
{
    printf("Saving dirty parameters\n");
    Param_SaveDirtyParameters();
    return ESP_OK;
}
