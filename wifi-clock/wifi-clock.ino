// clock-test

#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <math.h>

#include "LedControl.h"
#include "Time.h"


LedControl led(5, 0, 4, 1);

#define LML 80
#define WIFI_CONN_WAIT 80
#define NTP_WAIT 1000
//#define NTP_INTERVAL 3600
#define NTP_INTERVAL 600

unsigned const int ntp_localport = 4242;      // local port to listen for UDP packets

IPAddress timeServerIP;
//const char *ntpServerName = "fi.pool.ntp.org";
const char *ntpServerName = "ritsa.i.siu.ro";


// NTP time stamp is in the first 48 bytes of the message
#define NTP_PACKET_SIZE 48
byte ntp_buf[NTP_PACKET_SIZE];
// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
#define SEVENTY_YEARS 2208988800UL
WiFiUDP udp;


#include "wifi-access.h"

time_t t_begin, t_last_ntp, t_pre_ntp, t_ntp;


void setup()
{
    int c, i, r;
    char buf[LML];
    time_t t;

    t_begin = 0;
    t_last_ntp = 0;
    t_pre_ntp = 0;
    t_ntp = 0;

    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    pinMode(A0, INPUT);

    led.shutdown(0, false);
    led.setScanLimit(0, 4);
    led.setIntensity(0, 8);
    led.clearDisplay(0);
    led.setChar(0, 1, 'h', 0);
    led.setChar(0, 2, 'i', 1);
    delay(1000);

    Serial.println();
    Serial.println();
    snprintf(buf, LML, "Have %d WiFi networks to try.", NUM_SSID);
    Serial.println(buf);
    led.clearDisplay(0);
    led.setChar(0, 0, 'c', 1);
    led.setChar(0, 1, '_', 1);
    for (c=0, i=0; i<NUM_SSID && !c; ++i) {
        // Display progress in 7-segment too
        led.setDigit(0, 2, i/10, 0);
        led.setDigit(0, 3, i%10, 0);

        Serial.print("\nTrying \"");
        Serial.print(wifi_ssids[i*2]);
        Serial.println(wifi_ssids[i*2+1] ? "\" with password" : "\" without password");
        WiFi.begin(wifi_ssids[i*2], wifi_ssids[i*2+1]);
        for (r=0; r<WIFI_CONN_WAIT; ++r) {
            delay(100);
            Serial.print(".");
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("OK.");
                c = 1;
                break;
            }
        }
    }
    Serial.println();

    led.clearDisplay(0);
    led.setChar(0, 0, '_', 0);
    led.setChar(0, 1, 'C', 0);
    led.setChar(0, 3, '_', 1);
    if(c) {
        led.setDigit(0, 2, 1, 0);
        Serial.print("My IP address: ");
        Serial.println(WiFi.localIP());

        while (1) {
            if (t = get_ntp(1)) {
                Serial.println("NTP OK");
                t_begin = t;
                t_last_ntp = t;
                t_pre_ntp = t;
                break;
            }
            else {
                Serial.println("NTP FAIL - retrying");
            }
        }
    }
    else {
        led.setDigit(0, 2, 0, 0);
        Serial.println("No Wifi connection!");
    }
    Serial.println();
    delay(1000);
    led.setIntensity(0, 2);
}

void led_int_bp(unsigned i)
{
    // create bit pattern to the decimal points
    u8 lowbits;
    lowbits = i & 0b00001111;
    i = i > 9999 ? 9999 : i;

    led.setDigit(0, 0, i / 1000, (lowbits>>3) & 0b0001);
    led.setDigit(0, 1, (i%1000) / 100, (lowbits>>2) & 0b0001);
    led.setDigit(0, 2, (i%100) / 10, (lowbits>>1) & 0b0001);
    led.setDigit(0, 3, i % 10, lowbits & 0b0001);

}

void led_int(unsigned i)
{
    i = i > 9999 ? 9999 : i;

    led.setDigit(0, 0, i / 1000, 0);
    led.setDigit(0, 1, (i%1000) / 100, 0);
    led.setDigit(0, 2, (i%100) / 10, 0);
    led.setDigit(0, 3, i % 10, 0);
}

