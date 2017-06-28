// main.c

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "global.h"

#define CPOSLINE1 PSTR("\xFE\x80")
#define CPOSLINE2 PSTR("\xFE\xC0")
#define CPOSLINE3 PSTR("\xFE\x94")
#define CPOSLINE4 PSTR("\xFE\xD4")
#define CLS PSTR("\xFE\x01")
#define SCSIZE 9
#define TEMP_ERR 0x8000


static uint8_t sc[SCSIZE];

void uart_init(unsigned long baud)
{
    unsigned long ubrr;

    ubrr = (((F_CPU / (baud * 16UL))) - 1);
    // Set baud rate
    UBRR0H = (uint8_t)(ubrr>>8);
    UBRR0L = (uint8_t)ubrr;
    // Set frame format to 8 data bits, no parity, 1 stop bit
    UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);
    //enable transmission and reception
    UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
}

void uart_putc(char c)
{
    //wait while previous byte is completed
    while(!(UCSR0A&(1<<UDRE0))){};
    // Transmit data
    UDR0 = c;
}

char uart_getc()
{
    // Wait for byte to be received
    while(!(UCSR0A&(1<<RXC0))){};
    // Return received data
    return UDR0;
}

void uart_puts(const char *s )
{
    while (*s) {
        uart_putc(*s++);
    }
}

void uart_puts_p(const char *progmem_s)
{
    register char c;

    while ( (c = pgm_read_byte(progmem_s++)) ) {
        uart_putc(c);
    }
}
#define uart_puts_P(__s)       uart_puts_p(PSTR(__s))

#define lcd_puts uart_puts
#define lcd_puts_p uart_puts_p
#define lcd_puts_P uart_puts_P

#define UART_INFO_BUFSZ 40
void uart_info()
{
    char b[UART_INFO_BUFSZ];
    snprintf(b, UART_INFO_BUFSZ, "H:%02X L:%02X B:%02X C:%02X",
             (int)UBRR0H, (int)UBRR0L, (int)UCSR0B, (int)UCSR0C);
    uart_puts(b);
}


void delay_ms(unsigned long ms)
{
    while(ms)
    {
        if (ms>10)
        {
            _delay_ms(10);
            ms -= 10;
        }
        else
        {
            _delay_ms(1);
            ms--;
        }
    }
}

void led_init(void)
{
    DDRB |= 1<<DDB0; /* set DDB0 to output */
    PORTB |= 1<<DDB0; /* LED off */
}

void led(char i)
{
  if(i)
      PORTB &= ~(1<<DDB0); /* LED on */
  else
      PORTB |= 1<<DDB0; /* LED off */
}

void led_blink(unsigned int ms)
{
    led(1);
    delay_ms(ms);
    led(0);
    delay_ms(ms);
}


void lcd_init()
{
    uart_init(9600);
    delay_ms(200);
    // Reset to 9600 and wait
    uart_puts_P("\x12");
    delay_ms(2000);

    // Set speed 38400 and wait
    uart_puts_P("\x7C\x10");
    delay_ms(200);
    uart_init(38400);
    delay_ms(1000);

    // The LCD is 20x4 chars
    uart_puts_P("\x7C\x03\x7C\x05");
    delay_ms(100);

    // Set backlight brightness
    uart_puts_P("\x7C\x9D");
    delay_ms(100);

    // "Visual display on"
    uart_puts_P("\xFE\x0C");
    delay_ms(100);

    // Clear display
    uart_puts_p(CLS);
    delay_ms(100);
}

void reset_onewire(uint8_t bus)
{
    // set D0 to output
    sbi(DDRB, bus);
    sbi(PORTB, bus);
    cbi(PORTB, bus);
    _delay_us(550);
    // set D3 back to input
    cbi(DDRB, bus);
    _delay_us(500);
}

void write_onewire(uint8_t bus, uint8_t bit)
{
    // set D3 to output
    sbi(DDRB, bus);
    sbi(PORTB, bus);
    if (bit == 0) {
        cbi(PORTB, bus);
        _delay_us(65); // hold low for at least 60us
        sbi(PORTB, bus);
    } else { // bit == 1
        cbi(PORTB, bus);
        _delay_us(10); // < 15us for 1 bit
        sbi(PORTB, bus);
    }
    cbi(DDRB, bus); // set D0 to input
    if (bit == 1)
        _delay_us(60); // just to pad out the rest of the slot to >60us
}

