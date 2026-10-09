#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>
#include "gpio.h"

typedef struct
{
    GPIO_Port row_port;
    GPIO_Port col_port;
    uint8_t row_pins[4];
    uint8_t col_pins[4];
    const char *keymap;
} Keypad_Config;

void keypad_init(Keypad_Config *keypad);
char keypad_getkey(Keypad_Config *keypad);

#endif
