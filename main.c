```c
#include <stdint.h>

#include "gpio.h"
#include "timer.h"
#include "ultrasonic.h"
#include "seven_segment.h"

static Ultrasonic_Config ultrasonic =
{
    GPIO_PORTB,     /* Trigger port */
    GPIO_PORTB,     /* Echo port */
    0,              /* PB0: TRIG */
    1               /* PB1: ECHO */
};

static SevenSegment_Config display =
{
    GPIO_PORTF,     /* Segment port */
    GPIO_PORTG,     /* Digit-select port */
    COMMON_CATHODE,
    0x01,           /* PG0: tens digit */
    0x02            /* PG1: units digit */
};

int main(void)
{
    uint16_t distance_cm;
    uint8_t display_value;

    ultrasonic_init(&ultrasonic);
    seven_segment_init(&display);

    while (1)
    {
        distance_cm = ultrasonic_read_cm(&ultrasonic);

        if (distance_cm == 0U)
        {
            display_value = 0U;
        }
        else if (distance_cm > 99U)
        {
            display_value = 99U;
        }
        else
        {
            display_value = (uint8_t)distance_cm;
        }

        /*
         * Continuously refresh both digits.
         * Keep sensor measurements separate from display refresh.
         */
        {
            uint16_t refresh_count;

            for (refresh_count = 0; refresh_count < 100U;
                 refresh_count++)
            {
                seven_segment_show_number(&display, display_value);
            }
        }
    }
}
```
