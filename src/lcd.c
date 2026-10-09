#include "lcd.h"
#include "timer.h"

static void lcd_pulse_enable(LCD_Config *lcd)
{
    gpio_pinWrite(lcd->data_port, lcd->en_pin, HIGH);
    timer_delay_ms(1);
    gpio_pinWrite(lcd->data_port, lcd->en_pin, LOW);
    timer_delay_ms(1);
}

static void lcd_write_nibble(LCD_Config *lcd, uint8_t nibble)
{
    uint8_t value = 0;
    uint8_t i;

    for (i = 0; i < 4; i++)
    {
        if (nibble & (1U << i))
        {
            value |= (uint8_t)(1U << (lcd->data_start + i));
        }
    }

    gpio_portWrite(lcd->data_port, value);
    lcd_pulse_enable(lcd);
}

static void lcd_write_byte(LCD_Config *lcd, uint8_t value)
{
    lcd_write_nibble(lcd, (uint8_t)(value >> 4));
    lcd_write_nibble(lcd, (uint8_t)(value & 0x0F));
}

void lcd_command(LCD_Config *lcd, uint8_t command)
{
    gpio_pinWrite(lcd->data_port, lcd->rs_pin, LOW);
    lcd_write_byte(lcd, command);

    if (command == 0x01 || command == 0x02)
    {
        timer_delay_ms(2);
    }
}

void lcd_data(LCD_Config *lcd, uint8_t data)
{
    gpio_pinWrite(lcd->data_port, lcd->rs_pin, HIGH);
    lcd_write_byte(lcd, data);
}

void lcd_init(LCD_Config *lcd)
{
    uint8_t data_mask =
        (uint8_t)(0x0F << lcd->data_start);

    gpio_portMode(lcd->data_port,
                  (uint8_t)(data_mask |
                  (1U << lcd->rs_pin) |
                  (1U << lcd->en_pin)));

    gpio_pinWrite(lcd->data_port, lcd->rs_pin, LOW);
    gpio_pinWrite(lcd->data_port, lcd->en_pin, LOW);

    timer_delay_ms(20);

    lcd_write_nibble(lcd, 0x03);
    timer_delay_ms(5);
    lcd_write_nibble(lcd, 0x03);
    timer_delay_ms(1);
    lcd_write_nibble(lcd, 0x03);
    lcd_write_nibble(lcd, 0x02);

    lcd_command(lcd, 0x28);
    lcd_command(lcd, 0x0C);
    lcd_command(lcd, 0x06);
    lcd_command(lcd, 0x01);
}

void lcd_print(LCD_Config *lcd, const char *text)
{
    while (*text != '\0')
    {
        lcd_data(lcd, (uint8_t)*text);
        text++;
    }
}

void lcd_goto(LCD_Config *lcd, uint8_t row, uint8_t column)
{
    uint8_t address;

    if (row == 0)
    {
        address = (uint8_t)(0x80 + column);
    }
    else
    {
        address = (uint8_t)(0xC0 + column);
    }

    lcd_command(lcd, address);
}

void lcd_clear(LCD_Config *lcd)
{
    lcd_command(lcd, 0x01);
}
