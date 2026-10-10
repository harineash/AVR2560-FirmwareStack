#include <stdint.h>

#include "keypad.h"
#include "lcd.h"
#include "seven_segment.h"
#include "ultrasonic.h"
#include "ir.h"
#include "led.h"
#include "pwm.h"
#include "timer.h"

#define MODE_SLOT       0U
#define MODE_REVERSE    1U
#define MODE_BOTH       2U

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


/* Read slot occupancy */

static uint8_t read_slot_occupied(void)
{
    return ir_detected(&ir);
}


/* Update parking indicator LEDs */

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


/* Display slot status in Mode A */

static void show_slot_status(uint8_t occupied)
{
    lcd_goto(0, 0);
    lcd_print("PARKING SLOT    ");

    lcd_goto(1, 0);

    if (occupied)
    {
        lcd_print("SLOT OCCUPIED   ");
    }
    else
    {
        lcd_print("SLOT AVAILABLE  ");
    }
}


/* Determine reverse-parking warning */

static uint8_t get_distance_status(uint16_t cm)
{
    if (cm == 0U)
        return STATUS_NO_ECHO;

    if (cm > 30U)
        return STATUS_SAFE;

    if (cm >= 16U)
        return STATUS_CAUTION;

    if (cm >= 10U)
        return STATUS_WARNING;

    if (cm >= 5U)
        return STATUS_VERY_CLOSE;

    return STATUS_STOP;
}


/* Print distance; distances above 99 show -- */

static void print_distance(uint16_t cm)
{
    if (cm == 0U || cm > 99U)
    {
        lcd_print("--cm");
    }
    else
    {
        lcd_data((char)('0' + (cm / 10U)));
        lcd_data((char)('0' + (cm % 10U)));
        lcd_print("cm");
    }
}


/* Display reverse-parking status in Mode B */

static void show_distance(uint16_t cm, uint8_t status)
{
    lcd_goto(0, 0);
    lcd_print("DISTANCE:       ");

    lcd_goto(0, 10);
    print_distance(cm);

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


/* Display both functions in Mode C */

static void show_combined(uint8_t occupied,
                          uint16_t cm,
                          uint8_t status)
{
    lcd_goto(0, 0);

    if (occupied)
    {
        lcd_print("SLOT:OCCUPIED   ");
    }
    else
    {
        lcd_print("SLOT:FREE       ");
    }

    lcd_goto(1, 0);
    lcd_print("D:");

    print_distance(cm);
    lcd_print(" ");

    switch (status)
    {
        case STATUS_SAFE:
            lcd_print("SAFE   ");
            break;

        case STATUS_CAUTION:
            lcd_print("CAUTION");
            break;

        case STATUS_WARNING:
            lcd_print("WARN   ");
            break;

        case STATUS_VERY_CLOSE:
            lcd_print("CLOSE  ");
            break;

        case STATUS_STOP:
            lcd_print("STOP!  ");
            break;

        default:
            lcd_print("NO ECHO");
            break;
    }
}


/* Control buzzer according to distance */

static void update_buzzer(uint8_t status,
                          uint16_t phase_ms)
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
    uint8_t last_occupied = 2U;
    uint8_t status = STATUS_NO_ECHO;

    uint16_t distance_cm = 0U;
    uint16_t sample_counter = SAMPLE_INTERVAL;
    uint16_t phase_ms = 0U;

    char key;
    char last_key = '\0';

    /* Initialize drivers */

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

    occupied = read_slot_occupied();
    update_slot_leds(occupied);
    show_slot_status(occupied);

    while (1)
    {
        /* Keypad edge detection */

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
                last_occupied = 2U;

                seven_segment_off(&display);
                pwm_stop(&buzzer);

                show_slot_status(read_slot_occupied());
            }
            else if (key == 'B')
            {
                mode = MODE_REVERSE;
                distance_cm = 0U;
                status = STATUS_NO_ECHO;
                sample_counter = SAMPLE_INTERVAL;
                phase_ms = 0U;

                lcd_clear();
            }
            else if (key == 'C')
            {
                mode = MODE_BOTH;
                distance_cm = 0U;
                status = STATUS_NO_ECHO;
                sample_counter = SAMPLE_INTERVAL;
                last_occupied = 2U;
                phase_ms = 0U;

                lcd_clear();
            }

            last_key = key;
        }

        /* Slot sensor and LEDs always remain active */

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
            continue;
        }

        /*
         * Take a new ultrasonic reading periodically.
         * The seven-segment driver uses polling, so it
         * must be refreshed frequently in this loop.
         */

        if (sample_counter >= SAMPLE_INTERVAL)
        {
            distance_cm = ultrasonic_read_cm(&ultrasonic);
            status = get_distance_status(distance_cm);

            if (mode == MODE_REVERSE)
            {
                show_distance(distance_cm, status);
            }

            sample_counter = 0U;
        }

        if (mode == MODE_BOTH)
        {
            if (last_occupied != occupied ||
                sample_counter == 0U)
            {
                show_combined(occupied,
                              distance_cm,
                              status);

                last_occupied = occupied;
            }
        }

        /*
         * Numeric display supports only 0-99 cm.
         * Keep refreshing it during normal operation.
         */

        if (distance_cm == 0U || distance_cm > 99U)
        {
            seven_segment_off(&display);
        }
        else
        {
            seven_segment_show_number(
                &display,
                (uint8_t)distance_cm
            );
        }

        update_buzzer(status, phase_ms);

        phase_ms += 6U;
        sample_counter++;
    }
}
