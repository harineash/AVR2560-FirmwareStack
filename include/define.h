#ifndef DEFINE_H
#define DEFINE_H

#include <stdint.h>

#define INPUT   0
#define OUTPUT  1
#define LOW     0
#define HIGH    1
#define ENABLE  1
#define DISABLE 0

#define SET_BIT(reg, bit)    ((reg) |= (uint8_t)(1U << (bit)))
#define CLEAR_BIT(reg, bit)  ((reg) &= (uint8_t)~(1U << (bit)))
#define READ_BIT(reg, bit)   (((reg) >> (bit)) & 1U)
#define TOGGLE_BIT(reg, bit) ((reg) ^= (uint8_t)(1U << (bit)))

typedef struct
{
    uint16_t pin_address;
    uint16_t ddr_address;
    uint16_t port_address;
} GPIO_Port;

#define GPIO_PORTA ((GPIO_Port){0x20, 0x21, 0x22})
#define GPIO_PORTB ((GPIO_Port){0x23, 0x24, 0x25})
#define GPIO_PORTC ((GPIO_Port){0x26, 0x27, 0x28})
#define GPIO_PORTD ((GPIO_Port){0x29, 0x2A, 0x2B})
#define GPIO_PORTE ((GPIO_Port){0x2C, 0x2D, 0x2E})
#define GPIO_PORTF ((GPIO_Port){0x2F, 0x30, 0x31})
#define GPIO_PORTG ((GPIO_Port){0x32, 0x33, 0x34})
#define GPIO_PORTH ((GPIO_Port){0x100, 0x101, 0x102})
#define GPIO_PORTJ ((GPIO_Port){0x103, 0x104, 0x105})
#define GPIO_PORTK ((GPIO_Port){0x106, 0x107, 0x108})
#define GPIO_PORTL ((GPIO_Port){0x109, 0x10A, 0x10B})

#define TCCR0A_ADDR 0x44
#define TCCR0B_ADDR 0x45
#define TCNT0_ADDR  0x46
#define OCR0A_ADDR  0x47
#define OCR0B_ADDR  0x48
#define TIFR0_ADDR  0x35
#define TIMSK0_ADDR 0x6E

#define TIFR1_ADDR  0x36
#define TCCR1A_ADDR 0x80
#define TCCR1B_ADDR 0x81
#define TCCR1C_ADDR 0x82
#define TCNT1_ADDR  0x84
#define ICR1_ADDR   0x86
#define OCR1A_ADDR  0x88
#define OCR1B_ADDR  0x8A

#define TIFR2_ADDR  0x37
#define TCCR2A_ADDR 0xB0
#define TCCR2B_ADDR 0xB1
#define TCNT2_ADDR  0xB2
#define OCR2A_ADDR  0xB3
#define OCR2B_ADDR  0xB4
#define TIMSK2_ADDR 0x70

#define SREG_ADDR   0x5F
#define ADCL_ADDR   0x78
#define ADCH_ADDR   0x79
#define ADCSRA_ADDR 0x7A
#define ADCSRB_ADDR 0x7B
#define ADMUX_ADDR  0x7C

#define UCSR0A_ADDR 0xC0
#define UCSR0B_ADDR 0xC1
#define UCSR0C_ADDR 0xC2
#define UBRR0L_ADDR 0xC4
#define UBRR0H_ADDR 0xC5
#define UDR0_ADDR   0xC6

#define COMMON_CATHODE 0
#define COMMON_ANODE   1
#define ADC_REF_AREF   0
#define ADC_REF_AVCC   1
#define ADC_REF_1V1    3
#define TIMER_NORMAL   0
#define TIMER_CTC      1
#define TIMER0         0
#define TIMER1         1
#define TIMER2         2
#define PRESCALER_1    1
#define PRESCALER_8    8
#define PRESCALER_32   32
#define PRESCALER_64   64
#define PRESCALER_128  128
#define PRESCALER_256  256
#define PRESCALER_1024 1024
#define PWM_CHANNEL_A  0
#define PWM_CHANNEL_B  1

#endif
