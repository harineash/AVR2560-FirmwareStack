#include "gpio.h"

void gpio_portMode(GPIO_Port port, uint8_t mode)
{
    *(volatile uint8_t *)(uintptr_t)port.ddr_address = mode;
}

void gpio_portWrite(GPIO_Port port, uint8_t value)
{
    *(volatile uint8_t *)(uintptr_t)port.port_address = value;
}

uint8_t gpio_portRead(GPIO_Port port)
{
    return *(volatile uint8_t *)(uintptr_t)port.pin_address;
}

void gpio_pinMode(GPIO_Port port, uint8_t pin, uint8_t mode)
{
    volatile uint8_t *reg = (volatile uint8_t *)(uintptr_t)port.ddr_address;

    if (pin > 7)
    {
        return;
    }

    if (mode == OUTPUT)
    {
        *reg |= (uint8_t)(1U << pin);
    }
    else
    {
        *reg &= (uint8_t)~(1U << pin);
    }
}

void gpio_pinWrite(GPIO_Port port, uint8_t pin, uint8_t value)
{
    volatile uint8_t *reg = (volatile uint8_t *)(uintptr_t)port.port_address;

    if (pin > 7)
    {
        return;
    }

    if (value == HIGH)
    {
        *reg |= (uint8_t)(1U << pin);
    }
    else
    {
        *reg &= (uint8_t)~(1U << pin);
    }
}

uint8_t gpio_pinRead(GPIO_Port port, uint8_t pin)
{
    if (pin > 7)
    {
        return LOW;
    }

    return (uint8_t)((*(volatile uint8_t *)(uintptr_t)port.pin_address >> pin) & 1U);
}

void gpio_pinToggle(GPIO_Port port, uint8_t pin)
{
    if (pin > 7)
    {
        return;
    }

    *(volatile uint8_t *)(uintptr_t)port.port_address ^= (uint8_t)(1U << pin);
}
