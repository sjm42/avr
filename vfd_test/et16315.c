
/******************************************************************************/


#include "et16315.h"

#define DISP_SIZE 8

static struct et16315_chip *et16315_data;  // global values for IOCTL routines

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
// Note: values 0x80 - 0xff are handled by UTF8 routines
};


/****************************************************************************************
 *
 * Initial values and settings for ET16315
 *
 ****************************************************************************************/
static struct et16315_platform_data et16315_init_values =
{
	/* Wiring info */
	.gpio_din    = ET16315_DIN,
	.gpio_dout   = ET16315_DOUT,
	.gpio_clk    = ET16315_CLK,
	.gpio_stb    = ET16315_STB,

	/* VFD display */
	.digits      = et16315_config_8_digits_20_segments,  // 8 character display
	.brightness  = MAX_BRIGHT,
	.on          = 1, // display enable
	.char_tbl    = et16315_fp_chars,
#if defined(FOREVER_3434HD)
	.text        = "FOREVER_3434HD",  /* initial display text */
#elif defined(FOREVER_9898HD)
	.text        = "FOREVER_9898HD",
#elif defined(EP8000)
	.text        = "EP8000",
#elif defined(EPP8000)
	.text        = "EPP8000",
#elif defined(GPV8000)
	.text        = "GPV8000",
#else
	.text        = "--------",
#endif
	/* LEDs */
#if defined(FOREVER_3434HD)
	.led         = LED_BLUE + LED_LOGO,  // blue & logo on
#elif defined(FOREVER_9898HD)
	.led         = LED_RED, // all LEDs off
#elif defined(GPV8000) \
 ||   defined(EP8000) \
 ||   defined(EPP8000)
	.led         = LED_LOGO,  // logo on, rest off
#else
	.led         = 0, // all LEDs off
#endif

};

/****************************************************************************************
 *
 * GPIO serial routines for ET16315
 *
 ****************************************************************************************/
static void et16315_writeb(struct et16315_chip *chip, u8 byte)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		gpio_set_value(chip->gpio_din, byte & 0b00000001);
		gpio_set_value(chip->gpio_clk, 0);
		udelay(1);
		gpio_set_value(chip->gpio_clk, 1);
		udelay(1);

		byte >>= 1;
	}
}

static void et16315_readb(struct et16315_chip *chip, u8 *byte)
{  // Read one byte (key data)
	int i;

	*byte = 0;
	for (i = 0; i < 8; i++)
	{
		gpio_set_value(chip->gpio_clk, 0);
		udelay(2);
		gpio_set_value(chip->gpio_clk, 1);
		udelay(1);
		*byte <<= 1;  // note: bit order is reversed!
		*byte |= (gpio_get_value(chip->gpio_dout) & 0b00000001);
	}
}

static void et16315_send(struct et16315_chip *chip, u8 command, const void *buf, int len)
{  // Send command and [len] parameter bytes
	const u8 *data = buf;
	int i;

	BUG_ON(len > ET16315_DISPLAY_BUFFER_SIZE);

#if 0
	dprintk(20, "%s: command: 0x%02X (len = %d)", __func__, command, len);
	for (i = 0; i < len; i++)
	{
		printk(" [0x%02X]", data[i]);
	}
	printk("\n");
#endif
	gpio_set_value(chip->gpio_stb, 0);
	udelay(1);
	et16315_writeb(chip, command);
	udelay(1);

	for (i = 0; i < len; i++)
	{
		et16315_writeb(chip, *data++);
		udelay(1);
	}
	gpio_set_value(chip->gpio_stb, 1);
	udelay(2);
}

static void et16315_recv(struct et16315_chip *chip, u8 command, void *buf, int len)
{
	u8 *data = buf;
	int i;

	gpio_set_value(chip->gpio_stb, 0);
	udelay(1);
//	dprintk(10, "%s Command = 0x%02x, # args = %d\n", __func__, command, len);
	et16315_writeb(chip, command);
	udelay(3);

	for (i = 0; i < len; i++)
	{
		et16315_readb(chip, data++);
//		dprintk(10, "%s read byte #%02d = 0x%02x\n", __func__, i, *data);
		udelay(1);
	}
	gpio_set_value(chip->gpio_stb, 1);
	udelay(2);
}

