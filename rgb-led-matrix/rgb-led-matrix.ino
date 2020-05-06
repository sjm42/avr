// led-stripe-demo

#include <FastLED.h>

#define LED_PIN     12
#define NUM_LEDS    64
#define BRIGHTNESS  24
#define LED_TYPE    WS2812

#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 10


CRGBPalette16 currentPalette;
TBlendType    currentBlending;


void setup() {
    delay( 1000 ); // power-up safety delay
    Serial.begin(115200);
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(BRIGHTNESS);
}


void loop()
{
    for( int i = 0; i < NUM_LEDS/4; i++) {
        leds[random8()&(NUM_LEDS-1)].setRGB(random8()&(BRIGHTNESS-1), random8()&(BRIGHTNESS-1), random8()&(BRIGHTNESS-1));
    }
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
}

// EOF
