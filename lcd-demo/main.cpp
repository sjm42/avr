// Demo of a Keypad LCD Shield
// Common design sold by DFRobot, various vendors on eBay etc

#include <LiquidCrystal.h>
#include <DFR_LCD_Keypad.h>

// this is factory config shield initialisation
LiquidCrystal lcd(8,9,4,5,6,7); 

// below is for shield which has been modified (author's shield)
//LiquidCrystal lcd(8,3,4,5,6,7); 

// initialise the keypad
DFR_LCD_Keypad keypad(A0, &lcd);

int last_key, key;

void setup()
{
  lcd.begin(16,2);
  lcd.clear();
}

void loop()
{
    unsigned long i = 0;
    char b[16];

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
}

// EOF