/* LED routine; control LED port pins */
static void et16315_set_led(struct et16315_chip *chip, int leds)
{
	u8 data[1] = { (leds & 0b00001111) ^ 0b00001111 };

	dprintk(20, "%s > leds = 0x%02x\n", __func__, leds);
	chip->led = leds;

	dprintk(10, "Send cmd=0x%02x + data 0x%02x\n", (int)ET16315_CMD2_DATA_SET(0, ET16315_FIXED_ADDR, ET16315_CMD_SET_LED), (int)data[0]);
	et16315_send(chip, ET16315_CMD2_DATA_SET(0, ET16315_FIXED_ADDR, ET16315_CMD_SET_LED), data, 1);
}

/* Display routines */
static void et16315_clear(struct et16315_chip *chip)
{
	u8 data[ET16315_DISPLAY_BUFFER_SIZE];

	memset(data, 0, sizeof(data));  // clear display RAM

	et16315_send(chip, ET16315_CMD2_DATA_SET(0, ET16315_AUTO_ADDR_INC, ET16315_CMD_WRITE_DATA), NULL, 0);
	et16315_send(chip, ET16315_CMD3_SET_ADDRESS(0), data, sizeof(data));
	memcpy(chip->last_display, data, sizeof(data));
}

static void et16315_set_brightness(struct et16315_chip *chip, int brightness)
{
	if (brightness < 0)
	{
		brightness = 0;
	}
	else if (brightness > MAX_BRIGHT)
	{
		brightness = MAX_BRIGHT;
	}
	et16315_send(chip, ET16315_CMD4_DISPLAY_CONTROL(et16315_data->on, brightness), NULL, 0);
	et16315_data->brightness = brightness;
}

static void et16315_set_light(struct et16315_chip *chip, int on)
{
	if (on < 0)
	{
		on = 0;
	}
	else if (on > 1)
	{
		on = 1;
	}
	et16315_send(chip, ET16315_CMD4_DISPLAY_CONTROL(on, et16315_data->brightness), NULL, 0);
	et16315_data->on = on;
}

static void et16315_set_text(struct et16315_chip *chip, const char *text)
{
	u8 data[ET16315_DISPLAY_BUFFER_SIZE] = { 0, };
	int i, len;

	len = strlen(text);
	if (len > chip->digits)
	{
		len = chip->digits;
	}
//	dprintk(10, "%s Display [%s] (length = %02d)\n", __func__, text, len);

	/* On FOREVER_3434HD, FOREVER_9898HD, EP8000, EPP8000 & GPV8000, display digits are connected right to left */
	for (i = 3 * (chip->digits - 1); i > 3 * (chip->digits - 1 - len); i -= 3)
	{
		chip->last_display[i] = data[i] = chip->char_tbl[*text - 0x20].value0;
		chip->last_display[i + 1] = data[i + 1] = chip->char_tbl[*text - 0x20].value1 | (chip->last_display[i + 1] & 0b10000000);
//		chip->last_display[i + 2] = data[i + 2] = chip->char_tbl[*text - 0x20].value2;
		chip->last_display[i + 2] = data[i + 2] = 0;
		text++;
 
	}
	// update last display data
	for (i = 0; i < 3 * (chip->digits - len); i += 3)
	{
		chip->last_display[i] = data[i] = 0;
		chip->last_display[i + 1] = data[i + 1] = (chip->last_display[i + 1] & 0b10000000);
		chip->last_display[i + 2] = data[i + 2] = 0;
	}
	et16315_send(chip, ET16315_CMD3_SET_ADDRESS(0), data, (chip->digits * 3));
}

/****************************************************************************************
 *
 * LED routines
 *
 ****************************************************************************************/