uint8_t read_onewire(uint8_t bus)
{
    uint8_t result;
    // set D3 to output
    sbi(DDRB, bus);
    sbi(PORTB, bus);
    // hold D3 low for just greater than 1us.  Note: Only for 6Mhz crystals
    cbi(PORTB, bus);
    _delay_us(1);

    cbi(DDRB, bus); // get ready to read input
    _delay_us(10); // wait for thermo to respond (<15us)
    result = ((PINB) & (1<<bus)); // PB1 = 1st pin
    result = result ? 1 : 0; // always return either 1 or 0
    _delay_us(60); // make sure the slot is >60us long
    return result;
}

void write_onewire_byte(uint8_t bus, uint8_t command)
{
    uint8_t i;
    for (i = 0; i < 8; i++) {
        write_onewire(bus, command & 1);
        command >>= 1;
    }
}

uint8_t read_onewire_byte(uint8_t bus)
{
    uint8_t r,i;
    r = 0;
    for (i = 0; i < 8; i++) {
        r |= (read_onewire(bus) << (i));
    }
    return r;
}

void convert_temp(uint8_t bus)
{
    reset_onewire(bus);
    write_onewire_byte(bus, 0xCC); // skip ROM
    write_onewire_byte(bus, 0x44); // convert temperature
}

int8_t read_sc(uint8_t bus)
{
    uint8_t i;

    for (i = 0; i < SCSIZE; i++) sc[i] = 0;
    reset_onewire(bus);
    write_onewire_byte(bus, 0xCC); // skip ROM
    write_onewire_byte(bus, 0xBE); // read scratchpad (9 bytes)
    delay_ms(20);
    for (i = 0; i < SCSIZE; i++) {
        sc[i] = read_onewire_byte(bus);
    }
    return (sc[0] == 0xFF && sc[1] == 0xFF) ? 1 : 0;
}

int16_t read_temp()
{
    uint8_t sign;
    // return failure if scratchpad is bogus
    if (sc[0] == 0xFF && sc[1] == 0xFF)
	return INT16_MIN;

    if (sc[1] == 0)
        sign = 1;
    else
        sign = -1;

    // return the temp Celcius * 100 from the scratchpad.
    return (int16_t)sign * ((100 * (sc[0] >> 1)) - 25
			    + ((100*(sc[7]-sc[6])) / sc[7]));
}




#define LINESZ 40
int main(void)
{
    char buf[LINESZ];
    uint32_t i;
#ifdef DEBUG
    uint8_t j;
#endif
    int16_t temp1, temp2;

    lcd_init();
    led_init();

    //uart_puts_p(CPOSLINE4);
    //uart_info();

    //uart_puts_p(CPOSLINE1);
    //uart_puts_P("Hello, world!");

#if 0
    i = 0;
    while (1)
    {
        if (i % 10 == 0) led(i%20 == 0 ? 1 : 0);
        uart_puts_p(CPOSLINE2);
        snprintf_P(buf, LINESZ, PSTR("%09lu"), i++);
        uart_puts(buf);
    }
    exit(0);
#endif

    uart_puts_p(CLS);
    uart_puts_p(CPOSLINE1);
    uart_puts_P("Temperature meter");

    wdt_disable();
    i = 0;
    while (1)
    {
        ++i;
        convert_temp(PB1);
        convert_temp(PB2);
        delay_ms(1200);

        read_sc(PB1);
        temp1 = read_temp();
        read_sc(PB2);
        temp2 = read_temp();

#ifdef DEBUG
        for (j = 0; j < SCSIZE; j++)
            printf_P(PSTR("%02X"), sc[j]);
        fputs_P(PSTR("  "),stdout);
#endif

        if (i % 2)
            led(1);
        else
            led(0);

        uart_puts_p(CPOSLINE2);
        snprintf_P(buf, LINESZ, PSTR("measure #%ld"), i);
        uart_puts(buf);

        uart_puts_p(CPOSLINE3);
        if (temp1 == TEMP_ERR)
            uart_puts_P("t1 #err");
        else
        {
            snprintf_P(buf, LINESZ, PSTR("t1 %+d.%02d"),
                       temp1/100, abs(temp1%100));
            uart_puts(buf);
        }

        uart_puts_p(CPOSLINE4);
        if (temp2 == TEMP_ERR)
            uart_puts_P("t2 #err");
        else
        {
            snprintf_P(buf, LINESZ, PSTR("t2 %+d.%02d"),
                       temp2/100, abs(temp2%100));
            uart_puts(buf);
        }
        delay_ms(500);
    }
    return 0;
}

// EOF
