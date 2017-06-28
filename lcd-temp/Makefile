# Makefile

TGT		= main
OBJ		= $(TGT).o

MYAVR_GCC	= atmega328p
MYAVR_ISP	= m328p
MYISP		= avrisp2
OPTIMIZE	= -Os -mcall-prologues
DEFS		= -I. -DF_CPU=18000000UL
#			  -DUART_RX0_BUFFER_SIZE=256 -DUART_TX0_BUFFER_SIZE=256

CC			= avr-gcc
CFLAGS		= -Wall $(OPTIMIZE) -mmcu=$(MYAVR_GCC) $(DEFS)
LDFLAGS		= -Wl,-Map,$(TGT).map

OBJCOPY		= avr-objcopy
AVRPROG		= sudo /usr/local/avr/bin/avrdude

default	: $(TGT).hex

$(TGT).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

pgm: $(TGT).hex
	$(AVRPROG) -P usb -c $(MYISP) -p $(MYAVR_ISP) -y -U flash:w:$(TGT).hex
install : pgm

fuses:
	$(AVRPROG) -v -P usb -c $(MYISP) -p $(MYAVR_ISP) -y \
	  -U lfuse:w:0xFF:m -U hfuse:w:0xDE:m -U efuse:w:0xFD:m

%.obj: %.o
	$(CC) $(CFLAGS) $< -o $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

clean:
	rm -f *.o *.elf *.hex *.obj *.srec *.bin *.map *~

# EOF
