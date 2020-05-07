
/******************************************************************************/


#include "et16315.h"

#define DISP_SIZE 8

/****************************************************************************************
 *
 * Character table for 8 digit 15-segment VFD display
 *
 ****************************************************************************************
 *
 * In the Fortis FOREVER_3434HD/FOREVER_9898HD/GPV8000/EP8000/EPP8000 front panel the ET16315 driver IC
 * is wired as follows to the display (TODO/assumption: GPV/EP8000 are the same):
 *
 *     aaaaaaa
 *    fh  j  kb
 *    f h j k b  q
 *    f  hjk  b
 * Q   gggimmm
 *    e  rpn  c
 *    e r p n c  q
 *    er  p  nc
 *     ddddddd
 *
 * SG1 -> segment a (0x01, value0) 
 * SG2 -> segment k (0x02, value0)
 * SG3 -> segment j (0x04, value0)
 * SG4 -> segment h (0x08, value0)
 * SG5 -> segment b (0x10, value0)
 * SG6 -> segment f (0x20, value0)
 * SG7 -> segment m (0x40, value0)
 * SG8 -> segment i (0x80, value0)
 *
 * SG9 -> segment g (0x01, value1) 
 * SG10-> segment c (0x02, value1)
 * SG11-> segment e (0x04, value1)
 * SG12-> segment n (0x08, value1)
 * SG13-> segment p (0x10, value1)
 * SG14-> segment r (0x20, value1)
 * SG15-> segment d (0x40, value1)
 * SG16-> segment Q/q (0x80, value1) red dot on digit 1, colon on digits 2, 4 & 6
 *
 * SG17-SG24 not connected -> all value2 are zero
 *
 * GR1 is connected to the rightmost digit 8, GR8 to the leftmost digit 1
 * (reverse of what you would expect)
 *
 * The characters below must be in ASCII order, starting at space (0x20).
 */
