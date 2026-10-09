
#include "seven_segment.h"
#include "timer.h"

static const uint8_t segment_code[10] =
{
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

static void digit_select(
    SevenSegment_Config *s,
    uint8_t mask
)
{
    gpio_pinWrite(s->digit_port, 0, LOW);
    gpio_pinWrite(s->digit_port, 1, LOW);

    if (mask & s->tens_mask)
    {
        gpio_pinWrite(
            s->digit_port,
            (s->tens_mask == 0) ? 0 :
            (s->tens_mask == 1 ? 0 :
            (s->tens_mask & 0x01 ? 0 :
            (s->tens_mask & 0x02 ? 1 :
            (s->tens_mask & 0x04 ? 2 :
            (s->tens_mask & 0x08 ? 3 :
            (s->tens_mask & 0x10 ? 4 :
            (s->tens_mask & 0x20 ? 5 :
            (s->tens_mask & 0x40 ? 6 : 7)))))))),
            HIGH
        );
    }
}

void seven_segment_init(SevenSegment_Config *s)
{
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        gpio_pinMode(s->segment_port, i, OUTPUT);
    }

    for (i = 0; i < 8; i++)
    {
        if ((s->tens_mask | s->units_mask) & (1U << i))
        {
            gpio_pinMode(s->digit_port, i, OUTPUT);
        }
    }

    seven_segment_off(s);
}

void seven_segment_off(SevenSegment_Config *s)
{
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        if ((s->tens_mask | s->units_mask) & (1U << i))
        {
            gpio_pinWrite(s->digit_port, i, LOW);
        }
    }

    gpio_portWrite(
        s->segment_port,
        (s->common_type == COMMON_ANODE) ? 0xFF : 0x00
    );
}

void seven_segment_show_number(
    SevenSegment_Config *s,
    uint8_t number
)
{
    uint8_t tens;
    uint8_t units;
    uint8_t pattern;
    uint8_t i;

    if (number > 99)
    {
        return;
    }

    tens = number / 10;
    units = number % 10;

    /* Display tens digit */
    for (i = 0; i < 8; i++)
    {
        if ((s->tens_mask | s->units_mask) & (1U << i))
        {
            gpio_pinWrite(s->digit_port, i, LOW);
        }
    }

    pattern = segment_code[tens];

    if (s->common_type == COMMON_ANODE)
    {
        pattern = (uint8_t)~pattern;
    }

    gpio_portWrite(s->segment_port, pattern);

    for (i = 0; i < 8; i++)
    {
        if (s->tens_mask & (1U << i))
        {
            gpio_pinWrite(s->digit_port, i, HIGH);
        }
    }

    timer_delay_ms(1);

    /* Display units digit */
    for (i = 0; i < 8; i++)
    {
        if ((s->tens_mask | s->units_mask) & (1U << i))
        {
            gpio_pinWrite(s->digit_port, i, LOW);
        }
    }

    pattern = segment_code[units];

    if (s->common_type == COMMON_ANODE)
    {
        pattern = (uint8_t)~pattern;
    }

    gpio_portWrite(s->segment_port, pattern);

    for (i = 0; i < 8; i++)
    {
        if (s->units_mask & (1U << i))
        {
            gpio_pinWrite(s->digit_port, i, HIGH);
        }
    }

    timer_delay_ms(1);

    seven_segment_off(s);
}
