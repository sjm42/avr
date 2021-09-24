// tiny-vumeter.ino

#include <Arduino.h>
#include <stdlib.h>

#define PIN_LED 13
#define PIN_PWM 1

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting...");
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);
    pinMode(PIN_PWM, OUTPUT);
    analogWrite(PIN_PWM, 0);
    Serial.println("Setup complete.");
}

void loop()
{
    int i = 0;
    int up = 1;
    while (1)
    {
        analogWrite(PIN_PWM, i);
        if (up)
        {
            // counting up
            if (i == 255)
            {
                up = 0;
            }
            else
            {
                ++i;
            }
        }
        else
        {
            // counting down
            if (i == 0)
            {
                up = 1;
            }
            else
            {
                --i;
            }
        }
        delay(5);
    }
}
// EOF
