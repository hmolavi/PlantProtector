/**
 * @file adc_manager.h
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 *
 * @brief Definitions for ADC module
 *
 * @version 1.0
 * @date 2025-03-08
 *
 * @copyright Copyright (c) 2025
 */

#ifndef __ADC_MANAGER_H__
#define __ADC_MANAGER_H__

#include <stdint.h>

#include "esp_err.h"

/**
 * Default reference voltage used for analog components, in mV
 *
 * Last time I measured with multimeter it was 3270 mV on the S3 dev board
 * @todo: Check this value later for confirmation
 * @todo: Update this value if using another board
 *
 * Used for esp_adc_cal_characterize() and ADC calibration in other places
 */
#define DEFAULT_VREF 3270.0

typedef enum ADCChannel_e {
#define ADC(name, unit, channel, attenuation) ADC_##name,
#include "adc_table.inc"
    ADC_MAX
} ADCChannel_t;

/**
 * @brief Initialize the ADC and calibration tables.
 *
 * This function initializes the ADC channels and sets up the calibration
 * tables if the calibration scheme is supported. It checks the eFuse for
 * calibration values and characterizes the ADC channels accordingly.
 *
 * @return esp_err_t
 */
esp_err_t ADC_Init(void);

/**
 * @brief Update the ADC values.
 *
 * This function scans all the defined ADC channels, reads the raw ADC values,
 * and converts them to calibrated voltage values if calibration is valid.
 */
esp_err_t ADC_Update(void);

/**
 * @brief Read the ADC value for a specific channel.
 *
 * This function returns the calibrated ADC value for the specified channel.
 *
 * @param chan The ADC channel to read.
 * @return uint32_t The calibrated ADC value in millivolts.
 */
uint32_t ADC_Read(ADCChannel_t chan);

/**
 * @brief Print the ADC values for all channels.
 *
 * This function prints the raw and calibrated ADC values for all defined
 * channels to the console.
 */
void ADC_Print(void);

#endif  // __ADC_MANAGER_H__
