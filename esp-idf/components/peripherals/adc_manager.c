/**
 * @file adc_manager.c
 * @author Hossein Molavi
 * 
 * @brief ADC driver code
 *
 * This file contains the implementation of the ADC manager for the ESP32.
 * It initializes the ADC channels, starts the DMA, processes the data when a
 * buffer is ready.
 *
 * @version 1.0
 * @date 2025-03-08
 *
 * @copyright Copyright (c) 2025
 */

#include "adc_manager.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_err.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "soc/adc_channel.h"

static const char *TAG = "adc_Manager.c";

/**
 * @brief ADC Calibration Scheme Selection
 *
 * This section defines the calibration scheme for different ESP32 targets.
 * The calibration scheme is selected based on the target configuration.
 */
#if CONFIG_IDF_TARGET_ESP32
#define ADC_EXAMPLE_CALI_SCHEME ESP_ADC_CAL_VAL_EFUSE_VREF
#elif CONFIG_IDF_TARGET_ESP32S2
#define ADC_EXAMPLE_CALI_SCHEME ESP_ADC_CAL_VAL_EFUSE_TP
#elif CONFIG_IDF_TARGET_ESP32C3
#define ADC_EXAMPLE_CALI_SCHEME ESP_ADC_CAL_VAL_EFUSE_TP
#elif CONFIG_IDF_TARGET_ESP32S3
#define ADC_EXAMPLE_CALI_SCHEME ESP_ADC_CAL_VAL_EFUSE_TP_FIT
#endif

/**
 * @brief Structure to hold ADC channel configurations.
 */
typedef struct {
    char *name;      /**< Name of the ADC channel */
    uint8_t atten;   /**< Attenuation level of the ADC channel */
    uint8_t channel; /**< Channel number of the ADC channel */
    uint8_t unit;    /**< Unit number of the ADC channel */
} ADCChannels_t;

/**
 * @brief Array of ADC channel configurations.
 */
static ADCChannels_t ADCChannels[] = {
#define ADC(name_, unit_, channel_, attenuation_) \
    {.name = #name_,                              \
     .atten = attenuation_,                       \
     .channel = channel_,                         \
     .unit = unit_},
#include "adc_table.inc"
};

/**
 * @brief Storage for ADC channel values after calibration.
 */
static uint32_t values_cal[ADC_MAX];

/**
 * @brief Storage for raw ADC channel values before calibration.
 */
static int values_raw[ADC_MAX];

/**
 * @brief Holds calibration characteristics for the ADC channels.
 *
 * ESPRESSIF setting tings, if you nerdy you can read the docs here:
 * https://docs.espressif.com/projects/esp-idf/en/v4.1.1/api-reference/peripherals/adc.html
 */
static esp_adc_cal_characteristics_t cali_chars[ADC_MAX];

/**
 * @brief Flag indicating the validity of ADC calibration.
 */
static bool cali_enable = false;

/*-----------------------------------------------------------*/

esp_err_t ADC_Init(void)
{
    esp_err_t ret = esp_adc_cal_check_efuse(ADC_EXAMPLE_CALI_SCHEME);
    if (ret == ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGW(TAG, "Calibration scheme not supported, skip software calibration");
    }
    else if (ret == ESP_ERR_INVALID_VERSION) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    }
    else if (ret == ESP_OK) {
        cali_enable = true;
        for (uint32_t i = 0; i < ADC_MAX; i++)
            esp_adc_cal_characterize(
                ADCChannels[i].unit,
                ADCChannels[i].atten,
                ADC_WIDTH_BIT_DEFAULT,
                DEFAULT_VREF,
                &cali_chars[i]);
    }
    else {
        ESP_LOGE(TAG, "Invalid return value from adc_cal_check in %s()", __FUNCTION__);
        return ESP_FAIL;
    }

    return ESP_OK;
}

/*-----------------------------------------------------------*/

esp_err_t ADC_Update(void)
{
    for (uint32_t i = 0; i < ADC_MAX; i++) {
        if (ADCChannels[i].unit == ADC_UNIT_1) {
            ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
            ESP_ERROR_CHECK(adc1_config_channel_atten(ADCChannels[i].channel, ADCChannels[i].atten));
            values_raw[i] = adc1_get_raw(ADCChannels[i].channel);
        }
        else if (ADCChannels[i].unit == ADC_UNIT_2) {
            ESP_ERROR_CHECK(adc2_config_channel_atten(ADCChannels[i].channel, ADCChannels[i].atten));
            esp_err_t ret = ESP_ERR_TIMEOUT;
            while (ret == ESP_ERR_TIMEOUT) {
                ret = adc2_get_raw(ADCChannels[i].channel, ADC_WIDTH_BIT_DEFAULT, &values_raw[i]);
            }
        }
        else {
            ESP_LOGE(TAG, "Invalid ADC unit number in %s()", __FUNCTION__);
        }

        if (cali_enable) {
            values_cal[i] = esp_adc_cal_raw_to_voltage(values_raw[i], &cali_chars[i]);
        }
        else {
            /* Womp womp */
            values_cal[i] = values_raw[i];
        }
    }

    return ESP_OK;
}

/*-----------------------------------------------------------*/

uint32_t ADC_Read(ADCChannel_t chan)
{
    if (chan >= ADC_MAX) {
        ESP_LOGE(TAG, "Error: Channel %u is out of range 0..%u in %s()", chan,
                 ADC_MAX - 1, __FUNCTION__);
        return 0;
    }

    return values_cal[chan];
}

/*-----------------------------------------------------------*/

void ADC_Print()
{
    for (uint32_t i = 0; i < ADC_MAX; i++) {
        printf("%s: %u mV, %u mV Calibrated, %u mV UnCalibrated\n",
               ADCChannels[i].name, values_cal[i], values_cal[i], values_raw[i]);
    }
}
