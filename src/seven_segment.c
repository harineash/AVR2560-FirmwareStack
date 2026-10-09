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

    for (pin = 0; pin < 8; pin++)
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
    seven_segment_off(display);

    gpio_portWrite(display->segment_port, segment_code[digit]);

    /* Common-cathode digit enabled LOW */
    set_mask(display->digit_port, mask, LOW);
    timer_delay_ms(1);

    /* Disable digit */
    set_mask(display->digit_port, mask, HIGH);
}

void seven_segment_init(SevenSegment_Config *display)
{
    uint8_t pin;

    for (pin = 0; pin < 8; pin++)
    {
        gpio_pinMode(display->segment_port, pin, OUTPUT);
    }

    for (pin = 0; pin < 8; pin++)
    {
        if ((display->tens_mask | display->units_mask) & (1U << pin))
        {
            gpio_pinMode(display->digit_port, pin, OUTPUT);
        }
    }

    seven_segment_off(display);
}

void seven_segment_off(SevenSegment_Config *display)
{
    /* Disable both common-cathode digits */
    set_mask(
        display->digit_port,
        (uint8_t)(display->tens_mask | display->units_mask),
        HIGH
    );

    gpio_portWrite(display->segment_port, 0x00);
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