void led_time(char hour, char min, char dp)
{
    led.setDigit(0, 0, hour / 10, 0);
    led.setDigit(0, 1, hour % 10, dp);
    led.setDigit(0, 2, min / 10, 0);
    led.setDigit(0, 3, min % 10, 0);
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
    // Serial.println("sending NTP packet...");
    // set all bytes in the buffer to 0
    memset(ntp_buf, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    ntp_buf[0] = 0b11100011;   // LI, Version, Mode
    ntp_buf[1] = 0;     // Stratum, or type of clock
    ntp_buf[2] = 6;     // Polling Interval
    ntp_buf[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    ntp_buf[12]  = 49;
    ntp_buf[13]  = 0x4E;
    ntp_buf[14]  = 49;
    ntp_buf[15]  = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    udp.beginPacket(address, 123); //NTP requests are to port 123
    udp.write(ntp_buf, NTP_PACKET_SIZE);
    udp.endPacket();
}

time_t get_ntp(int set_time)
{
    int cb, w, t_d;
    time_t t, t_z;
    char buf[LML];

    led.clearDisplay(0);
    led.setChar(0, 0, 'u', 1);
    led.setDigit(0, 1, 1, 0);
    led.setDigit(0, 2, 2, 0);
    led.setDigit(0, 3, 3, 0);
    delay(1000);

    udp.begin(ntp_localport);
    Serial.print("Getting NTP time");
    WiFi.hostByName(ntpServerName, timeServerIP);
    sendNTPpacket(timeServerIP);
    for (w=0; w<NTP_WAIT; ++w) {
        delay(10);
        Serial.print(".");
        cb = udp.parsePacket();
        if (cb) {
            break;
        }
    }

    if (!cb) {
        led.clearDisplay(0);
        led.setChar(0, 0, '_', 0);
        led.setChar(0, 1, 'U', 0);
        led.setDigit(0, 2, 0, 0);
        led.setChar(0, 3, '_', 0);
        Serial.println("Timeout.");
        delay(2000);
        return 0;
    }

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    udp.read(ntp_buf, NTP_PACKET_SIZE);
    u32 highWord = word(ntp_buf[40], ntp_buf[41]);
    u32 lowWord = word(ntp_buf[42], ntp_buf[43]);
    // this is NTP time (seconds since Jan 1 1900):
    u32 secsSince1900 = highWord << 16 | lowWord;

    t = secsSince1900 - SEVENTY_YEARS;
    t_pre_ntp = t_pre_ntp ? now() : t;
    t_d = (int)((signed long)t - (signed long)t_pre_ntp);
    if (set_time) {
        setTime(t);
        Serial.println("Time SET");
    }
    t_ntp = t;
    t_last_ntp = t;

    Serial.println("OK!");
    Serial.print("packet received, length=");
    Serial.println(cb);
    led.clearDisplay(0);
    led.setChar(0, 0, 'U', 1);
    led.setDigit(0, 1, cb/100, 0);
    led.setDigit(0, 2, (cb%100)/10, 0);
    led.setDigit(0, 3, cb%10, 1);

#if 0
    // dump NTP reply packet
    for (w=0; w<cb; ) {
        snprintf(buf, LML, "%02X", ntp_buf[w]);
        Serial.print(buf);
        ++w;

        if (w % 16 == 0) {
            Serial.println();
        }
        else {
            Serial.print(w % 8 == 0 ? "  " : " ");
        }
    }
    Serial.println();
#endif

    snprintf(buf, LML, "UTC %04d-%02d-%02d %02d:%02d:%02d",
             year(t), month(t), day(t), hour(t), minute(t), second(t));
    Serial.println(buf);
    t_z = t+2*3600;
    snprintf(buf, LML, "EET %04d-%02d-%02d %02d:%02d:%02d",
             year(t_z), month(t_z), day(t_z), hour(t_z), minute(t_z), second(t_z));
    Serial.println(buf);
    t_z += 3600;
    snprintf(buf, LML, "EEST %04d-%02d-%02d %02d:%02d:%02d",
             year(t_z), month(t_z), day(t_z), hour(t_z), minute(t_z), second(t_z));
    Serial.println(buf);

    if (t_begin > 0) {
        snprintf(buf, LML, "Time drift after %d hours %d min: %d seconds",
                 (t - t_begin) / 3600, ((t-t_begin)%3600/60), t_d);
        Serial.println(buf);
    }
    return t;
}


void loop()
{
    int r;
    unsigned w;
    time_t t, t_z;

    while (1) {
        t = now();
        t_z = t + 3600*3;
        led_time(hour(t_z), minute(t_z), t_z%2==0 ? 1 : 0);

        unsigned a = analogRead(A0);
        Serial.print("Analog: ");
        Serial.println(a);

        if (t%2==0) {
            digitalWrite(LED_BUILTIN, HIGH);
        }
        else {
            // digitalWrite(LED_BUILTIN, LOW);
        }

        if (t > t_last_ntp + NTP_INTERVAL) {
            // End the loop when it's time to do NTP lookup
            break;
        }
        delay(1000);
    }

    if (WiFi.status() == WL_CONNECTED) {
        while (1) {
            if (t = get_ntp(0)) {
                Serial.println("NTP OK");
                break;
            }
            else {
                Serial.println("NTP FAIL - retrying");
            }
        }
    }
    else {
        Serial.println("No WiFi :(");
    }
    Serial.println();
}


// EOF
