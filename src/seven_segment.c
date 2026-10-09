
#include "seven_segment.h"
#include "timer.h"

static const uint8_t segment_code[10] =
{
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

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

static void show_digit(
    SevenSegment_Config *display,
    uint8_t digit,
    uint8_t mask)
{
    uint8_t pattern = segment_code[digit];

    /* Disable both digits before changing segments. */
    seven_segment_off(display);

    if (display->common_type == COMMON_ANODE)
    {
        pattern = (uint8_t)~pattern;
    }

    gpio_portWrite(display->segment_port, pattern);

    /* Assumes active-low digit selection. */
    set_mask(display->digit_port, mask, LOW);

    timer_delay_us(500U);

    set_mask(display->digit_port, mask, HIGH);
}

void seven_segment_init(SevenSegment_Config *display)
{
    uint8_t pin;

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
}

void seven_segment_off(SevenSegment_Config *display)
{
    /* Disable both active-low digit selects. */
    set_mask(
        display->digit_port,
        (uint8_t)(display->tens_mask | display->units_mask),
        HIGH
    );

    gpio_portWrite(
        display->segment_port,
        (display->common_type == COMMON_ANODE) ? 0xFF : 0x00
    );
}

void seven_segment_show_number(
    SevenSegment_Config *display,
    uint8_t number)
{
    if (number > 99U)
    {
        seven_segment_off(display);
        return;
    }

    show_digit(
        display,
        (uint8_t)(number / 10U),
        display->tens_mask
    );

    show_digit(
        display,
        (uint8_t)(number % 10U),
        display->units_mask
    );
}
