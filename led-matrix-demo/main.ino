// Demo of a Keypad LCD Shield
// Common design sold by DFRobot, various vendors on eBay etc

#include <stdlib.h>
#include <LiquidCrystal.h>
#include <DFR_LCD_Keypad.h>
#include <LedControl.h>


LiquidCrystal lcd(8,9,4,5,6,7); 
DFR_LCD_Keypad keypad(A0, &lcd);

#define N_LMODS 8
LedControl led(11, 12, 13, N_LMODS);

int last_key, key;

void setup()
{
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);

    pinMode(10, INPUT);
    lcd.begin(16,2);
    lcd.clear();
    lcd.setCursor(0, 0);

    for (int i=0; i < N_LMODS; ++i)
    {
        /*The MAX72XX is in power-saving mode on startup*/
        led.shutdown(i, false);
        /* Set the brightness to a medium values */
        led.setIntensity(i, 1);
        /* and clear the display */
        led.clearDisplay(i);
    }
    delay(1000);
}


void loop()
{
    unsigned long n = 0;
    long rnd;
    int d, r, c;
    char b[16];
    char buf[4];
    char flag = 0;

    while (n<1024) {
        flag = flag ? 0 : 1;
        //delay(500);
        for (r=0; r<8; ++r) {
            for (d=0; d<N_LMODS; ++d) {
                // delay(50);
                for (c=0; c<8; ++c) {
                    // delay(50);
                    led.setLed(d, r, c, flag);
                    delay(1);

                    snprintf(b, 16, "%lu", ++n);
                    lcd.setCursor(0, 0);
                    lcd.print(b);

                    last_key = keypad.get_last_key();
                    key      = keypad.read_key();
                    // only clear and update the LCD if they key state has changed
                    if (key != last_key) {
                        // key has changed
    
                        lcd.setCursor(0, 1);
                        // print the key selection to the LCD
                        switch (key) {
                        case KEY_RIGHT:
                            lcd.print("RIGHT ");
                            break;
                        case KEY_UP:
                            lcd.print("UP    ");
                            break;
                        case KEY_DOWN:
                            lcd.print("DOWN  ");
                            break;
                        case KEY_LEFT:
                            lcd.print("LEFT  ");
                            break;
                        case KEY_SELECT:
                            lcd.print("SELECT");
                            break;
                        case KEY_NONE:
                        default:
                            lcd.print("NONE  ");
                            break;
                        }
                    }
                }
            }
        }
    }

    for (int i=0; i < N_LMODS; ++i)
        led.clearDisplay(i);

    n = 0;
    while (n<3000) {
        ++n;

        rnd = random();
        d = rnd & 0b000000000111;
        r = (rnd & 0b000001110000) >> 4;
        c = (rnd & 0b011100000000) >> 8;
        led.setLed(d, r, c, 1);
    }

    n = 0;
    while (n<3000) {
        ++n;

        rnd = random();
        d = rnd & 0b000000000111;
        r = (rnd & 0b000001110000) >> 4;
        c = (rnd & 0b011100000000) >> 8;
        led.setLed(d, r, c, 0);
    }
}

// EOF
