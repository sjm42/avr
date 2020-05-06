//  I made this code out of multiple projects, so its a mess
//  I used it on a ESP32 with a WS2612B 5m LED-strip with 300LEDs (60LEDs/m)
//  The Data cable of the LED-strip needs to be wired to pin16
//  If you use the same LED-strip make sure you can deliver atleast 2Amps or more to it

#include <Arduino.h>
#include <FastLED.h>
#include "BS_LightToSerial.h"

#define SER_SPEED 9600
#define DATA_PIN_L 13
#define DATA_PIN_R 15

#define NUM_LEDS_L 30
#define NUM_LEDS_R 120

#define COLOR_ORDER RGB

CRGB leds_L[NUM_LEDS_L];
CRGB leds_R[NUM_LEDS_R];

CRGB ColorA = CRGB(255,0,0);
CRGB ColorB = CRGB(0,0,255);

int SpeedLeft;
int SpeedRight;
BS_LightToSerial bslts;

int BPM = 4;

float LeftT;
float RightT;

CRGB Colors[5];
CRGB Colo[5];
bool Chroma;
int Event[5];
float CurVal[5];

String incoming = "";   // for incoming serial string data
String inData;


void setup()
{
    int a;

    LEDS.addLeds<NEOPIXEL, DATA_PIN_L>(leds_L, NUM_LEDS_L);
    LEDS.addLeds<NEOPIXEL, DATA_PIN_R>(leds_R, NUM_LEDS_R);
    LEDS.clear();
    Serial.begin(SER_SPEED);

    for (a=0; a < NUM_LEDS_L; ++a) {
        leds_L[a] = CRGB(128, 0, 0);
        FastLED.show();
        delay(1200/NUM_LEDS_L);
    }
    for (a=0; a < NUM_LEDS_R; ++a) {
        leds_R[a] = CRGB(0, 0, 128);
        FastLED.show();
        delay(1200/NUM_LEDS_R);
    }
    delay(1000);
    for (a=0; a < NUM_LEDS_L; ++a) {
        leds_L[a] = CRGB(0, 0, 0);
        FastLED.show();
        delay(1200/NUM_LEDS_L);
    }
    for (a=0; a < NUM_LEDS_R; ++a) {
        leds_R[a] = CRGB(0, 0, 0);
        FastLED.show();
        delay(1200/NUM_LEDS_R);
    }
    LEDS.clear();
}


void oof()
{
    // Only do something if there's new data
    if (Serial.available()) {
        byte buf[4]; // Array for storing incoming data
        int index = 0; // How many bytes have been received

        // Wait until 4 bytes have been received
        while (index < 4) {
            // If there's a byte available, read and store it
            if (Serial.available()) {
                buf[index++] = Serial.read();
            }
        }

        // The ParseMessage function returns a BS_LightEvent containing the type of event and it's value 
        BS_LightEvent test = bslts.ParseMessage(buf);

        BPM = bslts.bpm;
        ColorA = CRGB(bslts.left.r,  bslts.left.g,  bslts.left.b);
        ColorB = CRGB(bslts.right.r, bslts.right.g, bslts.right.b);

        Colo[0] = CRGB(bslts.light0.r, bslts.light0.g, bslts.light0.b);
        Colo[1] = CRGB(bslts.light1.r, bslts.light1.g, bslts.light1.b);
        Colo[2] = CRGB(bslts.light2.r, bslts.light2.g, bslts.light2.b);
        Colo[3] = CRGB(bslts.light3.r, bslts.light3.g, bslts.light3.b);
        Colo[4] = CRGB(bslts.light4.r, bslts.light4.g, bslts.light4.b);

        Chroma = bslts.chroma;

        int Type  = test.type;
        int Value = test.value;
        if (Type == 12) {
            SpeedLeft = Value;  
            LeftT = random(0, 100);
        }
        if (Type == 13) {
            SpeedRight = Value;  
            RightT = random(0, 100);
        }
        if (Type <= 4) {
            CurVal[Type] = 0;
            Event[Type] = Value;
            if (Value == 5 || Value == 1) {
                CurVal[Type] = 1;
            }
            if (Value == 0) {
                CurVal[Type] = 0;
            }
            if (Value == 6 || Value == 2) {
                CurVal[Type] = 3;
            }
            if (Value == 7 || Value == 3) {
                CurVal[Type] = 1;
            }
        }
    }
}


