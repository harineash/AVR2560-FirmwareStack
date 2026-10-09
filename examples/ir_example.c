#include "ir.h"
#include "led.h"

int main(void)
{
    IR_Config ir = { GPIO_PORTA, 0, IR_ACTIVE_LOW };
    LED_Config led = { GPIO_PORTF, 0, HIGH };

    ir_init(&ir);
    led_init(&led);

    while (1)
    {
        if (ir_detected(&ir))
        {
            led_on(&led);
        }
        else
        {
            led_off(&led);
        }
    }
}
