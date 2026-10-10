```c
#include <stdint.h>

#include "gpio.h"
#include "keypad.h"
#include "lcd.h"
#include "seven_segment.h"
#include "ultrasonic.h"
#include "ir.h"
#include "led.h"
#include "pwm.h"
#include "timer.h"

#define MODE_SLOT       0
#define MODE_REVERSE    1
#define MODE_BOTH       2

#define STATUS_NO_ECHO   0
#define STATUS_SAFE      1
#define STATUS_CAUTION   2
#define STATUS_WARNING   3
#define STATUS_VERY_CLOSE 4
#define STATUS_STOP      5

#define SAMPLE_COUNT 25U

static Keypad_Config keypad =
{
    GPIO_PORTA, GPIO_PORTA,
    {4, 5, 6, 7},
    {0, 1, 2, 3},
    "123A456B789C*0#D"
};

static LCD_Config lcd =
{
    GPIO_PORTC, 4, 5, 0
};

static SevenSegment_Config display =
{
    GPIO_PORTF,
    GPIO_PORTG,
    COMMON_CATHODE,
    0x01,
    0x02
};

static Ultrasonic_Config ultrasonic =
{
    GPIO_PORTB, GPIO_PORTB, 0, 1
};

static IR_Config ir =
{
    GPIO_PORTK, 0, IR_ACTIVE_LOW
};

static LED_Config available_led =
{
    GPIO_PORTJ, 0, HIGH
};

static LED_Config occupied_led =
{
    GPIO_PORTJ, 1, HIGH
};

static PWM_Config buzzer =
{
    TIMER0, PWM_CHANNEL_A, 40
};

static uint8_t mode = MODE_SLOT;
static uint8_t status = STATUS_NO_ECHO;
static uint8_t distance_measured = 0;
static uint8_t last_occupied = 2;

static uint16_t distance_cm = 0;
static uint16_t sample_counter = 0;
static uint16_t phase_ms = 0;

static char last_key = '\0';


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


static void print_distance(void)
{
    if (!distance_measured ||
        distance_cm == 0 ||
        distance_cm > 99)
    {
        lcd_print(&lcd, "--cm");
    }
    else
    {
        lcd_data(&lcd, (uint8_t)('0' + distance_cm / 10U));
        lcd_data(&lcd, (uint8_t)('0' + distance_cm % 10U));
        lcd_print(&lcd, "cm");
    }
}


static void print_status(void)
{
    switch (status)
    {
        case STATUS_NO_ECHO:
            lcd_print(&lcd, "WAITING FOR ECHO");
            break;

        case STATUS_SAFE:
            lcd_print(&lcd, "SAFE            ");
            break;

        case STATUS_CAUTION:
            lcd_print(&lcd, "CAUTION         ");
            break;

        case STATUS_WARNING:
            lcd_print(&lcd, "WARNING         ");
            break;

        case STATUS_VERY_CLOSE:
            lcd_print(&lcd, "VERY CLOSE      ");
            break;

        default:
            lcd_print(&lcd, "STOP            ");
            break;
    }
}


static void show_slot_status(uint8_t occupied)
{
    lcd_goto(&lcd, 0, 0);
    lcd_print(&lcd, "PARKING SLOT    ");

    lcd_goto(&lcd, 1, 0);

    if (occupied)
    {
        lcd_print(&lcd, "SLOT OCCUPIED   ");
    }
    else
    {
        lcd_print(&lcd, "SLOT AVAILABLE  ");
    }
}


static void show_reverse_status(void)
{
    lcd_goto(&lcd, 0, 0);
    lcd_print(&lcd, "DISTANCE:       ");

    lcd_goto(&lcd, 0, 10);
    print_distance();

    lcd_goto(&lcd, 1, 0);
    print_status();
}


static void show_both_status(uint8_t occupied)
{
    lcd_goto(&lcd, 0, 0);

    if (occupied)
    {
        lcd_print(&lcd, "SLOT: OCCUPIED  ");
    }
    else
    {
        lcd_print(&lcd, "SLOT: AVAILABLE ");
    }

    lcd_goto(&lcd, 1, 0);
    lcd_print(&lcd, "D:");
    print_distance();

    lcd_goto(&lcd, 1, 8);
    print_status();
}


/*
 * Older buzzer pattern:
 * SAFE / no measurement : OFF
 * CAUTION               : 1 short beep per 800 ms
 * WARNING               : 2 short beeps per 500 ms
 * VERY CLOSE            : 3 short beeps per 300 ms
 * STOP                  : Continuous ON
 */
static void update_buzzer(void)
{
    uint16_t period;
    uint8_t beep_count;
    uint16_t position;
    uint16_t beep_position;

    if (!distance_measured ||
        status == STATUS_NO_ECHO ||
        status == STATUS_SAFE)
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

    if (status == STATUS_CAUTION)
    {
        period = 800;
        beep_count = 1;
    }
    else if (status == STATUS_WARNING)
    {
        period = 500;
        beep_count = 2;
    }
    else
    {
        period = 300;
        beep_count = 3;
    }

    position = phase_ms % period;

    /*
     * Each beep is approximately 70 ms ON.
     * Beeps are spaced 100 ms apart.
     */
    beep_position = position % 100U;

    if ((position / 100U) < beep_count &&
        beep_position < 70U)
    {
        pwm_setDuty(&buzzer, 40);
        pwm_start(&buzzer);
    }
    else
    {
        pwm_stop(&buzzer);
    }
}


static void select_mode(char key)
{
    if (key == 'A')
    {
        mode = MODE_SLOT;
        sample_counter = 0;
        phase_ms = 0;

        seven_segment_off(&display);
        pwm_stop(&buzzer);
        lcd_clear(&lcd);
    }
    else if (key == 'B')
    {
        mode = MODE_REVERSE;
        sample_counter = 0;
        phase_ms = 0;
        distance_measured = 0;
        distance_cm = 0;
        status = STATUS_NO_ECHO;

        seven_segment_off(&display);
        pwm_stop(&buzzer);
        lcd_clear(&lcd);
    }
    else if (key == 'C')
    {
        mode = MODE_BOTH;
        sample_counter = 0;
        phase_ms = 0;
        distance_measured = 0;
        distance_cm = 0;
        status = STATUS_NO_ECHO;

        seven_segment_off(&display);
        pwm_stop(&buzzer);
        lcd_clear(&lcd);
    }
}


int main(void)
{
    uint8_t occupied;
    char key;

    keypad_init(&keypad);
    lcd_init(&lcd);
    seven_segment_init(&display);
    ultrasonic_init(&ultrasonic);
    ir_init(&ir);

    led_init(&available_led);
    led_init(&occupied_led);

    pwm_init(&buzzer);
    pwm_stop(&buzzer);

    lcd_clear(&lcd);
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
            select_mode(key);
            last_key = key;
        }

        occupied = read_slot_occupied();
        update_slot_leds(occupied);

        if (mode == MODE_SLOT)
        {
            if (occupied != last_occupied)
            {
                show_slot_status(occupied);
            }

            last_occupied = occupied;

            seven_segment_off(&display);
            pwm_stop(&buzzer);
            phase_ms = 0;
        }
        else
        {
            if (sample_counter == 0)
            {
                uint16_t reading;

                reading = ultrasonic_read_cm(&ultrasonic);

                if (reading > 0)
                {
                    distance_cm = reading;
                    distance_measured = 1;
                }
                else
                {
                    distance_cm = 0;
                    distance_measured = 0;
                }

                status = get_distance_status(distance_cm);
            }

            if (distance_measured &&
                distance_cm >= 1 &&
                distance_cm <= 99)
            {
                seven_segment_show_number(
                    &display,
                    (uint8_t)distance_cm
                );
            }
            else
            {
                seven_segment_off(&display);
            }

            update_buzzer();

            phase_ms = (uint16_t)(phase_ms + 6U);

            sample_counter++;

            if (sample_counter >= SAMPLE_COUNT)
            {
                sample_counter = 0;
            }
        }

        if (mode == MODE_SLOT)
        {
            /* Slot status is refreshed only when it changes. */
        }
        else if (mode == MODE_REVERSE)
        {
            show_reverse_status();
        }
        else
        {
            show_both_status(occupied);
        }
    }
}
```
