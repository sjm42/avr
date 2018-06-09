// clock-test

#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <Wire.h>

#include <math.h>
#include "Time.h"
#include "Timezone.h"
#include "coap_client.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"


// #define FINNISH

#define HW_RESET D7
#define OLED_RESET D6

Adafruit_SSD1306 display(OLED_RESET);


#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


#define LML 80
#define ROWLEN 10
#define WIFI_CONN_WAIT 100
#define NTP_WAIT 1000
#define NTP_AT_TIME (1*3600+42*60) // at 01:42 UTC 03:42 EET 04:42 EEST

#ifdef FINNISH
const char *WDAY[] = { "None", "Su", "Ma", "Ti", "Ke", "To", "Pe", "La" };
const char *MON[] = { "None", "Tam", "Hel", "Maa", "Huh", "Tou", "Kesa",
                      "Hei", "Elo", "Syys", "Loka", "Mar", "Jou"};
#else
const char *WDAY[] = { "None", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
const char *MON[] = { "None", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
#endif


TimeChangeRule EEST = {"EEST", Last, Sun, Mar, 2, 180}; // East European Summer Time
TimeChangeRule EET = {"EET ", Last, Sun, Oct, 3, 120};  // East European Time
Timezone EE(EEST, EET);
TimeChangeRule *tcr;


IPAddress ntp_ip;
//const char *ntp_name = "fi.pool.ntp.org";
const char *ntp_name = "time.google.com";
#define NTP_LPORT 4242      // local port to listen for UDP packets

coapClient coap;
static float out_f = -666;
IPAddress coap_ip;
const char *coap_server = "coap.i.siu.ro";
#define COAP_PORT 5683


// NTP time stamp is in the first 48 bytes of the message
#define NTP_PACKET_SIZE 48
byte ntp_buf[NTP_PACKET_SIZE];
// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
#define SEVENTY_YEARS 2208988800UL

WiFiUDP ntp_udp;

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

static char row0[ROWLEN+1] = {0}, row1[ROWLEN+1] = {0};


void coap_callback(coapPacket &packet, IPAddress ip, int port)
{
    char p[packet.payloadlen + 1];
    memcpy(p, packet.payload, packet.payloadlen);
    p[packet.payloadlen] = NULL;

    // response from coap server
    if(packet.type==3 && packet.code==0) {
        Serial.println("ping ok");
    }
    Serial.print("coap read: "); Serial.println(p);
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
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.display();

    pinMode(HW_RESET, OUTPUT);
    digitalWrite(HW_RESET, HIGH);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.println();
    Serial.println("Hi");
#ifdef FINNISH
    disp_row(0, "Paellit");
    disp_row(1, "OJENNUS!");
#else
    disp_row(0, "Yo");
    disp_row(1, "   DUDE!");
#endif    
    delay(5000);

    Serial.println();
    Serial.println();
    snprintf(buf, LML, "Have %d WiFi networks to try.", NUM_SSID);
    Serial.println(buf);
    disp_row(0, "WiFi conn:");
    disp_row(1, "...");
    delay(2000);

    for (c=0, i=0; i<NUM_SSID && !c; ++i) {
        snprintf(buf, 8, "c %02d", i);
        disp_row(1, buf);
        delay(1000);

        Serial.print("\nTrying \"");
        Serial.print(wifi_ssids[i*2]);
        Serial.println(wifi_ssids[i*2+1] ? "\" with password" : "\" without password");
        WiFi.begin(wifi_ssids[i*2], wifi_ssids[i*2+1]);
        for (r=0; r<WIFI_CONN_WAIT; ++r) {
            delay(100);
            Serial.print(".");
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("OK!");
                disp_row(1, "OK!");
                c = 1;
                break;
            }
        }
    }
    Serial.println();
    delay(1000);

    if(!c) {
        Serial.println("No Wifi connection!");
        delay(2000);
        hw_reset();
    }
    else {
        Serial.print("My IP address: ");
        Serial.println(WiFi.localIP());

        for (i=0; i<4; ++i) {
            if (t = get_ntp(1)) {
                randomSeed(t);
                Serial.println("NTP OK");
                t_begin = t;
                t_pre_ntp = t;
                t_next_ntp = 86400 * (t/86400 + 1) + NTP_AT_TIME;

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
            else {
                Serial.println("NTP FAIL - retrying");
                delay(1000);
            }
        }
        if (t_begin == 0) {
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
    ntp_udp.beginPacket(address, 123); //NTP requests are to port 123
    ntp_udp.write(ntp_buf, NTP_PACKET_SIZE);
    ntp_udp.endPacket();
}


time_t get_ntp(int set_time)
{
    int cb, w, t_d;
    time_t t, t_z;
    char buf[LML];

    disp_row(0,"NTP?");
    disp_row(1, "");
    delay(1000);

    ntp_udp.begin(NTP_LPORT);
    Serial.print("Getting NTP time");
    WiFi.hostByName(ntp_name, ntp_ip);
    sendNTPpacket(ntp_ip);
    for (w=0; w<NTP_WAIT; ++w) {
        delay(10);
        Serial.print(".");
        cb = ntp_udp.parsePacket();
        if (cb) {
            break;
        }
    }

    if (!cb) {
        disp_row(0, "NTP FAIL!");
        disp_row(1, "");
        Serial.println("Timeout.");
        delay(2000);
        return 0;
    }

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    ntp_udp.read(ntp_buf, NTP_PACKET_SIZE);
    u32 highWord = word(ntp_buf[40], ntp_buf[41]);
    u32 lowWord = word(ntp_buf[42], ntp_buf[43]);
    // this is NTP time (seconds since Jan 1 1900):
    u32 secsSince1900 = highWord << 16 | lowWord;

    t = secsSince1900 - SEVENTY_YEARS;
    t_pre_ntp = t_pre_ntp ? now() : t;
    t_d = (int)((signed long)t - (signed long)t_pre_ntp);
    if (set_time) {
        setTime(t);
    }
    t_ntp = t;

    Serial.println("OK!");
    Serial.print("packet received, length=");
    Serial.println(cb);

    snprintf(buf, LML, "UTC %04d-%02d-%02d %02d:%02d:%02d",
             year(t), month(t), day(t), hour(t), minute(t), second(t));
    Serial.println(buf);
    t_z = EE.toLocal(t, &tcr);
    snprintf(buf, LML, "%s %04d-%02d-%02d %02d:%02d:%02d",
             tcr->abbrev, year(t_z), month(t_z), day(t_z),
             hour(t_z), minute(t_z), second(t_z));
    Serial.println(buf);

    if (t_begin > 0) {
        snprintf(buf, LML, "Time drift after %d hours %d min: %d seconds",
                 (t - t_begin) / 3600, ((t-t_begin)%3600/60), t_d);
        Serial.println(buf);
    }

    disp_row(1, "resp");
    delay(1000);
#ifdef FINNISH
    disp_row(0, "SAIN AJAN");
    disp_row(1, "");
#else
    disp_row(0, "GOT TIME");
    disp_row(1, "");
#endif
    delay(2000);

    return t;
}


int disp_row(u8 row, const char *s)
{
    switch (row) {
    case 0:
        strncpy(row0, s, ROWLEN);
        break;
    case 1:
        strncpy(row1, s, ROWLEN);
        break;
    default:
        return -1;
    }

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(row0);
    display.setCursor(0, 18);
    display.print(row1);
    display.display();

    return 0;
}

void disp_time(time_t t, float temp)
{
    char buf[11];
    snprintf(buf, 11, " %02d:%02d:%02d", hour(t), minute(t), second(t));
    disp_row(0, buf);
    snprintf(buf, 11, " %+.2f C ", temp);
    disp_row(1, buf);
}

void disp_date(time_t t)
{
    char buf[11];
    snprintf(buf, 11, "%s %d %s", WDAY[weekday(t)], day(t), MON[month(t)]);
    disp_row(0, buf);
    snprintf(buf, 11, "%d.%d.%d", day(t), month(t), year(t));
    disp_row(1, buf);
}


void loop()
{
    int a, i, x;
    unsigned f_d, f_t;

    unsigned long w;
    time_t t, t_z;

    w=0; f_d=0; f_t=100;
    while (1) {
        ++w;
        t = now();
        t_z = EE.toLocal(t, &tcr);

        if (f_d) {
            // display date

            disp_date(t_z);
            if (--f_d == 0) {
                f_t=100;
            }
        }
        else {
            // display time and out temp

            // have temp yet?
            if (out_f < -660) {
                f_t = 1;
            }
            else {
                disp_time(t_z, out_f);
            }
            if (--f_t == 0) {
                f_d=30;
            }
        }

        if (w % 600 == 0 && out_f > -660.0) {
            // Serial.print("OUT temp: "); Serial.println(out_f);
        }

        if (out_f < -660.0 || w % 1000 == 0) {
            WiFi.hostByName(coap_server, coap_ip);
            // Serial.print("coap ip: "); Serial.println(coap_ip);
            coap.response(coap_callback);
            coap.start();
            int msgid = coap.get(coap_ip, COAP_PORT, "avg_out");
            delay(200);
            bool ret = coap.loop();
            Serial.print("coap_loop() ret="); Serial.println(ret);
        }

        if (t > t_next_ntp) {
            // End the loop when it's time to do NTP lookup
            Serial.println("Time to make another NTP query.");
            break;
        }
        delay(50);
    }
    // We are rude and give a hardware reset.
    hw_reset();
}

// EOF
