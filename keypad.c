#include "keypad.h"

static void delay_short(void)
{
    volatile uint16_t i;

    for (i = 0; i < 300; i++)
    {
        __asm__ __volatile__("nop");
    }
}

void keypad_init(Keypad_Config *k)
{
    uint8_t i;

    for (i = 0; i < 4; i++)
    {
        gpio_pinMode(k->row_port, k->row_pins[i], OUTPUT);
        gpio_pinWrite(k->row_port, k->row_pins[i], HIGH);

        gpio_pinMode(k->col_port, k->col_pins[i], INPUT);
        gpio_pinWrite(k->col_port, k->col_pins[i], HIGH);
    }
}

char keypad_getkey(Keypad_Config *k)
{
    uint8_t r;
    uint8_t c;

    for (r = 0; r < 4; r++)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            gpio_pinWrite(k->row_port, k->row_pins[i], HIGH);
        }

        gpio_pinWrite(k->row_port, k->row_pins[r], LOW);
        delay_short();

        for (c = 0; c < 4; c++)
        {
            if (gpio_pinRead(k->col_port, k->col_pins[c]) == LOW)
            {
                delay_short();

                if (gpio_pinRead(k->col_port, k->col_pins[c]) == LOW)
                {
                    return k->keymap[(r * 4) + c];
                }
            }
        }
    }

    return '\0';
}
