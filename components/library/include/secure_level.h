///@file secure_level.h
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///@brief Functions for controlling the security level
///@version 1.0
///@date 2025-03-02
///
///@copyright Copyright (c) 2025
///

#ifndef __SECURELEVEL_H__
#define __SECURELEVEL_H__

#include <stdint.h>
#include "esp_err.h"

///@brief Return the current secure level
///
///@return uint8_t 
uint8_t SecureLevel(void);

/// @brief Modify the secure level
/// @param new_secure_level 
/// @return esp_err_t
esp_err_t SecureLevel_Change(uint8_t new_secure_level);

#endif
