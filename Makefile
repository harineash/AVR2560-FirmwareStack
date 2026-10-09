MCU = atmega2560
F_CPU = 16000000UL
CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -std=gnu11 -Os -Wall -Wextra -Iinclude
SRC = $(wildcard src/*.c)
APP ?= app/main.c
ELF = main.elf
HEX = main.hex

.PHONY: all clean

all: $(HEX)

$(ELF): $(SRC) $(APP)
	$(CC) $(CFLAGS) $(SRC) $(APP) -o $@

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex -R .eeprom $< $@

clean:
	rm -f $(ELF) $(HEX)
