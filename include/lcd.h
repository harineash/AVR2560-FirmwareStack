#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include "gpio.h"

typedef struct
{
    GPIO_Port data_port;
    uint8_t rs_pin;
    uint8_t en_pin;
    uint8_t data_start;
} LCD_Config;

void lcd_init(LCD_Config *lcd);
void lcd_command(LCD_Config *lcd, uint8_t command);
void lcd_data(LCD_Config *lcd, uint8_t data);
void lcd_print(LCD_Config *lcd, const char *text);
void lcd_goto(LCD_Config *lcd, uint8_t row, uint8_t column);
void lcd_clear(LCD_Config *lcd);

#endif
