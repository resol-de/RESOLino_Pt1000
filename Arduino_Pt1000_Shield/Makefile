PROJECT_NAME:=Arduino_Pt1000_Shield_0.1-SNAPSHOT

THIS_MAKEFILE_PATH:=$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
THIS_DIR:=$(shell cd $(dir $(THIS_MAKEFILE_PATH));pwd)

BASE_DIR:=$(THIS_DIR)
OUT_DIR=$(BASE_DIR)/output

TARGET_CC=avr-gcc
TARGET_CFLAGS=-g -Wall -Werror -Os -mmcu=atmega32 -I$(BASE_DIR)/src -I$(BASE_DIR)/src/Common -I$(BASE_DIR)/src/Application -I$(BASE_DIR)/include -I$(BASE_DIR)/src/Platform/atmega -DF_CPU=8000000UL -DRVM_FEATURE_HEAPLESS -DSPI_DISABLE_TIMEOUT=1

TARGET_LD=avr-gcc
TARGET_LDFLAGS=-g -Wall -Werror -mmcu=atmega32

TARGET_OBJCOPY=avr-objcopy
TARGET_SIZE=avr-size

AVRDUDE=avrdude
AVRDUDE_CONF_FILE:=/Applications/Arduino.app/Contents/Java/hardware/tools/avr/etc/avrdude.conf
AVRDUDE_OPTS=-p m32 -b 115200 -P usb -c avrispmkII -C $(AVRDUDE_CONF_FILE)


all: $(OUT_DIR)/bin/$(PROJECT_NAME).hex


SRCS=\
	$(BASE_DIR)/src/Common/RsRingBuffer.c \
	$(BASE_DIR)/src/Common/RsVBusMT.c \
	$(BASE_DIR)/src/Common/RsVBusST.c \
	$(BASE_DIR)/src/Application/Application.c \
	$(BASE_DIR)/src/Application/Communication.c \
	$(BASE_DIR)/src/Application/Value.c \
	$(BASE_DIR)/src/Platform/atmega/AdcAD7790.c \
	$(BASE_DIR)/src/Platform/atmega/HalAvrGcc.c \
	$(BASE_DIR)/src/Platform/atmega/Input.c \
	$(BASE_DIR)/src/Platform/atmega/Platform.c \
	$(BASE_DIR)/src/Platform/atmega/Spi.c \
	$(BASE_DIR)/src/Platform/atmega/Uart.c \
	$(BASE_DIR)/src/Platform/atmega/Xprint.c


$(OUT_DIR)/bin/$(PROJECT_NAME).elf: $(SRCS)
	mkdir -p $(OUT_DIR)/tmp/Common $(OUT_DIR)/tmp/Platform/atmega $(OUT_DIR)/tmp/Application $(OUT_DIR)/bin

	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $(OUT_DIR)/tmp/Common/RsRingBuffer.c.o $(BASE_DIR)/src/Common/RsRingBuffer.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $(OUT_DIR)/tmp/Common/RsVBusMT.c.o $(BASE_DIR)/src/Common/RsVBusMT.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $(OUT_DIR)/tmp/Common/RsVBusST.c.o $(BASE_DIR)/src/Common/RsVBusST.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $(OUT_DIR)/tmp/Application/Application.c.o $(BASE_DIR)/src/Application/Application.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $(OUT_DIR)/tmp/Application/Communication.c.o $(BASE_DIR)/src/Application/Communication.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $(OUT_DIR)/tmp/Application/Value.c.o $(BASE_DIR)/src/Application/Value.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $(OUT_DIR)/tmp/Platform/atmega/AdcAD7790.c.o $(BASE_DIR)/src/Platform/atmega/AdcAD7790.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $(OUT_DIR)/tmp/Platform/atmega/HalAvrGcc.c.o $(BASE_DIR)/src/Platform/atmega/HalAvrGcc.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $(OUT_DIR)/tmp/Platform/atmega/Input.c.o $(BASE_DIR)/src/Platform/atmega/Input.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $(OUT_DIR)/tmp/Platform/atmega/Platform.c.o $(BASE_DIR)/src/Platform/atmega/Platform.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $(OUT_DIR)/tmp/Platform/atmega/Spi.c.o $(BASE_DIR)/src/Platform/atmega/Spi.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $(OUT_DIR)/tmp/Platform/atmega/Uart.c.o $(BASE_DIR)/src/Platform/atmega/Uart.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $(OUT_DIR)/tmp/Platform/atmega/Xprint.c.o $(BASE_DIR)/src/Platform/atmega/Xprint.c

	$(TARGET_LD) -o $(OUT_DIR)/bin/$(PROJECT_NAME).elf  $(OUT_DIR)/tmp/Common/RsRingBuffer.c.o $(OUT_DIR)/tmp/Common/RsVBusMT.c.o $(OUT_DIR)/tmp/Common/RsVBusST.c.o $(OUT_DIR)/tmp/Application/Application.c.o $(OUT_DIR)/tmp/Application/Communication.c.o $(OUT_DIR)/tmp/Application/Value.c.o $(OUT_DIR)/tmp/Platform/atmega/AdcAD7790.c.o $(OUT_DIR)/tmp/Platform/atmega/HalAvrGcc.c.o $(OUT_DIR)/tmp/Platform/atmega/Input.c.o $(OUT_DIR)/tmp/Platform/atmega/Platform.c.o $(OUT_DIR)/tmp/Platform/atmega/Spi.c.o $(OUT_DIR)/tmp/Platform/atmega/Uart.c.o $(OUT_DIR)/tmp/Platform/atmega/Xprint.c.o $(TARGET_LDFLAGS) -Wl,-Map=$(OUT_DIR)/bin/$(PROJECT_NAME).elf.map -lm

	$(TARGET_SIZE) -C --mcu=atmega32 $(OUT_DIR)/bin/$(PROJECT_NAME).elf


$(OUT_DIR)/bin/$(PROJECT_NAME).hex: $(OUT_DIR)/bin/$(PROJECT_NAME).elf
	$(TARGET_OBJCOPY) -O ihex $(OUT_DIR)/bin/$(PROJECT_NAME).elf $(OUT_DIR)/bin/$(PROJECT_NAME).hex


flash: $(OUT_DIR)/bin/$(PROJECT_NAME).hex
	# fake chip erase
	$(AVRDUDE) $(AVRDUDE_OPTS) -e -n

	# Erase chip
	$(AVRDUDE) $(AVRDUDE_OPTS) -e

	# Upload bootloader code
	$(AVRDUDE) $(AVRDUDE_OPTS) -U flash:w:$(OUT_DIR)/bin/$(PROJECT_NAME).hex:i

.PHONY: flash
