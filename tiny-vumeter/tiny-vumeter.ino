// tiny-vumeter.ino

#include <Arduino.h>

#define PIN_PWM 4

// we define a dead simple protocol: 0x00 0xFF followed with command byte and value.
// Right now we only define command byte 0x01 (:
#define CMD_START1 0x00
#define CMD_START2 0xFF
#define CMD_SETV 0x01

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting...");

    pinMode(PIN_PWM, OUTPUT);
    analogWrite(PIN_PWM, 0);

    Serial.println("Setup complete.");
}

byte read_ser()
{
    // block until we have something
    while (1)
    {
        if (Serial.available())
        {
            byte c = Serial.read();
            return c;
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
        // seek for cmd0
        while (read_ser() != CMD_START1)
            ;
        // seek for cmd1
        if (read_ser() != CMD_START2)
            continue;

        // read cmd byte
        c = read_ser();
        // read value
        *value = read_ser();
        return c;
    }
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
// EOF
