
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

#define SAMPLE_INTERVAL 25U
#define BUZZER_DUTY     40U

typedef enum
{
    STATUS_NO_ECHO,
    STATUS_SAFE,
    STATUS_CAUTION,
    STATUS_WARNING,
    STATUS_VERY_CLOSE,
    STATUS_STOP
} ParkingStatus;

/* Keypad: rows PA4-PA7, columns PA0-PA3. */
static Keypad_Config keypad =
{
    GPIO_PORTA,
    GPIO_PORTA,
    {4, 5, 6, 7},
    {0, 1, 2, 3},
    "123A456B789C*0#D"
};

/* LCD: data PC0-PC3, RS PC4, E PC5, RW grounded. */
static LCD_Config lcd =
{
    GPIO_PORTC, 4, 5, 0
};

/* Seven-segment: PF0-PF7 segments, PG0/PG1 digit selects. */
static SevenSegment_Config display =
{
    GPIO_PORTF,
    GPIO_PORTG,
    COMMON_CATHODE,
    0x01,
    0x02
};

/* Ultrasonic: TRIG PB0, ECHO PB1. */
static Ultrasonic_Config ultrasonic =
{
    GPIO_PORTB,
    GPIO_PORTB,
    0,
    1
};

/* IR sensor: PK0, active LOW. */
static IR_Config ir =
{
    GPIO_PORTK,
    0,
    IR_ACTIVE_LOW
};

/* LEDs: PJ0 available, PJ1 occupied. */
static LED_Config available_led =
{
    GPIO_PORTJ, 0, HIGH
};

static LED_Config occupied_led =
{
    GPIO_PORTJ, 1, HIGH
};

/* Buzzer: Timer0 channel A, OC0A/PB7. */
static PWM_Config buzzer =
{
    TIMER0,
    PWM_CHANNEL_A,
    BUZZER_DUTY
};

static uint8_t mode = MODE_SLOT;
static uint8_t key_latched = 0U;
static uint8_t buzzer_on = 0U;
static uint8_t last_slot_state = 2U;

static uint16_t distance_cm = 0U;
static uint16_t sample_counter = SAMPLE_INTERVAL;
static uint16_t buzzer_phase = 0U;

/* -------------------------------------------------- */
/* Buzzer control                                     */
/* -------------------------------------------------- */

static void buzzer_set(uint8_t enable)
{
    if (enable && !buzzer_on)
    {
        pwm_setDuty(&buzzer, BUZZER_DUTY);
        pwm_start(&buzzer);
        buzzer_on = 1U;
    }
    else if (!enable && buzzer_on)
    {
        pwm_stop(&buzzer);
        buzzer_on = 0U;
    }
}

static void update_buzzer(ParkingStatus status, uint16_t phase)
{
    uint16_t position;
    uint8_t enable = 0U;

    switch (status)
    {
        case STATUS_CAUTION:
            position = phase % 800U;
            enable = (position < 100U);
            break;

        case STATUS_WARNING:
            position = phase % 500U;
            enable = (position < 70U) ||
                     (position >= 140U && position < 210U);
            break;

        case STATUS_VERY_CLOSE:
            position = phase % 300U;
            enable = (position < 50U) ||
                     (position >= 80U && position < 130U) ||
                     (position >= 160U && position < 210U);
            break;

        case STATUS_STOP:
            enable = 1U;
            break;

        case STATUS_SAFE:
        case STATUS_NO_ECHO:
        default:
            enable = 0U;
            break;
    }

    buzzer_set(enable);
}

/* -------------------------------------------------- */
/* Parking-slot functions                             */
/* -------------------------------------------------- */

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

    if (occupied)
    {
        lcd_print(&lcd, "SLOT OCCUPIED   ");
    }
    else
    {
        lcd_print(&lcd, "SLOT AVAILABLE  ");
    }
}

/* -------------------------------------------------- */
/* Distance classification                            */
/* -------------------------------------------------- */

