/**
 * @file gpio_manager.c
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 *
 * @brief GPIO control library for ESP32.
 *
 * This file contains functions to initialize and control GPIO pins on the ESP32.
 * The pin configurations are defined in gpio_table.inc.
 *
 * @version 1.0
 * @date 2025-03-08
 *
 * @copyright Copyright (c) 2025
 */

#include "gpio_manager.h"

#include <stdint.h>
#include <stdio.h>
#include <strings.h>

#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "gpio_manager.c";

/**
 * @brief Defines whether a GPIO pin's signal should be inverted or not.
 */
typedef enum GPIOPinInvert_e {
    GPIO_INV,  /**< Invert the GPIO signal */
    GPIO_TRUE, /**< Do not invert the GPIO signal */
} GPIOPinInvert_t;

/**
 * @brief Defines the different types of GPIO pins
 */
typedef enum GPIOPinType_e {
    GPIO_GPIO,  /**< Pin is a standard GPIO */
    GPIO_RTC,   /**< Pin is controlled by RTC GPIO */
    GPIO_IOMUX, /**< Pin is connected via the IOMUX to some peripheral */
} GPIOPinType_t;

/**
 * @brief Structure to define a GPIO pin configuration.
 */
typedef struct GPIOPinDef_s {
    char *name;                /**< Pin name for the user */
    uint8_t index;             /**< Bit index */
    GPIOPinType_t type;        /**< GPIO or RTC control */
    gpio_config_t config;      /**< ESP32 GPIO configuration structure */
    gpio_drive_cap_t strength; /**< Drive Capability setting */
    uint32_t signal_idx;       /**< IOMUX input signal mapping */
    uint32_t func_idx;         /**< IOMUX output signal mapping */
    bool oen_inv;              /**< Invert the Output enable for IOMUX out */
    GPIOPinInvert_t inv;       /**< Software Invert the signal */
    uint32_t def;              /**< Default value after Init */
    char *desc;                /**< Description of port */
} GPIOPinDef_t;

/**
 * @brief GPIO pin definitions and initialization.
 *
 * This section defines the GPIO pins used in the application and their configurations.
 * The GPIOPinDef_t structure holds the configuration for each pin, including its name,
 * bit index, type, mode, pull-up, pull-down, drive strength, signal index, function index,
 * output enable inversion, inversion, default state, and description.
 *
 * The GPIOPins array is populated using the PIN macro, which is defined to initialize
 * each field of the GPIOPinDef_t structure. The macro parameters are:
 * - name: The name of the GPIO pin.
 * - bit_index: The bit index of the GPIO pin.
 * - type: The type of the GPIO pin (e.g., input, output).
 * - mode: The mode of the GPIO pin (e.g., input, output, open-drain).
 * - pu: Pull-up configuration (enabled or disabled).
 * - pd: Pull-down configuration (enabled or disabled).
 * - str: Drive strength of the GPIO pin.
 * - sig_idx: Signal index associated with the GPIO pin.
 * - func_idx: Function index associated with the GPIO pin.
 * - oen_inv: Output enable inversion.
 * - inv: Inversion configuration.
 * - def: Default state of the GPIO pin.
 * - desc: Description of the GPIO pin.
 *
 * The gpio_table.inc file is included to provide the actual pin definitions.
 */