static void init_leds(struct et16315_chip *chip)
{
	struct et16315_platform_data *et16315_init = &et16315_init_values;

	dprintk(100, "%s >\n", __func__);
#if defined(EP8000) || defined(EPP8000) || defined(GPV8000) || defined(FOREVER_3434HD)
	// blink red led
	et16315_set_led(chip, LED_RED);
	msleep(200);
	et16315_set_led(chip, 0); // all LEDs off
	msleep(200);
	gpio_free(GPIO_LED_LOGO);
	gpio_request(GPIO_LED_LOGO, "LED_LOGO");
	gpio_direction_output(GPIO_LED_LOGO, 1); // drive of LED_LOGO is inverted
#if defined(FOREVER_3434HD)
	// blink blue led
	et16315_set_led(chip, LED_BLUE);
	msleep(200);
	et16315_set_led(chip, 0);  // all LEDs off
	msleep(200);
#endif
	/* Set initial LED state */
	chip->led = et16315_init->led;
	fortis_4gSetLED(chip->led, 1);
#elif defined(FOREVER_9898HD)
	dprintk(1, "%s FOREVER_9898HD detected: Red LED is GPIO driven\n", __func__);
//	gpio_free(GPIO_LED_RED);
//	gpio_request(GPIO_LED_RED, "LED_RED");
//	gpio_direction_output(GPIO_LED_RED, 1);
//	msleep(200);
//	gpio_direction_output(GPIO_LED_RED, 0);
#else
	dprintk(1, "%s Unsupported receiver model detected!\n", __func__);
#endif
	dprintk(100, "%s <\n", __func__);
}


/****************************************************************************************
 *
 * Initialize ET16315 front panel interface and LEDs
 *
 ****************************************************************************************/
int __init fortis_4g_if_init(void)
{
	int err;
	struct et16315_platform_data *et16315_init = &et16315_init_values;

	/* Initialize the GPIO interface to the ET16315 */
	et16315_data = kzalloc(sizeof(*et16315_data), GFP_KERNEL);
	if (!et16315_data)
	{
		dprintk(1, "kzalloc failed\n");
		return -ENOMEM;
	}

	/* Initialize the ET16315 */
	et16315_clear(et16315_data);  /* Clear display memory */

	/* Initialize character table */
	et16315_data->char_tbl = et16315_init->char_tbl;

	/* Set display mode and initial brightness */
	et16315_data->digits = et16315_init->digits;
	if (et16315_data->digits > ET16315_DISPLAY_MAX_DIGITS)
	{
		et16315_data->digits = ET16315_DISPLAY_MAX_DIGITS;
	}
	et16315_send(et16315_data, ET16315_CMD1_SET_DISPLAY_MODE(et16315_data->digits - et16315_config_4_digits_24_segments), NULL, 0);

	// enable display
	et16315_set_light(et16315_data, et16315_init->on);

	// set initial brightness
	fortis_4gSetBrightness(et16315_init->brightness);
	dprintk(10, "Display initialized, width = %d, brightness = %d\n", et16315_data->digits, et16315_data->brightness);

	/* Show initial text */
	et16315_set_text(et16315_data, et16315_init->text);
	msleep(1000);

	/* Initialize and set the LEDs and icons */
	fortis_4gSetIcon(ICON_DOT, 1);
	msleep(100);
	fortis_4gSetIcon(ICON_COLON1, 1);
	msleep(100);
	fortis_4gSetIcon(ICON_COLON2, 1);
	msleep(100);
	fortis_4gSetIcon(ICON_COLON3, 1);
	msleep(100);

	fortis_4gSetIcon(ICON_DOT, 0);
	msleep(100);
	fortis_4gSetIcon(ICON_COLON1, 0);
	msleep(100);
	fortis_4gSetIcon(ICON_COLON2, 0);
	msleep(100);
	fortis_4gSetIcon(ICON_COLON3, 0);
	msleep(100);
	dprintk(10, "Icons initialized\n");

	return 0;
}


/****************************************************************************************
 *
 * ET16315 IOCTL routines
 *
 ****************************************************************************************/
/* Function SetIcon */
int fortis_4gSetIcon(int which, int on)
{
	int ret = 0;
	int digit_offset;

	dprintk(100, "%s > which = %d, on = %d\n", __func__, which, on);

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
			dprintk(1, "Icon number %d out of range (1-%d)\n", which, ICON_MAX);
			return -EINVAL;
		}
	}

	if (on < 1)
	{
		et16315_data->last_display[digit_offset] &= 0b01111111;
	}
	else
	{
		et16315_data->last_display[digit_offset] |= 0b10000000;
	}
	et16315_send(et16315_data, ET16315_CMD3_SET_ADDRESS(0), et16315_data->last_display, (et16315_data->digits * 3));

	return ret;
}

