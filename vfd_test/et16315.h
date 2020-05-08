/*****************************************************************************************/

#ifndef _et16315_h
#define _et16315_h

#include <Arduino.h>
#include <SPI.h>

#define SS_PIN 10
#define BLEN 80

#define ET16315_MAX_BRIGHT   7
#define ET16315_MAX_DIGITS  12
#define ET16315_BUFFER_SIZE (ET16315_MAX_DIGITS * 3)

/* ET16315 command bytes
 
1. Display set command: 0b00XXnnnn
 nnnn  Display mode
 ----------------------------
 0000   4 digits, 24 segments
 0001   5 digits, 23 segments
 0010   6 digits, 22 segments
 0011   7 digits, 21 segments
 0100   8 digits, 20 segments <- FP has 8 digits, so command is 0b00XX0100 (0x04)
 0101   9 digits, 19 segments
 0110  10 digits, 18 segments
 0111  11 digits, 17 segments
 1XXX  12 digits, 16 segments
*/
#define ET16315_CMD1_SET_DISP(mode) \
		(0x00 | ((mode) & 0x0F))

#define ET16315_TEST_MODE      0b00001000  // -> option = + 0x08
#define ET16315_ADDR_FIX       0b00000100  // -> option = + 0x04
#define ET16315_ADDR_INC       0b00000000  // -> option = + 0x00
#define ET16315_CMD_WRITE_DATA 0b00000000  // -> option = + 0x00
#define ET16315_CMD_SET_LED    0b00000001  // -> option = + 0x01
#define ET16315_CMD_READ_KEY   0b00000010  // -> option = + 0x02
#define ET16315_CMD_DONTCARE   0b00000011  // not used

#define ET16315_CMD2_SET_MODE(test_mode, fixed_address, command) \
		(0b01000000 | \
		(test_mode ? ET16315_TEST_MODE : 0x00) | \
		(fixed_address ? ET16315_ADDR_FIX : ET16315_ADDR_INC) | \
		(command & 0x3))

/*
3. Set address command: 0b11nnnnnn
 nnnnnn is binary display address
 digit * 3 is address of LB data (1st display data byte)
 (digit * 3) + 1 is address of MB data (2nd display data byte)
 (digit * 3) + 2 is address of HB data (3rd display data byte)
*/
#define ET16315_CMD3_SET_ADDR(address) \
		(0b11000000 | (address & 0b00111111))

#define ET16315_DISPLAY_ENABLE 0b00001000

/*
4. Display control command: 0b10XXDbbb -> CMD = 0x80 + options
 D: 0=Display off
    1=Display on
 bbb = brightness level, 000 is lowest
*/
#define ET16315_CMD4_DISP_CTRL(on, brightness) \
		(0b10000000 | \
		(on ? ET16315_DISPLAY_ENABLE : 0x00) | \
		(brightness & 0b00000111))


#define ET16315_SYM_DOLBY  { 2, 0b00000001}
#define ET16315_SYM_DTS    { 5, 0b00000001}
#define ET16315_SYM_VIDEO  { 8, 0b00000001}
#define ET16315_SYM_AUDIO  {11, 0b00000001}
#define ET16315_SYM_LINK   {14, 0b00000001}
#define ET16315_SYM_HDD    {17, 0b00000001}
#define ET16315_SYM_DISC   {20, 0b00000001}
#define ET16315_SYM_DVB    {23, 0b00000001}
#define ET16315_SYM_EUR    {24, 0b00000001}
#define ET16315_SYM_PLAY   {24, 0b00000010}
#define ET16315_SYM_REW    {24, 0b00000100}
#define ET16315_SYM_PAUSE  {24, 0b00001000}
#define ET16315_SYM_FWD    {24, 0b00010000}
#define ET16315_SYM_NONE   {24, 0b00100000}
#define ET16315_SYM_REC    {24, 0b01000000}
#define ET16315_SYM_REDDOT {24, 0b10000000}
#define ET16315_SYM_CLOCK1 {25, 0b00000001}
#define ET16315_SYM_CLOCK2 {25, 0b00000010}
#define ET16315_SYM_CARD   {25, 0b00000100}
#define ET16315_SYM_1      {25, 0b00001000}
#define ET16315_SYM_2      {25, 0b00010000}
#define ET16315_SYM_KEY    {25, 0b00100000}
#define ET16315_SYM_16_9   {25, 0b01000000}
#define ET16315_SYM_USB    {25, 0b10000000}
#define ET16315_SYM_DVD    {26, 0b00000001}

enum et16315_symbol
{
  et16315_sym_DOLBY = 0,
  et16315_sym_DTS,
  et16315_sym_VIDEO,
  et16315_sym_AUDIO,
  et16315_sym_LINK,
  et16315_sym_HDD,
  et16315_sym_DISC,
  et16315_sym_DVB,
  et16315_sym_EUR,
  et16315_sym_PLAY,
  et16315_sym_REW,
  et16315_sym_PAUSE,
  et16315_sym_FWD,
  et16315_sym_NONE,
  et16315_sym_REC,
  et16315_sym_REDDOT,
  et16315_sym_CLOCK1,
  et16315_sym_CLOCK2,
  et16315_sym_CARD,
  et16315_sym_1,
  et16315_sym_2,
  et16315_sym_KEY,
  et16315_sym_16_9,
  et16315_sym_USB,
  et16315_sym_DVD,
};

typedef struct
{
    byte addr;
    byte bit;
} et16315_sym_addr;

typedef struct
{
	byte value0;
	byte value1;
	byte value2;
} et16315_char;

typedef struct
{
	/* LEDs */
	byte leds;
	/* VFD display */
	byte mode;
	byte on;
	byte brightness;
    byte display_data[ET16315_BUFFER_SIZE];
    byte scratch[ET16315_BUFFER_SIZE];
} et16315_chip;

// ET16315 display modes: d digits, s segments
typedef enum {
      et16315_4d_24s = 0,
      et16315_5d_23s,
      et16315_6d_22s,
      et16315_7d_21s,
      et16315_8d_20s,
      et16315_9d_19s,
      et16315_10d_18s,
      et16315_11d_17s,
      et16315_12d_16s
} et16315_display_mode;

void et16315_xfer(byte command, void *buf, int len);
void et16315_clear();
void et16315_set_leds(byte leds);
void et16315_set_light(byte on, byte brght);
void et16315_set_text(const char *text, int len);
void et16315_set_colon(byte i, byte on);
void et16315_set_symbol(byte s, byte on);
void et16315_start(void);


#endif // _et16315_h
// vim:ts=4
