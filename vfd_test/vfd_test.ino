// vfd_test.ino

#include <stdlib.h>
#include "et16315.h"

#define DEBUG 1
#define BLEN 40
static char buf[BLEN];


#if 0

void vfd_cmd(byte b)
{
#if DEBUG
    snprintf(buf, BLEN, "%02X\n", b);
    Serial.print(buf);
#endif
    digitalWrite(SS_PIN, LOW);
    SPI.beginTransaction(SPISettings(1000000, LSBFIRST, SPI_MODE3));
    SPI.transfer(b);
    SPI.endTransaction();
    digitalWrite(SS_PIN, HIGH);    
}

void vfd_put(byte a, byte b)
{
#if DEBUG
    snprintf(buf, BLEN, "%02X:%02X\n", a, b);
    Serial.print(buf);
#endif
    digitalWrite(SS_PIN, LOW);
    SPI.beginTransaction(SPISettings(1000000, LSBFIRST, SPI_MODE3));
    SPI.transfer(0b11000000 | (a & 0b00111111));
    SPI.transfer(b);
    SPI.endTransaction();
    digitalWrite(SS_PIN, HIGH);    
}

void vfd_mode()
{
    vfd_cmd(0b00000101);
    vfd_cmd(0b10001111);
    vfd_cmd(0b01000100);
}
#endif


void vfd_write()
{
    byte i;
    char buf[9];

    for (i=0; i<8; ++i) {
        buf[i] = 0x20 + random(0x60);
    }
    buf[8] = '\0';
    et16315_set_text(buf);
}


void setup()
{
    Serial.begin(115200);
    Serial.println("\n\nHi");
    et16315_start();
    Serial.println();
    randomSeed(analogRead(0));
    delay(10);
}

void loop()
{
    vfd_write();
    delay(1000000);
}

// EOF
