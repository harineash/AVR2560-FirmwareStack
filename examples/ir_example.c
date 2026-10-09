#include "ir.h"
#include "led.h"

int main(void)
{
    IR_Config ir =
    {
        .port = GPIO_PORTA,
        .pin = 0,
        .active_level = IR_ACTIVE_LOW
    };

    gpio_portMode(GPIO_PORTF, 0x01);
    ir_init(&ir);
    led_init(GPIO_PORTF, 0x01);

    while (1)
    {
        if (ir_detected(&ir))
        {
            led_on();
        }
        else
        {
            led_off();
        }
    }
}
