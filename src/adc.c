#include "adc.h"

static volatile uint8_t *reg8(uint16_t address)
{
    return (volatile uint8_t *)(uintptr_t)address;
}

static uint8_t adc_prescaler_bits(uint16_t prescaler)
{
    switch (prescaler)
    {
        case 2:   return 1;
        case 4:   return 2;
        case 8:   return 3;
        case 16:  return 4;
        case 32:  return 5;
        case 64:  return 6;
        case 128: return 7;
        default:  return 7;
    }
}

static void adc_select(ADC_Config *adc)
{
    uint8_t channel = adc->channel;
    uint8_t mux = (channel >= 8U) ? (uint8_t)(channel - 8U) : channel;

    *reg8(ADMUX_ADDR) = (uint8_t)(((adc->reference & 0x03U) << 6) | (mux & 0x07U));

    if (channel >= 8U)
    {
        *reg8(ADCSRB_ADDR) |= (1U << 3);
    }
    else
    {
        *reg8(ADCSRB_ADDR) &= (uint8_t)~(1U << 3);
    }
}

void adc_init(ADC_Config *adc)
{
    adc_select(adc);
    *reg8(ADCSRA_ADDR) = (uint8_t)((1U << 7) | adc_prescaler_bits(adc->prescaler));
}

uint16_t adc_read(ADC_Config *adc)
{
    uint16_t value;

    adc_select(adc);
    *reg8(ADCSRA_ADDR) |= (1U << 6);

    while ((*reg8(ADCSRA_ADDR) & (1U << 6)) != 0)
    {
    }

    value = *reg8(ADCL_ADDR);
    value |= (uint16_t)(*reg8(ADCH_ADDR) << 8);

    return value;
}
