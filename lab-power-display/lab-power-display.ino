// lab-power-display.ino

#include <stdlib.h>
#include "LedController.hpp"
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;

#define RUN_CALIBRATION 0

// #define I2C_ADDR 69

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

// CALIBRATION DATA -- to be determined by actual measurements.

// number to be subtracted from adc A reading with various output voltages
#define A_FACTOR 4348.33
#define A_OFFSET_0 42
#define A_OFFSET_PER_10V 30.0
// voltage ADC count per 1V
// #define V_FACTOR 785.824
#define V_FACTOR 785.0

//delay before incrementing the counter
#define DELAY_CALIBR 2000

//the uninitilized controller object
LedController<SEGMENTS, 1> lc = LedController<SEGMENTS, 1>();

//This function calculates the largest number that can be displayed
unsigned long long getLargestNumber()
{
    return (unsigned long long)pow(10, SEGMENTS * DIGITS);
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

// returns voltage times 100
int16_t calc_volt(int16_t adc)
{
    return (int16_t)(100.0 * ((float)adc / V_FACTOR) + 0.5);
}

// returns amps times 100
int16_t calc_amp(int16_t adc, int16_t volt)
{
    int16_t offset = A_OFFSET_0 + (int16_t)(((float)volt / 1000.0) * A_OFFSET_PER_10V);
    return (int16_t)(100.0 * ((float)(adc - offset) / A_FACTOR) + 0.5);
}

void update_leds(int16_t ch0, int16_t ch1)
{
    int16_t volt = calc_volt(ch1);
    if (volt < 0)
        volt = 0;

    int16_t amp = calc_amp(ch0, volt);
    if (amp < 0)
        amp = 0;

    lc.setDigit(0, 0, amp % 10, false);
    lc.setDigit(0, 1, (amp / 10) % 10, false);
    lc.setDigit(0, 2, (amp / 100) % 10, true);
    lc.setDigit(0, 4, volt % 10, false);
    lc.setDigit(0, 5, (volt / 10) % 10, false);
    lc.setDigit(0, 6, (volt / 100) % 10, true);
    lc.setDigit(0, 7, (volt / 1000) % 10, false);
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
#if RUN_CALIBRATION
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
        delay(DELAY_CALIBR);
    }
#else
    while (1)
    {
        // Measure current
        ads.setGain(GAIN_SIXTEEN);
        int16_t a0 = ads.readADC_Differential_0_1();
        if (a0 < 0)
            a0 = 0;

        // Measure voltage
        ads.setGain(GAIN_ONE);
        int16_t a1 = ads.readADC_Differential_2_3();
        if (a1 < 0)
            a1 = 0;

        // update display
        update_leds(a0, a1);
    }
#endif
}
// EOF
