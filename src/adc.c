#include "adc.h"

static volatile uint8_t *R(uint16_t a)
{
    return (volatile uint8_t *)a;
}

static uint8_t adc_prescaler_bits(uint16_t p)
{
    switch (p)
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

static void adc_select(ADC_Config *a)
{
    uint8_t mux = (uint8_t)(a->channel & 0x1F);

    *R(ADMUX_ADDR) =
        (uint8_t)((a->reference & 0x03) << 6) |
        (mux & 0x1F);

    if (a->channel >= 8)
    {
        *R(ADCSRB_ADDR) |= (1 << 3);
    }
    else
    {
        *R(ADCSRB_ADDR) &= (uint8_t)~(1 << 3);
    }
}

void adc_init(ADC_Config *a)
{
    adc_select(a);

    *R(ADCSRA_ADDR) =
        (uint8_t)(1 << 7) |
        adc_prescaler_bits(a->prescaler);
}

uint16_t adc_read(ADC_Config *a)
{
    uint16_t value;

    adc_select(a);
    *R(ADCSRA_ADDR) |= (1 << 6);

    while ((*R(ADCSRA_ADDR) & (1 << 6)) != 0)
    {
    }

    value = *R(ADCL_ADDR);
    value |= (uint16_t)(*R(ADCH_ADDR) << 8);

    return value;
}
