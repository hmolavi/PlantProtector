/** 
 * @file thermistor.inc
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 *
 * Thermistor configuration file.
 *
 * I think our thermistors are "NTCLE100E3109" (╥﹏╥)
 * Datasheet: https://www.vishay.com/docs/29049/ntcle100.pdf
 * 
 * @version 1.0
 * @date 2025-03-08
 * 
 * @copyright Copyright (c) 2025
 */


#ifndef THERMISTOR
#define THERMISTOR(name, chan, t0, rt0, beta, rs, vmin, vmax)
#endif

#define K_AT_0C 273.15  // 0 degrees Celsius in Kelvin

/**
 * THERMISTORs definitions:
 *  name    ideally match the one cerated in adc_table.inc
 *  chan    adc channel defined in adc_table.inc (should also be defined in gpio_table.inc)
 *  t0      pivital temprature typically defined in datasheet. Shuold be in Kelvins
 *  rt0     resistance of thermistor at t0
 *  beta    defined in datasheet
 *  rs      value of the resistor in series with the sensor
 *  vmin    minimum acceptable voltage. If reading anything below this it means its a short to ground
 *  vmax    maximum acceptable voltage. If reading anything above this it means its a short to vcc 
 */

THERMISTOR(THERM_SENS_AMB, ADC_THERM_SENS_AMB, K_AT_0C + 25, 10000, 3977, 9950, 100, 3200)

// ADC(THERM_SENS_AMB, ADC_UNIT_1, ADC1_GPIO9_CHANNEL, ADC_ATTEN_11db)
// ADC(THERM_SENS_SOI, ADC_UNIT_1, ADC1_GPIO6_CHANNEL, ADC_ATTEN_11db)
// ADC(MOIST_SENS_AMB, ADC_UNIT_1, ADC2_GPIO17_CHANNEL, ADC_ATTEN_11db)
// ADC(MOIST_SENS_SOI, ADC_UNIT_1, ADC1_GPIO8_CHANNEL, ADC_ATTEN_11db)


#undef THERMISTOR
