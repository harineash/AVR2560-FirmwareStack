#ifndef SEVEN_SEGMENT_H
#define SEVEN_SEGMENT_H

#include <stdint.h>
#include "gpio.h"

typedef struct
{
    GPIO_Port segment_port;
    GPIO_Port digit_port;
    uint8_t common_type;
    uint8_t tens_mask;
    uint8_t units_mask;
} SevenSegment_Config;

void seven_segment_init(SevenSegment_Config *display);
void seven_segment_show_number(SevenSegment_Config *display,
                               uint8_t number);
void seven_segment_off(SevenSegment_Config *display);

#endif
