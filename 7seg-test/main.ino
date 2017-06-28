// Demo of a Keypad LCD Shield
// Common design sold by DFRobot, various vendors on eBay etc

#include <stdlib.h>
#include <LedControl.h>


LedControl led(1, 3, 2, 1);

#define MYCHARS "01234567890AbcdEfHLP.-_"


void setup()
{
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);

    pinMode(10, INPUT);
    lcd.begin(16,2);
    lcd.clear();
    lcd.setCursor(0, 0);

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
    char b[16];
    char *mycset = PSTR(MYCHARS);
    unsigned n = 0;

    while (1) {
        snprintf(b, 16, "%u", n);
        lcd.setCursor(0, 0);
        lcd.print(b);

#if 0
        c = pgm_read_byte(mycset+i);
        snprintf(b, 16, "%c", c);
        lcd.setCursor(0, 1);
        lcd.print(b);

        led.setChar(0, 0, c, 1);
        led.setChar(0, 1, c, 1);
        led.setChar(0, 2, c, 1);
        led.setChar(0, 3, c, 1);

        i = i > sizeof(MYCHARS)-3 ? 0 : i+1;
#endif
        led_int(n);
        n = n >= 9999 ? 0 : n+1;
        delay(50);
    }
}

// EOF
