// main.c

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "DallasTemperature.h"


#if 0
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "global.h"
#endif


#define HW_RESET D2
#define LINESZ 80
#define delay_us delayMicroseconds
#define ONE_WIRE_BUS1 D3
#define MAX_DEV 64

OneWire ow1(ONE_WIRE_BUS1);
DallasTemperature ds1(&ow1);


void setup()
{
    uint8_t retv;

    Serial.begin(115200);
    
    pinMode(HW_RESET, OUTPUT);
    digitalWrite(HW_RESET, HIGH);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    delay(100);

    Serial.println();
    Serial.println("Hi");
    delay(1000);
    Serial.println();

    pinMode(ONE_WIRE_BUS1, INPUT_PULLUP);
    delay(100);

    retv = ow1.reset();
    Serial.print("ow1 reset: ");
    Serial.println(retv);
    delay(1000);

    ds1.begin();
    delay(1000);

    retv = ds1.getDeviceCount();
    Serial.print("ow1 device count: ");
    Serial.println(retv);
    delay(100);

    if (ds1.getWaitForConversion()) {
        Serial.println("ds1 wait");
    } else {
        Serial.println("ds1 nowait");
    }
    retv = ds1.getResolution();
    Serial.print("ds1 resolution: ");
    Serial.println(retv);
    ds1.setResolution(12);
    delay(100);
    retv = ds1.getResolution();
    Serial.print("ds1 resolution: ");
    Serial.println(retv);

    Serial.println();
    Serial.println();
    delay(1000);
}


void loop()
{
    float temp;
    uint8_t c, i, r;
    DeviceAddress a[MAX_DEV];
    char buf[LINESZ];

    ow1.reset();
    ds1.begin();
    ds1.setWaitForConversion(1);
    ds1.setResolution(12);

    c = ds1.getDeviceCount();
    c = c > MAX_DEV ? MAX_DEV : c;
    for (i=0; i<c; ++i) {
        r = ds1.getAddress(a[i], i);
        snprintf(buf, LINESZ, "Device %d addr %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", i,
                 a[i][0], a[i][1], a[i][2], a[i][3],
                 a[i][4], a[i][5], a[i][6], a[i][7]);
        Serial.println(buf);
    }

    digitalWrite(LED_BUILTIN, LOW);		/* LED ON */
    for (i=0; i<c; ++i) {
        Serial.print("Request temp #");
        Serial.println(i);
        ds1.requestTemperaturesByAddress(a[i]);
    }
    digitalWrite(LED_BUILTIN, HIGH);	/* LED OFF */

    for (i=0; i<c; ++i) {
        temp = ds1.getTempC(a[i]);
        snprintf(buf, LINESZ, "Temp #%d %d.%02u", i, (int)temp, (unsigned)(((int)(temp*100)) % 100));
        Serial.println(buf);
        Serial.println(temp);
    }

    Serial.println();
    delay(1000);
}

// EOF
