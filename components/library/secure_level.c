///@file secure_level.c
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///@brief Manage security levels where lower value indicates more control
///       Secure levels are defined as:
///         0 - Factory, full access
///         1 - Maintenance
///         2 - User level
///
///@version 1.0
///@date 2025-03-02
///
///@copyright Copyright (c) 2025
///

#include "include/secure_level.h"

#include <stdint.h>
#include <stdio.h>

#include "esp_err.h"

static uint8_t CurrentSecureLevel;

uint8_t SecureLevel(void) { return CurrentSecureLevel; }

esp_err_t SecureLevel_Change(uint8_t new_secure_level)
{
    printf("Secure Level (%u) -> (%u)\n", CurrentSecureLevel, new_secure_level);
    CurrentSecureLevel = new_secure_level;
    return ESP_OK;
}
