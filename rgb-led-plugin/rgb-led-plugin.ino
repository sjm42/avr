// rgb-led-plugin

#include <FastLED.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>


#define ECHO 0
#define LED_PIN     12
#define NUM_LEDS    30
#define BRIGHTNESS  64
#define LED_TYPE    WS2812

#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 10
#define FADE_SPEED 5
#define LED_RED 255,0,0
#define LED_BLUE 0,0,255

#define NUM_SEG 5
enum MODE {MODE_OFF, MODE_BON, MODE_BFLASH, MODE_BFADE, MODE_UNUSED1,
           MODE_RON, MODE_RFLASH, MODE_RFADE};
enum LEDCOLOR {C_NONE, C_BLUE, C_RED};

uint8_t seg_mode[NUM_SEG];
uint8_t seg_flash[NUM_SEG];

#define CBUF_SZ 16
char cbuf[CBUF_SZ];
// Will be set NULL when command is ready i.e. newline is found.
char *cptr; 

void setup() {
    uint8_t i;
    delay(1000); // power-up safety delay
    memset(cbuf, 0, CBUF_SZ);
    cptr = cbuf;
    Serial.begin(115200);
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    for (i=0; i<NUM_SEG; ++i)
    {
        seg_mode[i] = 0;
        seg_flash[i] = 0;
    }
}


void seg_update()
{
    uint8_t i, j, j1, j2, f;
    for (i=0; i<NUM_SEG; ++i)
    {
        j1 = i*(NUM_LEDS/NUM_SEG);
        j2 = (i+1)*(NUM_LEDS/NUM_SEG);
        f = C_NONE;
        switch (seg_mode[i])
        {
            case MODE_OFF:
            for (j=j1; j<j2; ++j)
            {
                leds[j].setRGB(0,0,0);
            }
            break;

            case MODE_BON:
            for (j=j1; j<j2; ++j)
            {
                leds[j].setRGB(LED_BLUE);
            }
            break;

            case MODE_BFLASH:
            f = C_BLUE;
            // pass thru
            case MODE_RFLASH:
            if (!f) f = C_RED;
            if (seg_flash[i])
            {
                seg_flash[i] = 0;
                for (j=j1; j<j2; ++j)
                {
                    if (f == C_BLUE) leds[j].setRGB(LED_BLUE);
                    if (f == C_RED) leds[j].setRGB(LED_RED);
                }
            }
            else
            {
                seg_flash[i] = 1;
                for (j=j1; j<j2; ++j)
                {
                    leds[j].setRGB(0,0,0);
                }
            }
            break;
            
            case MODE_BFADE:
            for (j=j1; j<j2; ++j)
            {
                if (leds[j].blue >= FADE_SPEED)
                {
                    leds[j].blue -= FADE_SPEED;
                }
                else
                {
                    leds[j].blue = 0;
                    seg_mode[i] = MODE_OFF;
                }
            }
            break;
            
            case MODE_RON:
            for (j=j1; j<j2; ++j)
            {
                leds[j].setRGB(LED_RED);
            }
            break;

            case MODE_RFADE:
            for (j=j1; j<j2; ++j)
            {
                if (leds[j].red >= FADE_SPEED)
                {
                    leds[j].red -= FADE_SPEED;
                }
                else
                {
                        leds[j].red = 0;
                        seg_mode[i] = MODE_OFF;
                }
            }
            break;
        }
    }
}


void serial_update()
{
    int c;
    while (Serial.available() > 0)
    {
        c = Serial.read();
        if (c>0)
        {
#if ECHO
            Serial.print((char)c);
#endif
            if (c == '\n' || c == '\r')
            {
                // newline seen, command is ready for parsing
#if ECHO
                Serial.println("");
#endif
                cptr = NULL;
            }
            // do not overflow cbuf!
            if (cptr && CBUF_SZ-1 > cptr-cbuf)
            {
                *cptr++ = (char)c;
            }
        }
        
    }
}


void serial_cmd()
{
    char *tc, *vc;
    int t, v;

    tc = cbuf;
    vc = strchr(tc, '/');
    if (vc)
    {
        *vc++ = '\0';
        t = atoi(tc);
        v = atoi(vc);
        if (t >= 0 && t < NUM_SEG && v >= 0 && v <= MODE_RFADE)
        {
            seg_mode[t] = (uint8_t)v;
        }
    }

    // clear command buffer
    memset(cbuf, 0, CBUF_SZ);
    cptr = cbuf;
}


void loop()
{
    uint16_t i;
    i = 0;
    while (1)
    {
        ++i;

        serial_update();
        if (!cptr) serial_cmd();

        if (i%10 == 0)
        {
            seg_update();
            FastLED.show();
        }
        FastLED.delay(5);
    }
}

// EOF
