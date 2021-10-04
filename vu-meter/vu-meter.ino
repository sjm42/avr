// vu-meter.ino

#include <Arduino.h>

#define PIN_PWM1 9
#define PIN_PWM2 10
#define PIN_PWM3 11

#define SERIAL_SPEED 115200

// we define a dead simple protocol: 0x00 0xFF followed with command byte and value.
// Right now we only define command byte 0x01 (:
#define CMD_START1 0x00
#define CMD_START2 0xFF
#define CMD_SETV1 0x01
#define CMD_SETV2 0x02
#define CMD_SETV3 0x03

void hello()
{
    byte i = 1;
    byte up = 1;
    while (i)
    {
        analogWrite(PIN_PWM1, i);
        analogWrite(PIN_PWM2, i);
        analogWrite(PIN_PWM3, i);
        if (up)
        {
            if (i == 0xff)
                up = 0;
            else
                ++i;
        }
        else
            --i;
        delay(5);
    }
}

void setup()
{
    Serial.begin(SERIAL_SPEED);
    Serial.println("Starting...");

    pinMode(PIN_PWM1, OUTPUT);
    pinMode(PIN_PWM2, OUTPUT);
    pinMode(PIN_PWM3, OUTPUT);
    analogWrite(PIN_PWM1, 0);
    analogWrite(PIN_PWM2, 0);
    analogWrite(PIN_PWM3, 0);
    hello();

    Serial.println("Setup complete.");
}

void loop()
{
    byte cmd, val;
    while (1)
    {
        switch (read_cmd(&val))
        {
        case CMD_SETV1:
            analogWrite(PIN_PWM1, val);
            break;
        case CMD_SETV2:
            analogWrite(PIN_PWM2, val);
            break;
        case CMD_SETV3:
            analogWrite(PIN_PWM3, val);
            break;
        }
    }
}

// we need CMD0 CMD1 <cmd> <value> sequence from serial port
// so implementing a simple state machine here
byte read_cmd(byte *value)
{
    byte c;

    while (1)
    {
        // seek for start sequence
        while (read_byte() != CMD_START1)
            ;

        // next in sequence
        if (read_byte() != CMD_START2)
            continue;

        // read cmd byte
        c = read_byte();
        // read value
        *value = read_byte();
        return c;
    }
}

byte read_byte()
{
    // Brutal: stay in busyloop until we have something
    while (1)
    {
        if (Serial.available())
        {
            byte c = Serial.read();
            return c;
        }
    }
}
// EOF
