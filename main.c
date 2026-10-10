
#include <stdint.h>

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

#define SAMPLE_INTERVAL 25U

enum
{
    STATUS_NO_ECHO,
    STATUS_SAFE,
    STATUS_CAUTION,
    STATUS_WARNING,
    STATUS_VERY_CLOSE,
    STATUS_STOP
};

/* Hardware configuration */

static Keypad_Config keypad =
{
    GPIO_PORTA,
    GPIO_PORTA,
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
    GPIO_PORTB,
    GPIO_PORTB,
    0,
    1
};

static IR_Config ir =
{
    GPIO_PORTK,
    0,
    IR_ACTIVE_LOW
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


/* Read parking slot occupancy */

static uint8_t read_slot_occupied(void)
{
    return ir_detected(&ir);
}


/* Update slot indicator LEDs */

static void update_slot_leds(uint8_t occupied)
{
    if (occupied)
    {
        led_on(&occupied_led);
        led_off(&available_led);
    }
    else
    {
        led_on(&available_led);
        led_off(&occupied_led);
    }
}


/* Display parking slot status */

static void show_slot_status(uint8_t occupied)
{
    lcd_clear();

    lcd_goto(0, 0);
    lcd_print("PARKING SLOT");

    lcd_goto(1, 0);

    if (occupied)
    {
        lcd_print("SLOT OCCUPIED");
    }
    else
    {
        lcd_print("SLOT AVAILABLE");
    }
}


/* Determine reverse-parking warning level */

static uint8_t get_distance_status(uint16_t cm)
{
    if (cm == 0)
        return STATUS_NO_ECHO;

    if (cm > 30)
        return STATUS_SAFE;

    if (cm >= 16)
        return STATUS_CAUTION;

    if (cm >= 10)
        return STATUS_WARNING;

    if (cm >= 5)
        return STATUS_VERY_CLOSE;

    return STATUS_STOP;
}


/* Display reverse-parking distance and status */

static void show_distance(uint16_t cm, uint8_t status)
{
    lcd_goto(0, 0);
    lcd_print("DISTANCE:       ");

    lcd_goto(0, 10);

    if (cm == 0)
    {
        lcd_print("--cm");
    }
    else
    {
        if (cm >= 100)
        {
            lcd_data((char)('0' + (cm / 100U)));
            cm %= 100U;
        }

        if (cm >= 10)
        {
            lcd_data((char)('0' + (cm / 10U)));
        }
        else
        {
            lcd_data('0');
        }

        lcd_data((char)('0' + (cm % 10U)));
        lcd_print("cm");
    }

    lcd_goto(1, 0);

    switch (status)
    {
        case STATUS_SAFE:
            lcd_print("SAFE            ");
            break;

        case STATUS_CAUTION:
            lcd_print("CAUTION         ");
            break;

        case STATUS_WARNING:
            lcd_print("WARNING         ");
            break;

        case STATUS_VERY_CLOSE:
            lcd_print("VERY CLOSE      ");
            break;

        case STATUS_STOP:
            lcd_print("STOP!           ");
            break;

        default:
            lcd_print("NO ECHO         ");
            break;
    }
}


/* Control buzzer according to distance */

static void update_buzzer(uint8_t status, uint16_t phase_ms)
{
    uint16_t period;
    uint16_t on_time;

    switch (status)
    {
        case STATUS_SAFE:
        case STATUS_NO_ECHO:
            pwm_stop(&buzzer);
            return;

        case STATUS_CAUTION:
            period = 800U;
            on_time = 120U;
            break;

        case STATUS_WARNING:
            period = 350U;
            on_time = 120U;
            break;

        case STATUS_VERY_CLOSE:
            period = 180U;
            on_time = 100U;
            break;

        case STATUS_STOP:
            pwm_start(&buzzer);
            return;

        default:
            pwm_stop(&buzzer);
            return;
    }

    if ((phase_ms % period) < on_time)
    {
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
    uint8_t last_occupied = 2U;

    uint16_t distance_cm = 0;
    uint16_t sample_counter = SAMPLE_INTERVAL;
    uint16_t phase_ms = 0;

    char key;
    char last_key = '\0';

    /* Initialize all drivers */

    keypad_init(&keypad);
    lcd_init(&lcd);
    seven_segment_init(&display);
    ultrasonic_init(&ultrasonic);
    ir_init(&ir);
    led_init(&available_led);
    led_init(&occupied_led);
    pwm_init(&buzzer);

    pwm_stop(&buzzer);
    seven_segment_off(&display);

    show_slot_status(read_slot_occupied());

    while (1)
    {
        /* Read keypad with key-press edge detection */

        key = keypad_getkey(&keypad);

        if (key == '\0')
        {
            last_key = '\0';
        }
        else if (key != last_key)
        {
            if (key == 'A')
            {
                /* Slot checking only */

                mode = MODE_SLOT;
                last_occupied = 2U;

                pwm_stop(&buzzer);
                seven_segment_off(&display);

                show_slot_status(read_slot_occupied());
            }
            else if (key == 'B')
            {
                /* Reverse parking only */

                mode = MODE_REVERSE;
                distance_cm = 0;
                sample_counter = SAMPLE_INTERVAL;

                lcd_clear();
                seven_segment_off(&display);
            }
            else if (key == 'C')
            {
                /* Slot checking + reverse parking */

                mode = MODE_BOTH;
                distance_cm = 0;
                sample_counter = SAMPLE_INTERVAL;
                last_occupied = 2U;

                lcd_clear();
            }

            last_key = key;
        }

        /* Slot checking runs in every mode */

        occupied = read_slot_occupied();
        update_slot_leds(occupied);

        if (mode == MODE_SLOT || mode == MODE_BOTH)
        {
            if (last_occupied != occupied)
            {
                show_slot_status(occupied);
                last_occupied = occupied;
            }
        }

        /* Reverse parking runs in B and C modes */

        if (mode == MODE_REVERSE || mode == MODE_BOTH)
        {
            if (sample_counter >= SAMPLE_INTERVAL)
            {
                distance_cm = ultrasonic_read_cm(&ultrasonic);
                status = get_distance_status(distance_cm);

                show_distance(distance_cm, status);

                sample_counter = 0;
            }

            /*
             * Keep calling this frequently for the original
             * polling-based seven-segment driver.
             */

            if (distance_cm == 0)
            {
                seven_segment_off(&display);
            }
            else
            {
                seven_segment_show_number(
                    &display,
                    (distance_cm > 99U)
                        ? 99U
                        : (uint8_t)distance_cm
                );
            }

            update_buzzer(status, phase_ms);

            phase_ms += 6U;
            sample_counter++;
        }
        else
        {
            /* Slot-only mode */

            seven_segment_off(&display);
            pwm_stop(&buzzer);
            phase_ms = 0;
        }
    }
}
