// wifi-clock2.ino

#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <math.h>
#include <coap-simple.h>

#include "LedControl.h"
#include "Time.h"
#include "Timezone.h"
#include "font8x8_latin.h"

// #define FINNISH

#define HW_RESET D2

#define LML 80
#define WIFI_CONN_WAIT 100
#define NTP_WAIT 1000
#define NTP_AT_TIME (1 * 3600 + 42 * 60) // at 01:42 UTC 03:42 EET 04:42 EEST

#if FINNISH
const char *WDAY[] = {"None", " Su ", " Ma ", " Ti ", " Ke ", " To ", " Pe ", " La "};
const char *MON[] = {"None", "Tam ", "Hel ", "Maa ", "Huh ", "Tou ", "Kes\xE4",
                     "Hei ", "Elo ", "Syys", "Loka", "Mar ", "Jou "};
#else
const char *WDAY[] = {"None", " Sun", " Mon", " Tue", " Wed", " Thu", " Fri", " Sat"};
const char *MON[] = {"None", " Jan", " Feb", " Mar", " Apr", " May", " Jun",
                     "Jul ", " Aug", " Sep", " Oct", " Nov", " Dec"};
#endif

// data_pin, clk_pin, cs_pin, num_dev
// LedControl led0(13, 14, 15, 4);
// LedControl led1(5, 12, 16, 4);

LedControl led0(D7, D5, D8, 4);
LedControl led1(D1, D6, D0, 4);

TimeChangeRule EEST = {"EEST", Last, Sun, Mar, 2, 180}; // East European Summer Time
TimeChangeRule EET = {"EET ", Last, Sun, Oct, 3, 120};  // East European Time
Timezone EE(EEST, EET);
TimeChangeRule *tcr;

WiFiUDP my_udp;

IPAddress ntp_ip;
const char *ntp_name = "fi.pool.ntp.org";
// const char *ntp_name = "time.google.com";
#define NTP_LPORT 4242 // local port to listen for UDP packets

Coap coap(my_udp);
float out_f = -666;
IPAddress coap_ip;
const char *coap_server = "coap.siu.ro";
#define COAP_PORT 5683

// NTP time stamp is in the first 48 bytes of the message
#define NTP_PACKET_SIZE 48
byte ntp_buf[NTP_PACKET_SIZE];
// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
#define SEVENTY_YEARS 2208988800UL



/* In wifi-access.h we expect to find something like this:
const char *wifi_ssids[] = {
    // ssid, password (or NULL)
    "internet", NULL,
    "guest", NULL,
    "foo", "bar",
};
#define NUM_SSID (sizeof(wifi_ssids)/(2*sizeof(char*)))
*/
#include "wifi-access.h"

time_t t_begin, t_next_ntp, t_pre_ntp, t_ntp;

void coap_callback(CoapPacket &packet, IPAddress ip, int port)
{
    char p[packet.payloadlen + 1];
    memcpy(p, packet.payload, packet.payloadlen);
    p[packet.payloadlen] = 0;

    // response from coap server
    if (packet.type == 3 && packet.code == 0)
    {
        Serial.println("ping ok");
    }
    Serial.print("coap read: ");
    Serial.println(p);
    out_f = atof(p);
}

void hw_reset()
{
    Serial.println("\n\n*** HARDWARE RESET ***");
    delay(100);
    pinMode(HW_RESET, OUTPUT);
    digitalWrite(HW_RESET, LOW);
    delay(1000000);
}

