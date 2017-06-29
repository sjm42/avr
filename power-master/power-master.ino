// power-ctl.ino

#include <stdlib.h>
#include <Wire.h>
#include "LedControl.h"

#define INT_I2C 1000 // milliseconds
#define INT_BTN 200 // milliseconds
#define PCT_STEP 2
#define PCT_MIN 0
#define PCT_MAX 100
#define PIN_BTN 7
#define PIN_LED 13
#define I2C_ADDR 69
#define BUFSZ 80
char buf[BUFSZ];
volatile signed char pct;
signed char pct_last;

static int renc_pina = 2;
static int renc_pinb = 3;

// let's us know when we're expecting a rising edge on renc_pina
// to signal that the encoder has arrived at a detent
volatile byte flag_a = 0;

// let's us know when we're expecting a rising edge on renc_pinb
// to signal that the encoder has arrived at a detent
// (opposite direction to when flag_a is set)
volatile byte flag_b = 0;

// This variable stores our current value of encoder position.
// Change to int or uin16_t instead of byte if you want to record
// a larger range than 0-255
volatile byte enc = 0;

// stores the last encoder position value so we can compare
// to the current reading and see if it has changed
// (so we know when to print to the serial monitor)
volatile byte oldEncPos = 0;

// somewhere to store the direct values we read from our
// interrupt pins before checking to see if we have moved a whole detent
volatile byte reading = 0;

LedControl led(8, 9, 10, 1);


void int_pina() {
  cli();
  reading = PIND & 0x0C;
  if(reading == B00001100 && flag_a) {
      pct -= PCT_STEP;
      if (pct < PCT_MIN) pct = PCT_MIN;
      flag_b = 0;
      flag_a = 0;
  }
  else if (reading == B00000100)
      flag_b = 1;
  sei();
}

void int_pinb() {
  cli();
  reading = PIND & 0x0C;
  if (reading == B00001100 && flag_b) {
      pct += PCT_STEP;
      if (pct > PCT_MAX) pct = PCT_MAX;
      flag_b = 0;
      flag_a = 0;
  }
  else if (reading == B00001000)
      flag_a = 1;
  sei();
}


void led_pct1(unsigned i)
{
    i = i > 999 ? 999 : i;
    led.setDigit(0, 7, (i%1000) / 100, 0);
    led.setDigit(0, 6, (i%100) / 10, 0);
    led.setDigit(0, 5, i % 10, 0);
}


void led_pct2(unsigned i)
{
    i = i > 999 ? 999 : i;
    led.setDigit(0, 2, (i%1000) / 100, 0);
    led.setDigit(0, 1, (i%100) / 10, 0);
    led.setDigit(0, 0, i % 10, 0);
}



void recv_i2c(int n)
{
    char c = Wire.read();
    switch (c) {
    case 'P':
        if (Wire.available() >= 1)
            pct = Wire.read();
        break;
    }
    while (Wire.available() > 0) c = Wire.read();
}

void send_i2c()
{
    Wire.write(pct);
}


void setup()
{
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);
    pinMode(PIN_BTN, INPUT_PULLUP);
    pinMode(renc_pina, INPUT_PULLUP);
    pinMode(renc_pinb, INPUT_PULLUP);
    attachInterrupt(0, int_pina, RISING);
    attachInterrupt(1, int_pinb, RISING);
    Wire.begin();
    Serial.begin(115200);

    /*The MAX72XX is in power-saving mode on startup*/
    led.shutdown(0, false);
    /* Set the brightness to a medium values */
    led.setIntensity(0, 2);
    /* and clear the display */
    led.clearDisplay(0);

    pct = 0;
    pct_last = pct;
    Serial.println("Setup() complete.");
}


void loop()
{
    unsigned int n;
    unsigned long t, t_next_i2c, t_next_btn;
    signed char pct_r;

    t_next_i2c = 0;
    t_next_btn = 0;
    pct_r = 0;
    n = 0;

    led_pct1(pct);
    Serial.println("Starting loop");
    while(1) {
        ++n;
        t = millis();

        if (t > t_next_btn) {
            t_next_btn = t + INT_BTN;
            if (!digitalRead(PIN_BTN)) {
                Serial.println("*** BTN ***");
                Wire.beginTransmission(I2C_ADDR);
                Wire.write('P');
                Wire.write(pct);
                Wire.endTransmission();
            }
        }

        if (t > t_next_i2c) {
            t_next_i2c = t + INT_I2C;
            Serial.println("I2C req");
            Wire.requestFrom(I2C_ADDR, 1);
            delay(20);
            while (Wire.available()) {
                Serial.println("I2C read");
                pct_r = Wire.read();
            }
            led_pct2(pct_r);
        }


        // do other stuff here

        if (pct != pct_last) {
            led_pct1(pct);
            // snprintf(buf, BUFSZ, "pct %02d", pct);
            // Serial.println(buf);
            pct_last = pct;
        }
    }
}

// EOF