/* Function SetLED (and SetPwrLED) */
int fortis_4gSetLED(int which, int level)
{
	int ret = 0;
	u8 led;

	dprintk(10, "%s > which = %d, level = %d\n", __func__, which, level);

	/* Caution: LED# is a bitmask with Fortis receivers (nuvoton compatibility) */

	if (which < 0 || which > MAX_LED)
	{
		dprintk(1, "LED number %d out of range (1-%d)\n", which, MAX_LED);
		return -EINVAL;
	}

	level = (level != 0 ? 1 : 0);  // normalize level to 0 or 1

	/* Red and blue LEDs are connected to the ET16315 VFD driver
	 * Logo LEDs are connected to a GPIO pin via a driver transistor,
	 * both the ET16315 and the transistor invert the drive, so 1 = on.
	 */
	// Handle ET16315 connected LEDs
	if ((which & LED_RED) || (which & LED_BLUE))
	{
		led = et16315_data->led; // get current values

		if (level)
		{
			led |= which;  // add led to bit mask for ET16315
		}
		else
		{
			led &= ~which;  // remove led from bit mask
		}
		et16315_data->led = led & 0x03;
		dprintk(1, "Setting LED pattern 0x%02x\n", led);
		et16315_set_led(et16315_data, led);
	}
	return ret;
}
/* export for later use in e2_proc */
EXPORT_SYMBOL(fortis_4gSetLED);

/* Function SetBrightness */
int fortis_4gSetBrightness(int level)
{
	int ret = 0;

	dprintk(100, "%s > level = %d\n", __func__, level);

	if (level < 0 || level > MAX_BRIGHT)
	{
		dprintk(1,"Display brightness level %d out of range (0-%d)\n", level, MAX_BRIGHT);
		return -EINVAL;
	}
	et16315_data->brightness = level;
	et16315_set_brightness(et16315_data, level);

	dprintk(100, "%s < RTV = %d\n", __func__, ret);
	return ret;
}

/* Function SetDisplayOnOff */
int fortis_4gSetDisplayOnOff(int on)
{
	dprintk(100, "%s > on = %d\n", __func__, on);

	et16315_data->on = on;
	et16315_set_light(et16315_data, et16315_data->on);
	dprintk(100, "%s <\n", __func__);
	return 0;
}


/* Function WriteString */
#if defined(GPV8000) || defined(EP8000) || defined(EPP8000) || defined(FOREVER_9898HD) || defined(FOREVER_3434HD)
int fortis_4gWriteString(unsigned char *aBuf, int len)
{
	int i;
	int ret = -1;
	unsigned char bBuf[64];

	dprintk(100, "%s > %d\n", __func__, len);

	aBuf[len] = 0;
	dprintk(100, "Display [%s] (len = %d)\n", aBuf, len);

#if defined(VFD_SCROLL)  /* Scroll normally handled in display thread */
	if (len > et16315_data->digits)
	{
		if (len > 63)
		{
			len = 63;
		}
		et16315_set_text(et16315_data + 2, aBuf);  // initial display

		msleep(300);
		for (i = 1; i <= len; i++)
		{
			memset(bBuf, ' ', et16315_data->digits);  // fill buffer with spaces
			memcpy(bBuf, aBuf + i, len);
			et16315_set_text(et16315_data, bBuf);
			msleep(300);
		}
	}
#endif
	et16315_set_text(et16315_data, aBuf);
	ret = 0;

	/* save last string written to fp */
	memcpy(&lastdata.data, aBuf, 64);
	lastdata.length = len;

	dprintk(100, "%s <\n", __func__);
	return ret;
}
#else  // not FOREVER_3434HD, GPV8000, EP8000, EPP8000 or FOREVER_9898HD
int fortis_4gWriteString(unsigned char* aBuf, int len)
{
	dprintk(100, "%s >\n", __func__);
	dprintk(100, "%s <\n", __func__);
	return -EFAULT;
}
#endif

int __init fortis_4gInit_core(void)
{
	int res;

	dprintk(100, "%s >\n", __func__);

	res = fortis_4g_if_init();

	dprintk(100, "%s < RTV = %d\n", __func__, res);
	return res;
}

void fortis_4gExit_core(void)
{
	dprintk(100, "%s >\n", __func__);

	dprintk(5, "[KBD] Unloading...\n");
	keyboard_dev_exit();

	/* free logo LED GPIO pin */
	cleanup_leds();	

	cleanup_et16315();

	dprintk(100, "%s <\n", __func__);
}
// vim:ts=4
