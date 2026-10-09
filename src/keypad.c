#include "keypad.h"
#include "timer.h"

void keypad_init(Keypad_Config *keypad)
{
    uint8_t i;

    for (i = 0; i < 4; i++)
    {
        gpio_pinMode(keypad->row_port, keypad->row_pins[i], OUTPUT);
        gpio_pinWrite(keypad->row_port, keypad->row_pins[i], HIGH);

        gpio_pinMode(keypad->col_port, keypad->col_pins[i], INPUT);
        gpio_pinWrite(keypad->col_port, keypad->col_pins[i], HIGH);
    }
}

char keypad_getkey(Keypad_Config *keypad)
{
    uint8_t row;
    uint8_t col;
    uint8_t i;

    for (row = 0; row < 4; row++)
    {
        for (i = 0; i < 4; i++)
        {
            gpio_pinWrite(keypad->row_port, keypad->row_pins[i], HIGH);
        }

        gpio_pinWrite(keypad->row_port, keypad->row_pins[row], LOW);
        timer_delay_ms(1);

        for (col = 0; col < 4; col++)
        {
            if (gpio_pinRead(keypad->col_port, keypad->col_pins[col]) == LOW)
            {
                timer_delay_ms(10);

                if (gpio_pinRead(keypad->col_port, keypad->col_pins[col]) == LOW)
                {
                    return keypad->keymap[(row * 4U) + col];
                }
            }
        }
    }

    return '\0';
}
