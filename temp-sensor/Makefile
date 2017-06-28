# Makefile

# Global options
ARDUINO_DIR		= $(HOME)/arduino
ARDMK_DIR		= $(HOME)/git/Arduino-Makefile
AVR_TOOLS_DIR	= /usr/local/avr
AVRDUDE			= /usr/local/avr/bin/avrdude
AVRDUDE_CONF	= /usr/local/avr/etc/avrdude.conf

# Options for this project
BOARD_TAG		= uno

#MCU			= atmega2560
#F_CPU			= 16000000L

ARDUINO_PORT	= /dev/ttyUSB0
ARDUINO_LIBS	= LiquidCrystal

# Get rid of -flto
CFLAGS_STD		= -std=gnu11
CXXFLAGS_STD	= -std=gnu++11

include $(ARDMK_DIR)/Arduino.mk

CFLAGS		+= -I.
CXXFLAGS	+= -I.

# No -flto with linker, either
LDFLAGS		:= $(filter-out -flto,$(LDFLAGS))


flags:
	echo CFLAGS $(CFLAGS)
	echo CXXFLAGS $(CXXFLAGS)
	echo LDFLAGS $(LDFLAGS)

# EOF
