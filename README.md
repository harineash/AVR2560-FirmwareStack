# AVR2560 FirmwareStack — Clean Smart Parking Build

Bare-metal C drivers for the ATmega2560. Application code uses driver APIs; register access stays inside the drivers.

## Build

```sh
make clean
make
```

The default target builds `app/main.c` for a 16 MHz ATmega2560. Requires `avr-gcc` and `avr-objcopy`.

## Pin configuration

| Component | ATmega2560 port pins |
|---|---|
| 4×4 keypad rows | PA4–PA7 |
| 4×4 keypad columns | PA0–PA3 |
| Ultrasonic TRIG / ECHO | PB0 / PB1 |
| LCD D4–D7 | PC0–PC3 |
| LCD RS / E | PC4 / PC5; RW to GND |
| Seven-segment a–g, dp | PF0–PF7 |
| Seven-segment tens / units enable | PG0 / PG1 |
| IR occupancy sensor | PK0 |
| Available / occupied LEDs | PJ0 / PJ1 |
| PWM buzzer | PB7 (Timer0 OC0A) |

This pin map assumes a common-cathode seven-segment display with active-high digit enables and an active-low IR sensor output. Change the IR active level if your module behaves differently.

## Operation

- Key `A`: show IR-based slot status on the LCD and available/occupied LEDs.
- Key `B`: measure rear-boundary distance, show it on the LCD and two-digit display, and drive buzzer warnings.
- Distance thresholds: over 30 cm SAFE, 16–30 cm CAUTION, 10–15 cm WARNING, 5–9 cm VERY CLOSE, under 5 cm STOP.
- A zero ultrasonic reading is treated as no echo, not as a safe distance.

## Drivers

GPIO, delay, timer, PWM, ADC, keypad, LCD, seven-segment, LED, switch, IR and ultrasonic.

Timer1 is used by the delay and ultrasonic drivers. Do not configure Timer1 for another purpose while using those functions.
