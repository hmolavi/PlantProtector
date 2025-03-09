/**
 * @file adc_table.inc
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 * 
 * @brief Define the adc channels for the PlantProtector project
 *
 * @version 1.0
 * @date 2025-03-08
 *
 * @copyright Copyright (c) 2025
 * 
 * ESP32-S3 Pinout...
 *    https://mischianti.org/wp-content/uploads/2023/06/esp32-S3-DevKitC-1-original-pinout-low-1024x550.jpg
 * 
 * ADC2 on ESP32 has known issues when Wi-Fi is enabled.
 * Try using ADC1 (i.e. ADC_UNIT_1) instead of ADC2 where possible...
 * 
 */

#ifndef ADC
#define ADC(name, unit, channel, attenuation)
#endif
 
ADC(THERM_SENS_AMB, ADC_UNIT_1, ADC1_GPIO9_CHANNEL, ADC_ATTEN_11db)
ADC(THERM_SENS_SOI, ADC_UNIT_1, ADC1_GPIO6_CHANNEL, ADC_ATTEN_11db)
ADC(MOIST_SENS_AMB, ADC_UNIT_1, ADC2_GPIO17_CHANNEL, ADC_ATTEN_11db)
ADC(MOIST_SENS_SOI, ADC_UNIT_1, ADC1_GPIO8_CHANNEL, ADC_ATTEN_11db)

#undef ADC
 