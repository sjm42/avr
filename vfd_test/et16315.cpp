
/******************************************************************************/


#include "et16315.h"

#define DISP_SIZE 8
static char pbuf[BLEN];

/****************************************************************************************
 *
 * Character table for 8 digit 15-segment VFD display
 *
 ****************************************************************************************
 *
 * In the VFD front panel the ET16315 driver IC is wired as follows:
 *
 *     aaaaaaa
 *    fh  j  kb
 *    f h j k b  q
 *    f  hjk  b
 *     gggimmm
 *    e  rpn  c
 *    e r p n c  q
 *    er  p  nc
 *     ddddddd
 *
 * SG0 -> seg a (0x01, value0) 
 * SG1 -> seg k (0x02, value0)
 * SG2 -> seg j (0x04, value0)
 * SG3 -> seg h (0x08, value0)
 * SG4 -> seg b (0x10, value0)
 * SG5 -> seg f (0x20, value0)
 * SG6 -> seg m (0x40, value0)
 * SG7 -> seg i (0x80, value0)
 *
 * SG8 -> seg g (0x01, value1) 
 * SG9 -> seg c (0x02, value1)
 * SGA -> seg e (0x04, value1)
 * SGB -> seg n (0x08, value1)
 * SGC -> seg p (0x10, value1)
 * SGD -> seg r (0x20, value1)
 * SGE -> seg d (0x40, value1)
 * SGF -> seg q (0x80, value1) colon on digits 2, 4 & 6
 *
 * SG10 is a varied symbol above digit
 * The left-right order is reversed:
 * GR0 is connected to the rightmost digit 7, GR7 to the leftmost digit 0
 *
 * The characters below are in ASCII order, starting with space (0x20).
 */
static et16315_char et16315_xlate[] =
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

et16315_sym_addr et16315_sym_xlate[] =
{
  ET16315_SYM_DOLBY,
  ET16315_SYM_DTS,
  ET16315_SYM_VIDEO,
  ET16315_SYM_AUDIO,
  ET16315_SYM_LINK,
  ET16315_SYM_HDD,
  ET16315_SYM_DISC,
  ET16315_SYM_DVB,
  ET16315_SYM_EUR,
  ET16315_SYM_PLAY,
  ET16315_SYM_REW,
  ET16315_SYM_PAUSE,
  ET16315_SYM_FWD,
  ET16315_SYM_NONE,
  ET16315_SYM_REC,
  ET16315_SYM_REDDOT,
  ET16315_SYM_CLOCK1,
  ET16315_SYM_CLOCK2,
  ET16315_SYM_CARD,
  ET16315_SYM_1,
  ET16315_SYM_2,
  ET16315_SYM_KEY,
  ET16315_SYM_16_9,
  ET16315_SYM_USB,
  ET16315_SYM_DVD,
};


/****************************************************************************************
 *
 * Initial values and settings for ET16315
 *
 ****************************************************************************************/

et16315_chip chip =
{
  .leds        = 0,
  .mode        = et16315_11d_17s,
  .on          = 1,
  .brightness  = ET16315_MAX_BRIGHT,
  .display_data = {0},
  .scratch      = {0},
};


void et16315_xfer(byte command, void *buf, int len)
{
    // Send command and [len] parameter bytes
    // BEWARE: buf will be overwritten!
    digitalWrite(SS_PIN, LOW);
    SPI.beginTransaction(SPISettings(1000000, LSBFIRST, SPI_MODE3));
    SPI.transfer(command);
    if (buf != NULL) SPI.transfer(buf, len);
    SPI.endTransaction();
    digitalWrite(SS_PIN, HIGH);
}


void et16315_clear()
{
    // clear display RAM
    memset(chip.display_data, 0x00, sizeof(chip.display_data));
    memset(chip.scratch, 0x00, sizeof(chip.scratch));
    et16315_xfer(ET16315_CMD2_SET_MODE(0, 0, ET16315_CMD_WRITE_DATA), NULL, 0);
    et16315_xfer(ET16315_CMD3_SET_ADDR(0), chip.scratch, sizeof(chip.scratch));
}


