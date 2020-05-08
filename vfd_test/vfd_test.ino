// vfd_test.ino

#include <stdlib.h>
#include "et16315.h"

#define DEBUG 1
#define BLEN 40
static char pbuf[BLEN];

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

void vfd_test()
{
    char c, buf[8];
    int a, i, j, k;

#if 1
    for (i=0x30; i<0x80; i += 8) {
        for (j=0; j<8; ++j) {
            buf[j] = i+j;
        }
        et16315_set_text(buf, 8);
        delay(200);

        for (j=0; j<2; ++j) {
            et16315_set_colon(1,1);
            et16315_set_colon(2,1);
            et16315_set_colon(3,1);
            delay(100);

            et16315_set_colon(1,0);
            et16315_set_colon(2,0);
            et16315_set_colon(3,0);
            delay(100);
        }
    }
#endif

    memset(buf, 'X', 8);
    et16315_set_text(buf, 8);
    delay(1000);

#if 0
    for (a = 24; a<36; ++a) {
        snprintf(pbuf, BLEN, "Addr: %02d", a);
        Serial.println(pbuf);
        et16315_set_text(pbuf, 8);
        delay(1000);

        for (k=0; k<8; ++k) {
            c = 1 << k;
            snprintf(pbuf, BLEN, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(c));
            Serial.println(pbuf);
            et16315_set_text(pbuf, 8);

            et16315_xfer(ET16315_CMD2_SET_MODE(0, 1, ET16315_CMD_WRITE_DATA), NULL, 0);
            et16315_xfer(ET16315_CMD3_SET_ADDR(a), &c, 1);
            delay(1000);
        }
        c = 0;
        et16315_xfer(ET16315_CMD2_SET_MODE(0, 1, ET16315_CMD_WRITE_DATA), NULL, 0);
        et16315_xfer(ET16315_CMD3_SET_ADDR(a), &c, 1);

        for (k=7; k>=0; --k) {
            c = 1 << k;
            snprintf(pbuf, BLEN, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(c));
            Serial.println(pbuf);
            et16315_set_text(pbuf, 8);

            et16315_xfer(ET16315_CMD2_SET_MODE(0, 1, ET16315_CMD_WRITE_DATA), NULL, 0);
            et16315_xfer(ET16315_CMD3_SET_ADDR(a), &c, 1);
            delay(1000);
        }
        c = 0;
        et16315_xfer(ET16315_CMD2_SET_MODE(0, 1, ET16315_CMD_WRITE_DATA), NULL, 0);
        et16315_xfer(ET16315_CMD3_SET_ADDR(a), &c, 1);
    }
#endif

#if 1
    memset(buf, '8', 8);
    et16315_set_text(buf, 8);

    for (i = et16315_sym_DOLBY; i <= et16315_sym_DVD; ++i) {
        et16315_set_symbol(i, 1);
        delay(500);
        et16315_set_symbol(i, 0);
    }
    delay(1000);
    for (i = et16315_sym_DVD; i >= et16315_sym_DOLBY; --i) {
        et16315_set_symbol(i, 1);
        delay(500);
        et16315_set_symbol(i, 0);
    }
#endif
}


void setup()
{
    Serial.begin(115200);
    Serial.println("\n\nHi");
    et16315_start();
    delay(1000);
    Serial.println();
    randomSeed(analogRead(0));
}

void loop()
{
    vfd_test();
    delay(10000);
}

// EOF
