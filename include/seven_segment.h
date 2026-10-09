
#ifndef SEVEN_SEGMENT_H
#define SEVEN_SEGMENT_H

#include "gpio.h"
#include <stdint.h>

typedef struct
{
    GPIO_Port port;       // Shared segment and digit-enable port
    uint8_t common_type;
    uint8_t tens_mask;    // Bit for tens digit enable
    uint8_t units_mask;   // Bit for units digit enable
    uint8_t number;
    uint8_t digit;
} SevenSegment_Config;

void seven_segment_init(SevenSegment_Config *s);
void seven_segment_display(SevenSegment_Config *s, uint8_t number);
void seven_segment_refresh(SevenSegment_Config *s);
void seven_segment_off(SevenSegment_Config *s);

#endif
