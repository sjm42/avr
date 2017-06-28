// Demo of a Keypad LCD Shield
// Common design sold by DFRobot, various vendors on eBay etc

#include <LiquidCrystal.h>
#include <DFR_LCD_Keypad.h>
#include <OneWire.h>
#include <DallasTemperature.h>



LiquidCrystal lcd(8,9,4,5,6,7); 

DFR_LCD_Keypad keypad(A0, &lcd);

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress t_addr;


int last_key, key;

void setup()
{
    pinMode(13, OUTPUT);

    pinMode(10, INPUT);
    lcd.begin(16,2);
    lcd.clear();

    sensors.begin();
    delay(1000);
}


void loop()
{
    unsigned long i = 0;
    char b[16];
    char buf[4];

    digitalWrite(13, HIGH);

    lcd.clear();
    lcd.setCursor(0, 0);
    if (!sensors.getAddress(t_addr, 0))
        lcd.print("Unknown!");
    else
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            snprintf(buf, 4, "%02X", (unsigned char)t_addr[i]);
            lcd.print(buf);
        }
    }

    lcd.setCursor(0, 1);
    if (sensors.isParasitePowerMode())
        lcd.print("P1 ");
    else
        lcd.print("P0 ");

    sensors.requestTemperatures();
    delay(2000);
    digitalWrite(13, LOW);

    lcd.print("#0: ");
    lcd.print(sensors.getTempCByIndex(0));

    delay(2000);

#if 0
    while (1) {
        snprintf(b, 16, "%lu", ++i);
        lcd.setCursor(0, 0);
        lcd.print(b);

        last_key = keypad.get_last_key();
        key      = keypad.read_key();

        // only clear and update the LCD if they key state has changed
        if (key != last_key) {
            // key has changed
    
            lcd.setCursor(0, 1);
            // print the key selection to the LCD
            switch (key) {
            case KEY_RIGHT:
                lcd.print("RIGHT ");
                break;

            case KEY_UP:
                lcd.print("UP    ");
                break;

            case KEY_DOWN:
                lcd.print("DOWN  ");
                break;

            case KEY_LEFT:
                lcd.print("LEFT  ");
                break;

            case KEY_SELECT:
                lcd.print("SELECT");
                break;

            case KEY_NONE:
            default:
                lcd.print("NONE  ");
                break;
            }
        }
    }
#endif
}

// EOF
