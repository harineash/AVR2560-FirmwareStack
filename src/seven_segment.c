```c
#include "seven_segment.h"

static const uint8_t segment_code[10] =
{
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F
};


/* Busy-loop delay copied from the timing approach in your test. */
static void display_delay(void)
{
    volatile uint32_t i;

    for (i = 0; i < 2000UL; i++)
    {
        /* Wait */
    }
}


/* Digit selects are active LOW in this version. */
static void disable_digits(SevenSegment_Config *display)
{
    gpio_pinWrite(display->digit_port, 0, HIGH);
    gpio_pinWrite(display->digit_port, 1, HIGH);
}


/* Disable only the configured digit-select pins. */
static void set_digit_selects(
    SevenSegment_Config *display,
    uint8_t tens_level,
    uint8_t units_level)
{
    uint8_t pin;

    for (pin = 0; pin < 8; pin++)
    {
        if (display->tens_mask & (1U << pin))
        {
            gpio_pinWrite(
                display->digit_port,
                pin,
                tens_level
            );
        }

        if (display->units_mask & (1U << pin))
        {
            gpio_pinWrite(
                display->digit_port,
                pin,
                units_level
            );
        }
    }
}


static void show_digit(
    SevenSegment_Config *display,
    uint8_t digit,
    uint8_t digit_mask)
{
    uint8_t pattern;

    if (digit > 9)
    {
        return;
    }

    /* Disable both digits before changing the segments. */
    set_digit_selects(display, HIGH, HIGH);

    pattern = segment_code[digit];

    if (display->common_type == COMMON_ANODE)
    {
        pattern = (uint8_t)~pattern;
    }

    gpio_portWrite(display->segment_port, pattern);

    /* Enable the selected digit; digit select is active LOW. */
    set_digit_selects(
        display,
        (digit_mask == display->tens_mask) ? LOW : HIGH,
        (digit_mask == display->units_mask) ? LOW : HIGH
    );

    display_delay();

    /* Disable the digit before refreshing the next one. */
    set_digit_selects(display, HIGH, HIGH);
}


void seven_segment_init(SevenSegment_Config *display)
{
    uint8_t pin;

    for (pin = 0; pin < 8; pin++)
    {
        gpio_pinMode(
            display->segment_port,
            pin,
            OUTPUT
        );
    }

    for (pin = 0; pin < 8; pin++)
    {
        if ((display->tens_mask | display->units_mask) &
            (1U << pin))
        {
            gpio_pinMode(
                display->digit_port,
                pin,
                OUTPUT
            );
        }
    }

    seven_segment_off(display);
}


void seven_segment_off(SevenSegment_Config *display)
{
    set_digit_selects(display, HIGH, HIGH);

    gpio_portWrite(
        display->segment_port,
        (display->common_type == COMMON_ANODE)
            ? 0xFF
            : 0x00
    );
}


void seven_segment_show_number(
    SevenSegment_Config *display,
    uint8_t number)
{
    if (number > 99)
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
```