void loop()
{
    oof();
  
    for (int a=0; a < 5; a++) {
        int v = Event[a];

        if(v == 6 || v == 2) {
            CurVal[a] = CurVal[a] + (1 - CurVal[a]) / 5;
        }

        if(v == 7 || v == 3) {
            CurVal[a] = CurVal[a] - CurVal[a] / 10;
        }

        Colors[a] = CRGB(0,0,0);

        if(Chroma) {
            Colors[a] = CRGB(CurVal[a] * Colo[a].r/4,CurVal[a] * Colo[a].g/4,CurVal[a] * Colo[a].b/4);  
        }
        else {
            if(v == 5 || v == 6 || v == 7) {
                Colors[a] = CRGB(CurVal[a] * ColorA.r/4,CurVal[a] * ColorA.g/4,CurVal[a] * ColorA.b/4);
            }
            if(v == 1 || v == 2 || v == 3) {
                Colors[a] = CRGB(CurVal[a] * ColorB.r/4,CurVal[a] * ColorB.g/4,CurVal[a] * ColorB.b/4);
            }
        }
    }

    LeftT  = SpeedLeft  * BPM * millis() / 76433.0;
    RightT = SpeedRight * BPM * millis() / 76433.0;

    int LSinA = 30 + (1.0 + sin(LeftT + TWO_PI/3)) * 45;
    int LSinB = 30 + (1.0 + sin(LeftT + 2*TWO_PI/3)) * 45;   
    int LSinC = 30 + (1.0 + sin(LeftT)) * 45;  

    int RSinA = 30 + (1.0 + sin(RightT + TWO_PI/3)) * 45;  
    int RSinB = 30 + (1.0 + sin(RightT + 2*TWO_PI/3)) * 45;  
    int RSinC = 30 + (1.0 + sin(RightT)) * 45;  

    CRGB Col;
    int a, Dist;

    for (a=0; a < NUM_LEDS_L; ++a) {
        Dist = min(abs(a-LSinA), min(abs(a-LSinB), abs(a-LSinC)));
        if(Dist < 5) {
            Col = Colors[2]; 
        }
        else {
            Col = CRGB(0,0,0);
        }

        if (a < NUM_LEDS_L / 5 && CurVal[4] != 0) {
            Col += Colors[4];
        }
        if (a > NUM_LEDS_L / 2 && a < 2 * NUM_LEDS_L / 3 && CurVal[1] != 0) {
            Col = Col + Colors[1];
        }
        if (a > 4 * NUM_LEDS_L / 5 && CurVal[0] != 0) {
            Col += Colors[0];
        }
        leds_L[a] = Col;
    }

    for (a=0; a < NUM_LEDS_R; ++a) {
        Dist = min(abs(a-RSinA), min(abs(a-RSinB), abs(a-RSinC)));
        if(Dist < 5) {
            Col = Colors[3]; 
        }
        else {
            Col = CRGB(0,0,0);
        }

        if (a < NUM_LEDS_R / 5 && CurVal[4] != 0) {
            Col += Colors[4];
        }
        if (a > NUM_LEDS_R / 2 && a < 2 * NUM_LEDS_R / 3 && CurVal[1] != 0) {
            Col = Col + Colors[1];
        }
        if (a > 4 * NUM_LEDS_R / 5 && CurVal[0] != 0) {
            Col += Colors[0];
        }
        leds_R[a] = Col;
    }
    FastLED.show();
}

// EOF
