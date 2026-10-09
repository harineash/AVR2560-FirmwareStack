#ifndef GPIO_H
#define GPIO_H

#include "define.h"

void gpio_portMode(GPIO_Port port, uint8_t mode);
void gpio_portWrite(GPIO_Port port, uint8_t value);
uint8_t gpio_portRead(GPIO_Port port);
void gpio_pinMode(GPIO_Port port, uint8_t pin, uint8_t mode);
void gpio_pinWrite(GPIO_Port port, uint8_t pin, uint8_t value);
uint8_t gpio_pinRead(GPIO_Port port, uint8_t pin);
void gpio_pinToggle(GPIO_Port port, uint8_t pin);

#endif