void et16315_set_leds(byte leds)
{
    byte data[1] = { (leds & 0b00001111) ^ 0b00001111 };
    chip.leds = leds;
    et16315_xfer(ET16315_CMD2_SET_MODE(0, 1, ET16315_CMD_SET_LED), data, 1);
}


void et16315_set_light(byte on, byte brght)
{
    if (on > 1)
    {
        on = 1;
    }
    if (brght > ET16315_MAX_BRIGHT)
    {
        brght = ET16315_MAX_BRIGHT;
    }
    chip.on = on;
    chip.brightness = brght;
    et16315_xfer(ET16315_CMD4_DISP_CTRL(on, brght), NULL, 0);
}


void et16315_set_text(const char *text, int len)
{
    char i, j;

    if (len > DISP_SIZE)
    {
        len = DISP_SIZE;
    }

    /* Digits run backwards! */
    for (j=0, i = 3 * (DISP_SIZE-1); j<len && i >= 0; ++j, i -= 3)
    {
        char c = text[j];
        if (c < 0x20) c = 0x20;
        else if (c > 0x7E) c = 0x7E;
        c -= 0x20;

        chip.display_data[i]   = et16315_xlate[c].value0;
        chip.display_data[i+1] = et16315_xlate[c].value1 | (chip.display_data[i+1] & 0b10000000);
        chip.display_data[i+2] = et16315_xlate[c].value2;
    }
    // Must copy to scratchpad since it gets overwritten by SPI routines!
    memcpy(chip.scratch, chip.display_data, DISP_SIZE*3);
    et16315_xfer(ET16315_CMD2_SET_MODE(0, 0, ET16315_CMD_WRITE_DATA), NULL, 0);
    et16315_xfer(ET16315_CMD3_SET_ADDR(0), chip.scratch, DISP_SIZE*3);
}


void et16315_scroll(const char *text, int len, byte times)
{
    int i;
    byte r;

    if (len > BLEN - 2*DISP_SIZE) len = BLEN - 2*DISP_SIZE;
    memset(pbuf, ' ', DISP_SIZE);
    memcpy(pbuf + DISP_SIZE, text, len);
    memset(pbuf + DISP_SIZE + len, ' ', DISP_SIZE);
    for (r=0; r < times; ++r) {
        for (i = 0; i <= len+DISP_SIZE; ++i) {
            et16315_set_text(pbuf+i, DISP_SIZE);
            delay(200);
        }
        delay(200);
    }
}


void et16315_set_colon(byte i, byte on)
{
    int a;
    char c;

    if (i > 3) i = 3;
    a = 3*DISP_SIZE - (3*2*i) + 1;
    c = chip.display_data[a];
    if (on) {
        c |= 0b10000000;
    }
    else {
        c &= 0b01111111;
    }
    chip.display_data[a] = c;
    et16315_xfer(ET16315_CMD2_SET_MODE(0, 1, ET16315_CMD_WRITE_DATA), NULL, 0);
    et16315_xfer(ET16315_CMD3_SET_ADDR(a), &c, 1);
}


void et16315_set_symbol(byte s, byte on)
{
    et16315_sym_addr sym_a = et16315_sym_xlate[s];
    char c = chip.display_data[sym_a.addr];
    if (on) {
        // set bit
        c |= sym_a.bit;
    }
    else {
        // clear bit
        c &= (sym_a.bit ^ 0xFF);
    }
    chip.display_data[sym_a.addr] = c;
    et16315_xfer(ET16315_CMD2_SET_MODE(0, 1, ET16315_CMD_WRITE_DATA), NULL, 0);
    et16315_xfer(ET16315_CMD3_SET_ADDR(sym_a.addr), &c, 1);
}


void et16315_start(void)
{
    /* Initialize the ET16315 */
    pinMode(SS_PIN, OUTPUT);
    SPI.begin();

    /* Clear display memory */
    et16315_clear();

    /* Set display mode and initial brightness */
    Serial.print("Mode: ");
    Serial.println(chip.mode);
    et16315_xfer(ET16315_CMD1_SET_DISP(chip.mode), NULL, 0);

    // Enable display, set brightness
    et16315_set_light(1, ET16315_MAX_BRIGHT);
}


// vim:ts=4
