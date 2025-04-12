/**
 * @file gpio_table.inc
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 * 
 * @brief Define the GPIO pins for the PlantProtector project
 *
 * @version 1.0
 * @date 2025-03-08
 *
 * @copyright Copyright (c) 2025
 * 
 * ESP32-S3 Pinout...
 *    https://mischianti.org/wp-content/uploads/2023/06/esp32-S3-DevKitC-1-original-pinout-low-1024x550.jpg
 * 
 */

#ifndef PIN
#define PIN(name, bit_index, type, mode, pull_up, pull_down, str, sig_idx, func_idx, output_enable_inv, software_inv, default, desc)
#endif

/** PIN Definition
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
 */

/* SPI Connections */
PIN(SPI_MOSI, 35, IOMUX, OUTPUT, DISABLE, DISABLE, 0, 0, 0, 0, TRUE, 0, "SPI Interface")
PIN(SPI_MISO, 37, IOMUX, INPUT, DISABLE, DISABLE, 0, 0, 0, 0, TRUE, 0, "SPI Interface")
PIN(SPI_SS, 45, GPIO, OUTPUT, DISABLE, DISABLE, 0, 0, 0, 0, TRUE, 0, "SPI Chip select")
PIN(SPI_SCK, 36, IOMUX, OUTPUT, DISABLE, DISABLE, 0, 0, 0, 0, TRUE, 0, "SPI Interface")

/* Analog Thermistors */
PIN(THERM_SENS_AMB, 3, RTC, INPUT, DISABLE, DISABLE, 0, 0, 0, 0, TRUE, 0, "Thermal Sensor Ambient")
PIN(THERM_SENS_SOI, 15, RTC, INPUT, DISABLE, DISABLE, 0, 0, 0, 0, TRUE, 0, "Thermal Sensor In Soil")
PIN(MOIST_SENS_AMB, 17, RTC, INPUT, DISABLE, DISABLE, 0, 0, 0, 0, TRUE, 0, "Moisture Sensor Ambient")
PIN(MOIST_SENS_SOI, 6, RTC, INPUT, DISABLE, DISABLE, 0, 0, 0, 0, TRUE, 0, "Moisture Sensor In Soil")

/* LED control */
PIN(LED_EYE_L, 5, GPIO, OUTPUT, DISABLE, DISABLE, 2, 0, 0, 0, TRUE, 0, "")
PIN(LED_EYE_R, 19, GPIO, OUTPUT, DISABLE, DISABLE, 2, 0, 0, 0, TRUE, 0, "")
PIN(LED_ROUND, 4, GPIO, OUTPUT, ENABLE, DISABLE, 2, 0, 0, 0, TRUE, 0, "")

#undef PIN