static ParkingStatus get_distance_status(uint16_t cm)
{
    if (cm == 0U)
    {
        return STATUS_NO_ECHO;
    }

    if (cm > 30U)
    {
        return STATUS_SAFE;
    }

    if (cm >= 16U)
    {
        return STATUS_CAUTION;
    }

    if (cm >= 10U)
    {
        return STATUS_WARNING;
    }

    if (cm >= 5U)
    {
        return STATUS_VERY_CLOSE;
    }

    return STATUS_STOP;
}

/* -------------------------------------------------- */
/* LCD distance output                                */
/* -------------------------------------------------- */

static void show_distance(uint16_t cm, ParkingStatus status)
{
    lcd_goto(&lcd, 0, 0);
    lcd_print(&lcd, "DISTANCE:       ");

    lcd_goto(&lcd, 0, 10);

    if (cm == 0U)
    {
        lcd_print(&lcd, "--cm");
    }
    else
    {
        if (cm >= 100U)
        {
            lcd_data(&lcd, (uint8_t)('0' + (cm / 100U)));
            cm %= 100U;
        }

        if (cm >= 10U)
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
        case STATUS_NO_ECHO:
            lcd_print(&lcd, "NO ECHO - CHECK ");
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

        case STATUS_STOP:
        default:
            lcd_print(&lcd, "STOP            ");
            break;
    }
}

/* -------------------------------------------------- */
/* Main application                                   */
/* -------------------------------------------------- */

int main(void)
{
    char key;
    uint8_t occupied;
    ParkingStatus status = STATUS_NO_ECHO;

    keypad_init(&keypad);
    lcd_init(&lcd);
    seven_segment_init(&display);
    ultrasonic_init(&ultrasonic);
    ir_init(&ir);

    led_init(&available_led);
    led_init(&occupied_led);

    pwm_init(&buzzer);
    buzzer_set(0U);

    occupied = read_slot_occupied();
    last_slot_state = occupied;

    update_slot_leds(occupied);
    show_slot_status(occupied);

    while (1)
    {
        /* Process keypad presses. */
        key = keypad_getkey(&keypad);

        if (key == '\0')
        {
            key_latched = 0U;
        }
        else if (!key_latched)
        {
            key_latched = 1U;

            if (key == 'A' && mode != MODE_SLOT)
            {
                mode = MODE_SLOT;
                sample_counter = SAMPLE_INTERVAL;

                buzzer_set(0U);
                seven_segment_off(&display);

                occupied = read_slot_occupied();
                last_slot_state = occupied;

                update_slot_leds(occupied);
                show_slot_status(occupied);
            }
            else if (key == 'B' && mode != MODE_REVERSE)
            {
                mode = MODE_REVERSE;
                distance_cm = 0U;
                sample_counter = SAMPLE_INTERVAL;
                buzzer_phase = 0U;

                buzzer_set(0U);
                seven_segment_show_number(&display, 0U);

                lcd_clear(&lcd);
                show_distance(0U, STATUS_NO_ECHO);
            }
        }

        /* ------------------------------------------ */
        /* Parking-slot monitoring mode               */
        /* ------------------------------------------ */

        if (mode == MODE_SLOT)
        {
            occupied = read_slot_occupied();
            update_slot_leds(occupied);

            if (occupied != last_slot_state)
            {
                show_slot_status(occupied);
                last_slot_state = occupied;
            }

            seven_segment_off(&display);
            buzzer_set(0U);
        }

        /* ------------------------------------------ */
        /* Reverse-assistance mode                    */
        /* ------------------------------------------ */

        else
        {
            if (sample_counter >= SAMPLE_INTERVAL)
            {
                distance_cm = ultrasonic_read_cm(&ultrasonic);
                status = get_distance_status(distance_cm);

                show_distance(distance_cm, status);
                sample_counter = 0U;
            }

            /*
             * Update the displayed number only.
             * Timer3 handles multiplexing independently.
             */
            if (distance_cm > 99U)
            {
                seven_segment_show_number(&display, 99U);
            }
            else
            {
                seven_segment_show_number(
                    &display,
                    (uint8_t)distance_cm
                );
            }

            update_buzzer(status, buzzer_phase);

            buzzer_phase =
                (uint16_t)((buzzer_phase + 6U) % 6000U);

            if (sample_counter < SAMPLE_INTERVAL)
            {
                sample_counter++;
            }
        }
    }
}
