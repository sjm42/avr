// clock-test

#include <stdlib.h>
#include "LedControl.h"


LedControl led(5, 0, 4, 1);

#define MYCHARS "01234567890AbcdEfHLP.-_"


void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    /*The MAX72XX is in power-saving mode on startup*/
    led.shutdown(0, false);
    led.setScanLimit(0, 4);
    /* Set the brightness to a medium values */
    led.setIntensity(0, 15);
    /* and clear the display */
    led.clearDisplay(0);
    delay(1000);
}

void led_int(unsigned i)
{
    u8 lowbits;
    lowbits = i & 0b00001111;
    i = i > 9999 ? 9999 : i;
    led.setDigit(0, 0, i / 1000, (lowbits>>3) & 0b0001);
    led.setDigit(0, 1, (i%1000) / 100, (lowbits>>2) & 0b0001);
    led.setDigit(0, 2, (i%100) / 10, (lowbits>>1) & 0b0001);
    led.setDigit(0, 3, i % 10, lowbits & 0b0001);
}

void loop()
{
    u8 i = 0, c;
    u8 flag = 0;
    char b[16];
    char *mycset = MYCHARS;
    unsigned n = 0;

    while (1) {
        snprintf(b, 16, "%u", n);
        
#if 0
        c = pgm_read_byte(mycset+i);
        snprintf(b, 16, "%c", c);

        led.setChar(0, 0, c, 1);
        led.setChar(0, 1, c, 1);
        led.setChar(0, 2, c, 1);
        led.setChar(0, 3, c, 1);

        i = i > sizeof(MYCHARS)-3 ? 0 : i+1;
#endif
        led_int(n);
        n = n >= 9999 ? 0 : n+1;
        if ((n % 10 ) == 0) {
          if (flag) {
            digitalWrite(LED_BUILTIN, HIGH);
            flag = 0;
          }
          else {
            digitalWrite(LED_BUILTIN, LOW);
            flag = 1;
          }
        }
        delay(200);
    }
}

// EOF
