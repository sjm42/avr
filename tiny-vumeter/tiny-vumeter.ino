// tiny-vumeter.ino

#include <Arduino.h>

#define PIN_PWM 4
#define SERIAL_SPEED 9600

// we define a dead simple protocol: 0x00 0xFF followed with command byte and value.
// Right now we only define command byte 0x01 (:
#define CMD_START1 0x00
#define CMD_START2 0xFF
#define CMD_SETV 0x01

void hello()
{
    byte i = 1;
    byte up = 1;
    while (i)
    {
        analogWrite(PIN_PWM, i);
        if (up)
        {
            if (i == 0xff)
                up = 0;
            else
                ++i;
        }
        else
            --i;
        delay(10);
    }
    analogWrite(PIN_PWM, 0);
}

void setup()
{
    Serial.begin(SERIAL_SPEED);
    Serial.println("Starting...");

    pinMode(PIN_PWM, OUTPUT);
    analogWrite(PIN_PWM, 0);
    hello();

    Serial.println("Setup complete.");
}

void loop()
{
    byte cmd, val;
    while (1)
    {
        cmd = read_cmd(&val);
        if (cmd == CMD_SETV)
            analogWrite(PIN_PWM, val);
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