static struct et16315_char et16315_fp_chars[] =
{
  /*    value0      value1      value2                   */
 /*  imfbhjka    qdrpnecg             <-display segment */
 /*  76543210    76543210    76543210                   */
 { 0b00000000, 0b00000000, 0b00000000 },  // 020, space 
 { 0b10000100, 0b00010000, 0b00000000 },  // 021, '!'
 { 0b00110000, 0b00000000, 0b00000000 },  // 022, double quote
 { 0b11000000, 0b01000111, 0b00000000 },  // 023, '#'
 { 0b11100101, 0b01010011, 0b00000000 },  // 024, '$'
 { 0b10100010, 0b00100010, 0b00000000 },  // 025, '%'
 { 0b10101000, 0b01001101, 0b00000000 },  // 026, '&'
 { 0b00000010, 0b00000000, 0b00000000 },  // 027, quote
 { 0b10000010, 0b00001000, 0b00000000 },  // 028, '('
 { 0b10001000, 0b00100000, 0b00000000 },  // 029, ')'
 { 0b11001010, 0b00101001, 0b00000000 },  // 02a, '*'
 { 0b11000100, 0b00010001, 0b00000000 },  // 02b, '+'
 { 0b10000000, 0b00100000, 0b00000000 },  // 02c, ','
 { 0b11000000, 0b00000001, 0b00000000 },  // 02d, '-'
 { 0b00000000, 0b00100000, 0b00000000 },  // 02e, '.'
 { 0b10000010, 0b00100000, 0b00000000 },  // 02f, '/'
 { 0b10110001, 0b01000110, 0b00000000 },  // 030, '0'
 { 0b00010010, 0b00000010, 0b00000000 },  // 031, '1'
 { 0b11010001, 0b01100000, 0b00000000 },  // 032, '2'
 { 0b11010001, 0b01000010, 0b00000000 },  // 033, '3'
 { 0b11110000, 0b00000011, 0b00000000 },  // 034, '4'
 { 0b11100001, 0b01000011, 0b00000000 },  // 035, '5'
 { 0b11100001, 0b01000111, 0b00000000 },  // 036, '6'
 { 0b10000011, 0b00100000, 0b00000000 },  // 037, '7'
 { 0b11110001, 0b01000111, 0b00000000 },  // 038, '8'
 { 0b11110001, 0b01000011, 0b00000000 },  // 039, '9'
 { 0b00000000, 0b00000000, 0b00000000 },  // 03a, ':'
 { 0b00000000, 0b00000000, 0b00000000 },  // 03b, ';'
 { 0b10000010, 0b00001000, 0b00000000 },  // 03c, '<'
 { 0b11000000, 0b01000001, 0b00000000 },  // 03d, '='
 { 0b10001000, 0b00100000, 0b00000000 },  // 03e, '>'
 { 0b00000000, 0b00000000, 0b00000000 },  // 03f, '?'
 { 0b11110001, 0b01000101, 0b00000000 },  // 040, '@'
 { 0b11110001, 0b00000111, 0b00000000 },  // 041, 'A'
 { 0b11010101, 0b01010010, 0b00000000 },  // 042, 'B'
 { 0b00100001, 0b01000100, 0b00000000 },  // 043, 'C'
 { 0b10010101, 0b01010010, 0b00000000 },  // 044, 'D'
 { 0b11100001, 0b01000101, 0b00000000 },  // 045, 'E'
 { 0b11100001, 0b00000101, 0b00000000 },  // 046, 'F'
 { 0b01100001, 0b01000110, 0b00000000 },  // 047, 'G'
 { 0b11110000, 0b00000111, 0b00000000 },  // 048, 'H'
 { 0b10000101, 0b01010000, 0b00000000 },  // 049, 'I'
 { 0b00010000, 0b01000010, 0b00000000 },  // 04a, 'J'
 { 0b10100010, 0b00001101, 0b00000000 },  // 04b, 'K'
 { 0b00100000, 0b01000100, 0b00000000 },  // 04c, 'L'
 { 0b10111010, 0b00000110, 0b00000000 },  // 04d, 'M'
 { 0b10111000, 0b00001110, 0b00000000 },  // 04e, 'N'
 { 0b00110001, 0b01000110, 0b00000000 },  // 04f, 'O'
 { 0b11110001, 0b00000101, 0b00000000 },  // 050, 'P'
 { 0b00110001, 0b01001110, 0b00000000 },  // 051, 'Q'
 { 0b11110001, 0b00001101, 0b00000000 },  // 052, 'R'
 { 0b11100001, 0b01000011, 0b00000000 },  // 053, 'S'
 { 0b10000101, 0b00010000, 0b00000000 },  // 054, 'T'
 { 0b00110000, 0b01000110, 0b00000000 },  // 055, 'U'
 { 0b10100010, 0b00100100, 0b00000000 },  // 056, 'V'
 { 0b10110000, 0b00101110, 0b00000000 },  // 057, 'W'
 { 0b10001010, 0b00101000, 0b00000000 },  // 058, 'X'
 { 0b11110000, 0b00010001, 0b00000000 },  // 059, 'Y'
 { 0b10000011, 0b01100000, 0b00000000 },  // 05a, 'Z'
 { 0b00100001, 0b01000100, 0b00000000 },  // 05b, '['
 { 0b10001000, 0b00001000, 0b00000000 },  // 05c, back slash
 { 0b00010001, 0b01000010, 0b00000000 },  // 05d, ']'
 { 0b00110001, 0b00000000, 0b00000000 },  // 05e, '^'
 { 0b00000000, 0b01000000, 0b00000000 },  // 05f, '_'
 { 0b00001000, 0b00000000, 0b00000000 },  // 060, back quote
 { 0b11010001, 0b01000111, 0b00000000 },  // 061, 'a'
 { 0b11100000, 0b01000111, 0b00000000 },  // 062, 'b'
 { 0b11000000, 0b01000101, 0b00000000 },  // 063, 'c'
 { 0b11010000, 0b01000111, 0b00000000 },  // 064, 'd'
 { 0b11110001, 0b01000101, 0b00000000 },  // 065, 'e'
 { 0b10100001, 0b00000101, 0b00000000 },  // 066, 'f'
 { 0b11110001, 0b01000011, 0b00000000 },  // 067, 'g'
 { 0b11100000, 0b00000111, 0b00000000 },  // 068, 'h'
 { 0b10000000, 0b00010000, 0b00000000 },  // 069, 'i'
 { 0b00010000, 0b01000010, 0b00000000 },  // 06a, 'j'
 { 0b11100000, 0b00001101, 0b00000000 },  // 06b, 'k'
 { 0b10000100, 0b00010000, 0b00000000 },  // 06c, 'l'
 { 0b11000000, 0b00010111, 0b00000000 },  // 06d, 'm'
 { 0b11000000, 0b00000111, 0b00000000 },  // 06e, 'n'
 { 0b11000000, 0b01000111, 0b00000000 },  // 06f, 'o'
 { 0b11110001, 0b00000101, 0b00000000 },  // 070, 'p'
 { 0b11110001, 0b00000011, 0b00000000 },  // 071, 'q'
 { 0b11000000, 0b00000101, 0b00000000 },  // 072, 'r'
 { 0b11100001, 0b01000011, 0b00000000 },  // 073, 's'
 { 0b10100000, 0b01000101, 0b00000000 },  // 074, 't'
 { 0b00000000, 0b01000110, 0b00000000 },  // 075, 'u'
 { 0b10100010, 0b00100100, 0b00000000 },  // 076, 'v'
 { 0b10110000, 0b00101110, 0b00000000 },  // 077, 'w'
 { 0b10001010, 0b00101000, 0b00000000 },  // 078, 'x'
 { 0b10001010, 0b00100000, 0b00000000 },  // 079, 'y'
 { 0b10000011, 0b01100000, 0b00000000 },  // 07a, 'z'
 { 0b10000010, 0b00001000, 0b00000000 },  // 07b, '{'
 { 0b00000100, 0b00010000, 0b00000000 },  // 07c, '|'
 { 0b10001000, 0b00100000, 0b00000000 },  // 07d, '}'
 { 0b01000000, 0b00000001, 0b00000000 },  // 07e, '~'
 { 0b11000000, 0b01000111, 0b00000000 }   // 07f, DEL
};


