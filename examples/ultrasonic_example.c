#include "ultrasonic.h"
#include "led.h"

int main(void)
{
    Ultrasonic_Config sonar =
    {
        .trigger_port = GPIO_PORTB,
        .echo_port = GPIO_PORTB,
        .trigger_pin = 0,
        .echo_pin = 1
    };

    volatile uint16_t distance_cm;

    ultrasonic_init(&sonar);

    while (1)
    {
        distance_cm = ultrasonic_read_cm(&sonar);

        /* distance_cm == 0 means timeout/no valid echo */
    }
}
