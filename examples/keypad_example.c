#include "keypad.h"

int main(void)
{
    Keypad_Config keypad =
    {
        .row_port = GPIO_PORTA,
        .col_port = GPIO_PORTC,
        .row_pins = {0, 1, 2, 3},
        .col_pins = {0, 1, 2, 3},
        .keymap = "123A456B789C*0#D"
    };

    volatile char key;

    keypad_init(&keypad);

    while (1)
    {
        key = keypad_getkey(&keypad);
    }
}
