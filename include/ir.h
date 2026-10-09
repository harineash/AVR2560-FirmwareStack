#ifndef IR_H
#define IR_H

#include <stdint.h>
#include "gpio.h"

#define IR_ACTIVE_LOW   0
#define IR_ACTIVE_HIGH  1

typedef struct
{
    GPIO_Port port;
    uint8_t pin;
    uint8_t active_level;
} IR_Config;

void ir_init(IR_Config *ir);
uint8_t ir_detected(IR_Config *ir);
uint8_t ir_read(IR_Config *ir);

#endif
