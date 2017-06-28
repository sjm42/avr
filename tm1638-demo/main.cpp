
#include <TM16XX.h>
#include <InvertedTM1638.h>
#include <TM1638QYF.h>
#include <TM16XXFonts.h>
#include <TM1638.h>

/*
  LED/key module TM1638 demo/example
 */


TM1638 module(5, 6, 7);

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
  
}

// the loop function runs over and over again forever
void loop() {
  byte a, b, b2, d, r;
  signed long i;
  int m;
  signed long n;

  // Variables:
  // a -- toggle main board LED blink
  // b -- button input byte
  // b2 -- previous button input byte
  // d -- button press delay counter, to avoid "autofire effect"
  // i -- counter increment
  // m -- counter for board LED
  // n -- counter whose value is put on the LED display
  // r -- counter running? Used for stop/start

  a=0; b=0; b2=0, d=0; i=1; m=0; n=0; r=1;
  while(1)
  {
    // Arrange for board LED blink
    if (!(m%32)) {
      if (a)
      {
        digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
        a=0;
      }
      else
      {
        digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
        a=1;
      }
    }
    ++m;

    b = module.getButtons();
    if (b && d==0)
    {
      // Button S1: stop/start
      if (b & 0b00000001)
      {
        r = r ? 0 : 1;
      }

      // Button S2: reset
      if (b & 0b00000010)
      {
        n = 0;
      }

      // Button S3: Change counter direction
      if (b & 0b00000100)
      {
        i = -i;
      }

      // Button S4: slow down counter
      if (b & 0b00001000)
      {
        // do not stop the counter here
        if (i < -7 || i > 7)
        {
           i = i>0 ? i-7 : i+7;
        }
      }

      // Button S5: Speed up counter
      if (b & 0b00010000)
      {
        // max counter increment/decrement is 300
        if (i < 300 || i > -300)
        {
          i = i>0 ? i+7 : i-7;
        }
      }

      // Button S6: Reset counter speed to 1
      if (b & 0b00100000)
      {
          i = i>0 ? 1 : -1;
      }
    }

    // Limit button repeat rate
    if (b == b2)
    {
      ++d;
    }
    else
    {
      d = 0;
    }
    b2 = b;

    module.setDisplayToDecNumber(n, 0, true);
    module.setLEDs((n>>4)%256);

    // No counter operations done if r==0 - we are stopped.
    if (r)
    {
      n += i;
      if (n > 99999999) n=0;
      if (n < 0) n=99999999;
    }
  }
}

// EOF

