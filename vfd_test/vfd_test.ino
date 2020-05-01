// vfd_test.ino

#include <stdlib.h>
#include <SPI.h>

#define DEBUG 1
#define BLEN 40
#define SS_PIN 10
static char buf[BLEN];

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

void vfd_write()
{
    byte i, c;

    for (i=0; i<0x24; ++i) {
	vfd_put(i, 0);
    }
    for (i=0; i<0x24; ++i) {
	// c = random(0x100);
	c = 0xff;
	Serial.println("\nPUT");
	vfd_put(i, 0);
	delay(1000);
	vfd_put(i, c);
	delay(1000);
	vfd_put(i, 0);
	delay(1000);
	vfd_put(i, c);
	delay(1000);
	//vfd_put(i, 0);
    }

}

void setup()
{
    Serial.begin(115200);
    pinMode(SS_PIN, OUTPUT);
    SPI.begin();
    Serial.println("\n\nHi");
    vfd_mode();
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
