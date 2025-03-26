/**
 * @file gpio_manager.h
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 * 
 * @brief GPIO control library for ESP32.
 *
 * The GPIO pins are defined in an external file (gpio_table.inc) and are 
 * enumerated in the GPIOPins_t enumeration.
 * 
 * @version 1.0
 * @date 2025-03-08
 *
 * @copyright Copyright (c) 2025
 */

#ifndef __GPIO_MANAGER_H__
#define __GPIO_MANAGER_H__

#include <stdint.h>
#include "esp_err.h"

/**
 * @enum GPIOPins_t
 * @brief Enumeration of GPIO pins.
 *
 * This enumeration lists all the GPIO pins from the include file.
 *
 */
typedef enum GPIOPins_e {
#define PIN(name, bit_index, type, mode, pu, pd, str, sig_idx, func_idx, oen_inv, inv, default, desc) \
    PIN_##name,
#include "gpio_table.inc"
    MAX_GPIO_PINS,
} GPIOPins_t;

/**
 * @brief Check for accidental duplicate GPIO pin definitions.
 *
 * The compiler will generate an error if there are duplicate pin numbers.
 * This is achieved by defining a fake pin for each bit index in the GPIO
 * table.
 *
 * The pin numbers should be unique and should not have leading zeros.
 */
typedef enum GPIOPinsDuplicates_e {
#define PIN(name, bit_index, type, mode, pu, pd, str, sig_idx, func_idx, oen_inv, inv, default, desc) \
    DUPLICATE_PIN_##bit_index,
#include "gpio_table.inc"
} GPIOPinsDuplicates_t;

/**
 * @brief Initialize all GPIO pins.
 *
 * This function initializes all GPIO pins as defined in the GPIOPins array.
 * It configures each pin according to its settings and sets the default value for output pins.
 */
esp_err_t GPIO_Init(void);

/**
 * @brief Return the natural GPIO index for a given pin.
 *
 * This function returns the GPIO index for a given pin, which is useful for configuring peripherals
 * in the ESP HAL code. It allows the code to query the index, so if the pin numbers change,
 * only the master GPIO table needs to be updated.
 *
 * @param pin The pin to query.
 * @return The GPIO index for the given pin, or -1 if the pin is not allowed.
 */
uint32_t GPIO_Pin(GPIOPins_t pin);

/**
 * @brief Set the value of a GPIO pin.
 *
 * This function sets the value of a GPIO pin. It handles inversion and different pin types (GPIO, RTC).
 *
 * @param pin The pin to set.
 * @param val The value to set the pin to (0 or 1).
 */
esp_err_t GPIO_Set(GPIOPins_t pin, uint32_t val);

/**
 * @brief Read the value of a GPIO pin.
 *
 * This function reads the value of a GPIO pin. It handles inversion and different pin types (GPIO, RTC).
 *
 * @param pin The pin to read.
 * @return The value of the pin (0 or 1).
 */
uint32_t GPIO_Read(GPIOPins_t pin);

/**
 * @brief Print the names of all the GPIO pins.
 *
 * This function prints the names and descriptions of all the GPIO pins defined in the GPIOPins array.
 */
void GPIO_PrintNames(void);

/**
 * @brief Find a pin by name.
 *
 * This function searches for a pin by its name and returns its index.
 *
 * @param name The name of the pin to find.
 * @return The index of the pin, or MAX_GPIO_PINS if the pin is not found.
 */
uint32_t GPIO_FindPin(char *name);

#endif  // __GPIO_MANAGER_H__