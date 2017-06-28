// power-ctl.ino

#include <stdlib.h>
#include "LedControl.h"


#define INTERVAL 1000  // milliseconds
#define MAX_W 3600

LedControl led(4, 5, 6, 1);
#define PIN_LED 13
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


void int_pina() {
  cli();
  reading = PIND & 0x0C;
  if(reading == B00001100 && flag_a) {
      if (pct > 0) --pct;
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
      if (pct < 100) ++pct;
      flag_b = 0;
      flag_a = 0;
  }
  else if (reading == B00001000)
      flag_a = 1;
  sei();
}


void led_pwr(unsigned i)
{
    i = i > 9999 ? 9999 : i;
    led.setDigit(0, 3, i / 1000, 0);
    led.setDigit(0, 2, (i%1000) / 100, 0);
    led.setDigit(0, 1, (i%100) / 10, 0);
    led.setDigit(0, 0, i % 10, 0);
}

void led_pct(unsigned i)
{
    i = i > 999 ? 999 : i;
    led.setDigit(0, 7, (i%1000) / 100, 0);
    led.setDigit(0, 6, (i%100) / 10, 0);
    led.setDigit(0, 5, i % 10, 0);
}


void setup()
{
    pinMode(PIN_LED, OUTPUT);
    Serial.begin(115200);

    pinMode(renc_pina, INPUT_PULLUP);
    pinMode(renc_pinb, INPUT_PULLUP);
    attachInterrupt(0, int_pina, RISING);
    attachInterrupt(1, int_pinb, RISING);

    /*The MAX72XX is in power-saving mode on startup*/
    led.shutdown(0, false);
    /* Set the brightness to a medium values */
    led.setIntensity(0, 2);
    /* and clear the display */
    led.clearDisplay(0);

    digitalWrite(PIN_LED, HIGH);
    delay(3000);
    digitalWrite(PIN_LED, LOW);
    pct = 0;
    pct_last = pct;
    Serial.println("Setup() complete.");
}


void loop()
{
    unsigned long t, tt, t_on, t_off, t_next;
    signed char updown = 1;

    delay(INTERVAL - (millis()%INTERVAL));
    t_next = 0;

    Serial.println("Starting loop");
    led_pct(pct);
    led_pwr((MAX_W * pct) / 100);

    while(1) {
        t = millis();
        tt = t - (t%INTERVAL);
        if (t > t_next) {
            t_next = tt + INTERVAL;
            t_on = tt;
            t_off = tt + pct * (INTERVAL/100);

#if 0
            snprintf(buf, BUFSZ, "t %lu, tt %lu, t_next %lu", t, tt, t_next);
            Serial.println(buf);
            snprintf(buf, BUFSZ, "PCT %02d t_on %lu, t_off %lu",
                     pct, t_on, t_off);
            Serial.println(buf);
#endif

#if 0
            if (updown > 0) {
                if (++pct == 100) {
                    updown = -1;
                }
            }
            else {
                if (--pct == 0) {
                    updown = 1;
                }
            }
#endif
            continue;
        }

        if (pct > 0 && t >= t_on && t < t_off) {
            // led ON
            digitalWrite(PIN_LED, HIGH);
        }
        else if (t >= t_off) {
            // led OFF
            digitalWrite(PIN_LED, LOW);
        }

        // do other stuff here
        if (pct != pct_last) {
            led_pct(pct);
            led_pwr((MAX_W * pct) / 100);
            //snprintf(buf, BUFSZ, "pct %02d", pct);
            //Serial.println(buf);
            pct_last = pct;
        }
    }
}

// EOF