static const GPIOPinDef_t GPIOPins[] = {
#define PIN(name, bit_index, type, mode, pu, pd, str, sig_idx, func_idx, oen_inv, inv, def, desc) \
    {#name,                                                                                       \
     bit_index,                                                                                   \
     GPIO_##type,                                                                                 \
     {1ULL << bit_index, GPIO_MODE_##mode, GPIO_PULLUP_##pu, GPIO_PULLDOWN_##pd,                  \
      GPIO_INTR_DISABLE},                                                                         \
     GPIO_DRIVE_CAP_##str,                                                                        \
     sig_idx,                                                                                     \
     func_idx,                                                                                    \
     oen_inv,                                                                                     \
     GPIO_##inv,                                                                                  \
     def,                                                                                         \
     desc},

#include "gpio_table.inc"

    /* Empty record to account for PIN_MAX */
    {NULL, 0, 0, {0, 0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, NULL},
};

/*-----------------------------------------------------------*/

esp_err_t GPIO_Init(void)
{
    const GPIOPinDef_t *pin;
    const gpio_config_t *config;

    for (uint32_t i = 0; i < MAX_GPIO_PINS; i++) {
        pin = &GPIOPins[i];
        config = &pin->config;
        ESP_LOGI(TAG, "Configuring %s:", pin->name);
        gpio_config(config);
        if (config->mode & GPIO_MODE_OUTPUT) {
            /* Only set the drive strength if we are being an output */
            gpio_set_drive_capability(pin->index, pin->strength);
        }

        if (pin->type == GPIO_RTC) {
            /* this is an RTC domain controlled pin, initialize it that way */
            rtc_gpio_init(pin->index);
        }

        if (pin->type == GPIO_IOMUX) {
            gpio_iomux_in(pin->index, pin->signal_idx);
            gpio_iomux_out(pin->index, pin->func_idx, pin->oen_inv);
        }

        /* Set the default value, but only for outputs, avoid also trying
           to change the IOMUX controlled ones. */
        if ((pin->type != GPIO_IOMUX) && (config->mode & GPIO_MODE_OUTPUT)) {
            GPIO_Set(i, pin->def);
        }
    }

    return ESP_OK;
}

/*-----------------------------------------------------------*/

uint32_t GPIO_Pin(GPIOPins_t pin)
{
    if (pin >= MAX_GPIO_PINS) {
        ESP_LOGE(TAG, "pin %u is not allowed in %s()", pin, __FUNCTION__);
        return -1;
    }

    return GPIOPins[pin].index;
}

/*-----------------------------------------------------------*/

esp_err_t GPIO_Set(GPIOPins_t pin, uint32_t val)
{
    uint32_t pinval;
    if (pin >= MAX_GPIO_PINS) {
        ESP_LOGE(TAG, "pin %u out of range 0..%u in %s()", pin, MAX_GPIO_PINS - 1, __FUNCTION__);
        return ESP_ERR_INVALID_ARG;
    }

    if (pin == MAX_GPIO_PINS) {
        ESP_LOGE(TAG, "pin %u is not allowed in %s()", pin, __FUNCTION__);
        return ESP_ERR_INVALID_ARG;
    }

    if (GPIOPins[pin].inv == GPIO_INV) {
        pinval = !val;
    }
    else {
        pinval = val;
    }

    if (pin < MAX_GPIO_PINS) {
        switch (GPIOPins[pin].type) {
            case GPIO_GPIO:
                gpio_set_level(GPIOPins[pin].index, pinval);
                break;
            case GPIO_RTC:
                rtc_gpio_set_level(GPIOPins[pin].index, pinval);
                break;
            default:
                ESP_LOGE(TAG, "pin %u is not a GPIO controlled output allowed in %s()", pin, __FUNCTION__);
                return ESP_ERR_INVALID_ARG;
        }
    }
    else {
        ESP_LOGE(TAG, "pin %u is MAX_GPIO_PINS %s()", pin, __FUNCTION__);
        return ESP_ERR_INVALID_ARG;
    }

    return ESP_OK;
}

/*-----------------------------------------------------------*/

uint32_t GPIO_Read(GPIOPins_t pin)
{
    uint32_t val;
    if (pin > MAX_GPIO_PINS) {
        ESP_LOGE(TAG, "pin %u out of range 0..%u in %s()", pin, MAX_GPIO_PINS - 1, __FUNCTION__);
        return 0;
    }
    else if (pin == MAX_GPIO_PINS) {
        ESP_LOGE(TAG, "pin %u is MAX_GPIO_PINS %s()", pin, __FUNCTION__);
        return 0;
    }

    switch (GPIOPins[pin].type) {
        case GPIO_GPIO:
            val = gpio_get_level(GPIOPins[pin].index);
            break;
        case GPIO_RTC:
            val = rtc_gpio_get_level(GPIOPins[pin].index);
            break;
        default:
            ESP_LOGE(TAG, "pin %u is not a GPIO controlled input allowed in %s()", pin, __FUNCTION__);
            return 0;
    }

    if (GPIOPins[pin].inv == GPIO_INV) {
        val = !val;
    }

    return val ? 1 : 0;
}

/*-----------------------------------------------------------*/

void GPIO_PrintNames(void)
{
    for (uint32_t i = 0; i < MAX_GPIO_PINS; i++) {
        printf("%17s: (PIN %u) %s\n", GPIOPins[i].name, GPIOPins[i].index, GPIOPins[i].desc);
    }
}

/*-----------------------------------------------------------*/

uint32_t GPIO_FindPin(char *name)
{
    for (uint32_t i = 0; i < MAX_GPIO_PINS; i++) {
        if ((GPIOPins[i].name != NULL) && (strcasecmp(name, GPIOPins[i].name) == 0)) {
            return i;
        }
    }
    return MAX_GPIO_PINS;
}