void setup()
{
    int c, i, r;
    char buf[LML];
    time_t t, t_z;

    t_begin = 0;
    t_next_ntp = 0;
    t_pre_ntp = 0;
    t_ntp = 0;

    Serial.begin(115200);
    pinMode(HW_RESET, OUTPUT);
    digitalWrite(HW_RESET, HIGH);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    pinMode(A0, INPUT);

    for (i = 0; i < 8; ++i)
    {
        led0.shutdown(i, false);
        led0.setScanLimit(i, 8);
        led0.setIntensity(i, 0);
        led0.clearDisplay(i);
        led1.shutdown(i, false);
        led1.setScanLimit(i, 8);
        led1.setIntensity(i, 0);
        led1.clearDisplay(i);
    }
    delay(100);

    Serial.println();
    Serial.println("Hi");
#ifdef FINNISH
    led_row(0, "P\xE4l ");
    led_row(1, " lit");
    delay(1000);
    led_row(0, "OJEN");
    led_row(1, "NUS!");
#else
    led_row(0, " YO ");
    led_row(1, "DUDE");
#endif
    delay(2000);

    Serial.println();
    Serial.println();
    snprintf(buf, LML, "Have %d WiFi networks to try.", NUM_SSID);
    Serial.println(buf);
    led_row(0, "WiFi");
    led_row(1, "init");
    delay(5000);

    for (c = 0, i = 0; i < NUM_SSID && !c; ++i)
    {
        snprintf(buf, 8, "c %02d", i);
        led_row(1, buf);
        delay(1000);

        Serial.print("\nTrying \"");
        Serial.print(wifi_ssids[i * 2]);
        Serial.println(wifi_ssids[i * 2 + 1] ? "\" with password" : "\" without password");
        WiFi.begin(wifi_ssids[i * 2], wifi_ssids[i * 2 + 1]);
        for (r = 0; r < WIFI_CONN_WAIT; ++r)
        {
            delay(100);
            Serial.print(".");
            if (WiFi.status() == WL_CONNECTED)
            {
                Serial.println("OK!");
                led_row(1, " OK!");
                c = 1;
                break;
            }
        }
    }
    Serial.println();
    delay(1000);

    if (!c)
    {
        Serial.println("No Wifi connection!");
        led_row(0, " NO ");
        led_row(1, "WIFI");
        delay(2000);
        hw_reset();
    }
    else
    {
        Serial.print("My IP address: ");
        Serial.println(WiFi.localIP());

        for (i = 0; i < 4; ++i)
        {
            if (t = get_ntp(1))
            {
                randomSeed(t);
                Serial.println("NTP OK");
                t_begin = t;
                t_pre_ntp = t;
                t_next_ntp = 86400 * (t / 86400 + 1) + NTP_AT_TIME;

                t = t_next_ntp;
                Serial.println("Next NTP query will be done at:");
                snprintf(buf, LML, "UTC %04d-%02d-%02d %02d:%02d:%02d",
                         year(t), month(t), day(t), hour(t), minute(t), second(t));
                Serial.println(buf);
                t_z = EE.toLocal(t, &tcr);
                snprintf(buf, LML, "%s %04d-%02d-%02d %02d:%02d:%02d",
                         tcr->abbrev, year(t_z), month(t_z), day(t_z),
                         hour(t_z), minute(t_z), second(t_z));
                Serial.println(buf);
                break;
            }
            else
            {
                Serial.println("NTP FAIL - retrying");
                delay(1000);
            }
        }
        if (t_begin == 0)
        {
            Serial.println("NTP giving up");
            delay(1000);
            hw_reset();
        }
    }

    // Serial.println("\nTurning off WiFi.\n\n");
    // WiFi.mode(WIFI_OFF);
    delay(1000);
}

