// tiny-temp.ino

#include <stdlib.h>
#include "LedControl.h"



LedControl led(2, 3, 4);
#define PIN_LED 5


void setup()
{
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, HIGH);

#if 1
    /*The MAX72XX is in power-saving mode on startup*/
    led.shutdown(0, false);
    /* Set the brightness to a medium values */
    led.setIntensity(0, 2);
    /* and clear the display */
    led.clearDisplay(0);
#endif
    
    delay(1000);
}


void loop()
{
    unsigned long n;
    long rnd;

    n = 0;
    while (n<3000) {
        ++n;
        rnd = random();
#if 1
        led.setDigit(0, 0, rnd % 10, 0);
        if (n % 2 == 0)
            led.setDigit(0, 1, rnd / 10 % 10, 0);
        if (n % 3 == 0)
            led.setDigit(0, 2, rnd / 100 % 10, 0);
        if (n % 4 == 0)
            led.setDigit(0, 3, rnd / 1000 % 10, 0);
        rnd = random();
        if (n % 5 == 0)
            led.setDigit(0, 4, rnd % 10, 0);
        if (n % 6 == 0)
            led.setDigit(0, 5, rnd / 10 % 10, 0);
        if (n % 7 == 0)
            led.setDigit(0, 6, rnd / 100 % 10, 0);
        if (n % 8 == 0)
            led.setDigit(0, 7, rnd / 1000 % 10, 0);
#endif

        if (n % 2 == 0)
        {
            digitalWrite(PIN_LED, LOW);
        }
        else {
            digitalWrite(PIN_LED, HIGH);
        }

        delay(100);
    }
}

// EOF
