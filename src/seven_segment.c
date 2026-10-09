
#include <avr/interrupt.h>
#include <stdint.h>

#include "seven_segment.h"

static const uint8_t segment_code[10] =
{
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

static SevenSegment_Config *active_display;

static volatile uint8_t display_value;
static volatile uint8_t display_enabled;
static volatile uint8_t active_digit;

static void set_mask(GPIO_Port port, uint8_t mask, uint8_t value)
{
    uint8_t pin;

    for (pin = 0; pin < 8U; pin++)
    {
        if (mask & (1U << pin))
        {
            gpio_pinWrite(port, pin, value);
        }
    }
}

void seven_segment_init(SevenSegment_Config *display)
{
    uint8_t pin;

    active_display = display;
    display_value = 0U;
    display_enabled = 0U;
    active_digit = 0U;

    for (pin = 0; pin < 8U; pin++)
    {
        gpio_pinMode(display->segment_port, pin, OUTPUT);
    }

    for (pin = 0; pin < 8U; pin++)
    {
        if ((display->tens_mask | display->units_mask) &
            (1U << pin))
        {
            gpio_pinMode(display->digit_port, pin, OUTPUT);
        }
    }

    seven_segment_off(display);

    /* Timer3 CTC mode: interrupt every 1 ms at 16 MHz. */
    TCCR3A = 0U;
    TCCR3B = 0U;
    TCNT3 = 0U;
    OCR3A = 249U;
    TIFR3 = (1U << OCF3A);
    TIMSK3 = (1U << OCIE3A);
    TCCR3B = (1U << WGM32) |
             (1U << CS31) |
             (1U << CS30);
}

void seven_segment_off(SevenSegment_Config *display)
{
    uint8_t saved_sreg = SREG;

    cli();
    display_enabled = 0U;
    set_mask(
        display->digit_port,
        (uint8_t)(display->tens_mask | display->units_mask),
        LOW
    );

    gpio_portWrite(
        display->segment_port,
        (display->common_type == COMMON_ANODE) ? 0xFFU : 0x00U
    );

    SREG = saved_sreg;
}

void seven_segment_show_number(
    SevenSegment_Config *display,
    uint8_t number)
{
    uint8_t saved_sreg;

    if (number > 99U)
    {
        seven_segment_off(display);
        return;
    }

    saved_sreg = SREG;
    cli();

    display_value = number;
    active_display = display;
    display_enabled = 1U;

    SREG = saved_sreg;
}

ISR(TIMER3_COMPA_vect)
{
    uint8_t digit;
    uint8_t pattern;
    uint8_t mask;

    if ((active_display == 0) || !display_enabled)
    {
        return;
    }

    /* Disable both digits before changing segments. */
    set_mask(
        active_display->digit_port,
        (uint8_t)(active_display->tens_mask |
                  active_display->units_mask),
        LOW
    );

    if (active_digit == 0U)
    {
        digit = (uint8_t)(display_value / 10U);
        mask = active_display->tens_mask;
        active_digit = 1U;
    }
    else
    {
        digit = (uint8_t)(display_value % 10U);
        mask = active_display->units_mask;
        active_digit = 0U;
    }

    pattern = segment_code[digit];

    if (active_display->common_type == COMMON_ANODE)
    {
        pattern = (uint8_t)~pattern;
    }

    gpio_portWrite(active_display->segment_port, pattern);
    set_mask(active_display->digit_port, mask, HIGH);
}
