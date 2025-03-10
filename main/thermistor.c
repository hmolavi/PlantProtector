/**
 * @file thermistor.c
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 * 
 * @brief Thermistor linearization code
 * 
 * The thermistors are configured by editing the thermistor.inc file
 * 
 * @version 1.0
 * @date 2025-03-08
 * 
 * @copyright Copyright (c) 2025
 */

#include "thermistor.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "adc_manager.h"

typedef struct Thermistor_s {
    char *name;
    ADCChannel_t chan;
    uint32_t t0;
    uint32_t rt0;
    uint32_t beta;
    uint32_t rs;
    uint32_t failed;
    uint32_t vmin;
    uint32_t vmax;
} Thermistor_t;

Thermistor_t Thermistor[] = {
#define THERMISTOR(name_, chan_, t0_, rt0_, beta_, rs_, vmin_, vmax_) \
    {                                                                 \
        .name = #name_,                                               \
        .chan = chan_,                                                \
        .t0 = t0_,                                                    \
        .rt0 = rt0_,                                                  \
        .beta = beta_,                                                \
        .rs = rs_,                                                    \
        .failed = 0,                                                  \
        .vmin = vmin_,                                                \
        .vmax = vmax_,                                                \
    },
#include "thermistor.inc"
};

uint32_t ThermistorFailed(ThermistorChannel_t chan)
{
    if (chan >= THERMISTOR_MAX) {
        printf("Error: Thermistor channel %d is out of range 0..%d in %s()\n",
               chan, THERMISTOR_MAX - 1, __FUNCTION__);
        return 1;
    }

    return Thermistor[chan].failed;
}

float ThermistorTemp(ThermistorChannel_t chan)
{
    if (chan >= THERMISTOR_MAX) {
        printf("Error: Thermistor channel %d is out of range 0..%d in %s()\n",
               chan, THERMISTOR_MAX - 1, __FUNCTION__);
        return 0;
    }

    double Vrs = ADC_Read(Thermistor[chan].chan);

    if ((Vrs < Thermistor[chan].vmin) || (Vrs > Thermistor[chan].vmax)) {
        Thermistor[chan].failed = 1;
        return 0;
    }
    else {
        Thermistor[chan].failed = 0;
    }

    /** NTC B-Parameter Equation
     *  1/T = 1/T0 + ln(R/R0)/B
     *
     * We have the pull up configuration
     *
     * For pull-down formula (or if your nerdy...) read into this
     * https://arduinodiy.wordpress.com/2015/11/10/measuring-temperature-with-ntc-the-steinhart-hart-formula/
     */
    /* Pull-up configuration of ADC */
    double Rt = (double)Thermistor[chan].rs * (DEFAULT_VREF / (double)Vrs - 1.0);

    double steinhart, T;
    steinhart = Rt / Thermistor[chan].rt0;     // (R/Ro)
    steinhart = log(steinhart);                // ln(R/Ro)
    steinhart /= Thermistor[chan].beta;        // ln(R/Ro) * 1/B
    steinhart += 1.0 / (Thermistor[chan].t0);  // + (1/To)
    T = 1.0 / steinhart;                       // Invert to get T
    T -= K_AT_0C;                              // K to C

    return T;
}

#define NUM_SAMPLES 25
static float samples[NUM_SAMPLES];
static uint32_t index = 0;
static uint32_t samplesTaken = 0;

void Thermistor_Print(void)
{
    for (uint32_t i = 0; i < THERMISTOR_MAX; i++) {
        float temp = ThermistorTemp(i);

        // Update samples array with the latest temperature reading
        samples[index] = temp;
        index = (index + 1) % NUM_SAMPLES;
        if (samplesTaken < NUM_SAMPLES) {
            samplesTaken++;
        }

        // Calculate the moving average
        float sum = 0;
        for (uint32_t j = 0; j < samplesTaken; j++) {
            sum += samples[j];
        }
        float movingAverage = sum / samplesTaken;

        printf("%20s: %5.1f C, adc: %u, state:%s, moving average: %5.1f C\n", 
            Thermistor[i].name, (double)temp, ADC_Read(Thermistor[i].chan),
            ThermistorFailed(i) ? "Failed" : "Working", (double)movingAverage);
    }
}