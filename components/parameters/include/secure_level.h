/**
 * @file secure_level.h
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 * @brief Manage security levels where lower value indicates more control
 *        Secure levels are defined in seucre_level.c
 *
 * @version 1.0
 * @date 2025-03-02
 *
 * @copyright Copyright (c) 2025
 */

#ifndef __SECURE_LEVEL_H__
#define __SECURE_LEVEL_H__

#include <stdint.h>

#include "esp_err.h"

/**
 * @brief Get the current security level.
 *
 * @return uint8_t The current security level.
 */
uint8_t SecureLevel(void);

/**
 * @brief Change the current security level.
 *
 * @param new_secure_level The new security level to set.
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t SecureLevel_Change(uint8_t new_secure_level);

#endif  // __SECURE_LEVEL_H__
