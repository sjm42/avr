/*****************************************************************************************/

#ifndef _et16315_h
#define _et16315_h

#include <Arduino.h>

/* Comment or delete next line to enable text
   scrolling in low level VFD driver */
#define VFD_SCROLL     1

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
#define ET16315_CMD1_SET_DISPLAY_MODE(mode) \
		(0x00 | ((mode) & 0x0F))

#define ET16315_TEST_MODE      0b00001000  // -> option = + 0x08
#define ET16315_FIXED_ADDR     0b00000100  // -> option = + 0x04
#define ET16315_AUTO_ADDR_INC  0b00000000  // -> option = + 0x00
#define ET16315_CMD_WRITE_DATA 0b00000000  // -> option = + 0x00
#define ET16315_CMD_SET_LED    0b00000001  // -> option = + 0x01
#define ET16315_CMD_READ_KEY   0b00000010  // -> option = + 0x02
#define ET16315_CMD_DONTCARE   0b00000011  // not used

#define ET16315_CMD2_DATA_SET(test_mode, fixed_address, command) \
		(0b01000000 | \
		(test_mode ? ET16315_TEST_MODE : 0x00) | \
		(fixed_address ? ET16315_FIXED_ADDR : ET16315_AUTO_ADDR_INC) | \
		(command & 0x3))

/*
3. Set address command: 0b11nnnnnn
 nnnnnn is binary display address
 digit * 3 is address of LB data (1st display data byte)
 (digit * 3) + 1 is address of MB data (2nd display data byte)
 (digit * 3) + 2 is address of HB data (3rd display data byte)
*/
#define ET16315_CMD3_SET_ADDRESS(address) \
		(0b11000000 | (address & 0b00111111))

#define ET16315_DISPLAY_ENABLE 0b00001000

/*
4. Display control command: 0b10XXDbbb -> CMD = 0x80 + options
 D: 0=Display off
    1=Display on
 bbb = brightness level, 000 is lowest
*/
#define ET16315_CMD4_DISPLAY_CONTROL(on, brightness) \
		(0b10000000 | \
		(on ? ET16315_DISPLAY_ENABLE : 0x00) | \
		(brightness & 0b00000111))


#define ET16315_DISPLAY_MAX_DIGITS   12
#define ET16315_DISPLAY_BUFFER_SIZE  ET16315_DISPLAY_MAX_DIGITS * 3

struct et16315_char
{
	byte value0;
	byte value1;
	byte value2;  // not needed with 15 segment display, always zero
};

struct et16315_platform_data
{
	enum
	{
		et16315_config_4_digits_24_segments = 4,
		et16315_config_5_digits_23_segments,
		et16315_config_6_digits_22_segments,
		et16315_config_7_digits_21_segments,
		et16315_config_8_digits_20_segments,
		et16315_config_9_digits_19_segments,
		et16315_config_10_digits_18_segments,
		et16315_config_11_digits_17_segments,
		et16315_config_12_digits_16_segments
	} digits;

	/* LEDs */
	byte led;

	/* Display control */
	int        brightness;  /* initial value, 0 (lowest) - 7 (max) */
	int        on;  /* initial value for display enable */
	struct     et16315_char *char_tbl;
	const char *text;  /* initial display text */
};

struct et16315_chip
{
	unsigned gpio_din, gpio_dout, gpio_clk, gpio_stb;  /* Wiring info */

	/* VFD display */
	int    digits;
	int    on; // state of display enable
	int    brightness;
	byte     last_display[ET16315_DISPLAY_BUFFER_SIZE];
	struct et16315_char *char_tbl;

	/* LEDs */
	byte led;

};
#endif // _et16315_h
// vim:ts=4
