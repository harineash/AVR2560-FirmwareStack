#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include "define.h"

#define ADC_CHANNEL_0  0
#define ADC_CHANNEL_1  1
#define ADC_CHANNEL_2  2
#define ADC_CHANNEL_3  3
#define ADC_CHANNEL_4  4
#define ADC_CHANNEL_5  5
#define ADC_CHANNEL_6  6
#define ADC_CHANNEL_7  7
#define ADC_CHANNEL_8  8
#define ADC_CHANNEL_9  9
#define ADC_CHANNEL_10 10
#define ADC_CHANNEL_11 11
#define ADC_CHANNEL_12 12
#define ADC_CHANNEL_13 13
#define ADC_CHANNEL_14 14
#define ADC_CHANNEL_15 15

#define ADC_REF_INTERNAL ADC_REF_1V1

#define ADC_PRESCALER_2   2
#define ADC_PRESCALER_4   4
#define ADC_PRESCALER_8   8
#define ADC_PRESCALER_16  16
#define ADC_PRESCALER_32  32
#define ADC_PRESCALER_64  64
#define ADC_PRESCALER_128 128

typedef struct
{
    uint8_t reference;
    uint8_t channel;
    uint16_t prescaler;
} ADC_Config;

void adc_init(ADC_Config *adc);
uint16_t adc_read(ADC_Config *adc);

#endif
