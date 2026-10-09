
#ifndef SEVEN_SEGMENT_H
#define SEVEN_SEGMENT_H

#include <stdint.h>
#include "gpio.h"

#define COMMON_CATHODE 0
#define COMMON_ANODE   1

typedef struct
{
    GPIO_Port segment_port;
    GPIO_Port digit_port;

    uint8_t common_type;
    uint8_t tens_mask;
    uint8_t units_mask;

} SevenSegment_Config;

void seven_segment_init(SevenSegment_Config *s);

void seven_segment_show_number(
    SevenSegment_Config *s,
    uint8_t number
);

void seven_segment_off(SevenSegment_Config *s);

#endif
