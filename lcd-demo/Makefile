# Makefile

TGT		= main
OBJ		= $(TGT).o

# where you installed the Arduino app
ARDUINO_DIR = $(HOME)/arduino
LCD_DIR		= $(ARDUINO_DIR)/libraries/LiquidCrystal/src
LCDKP_DIR = ./DFR_LCD_Keypad
F_CPU		= 18000000
MCU		= atmega328p
PGM_PTYPE	= arduino
PGM_PORT	= /dev/ttyUSB1
PGM_SPEED	= 115200
MYISP		= avrisp2
OPTIMIZE	= -Os -Wall -mcall-prologues -ffunction-sections -fdata-sections
DEFS		= -DF_CPU=$(F_CPU)UL -MMD -DUSB_VID=null -DUSB_PID=null \
		  -I. -I$(LCDKP_DIR) -DARDUINO=106

LCD_INC		= -I$(LCD_DIR)
ARDUINO_INC	= -I$(ARDUINO_DIR)/hardware/arduino/avr/cores/arduino \
		  -I$(ARDUINO_DIR)/hardware/arduino/avr/variants/standard \
		  $(LCD_INC)
ARDUINO_LIB	= $(ARDUINO_DIR)/hardware/arduino/avr/cores/arduino

CC		= avr-gcc
CPP		= avr-gcc
CFLAGS		= $(OPTIMIZE) -mmcu=$(MCU) $(DEFS)
CPPFLAGS	= $(CFLAGS) -fno-exceptions
LDFLAGS		= -Os -Wl,--gc-sections,-Map,$(TGT).map -mmcu=$(MCU)
LIBS		= lcd_kp.a arduino.a -lm
AR		= avr-ar
OBJCOPY		= avr-objcopy
AVRPROG		= sudo /usr/local/avr/bin/avrdude


default	: $(TGT).hex $(TGT).elf
	$(OBJCOPY) -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load \
	  --no-change-warnings --change-section-lma .eeprom=0 $(TGT).elf $(TGT).eep
	$(OBJCOPY) -O ihex -R .eeprom $(TGT).elf $(TGT).hex

$(TGT).elf:
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(TGT).cpp -o $(TGT).o
	$(CC) -c $(CFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/hooks.c -o hooks.c.o
	$(CC) -c $(CFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/WInterrupts.c -o WInterrupts.c.o
	$(CC) -c $(CFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/wiring.c -o wiring.c.o
	$(CC) -c $(CFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/wiring_analog.c -o wiring_analog.c.o
	$(CC) -c $(CFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/wiring_digital.c -o wiring_digital.c.o
	$(CC) -c $(CFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/wiring_pulse.c -o wiring_pulse.c.o
	$(CC) -c $(CFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/wiring_shift.c -o wiring_shift.c.o
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/abi.cpp -o abi.cpp.o
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/CDC.cpp -o CDC.cpp.o
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/HardwareSerial.cpp -o HardwareSerial.cpp.o
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/IPAddress.cpp -o IPAddress.cpp.o
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/main.cpp -o main.cpp.o
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/new.cpp -o new.cpp.o
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/PluggableUSB.cpp -o PluggableUSB.cpp.o
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/Print.cpp -o Print.cpp.o
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/Stream.cpp -o Stream.cpp.o
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/Tone.cpp -o Tone.cpp.o
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/USBCore.cpp -o USBCore.cpp.o
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/WMath.cpp -o WMath.cpp.o
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/WString.cpp -o WString.cpp.o
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(LCD_DIR)/LiquidCrystal.cpp -o LiquidCrystal.cpp.o
	rm -f arduino.a
	$(AR) rcs arduino.a hooks.c.o WInterrupts.c.o wiring.c.o wiring_analog.c.o wiring_digital.c.o \
	  wiring_pulse.c.o wiring_shift.c.o \
	  abi.cpp.o CDC.cpp.o HardwareSerial.cpp.o IPAddress.cpp.o \
	  main.cpp.o new.cpp.o PluggableUSB.cpp.o \
	  Print.cpp.o Stream.cpp.o Tone.cpp.o USBCore.cpp.o WMath.cpp.o  WString.cpp.o \
	  LiquidCrystal.cpp.o
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) -fpermissive $(LCDKP_DIR)/DFR_LCD_Keypad.cpp -o DFR_LCD_Keypad.o
	rm -f lcd_kp.a
	$(AR) rcs lcd_kp.a DFR_LCD_Keypad.o
	$(CC) $(LDFLAGS) -o $(TGT).elf $(OBJ) $(LIBS)

arduino: $(TGT).hex
	$(AVRPROG) -v -P $(PGM_PORT) -b $(PGM_SPEED) -c $(PGM_PTYPE) -p $(MCU) -y -U flash:w:$(TGT).hex

install: arduino
pgm: arduino

fuses:
	$(AVRPROG) -v -P usb -c $(MYISP) -p $(MCU) -y \
	  -U lfuse:w:0xFF:m -U hfuse:w:0xD9:m -U efuse:w:0xFD:m

%.obj: %.o
	$(CC) $(CFLAGS) $< -o $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

clean:
	rm -f *.o *.a *.d *.eep *.elf *.hex *.obj *.srec *.bin *.map *~

# EOF