/****************************************************************************************
 *
 * Initial values and settings for ET16315
 *
 ****************************************************************************************/

static struct et16315_chip chip =
{
  .leds        = 0,
  .mode        = et16315_config_9_digits_19_segments,
  .on          = 1,
  .brightness  = ET16315_MAX_BRIGHT,
  .char_tbl    = et16315_fp_chars,
};


static void et16315_send(byte command, const void *buf, int len)
{
    // Send command and [len] parameter bytes
    const byte *data = (const byte*)buf;
    byte i;

    digitalWrite(SS_PIN, LOW);
    SPI.beginTransaction(SPISettings(1000000, LSBFIRST, SPI_MODE3));
    SPI.transfer(command);
    SPI.transfer((void*)data, len);
    SPI.endTransaction();
    digitalWrite(SS_PIN, HIGH);
}


static void et16315_recv(byte command, void *buf, int len)
{
    byte *data = (byte*)buf;
    int i;

    digitalWrite(SS_PIN, LOW);
    SPI.beginTransaction(SPISettings(1000000, LSBFIRST, SPI_MODE3));
    SPI.transfer(command);

    for (i = 0; i < len; i++)
    {
        data[i] = SPI.transfer(0x00);
    }
    SPI.endTransaction();
    digitalWrite(SS_PIN, HIGH);
}


/* LED routine; control LED port pins */
void et16315_set_leds(byte leds)
{
    byte data[1] = { (leds & 0b00001111) ^ 0b00001111 };
    chip.leds = leds;
    et16315_send(ET16315_CMD2_DATA_SET(0, ET16315_FIXED_ADDR, ET16315_CMD_SET_LED),
                 data, 1);
}


/* Display routines */
void et16315_clear()
{
    // clear display RAM
    memset(chip.display_data, 0, sizeof(chip.display_data));
    et16315_send(ET16315_CMD2_DATA_SET(0, ET16315_AUTO_ADDR_INC,
                                       ET16315_CMD_WRITE_DATA),
                 NULL, 0);
    et16315_send(ET16315_CMD3_SET_ADDRESS(0), chip.display_data,
                 sizeof(chip.display_data));
}


void et16315_set_brightness(byte brght)
{
    if (brght < 0)
    {
        brght = 0;
    }
    else if (brght > ET16315_MAX_BRIGHT)
    {
        brght = ET16315_MAX_BRIGHT;
    }
    chip.brightness = brght;
    et16315_send(ET16315_CMD4_DISPLAY_CONTROL(chip.on, chip.brightness), NULL, 0);
}


void et16315_set_light(byte on)
{
    if (on < 0)
    {
        on = 0;
    }
    else if (on > 1)
    {
        on = 1;
    }
    chip.on = on;
    et16315_send(ET16315_CMD4_DISPLAY_CONTROL(chip.on, chip.brightness), NULL, 0);
}


void et16315_set_text(const char *text)
{
    byte i, len, digits;

    digits = chip.mode + 4;
    len = strlen(text);
    if (len > digits)
    {
        len = digits;
    }

	/* display digits are connected right to left */
    for (i = 3 * (digits - 1); i > 3 * (digits - 1 - len); i -= 3)
        {
        chip.display_data[i]   = chip.char_tbl[*text-0x20].value0;
        chip.display_data[i+1] = chip.char_tbl[*text-0x20].value1 | (chip.display_data[i+1] & 0b10000000);
        // chip.display_data[i+2] = chip.char_tbl[*text-0x20].value2;
        chip.display_data[i+2] = 0;
        ++text;
    }
    et16315_send(ET16315_CMD3_SET_ADDRESS(0), chip.display_data, (digits * 3));
}


int et16315_start(void)
{
	/* Initialize the ET16315 */
    pinMode(SS_PIN, OUTPUT);
    SPI.begin();

    /* Clear display memory */
	et16315_clear();
	/* Set display mode and initial brightness */
	et16315_send(ET16315_CMD1_SET_DISPLAY_MODE(chip.mode), NULL, 0);
	// enable display
	et16315_set_light(1);
	// set initial brightness
    et16315_set_brightness(ET16315_MAX_BRIGHT);
	return 0;
}


int et16315_seticon(int which, int on)
{
    byte ret = 0;
    byte digit_offset;
    byte digits;

    digits = chip.mode + 4;
    switch(which)
    {
        case ICON_DOT:
        {
            digit_offset = 7 * 3 + 1;
            break;
        }
        case ICON_COLON1:
        {
            digit_offset = 6 * 3 + 1;
            break;
        }
        case ICON_COLON2:
        {
            digit_offset = 4 * 3 + 1;
            break;
        }
        case ICON_COLON3:
        {
            digit_offset = 2 * 3 + 1;
            break;
        }
        default:
        {
            return -1;
        }
    }

    if (on < 1) {
        chip.display_data[digit_offset] &= 0b01111111;
    }
    else {
        chip.display_data[digit_offset] |= 0b10000000;
    }
    et16315_send(ET16315_CMD3_SET_ADDRESS(0),
                 chip.display_data, (digits * 3));

    return ret;
}


// vim:ts=4