// send an NTP request to the time server at the given address
int sendNTPpacket(IPAddress &address)
{
    // Serial.println("sending NTP packet...");
    // set all bytes in the buffer to 0
    memset(ntp_buf, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    ntp_buf[0] = 0b11100011; // LI, Version, Mode
    ntp_buf[1] = 0;          // Stratum, or type of clock
    ntp_buf[2] = 6;          // Polling Interval
    ntp_buf[3] = 0xEC;       // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    ntp_buf[12] = 49;
    ntp_buf[13] = 0x4E;
    ntp_buf[14] = 49;
    ntp_buf[15] = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    my_udp.beginPacket(address, 123); //NTP requests are to port 123
    my_udp.write(ntp_buf, NTP_PACKET_SIZE);
    return my_udp.endPacket();
}

time_t get_ntp(int set_time)
{
    int cb, w, t_d;
    time_t t, t_z;
    char buf[LML];

    led_row(0, "NTP?");
    led_row(1, "    ");
    delay(1000);

    my_udp.begin(NTP_LPORT);
    Serial.print("Getting NTP time");
    WiFi.hostByName(ntp_name, ntp_ip);
    sendNTPpacket(ntp_ip);
    for (w = 0; w < NTP_WAIT; ++w)
    {
        delay(10);
        Serial.print(".");
        cb = my_udp.parsePacket();
        if (cb)
        {
            break;
        }
    }

    if (!cb)
    {
        led_row(0, "NTP!");
        led_row(1, "FAIL");
        Serial.println("Timeout.");
        delay(2000);
        return 0;
    }

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    my_udp.read(ntp_buf, NTP_PACKET_SIZE);
    u32 highWord = word(ntp_buf[40], ntp_buf[41]);
    u32 lowWord = word(ntp_buf[42], ntp_buf[43]);
    // this is NTP time (seconds since Jan 1 1900):
    u32 secsSince1900 = highWord << 16 | lowWord;

    t = secsSince1900 - SEVENTY_YEARS;
    t_pre_ntp = t_pre_ntp ? now() : t;
    t_d = (int)((signed long)t - (signed long)t_pre_ntp);
    if (set_time)
    {
        setTime(t);
    }
    t_ntp = t;

    Serial.println("OK!");
    Serial.print("packet received, length=");
    Serial.println(cb);

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
    t_z = EE.toLocal(t, &tcr);
    snprintf(buf, LML, "%s %04d-%02d-%02d %02d:%02d:%02d",
             tcr->abbrev, year(t_z), month(t_z), day(t_z),
             hour(t_z), minute(t_z), second(t_z));
    Serial.println(buf);

    if (t_begin > 0)
    {
        snprintf(buf, LML, "Time drift after %d hours %d min: %d seconds",
                 (t - t_begin) / 3600, ((t - t_begin) % 3600 / 60), t_d);
        Serial.println(buf);
    }

    led_row(0, "resp");
    led_int(1, cb);
    delay(1000);
#ifdef FINNISH
    led_row(0, "SAIN");
    led_row(1, "AJAN");
#else
    led_row(0, "GOT ");
    led_row(1, "TIME");
#endif
    delay(2000);

    return t;
}

// We have a 4x2 matrix
// row: 0..1
// col: 0..3
int led_char(u8 row, u8 col, u16 chr)
{
    const char *chr_d;
    int x;

    chr_d = NULL;
    if (row > 1 || col > 3)
        return 0;

    if (chr >= 0x0000 && chr <= 0x007F)
    {
        chr_d = font8x8_basic[chr];
    }
    if (chr >= 0x0080 && chr <= 0x009F)
    {
        chr_d = font8x8_control[chr - 0x0080];
    }
    if (chr >= 0x00A0 && chr <= 0x00FF)
    {
        chr_d = font8x8_ext_latin[chr - 0x00A0];
    }
    if (!chr_d)
        return 0;

    for (x = 0; x < 8; ++x)
    {
        if (row == 0)
        {
            led0.setRow(col, 7 - x, chr_d[x]);
        }
        else
        {
            led1.setRow(col, 7 - x, chr_d[x]);
        }
    }
    return 1;
}

int led_row(u8 row, const char *s)
{
    int col;

    if (row > 1)
        return 0;
    for (col = 0; col < 4; ++col)
    {
        if (s[col])
            led_char(row, col, s[col]);
        else
            break;
    }
    return col - 1;
}

void led_int(u8 row, int i)
{
    char buf[8];
    i = i < -999 ? -999 : i;
    i = i > 9999 ? 9999 : i;
    snprintf(buf, 8, "%4d", i);
    led_row(row, buf);
}

void led_day(time_t t)
{
    char buf[8];
    led_row(0, WDAY[weekday(t)]);
    snprintf(buf, 8, " %2d. ", day(t));
    led_row(1, buf);
}

void led_month(time_t t)
{
    char buf[8];
    led_row(0, MON[month(t)]);
    snprintf(buf, 8, "%04d", year(t));
    led_row(1, buf);
}

void led_time(time_t t)
{
    char buf[8];
    snprintf(buf, 8, "%02d%02d", hour(t), minute(t));
    led_row(0, buf);
    snprintf(buf, 8, " :%02d", second(t));
    led_row(1, buf);
}

void led_temp(int t)
{
    char buf[8];
    led_row(0, "Out:");
    snprintf(buf, 8, "%+2dC ", t);
    led_row(1, buf);
}

void loop()
{
    int a, i, x, out_temp = -666;
    unsigned f_o, f_m, f_d;

    unsigned long w;
    time_t t, t_z;

    w = 0;
    f_o = 0;
    f_d = 0;
    f_m = 0;
    int trig = 0;
    while (1)
    {
        // one w is 100ms
        ++w;
        t = now();
        t_z = EE.toLocal(t, &tcr);

        int c = second(t_z);
        if (!trig) {
          switch (c) {
            case 11:
            case 41:
              f_o = 15;
              trig = 1;
              break;
            case 15:
            case 45:
              f_d = 15;
              trig = 1;
              break;
            case 19:
            case 49:
              f_m = 15;
              trig = 1;
              break;
            default:
              break;
          }
        }

        if (f_o)
        {
            // display OUT TEMP

            // have temp yet?
            if (out_f < -660)
            {
                f_o = 0;
            }
            else
            {
                out_temp = out_f < 0 ? (int)(out_f - 0.5) : (int)(out_f + 0.5);
                led_temp(out_temp);
                --f_o;
            }
        }
        else if (f_d)
        {
            // display weekday and day of month
            led_day(t_z);
            --f_d;
        }
        else if (f_m)
        {
            // display month and year
            led_month(t_z);
            --f_m;
        }
        else
        {
            // display time with seconds
            led_time(t_z);
            trig = 0;
        }

        if (w % 50 == 0)
        {
            // read LDR value, set display intensity
            a = analogRead(A0);
            //Serial.print("Analog: ");
            //Serial.println(a);

            i = (a - 200) / 100;
            i = i < 0 ? 0 : i;
            i = i > 15 ? 15 : i;
            //Serial.print("Led intensity: ");
            //Serial.println(i);
            for (x = 0; x < 4; ++x)
            {
                led0.setIntensity(x, i);
                led1.setIntensity(x, i);
            }

            // Turn off the on-board blue led
            digitalWrite(LED_BUILTIN, HIGH);
        }
        else
        {
            // Blink the blue led only when it is not too dark :D
            // That would disturb sleeping...
            if (1)
            {
                // Turn on the on-board blue led
                digitalWrite(LED_BUILTIN, LOW);
            }
        }

        if (w % 300 == 0 && out_f > -660)
        {
            Serial.print("OUT temp: ");
            Serial.println(out_f);
        }

        if (out_temp < -660 || w % 1000 == 0)
        {
            WiFi.hostByName(coap_server, coap_ip);
            // Serial.print("coap ip: "); Serial.println(coap_ip);
            coap.response(coap_callback);
            coap.start();
            int msgid = coap.get(coap_ip, COAP_PORT, "avg_out");
            delay(200);
            bool ret = coap.loop();
            Serial.print("coap_loop() ret=");
            Serial.println(ret);
            // coap_callback() should have saved the temp into global out_f
            if (out_temp < -660 && out_f > -660)
                out_temp = out_f < 0 ? (int)(out_f - 0.5) : (int)(out_f + 0.5);
        }

        if (t > t_next_ntp)
        {
            // End the loop when it's time to do NTP lookup
            Serial.println("Time to make another NTP query.");
            break;
        }
        delay(100);
    }
    // We are rude and give a hardware reset.
    hw_reset();
}

// EOF
