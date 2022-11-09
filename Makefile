# SPDX-License-Identifier: GPL-2.0-only
#
# Project : usbrelay-firmware
#
# Copyright (c) 2022 Thomas Mahe <contact@tmahe.dev>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

##########################################
#           Editable options             #
##########################################

DEVICE  	= atmega8
F_CPU   	= 16000000

FUSE_L	= 0xdf
FUSE_H	= 0xca

PROGRAMMER 		= avrisp
PROGRAMMER_PORT = /dev/ttyUSB0
PROGRAMMER_BAUD = 19200

##########################################
#    Don't touch anything below this     #
##########################################

## Directories
SRC_PROJECT = src
SRC_USBDRV 	= submodules/v-usb/usbdrv
OUTPUT_DIR 	= build
OBJECT_DIR 	= build/obj

## avrdude config
AVRDUDE_FLAGS  = -c $(PROGRAMMER) -p $(DEVICE)
AVRDUDE_FLAGS += -P $(PROGRAMMER_PORT) -b $(PROGRAMMER_BAUD)

## compiler config
CC 			= avr-gcc
CCFLAGS		= -Wall -Wno-discarded-qualifiers -Os -DDEBUG_LEVEL=0
CCFLAGS    += -DF_CPU=$(F_CPU) -mmcu=$(DEVICE)
CCFLAGS    += -I$(SRC_PROJECT) -I$(SRC_USBDRV)

## commands
COMPILE = $(CC) $(CCFLAGS)
AVRDUDE = avrdude $(AVRDUDE_FLAGS)

## Setup source & obj files

SOURCES 	    = $(SRC_USBDRV)/usbdrv.c $(SRC_USBDRV)/usbdrvasm.c $(SRC_USBDRV)/oddebug.c
SOURCES 	   += $(SRC_PROJECT)/main.c
SOURCES_FILES 	= $(notdir $(SOURCES))
VPATH 			= $(sort $(dir $(SOURCES)))
OBJECTS 		= $(SOURCES_FILES:%.c=${OBJECT_DIR}/%.o)

##########################################
#                Targets                 #
##########################################

help:
	@echo "Usage: make <target>\n"
	@echo "  make firmware ....... build firmware"
	@echo "  make flash-fuse ..... flash fuses"
	@echo "  make flash .......... flash firmware"
	@echo "  make clean .......... delete objects and hex file"

firmware: clean _directories $(OBJECTS)
	$(COMPILE) $(OBJECTS) -o $(OUTPUT_DIR)/firmware.elf
	avr-objcopy -j .text -j .data -O ihex $(OUTPUT_DIR)/firmware.elf $(OUTPUT_DIR)/firmware.hex
	avr-size $(OUTPUT_DIR)/firmware.hex

fuse:
	@[ "$(FUSE_H)" != "" -a "$(FUSE_L)" != "" ] || \
		{ echo "*** Edit Makefile and choose values for FUSE_L and FUSE_H!"; exit 1; }
	$(AVRDUDE) -U hfuse:w:$(FUSE_H):m -U lfuse:w:$(FUSE_L):m -F

flash: firmware
	$(AVRDUDE) -U flash:w:$(OUTPUT_DIR)/firmware.hex:i -F

clang-format:
	clang-format -i src/main.c

clean:
	@rm -rf $(OUTPUT_DIR)

_directories:
	@mkdir -p "$(OBJECT_DIR)"

# Generic rule for compiling C files
$(OBJECT_DIR)/%.o: %.c $(DEPS)
	$(COMPILE) -c $< -o $@

# Generic rule for compiling assembler files
$(OBJECT_DIR)/%.o: %.S $(DEPS)
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
