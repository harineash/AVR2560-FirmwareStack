#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>
#include "gpio.h"

typedef struct
{
    GPIO_Port trigger_port;
    GPIO_Port echo_port;
    uint8_t trigger_pin;
    uint8_t echo_pin;
} Ultrasonic_Config;

void ultrasonic_init(Ultrasonic_Config *sensor);
uint16_t ultrasonic_read_cm(Ultrasonic_Config *sensor);

#endif
