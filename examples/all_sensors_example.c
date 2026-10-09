#include "adc.h"
#include "ir.h"
#include "keypad.h"
#include "ultrasonic.h"

int main(void)
{
    ADC_Config adc =
    {
        .reference = ADC_REF_AVCC,
        .channel = ADC_CHANNEL_0,
        .prescaler = ADC_PRESCALER_128
    };

    IR_Config ir =
    {
        .port = GPIO_PORTA,
        .pin = 0,
        .active_level = IR_ACTIVE_LOW
    };

    Keypad_Config keypad =
    {
        .row_port = GPIO_PORTC,
        .col_port = GPIO_PORTD,
        .row_pins = {0, 1, 2, 3},
        .col_pins = {0, 1, 2, 3},
        .keymap = "123A456B789C*0#D"
    };

    Ultrasonic_Config sonar =
    {
        .trigger_port = GPIO_PORTB,
        .echo_port = GPIO_PORTB,
        .trigger_pin = 0,
        .echo_pin = 1
    };

    volatile uint16_t adc_value;
    volatile uint16_t distance_cm;
    volatile uint8_t ir_status;
    volatile char key;

    adc_init(&adc);
    ir_init(&ir);
    keypad_init(&keypad);
    ultrasonic_init(&sonar);

    while (1)
    {
        adc_value = adc_read(&adc);
        distance_cm = ultrasonic_read_cm(&sonar);
        ir_status = ir_detected(&ir);
        key = keypad_getkey(&keypad);
    }
}
