// lab-power-display.ino

#include <stdlib.h>
#include "LedController.hpp"
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;

#define I2C_ADDR 69

#define BUFSZ 80
char buf[BUFSZ];

#define PIN_LED 13

#define LED_BRIGHTNESS 4
#define CS 10
#define SEGMENTS 1
#define DIGITS 5
#define POS_OFFSET 1

#define CH1_OFFSET 0
#define CH2_OFFSET 5

//delay before incrementing the counter
#define delayTime 2000

//the uninitilized controller object
LedController<SEGMENTS, 1> lc = LedController<SEGMENTS, 1>();

//This function calculates the largest number that can be displayed
unsigned long long getLargestNumber()
{
    return (unsigned long long)pow(10, SEGMENTS * DIGITS);
}

// useless?
void clear_leds()
{
    //disables all Digits by default
    for (unsigned int i = 0; i < SEGMENTS; i++)
    {
        for (unsigned int j = 0; j < 8; j++)
        {
            lc.setRow(i, j, 0x00);
        }
    }
}

void init_leds()
{
    //just make sure that the config is valid
    static_assert(POS_OFFSET + DIGITS < 9, "invalid configuration");

    //initilize a ledcontroller with a hardware spi and one row
    lc.init(CS);
    lc.clearMatrix();
    // clear_leds();
    lc.setIntensity(LED_BRIGHTNESS);
}

void set_leds(unsigned long number)
{
    //the loop is used to split the given number and set the right digit on the Segments
    for (unsigned int i = 0; i < SEGMENTS * DIGITS; i++)
    {
        unsigned long long divisor = 1;
        for (unsigned int j = 0; j < i; j++)
        {
            divisor *= 10;
        }

        byte num = number / divisor % 10;
        lc.setDigit(SEGMENTS - i / DIGITS - 1, i % DIGITS + POS_OFFSET, num, false);
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting...");
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    init_leds();

    ads.setGain(GAIN_ONE);
    ads.setDataRate(RATE_ADS1115_8SPS);
    if (!ads.begin())
    {
        Serial.println("Failed to initialize ADS.");
        while (1)
            ;
    }
    Serial.println("Setup complete.");
}

void loop()
{
    while (1)
    {
        // Measure current
        ads.setGain(GAIN_SIXTEEN);
        int16_t a0 = ads.readADC_Differential_0_1();
        Serial.print("0: ");
        Serial.println(a0);
        lc.setDigit(0, 7, 0, true);
        set_leds(a0);
        delay(delayTime);

        // Measure voltage
        ads.setGain(GAIN_ONE);
        int16_t a1 = ads.readADC_Differential_2_3();
        Serial.print("1: ");
        Serial.println(a1);
        lc.setDigit(0, 7, 1, true);
        set_leds(a1);
        delay(delayTime);
    }
}
// EOF
