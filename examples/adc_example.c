#include "adc.h"

int main(void)
{
    ADC_Config adc =
    {
        .reference = ADC_REF_AVCC,
        .channel = ADC_CHANNEL_0,
        .prescaler = ADC_PRESCALER_128
    };

    volatile uint16_t adc_value;

    adc_init(&adc);

    while (1)
    {
        adc_value = adc_read(&adc);
        (void)adc_value;
    }
}
