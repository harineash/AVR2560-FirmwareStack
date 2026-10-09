#include <stdint.h>
#include "keypad.h"
#include "lcd.h"
#include "seven_segment.h"
#include "ultrasonic.h"
#include "ir.h"
#include "led.h"
#include "pwm.h"
#include "timer.h"

#define MODE_SLOT    0
#define MODE_REVERSE 1

enum
{
    STATUS_NO_ECHO,
    STATUS_SAFE,
    STATUS_CAUTION,
    STATUS_WARNING,
    STATUS_VERY_CLOSE,
    STATUS_STOP
};

static Keypad_Config keypad =
{
    GPIO_PORTA, GPIO_PORTA,
    {4, 5, 6, 7}, {0, 1, 2, 3},
    "123A456B789C*0#D"
};

static LCD_Config lcd = { GPIO_PORTC, 4, 5, 0 };
static SevenSegment_Config display =
{
    GPIO_PORTF, GPIO_PORTG, COMMON_CATHODE, 0x01, 0x02
};
static Ultrasonic_Config ultrasonic = { GPIO_PORTB, GPIO_PORTB, 0, 1 };
static IR_Config ir = { GPIO_PORTK, 0, IR_ACTIVE_LOW };
static LED_Config available_led = { GPIO_PORTJ, 0, HIGH };
static LED_Config occupied_led = { GPIO_PORTJ, 1, HIGH };
static PWM_Config buzzer = { TIMER0, PWM_CHANNEL_A, 40 };

static uint8_t read_slot_occupied(void)
{
    return ir_detected(&ir);
}

static void update_slot_leds(uint8_t occupied)
{
    if (occupied)
    {
        led_off(&available_led);
        led_on(&occupied_led);
    }
    else
    {
        led_on(&available_led);
        led_off(&occupied_led);
    }
}

static void show_slot_status(uint8_t occupied)
{
    lcd_clear(&lcd);
    lcd_goto(&lcd, 0, 0);
    lcd_print(&lcd, "PARKING SLOT    ");
    lcd_goto(&lcd, 1, 0);
    lcd_print(&lcd, occupied ? "SLOT OCCUPIED   " : "SLOT AVAILABLE  ");
}

static uint8_t get_distance_status(uint16_t cm)
{
    if (cm == 0)
    {
        return STATUS_NO_ECHO;
    }
    if (cm > 30)
    {
        return STATUS_SAFE;
    }
    if (cm >= 16)
    {
        return STATUS_CAUTION;
    }
    if (cm >= 10)
    {
        return STATUS_WARNING;
    }
    if (cm >= 5)
    {
        return STATUS_VERY_CLOSE;
    }

    return STATUS_STOP;
}

static void show_distance(uint16_t cm, uint8_t status)
{
    lcd_goto(&lcd, 0, 0);
    lcd_print(&lcd, "DISTANCE:       ");
    lcd_goto(&lcd, 0, 10);

    if (cm == 0)
    {
        lcd_print(&lcd, "--cm");
    }
    else
    {
        if (cm >= 100)
        {
            lcd_data(&lcd, (uint8_t)('0' + (cm / 100U)));
            cm %= 100U;
        }
        if (cm >= 10)
        {
            lcd_data(&lcd, (uint8_t)('0' + (cm / 10U)));
        }
        else
        {
            lcd_data(&lcd, '0');
        }
        lcd_data(&lcd, (uint8_t)('0' + (cm % 10U)));
        lcd_print(&lcd, "cm");
    }

    lcd_goto(&lcd, 1, 0);
    switch (status)
    {
        case STATUS_NO_ECHO: lcd_print(&lcd, "NO ECHO - CHECK "); break;
        case STATUS_SAFE: lcd_print(&lcd, "SAFE            "); break;
        case STATUS_CAUTION: lcd_print(&lcd, "CAUTION         "); break;
        case STATUS_WARNING: lcd_print(&lcd, "WARNING         "); break;
        case STATUS_VERY_CLOSE: lcd_print(&lcd, "VERY CLOSE      "); break;
        default: lcd_print(&lcd, "STOP            "); break;
    }
}

static void update_buzzer(uint8_t status, uint16_t phase_ms)
{
    uint16_t period;
    uint16_t on_time;

    if (status == STATUS_SAFE)
    {
        pwm_stop(&buzzer);
        return;
    }

    if (status == STATUS_STOP)
    {
        pwm_setDuty(&buzzer, 40);
        pwm_start(&buzzer);
        return;
    }

    if (status == STATUS_WARNING)
    {
        period = 350;
        on_time = 120;
    }
    else if (status == STATUS_VERY_CLOSE)
    {
        period = 180;
        on_time = 100;
    }
    else
    {
        period = 800;
        on_time = 120;
    }

    if ((phase_ms % period) < on_time)
    {
        pwm_setDuty(&buzzer, 40);
        pwm_start(&buzzer);
    }
    else
    {
        pwm_stop(&buzzer);
    }
}

int main(void)
{
    uint8_t mode = MODE_SLOT;
    uint8_t occupied;
    uint8_t status = STATUS_NO_ECHO;
    uint8_t last_occupied = 2;
    uint16_t distance = 0;
    uint16_t sample_counter = 0;
    uint16_t phase_ms = 0;
    char key;
    char last_key = '\0';

    keypad_init(&keypad);
    lcd_init(&lcd);
    seven_segment_init(&display);
    ultrasonic_init(&ultrasonic);
    ir_init(&ir);
    led_init(&available_led);
    led_init(&occupied_led);
    pwm_init(&buzzer);
    pwm_stop(&buzzer);

    show_slot_status(read_slot_occupied());

    while (1)
    {
        key = keypad_getkey(&keypad);

        if (key == '\0')
        {
            last_key = '\0';
        }
        else if (key != last_key)
        {
            if (key == 'A')
            {
                mode = MODE_SLOT;
                show_slot_status(read_slot_occupied());
            }
            else if (key == 'B')
            {
                mode = MODE_REVERSE;
                sample_counter = 0;
                lcd_clear(&lcd);
            }
            last_key = key;
        }

        occupied = read_slot_occupied();
        update_slot_leds(occupied);

        if (mode == MODE_SLOT)
        {
            if (last_occupied != occupied)
            {
                show_slot_status(occupied);
                last_occupied = occupied;
            }
            seven_segment_off(&display);
            pwm_stop(&buzzer);
        }
        else
        {
            if (sample_counter == 0)
            {
                distance = ultrasonic_read_cm(&ultrasonic);
                status = get_distance_status(distance);
                show_distance(distance, status);
            }

            if (distance == 0)
            {
                seven_segment_off(&display);
            }
            else
            {
                seven_segment_show_number(&display,
                    (distance > 99U) ? 99U : (uint8_t)distance);
            }

            update_buzzer(status, phase_ms);
            phase_ms = (uint16_t)(phase_ms + 6U);
            sample_counter++;
            if (sample_counter >= 25U)
            {
                sample_counter = 0;
            }
        }
    }
}
