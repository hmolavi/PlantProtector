/**
 * @file secure_level.c
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 * @brief Manage security levels where lower value indicates more control
 *        Secure levels are defined as:
 *          0 - Full access
 *          1 - Maintenance
 *          2 - User level
 *
 * @version 1.0
 * @date 2025-03-02
 *
 * @copyright Copyright (c) 2025
 */

#include "secure_level.h"

#include <stdint.h>
#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "secure_level.c";

/*

TODO: Create param_table.inc file for the security levels

*/

const char *level_meanings[] = {"Full access", "Maintenance", "User level"};

static uint8_t CurrentSecureLevel = 2;

uint8_t SecureLevel(void) { return CurrentSecureLevel; }

esp_err_t SecureLevel_Change(uint8_t new_secure_level)
{
    ESP_LOGW(TAG, "Secure Level Changing (%u %s) -> (%u %s)",
             CurrentSecureLevel, level_meanings[CurrentSecureLevel],
             new_secure_level, level_meanings[new_secure_level]);
    CurrentSecureLevel = new_secure_level;
    return ESP_OK;
}
