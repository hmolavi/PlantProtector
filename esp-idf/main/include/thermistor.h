/**
 * @file thermistor.h
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 * 
 * @brief Thermistor functionality: declaration of functions and data types used in thermistor.c
 * 
 * This header defines the interface for managing thermistors, including
 * enumerating thermistor channels, checking thermistor failure states,
 * reading temperatures, and printing thermistor data.
 * 
 * @version 1.0
 * @date 2025-03-08
 *
 * @copyright Copyright (c) 2025
 */

#ifndef __THERMISTOR_H__
#define __THERMISTOR_H__

#include <stdint.h>

/**
 * @enum ThermistorChannel_t
 * @brief Identifiers for each thermistor channel in the system.
 *
 * This enum defines named constants for each thermistor configured
 * in thermistor.inc. The final value THERMISTOR_MAX indicates
 * the total number of thermistor channels.
 */
typedef enum {
#define THERMISTOR(name, adc, t0, rt0, b, r, vmin, vmax) THERMISTOR_##name,
#include "thermistor.inc"
    THERMISTOR_MAX
} ThermistorChannel_t;

/**
 * @brief Checks if the specified thermistor has failed.
 *
 * A thermistor’s status is considered failed if its readings fall outside
 * the configured minimum and maximum boundaries.
 *
 * @param chan The thermistor channel to query.
 * @return Non-zero if the thermistor has failed, 0 otherwise.
 */
uint32_t ThermistorFailed(ThermistorChannel_t chan);

/**
 * @brief Reads and returns the temperature in Celsius for the specified thermistor channel.
 *
 * If the thermistor has failed (value out of range), this function
 * returns 0.0 and sets the thermistor’s failed state accordingly.
 *
 * @param chan The thermistor channel to read.
 * @return Temperature in Celsius as a float.
 */
float ThermistorTemp(ThermistorChannel_t chan);

/**
 * @brief Prints diagnostic information for all thermistors.
 *
 * This function prints the temperature, raw ADC reading,
 * and the status (working or failed) for each thermistor.
 */
void Thermistor_Print(void);

#endif  // __THERMISTOR_H__
