# Makefile

TGT		= main
OBJ		= $(TGT).o

# where you installed the Arduino app
ARDUINO_DIR = $(HOME)/arduino
TM1638_DIR = ./tm1638-library
F_CPU		= 18000000
MCU		= atmega328p
MYISP		= avrisp2
OPTIMIZE	= -Os -Wall -mcall-prologues -ffunction-sections -fdata-sections
DEFS		= -DF_CPU=$(F_CPU)UL -MMD -DUSB_VID=null -DUSB_PID=null \
		  -I. -I$(TM1638_DIR) -DARDUINO=106
#		  -DUART_RX0_BUFFER_SIZE=256 -DUART_TX0_BUFFER_SIZE=256

ARDUINO_INC	= "-I$(ARDUINO_DIR)/hardware/arduino/avr/cores/arduino" \
		  "-I$(ARDUINO_DIR)/hardware/arduino/avr/variants/standard"
ARDUINO_LIB	= "$(ARDUINO_DIR)/hardware/arduino/avr/cores/arduino/"

CC		= avr-gcc
CPP		= avr-gcc
CFLAGS		= $(OPTIMIZE) -mmcu=$(MCU) $(DEFS)
CPPFLAGS	= $(CFLAGS) -fno-exceptions
LDFLAGS		= -Os -Wl,--gc-sections,-Map,$(TGT).map -mmcu=$(MCU)
LIBS		= tm1638.a core.a -lm
AR		= avr-ar
OBJCOPY		= avr-objcopy
AVRPROG		= sudo /usr/local/avr/bin/avrdude


default	: $(TGT).hex $(TGT).elf
	$(OBJCOPY) -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load \
	  --no-change-warnings --change-section-lma .eeprom=0 $(TGT).elf $(TGT).eep
	$(OBJCOPY) -O ihex -R .eeprom $(TGT).elf $(TGT).hex 

$(TGT).elf:
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(TGT).cpp -o $(TGT).o
#	$(CC) -c $(CFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/avr-libc/malloc.c -o malloc.c.o 
#	$(CC) -c $(CFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/avr-libc/realloc.c -o realloc.c.o 
#	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/HID.cpp -o HID.cpp.o 
	$(CC) -c $(CFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/WInterrupts.c -o WInterrupts.c.o 
	$(CC) -c $(CFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/wiring.c -o wiring.c.o 
	$(CC) -c $(CFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/wiring_analog.c -o wiring_analog.c.o 
	$(CC) -c $(CFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/wiring_digital.c -o wiring_digital.c.o 
	$(CC) -c $(CFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/wiring_pulse.c -o wiring_pulse.c.o 
	$(CC) -c $(CFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/wiring_shift.c -o wiring_shift.c.o 
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/CDC.cpp -o CDC.cpp.o 
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/HardwareSerial.cpp -o HardwareSerial.cpp.o 
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/IPAddress.cpp -o IPAddress.cpp.o 
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/main.cpp -o main.cpp.o 
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/new.cpp -o new.cpp.o 
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/Print.cpp -o Print.cpp.o 
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/Stream.cpp -o Stream.cpp.o 
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/Tone.cpp -o Tone.cpp.o 
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/USBCore.cpp -o USBCore.cpp.o 
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/WMath.cpp -o WMath.cpp.o 
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(ARDUINO_LIB)/WString.cpp -o WString.cpp.o
	rm -f core.a
#	$(AR) rcs core.a malloc.c.o realloc.c.o
#	$(AR) rcs core.a HID.cpp.o
	$(AR) rcs core.a WInterrupts.c.o wiring.c.o wiring_analog.c.o wiring_digital.c.o \
	  wiring_pulse.c.o wiring_shift.c.o CDC.cpp.o HardwareSerial.cpp.o IPAddress.cpp.o \
	  main.cpp.o new.cpp.o \
	  Print.cpp.o Stream.cpp.o Tone.cpp.o USBCore.cpp.o WMath.cpp.o  WString.cpp.o
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(TM1638_DIR)/InvertedTM1638.cpp
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(TM1638_DIR)/TM1638.cpp
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(TM1638_DIR)/TM1638QYF.cpp
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(TM1638_DIR)/TM1640.cpp
	$(CPP) -c $(CPPFLAGS) $(ARDUINO_INC) $(TM1638_DIR)/TM16XX.cpp
	rm -f tm1638.a
	$(AR) rcs tm1638.a InvertedTM1638.o TM1638.o TM1638QYF.o TM1640.o TM16XX.o
	$(CC) $(LDFLAGS) -o $(TGT).elf $(OBJ) $(LIBS)

avrisp2: $(TGT).hex
	$(AVRPROG) -v -P usb -c $(MYISP) -p $(MCU) -y -U flash:w:$(TGT).hex

arduino: $(TGT).hex
	$(AVRPROG) -v -P /dev/ttyUSB0 -c avrisp -b 57600 -p $(MCU) -y -U flash:w:$(TGT).hex
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
