#ifndef CFONTZPACKET_H
#define CFONTZPACKET_H

#include "lcd.h"

#define DEFAULT_CELL_WIDTH	6
#define DEFAULT_CELL_HEIGHT	8
#define DEFAULT_CONTRAST	560
#define DEFAULT_DEVICE		"/dev/lcd"
#define DEFAULT_SPEED		19200
#define DEFAULT_BRIGHTNESS	1000
#define DEFAULT_OFFBRIGHTNESS	0
#define DEFAULT_SIZE		"16x2"

#define CFA_HAS_FAN		0x0001
#define CFA_HAS_TEMP		0x0002
#define CFA_HAS_4_TEMP_SLOTS	0x0004
#define CFA_HAS_KS0073		0x0008

typedef struct CFA_Model {
	int model;
	char *size;
	int speed;
	const unsigned char *charmap;
	int flags;
} CFA_Model;

MODULE_EXPORT int  CFontzPacket_init (Driver *drvthis);
MODULE_EXPORT void CFontzPacket_close (Driver *drvthis);
MODULE_EXPORT int  CFontzPacket_width (Driver *drvthis);
MODULE_EXPORT int  CFontzPacket_height (Driver *drvthis);
MODULE_EXPORT int  CFontzPacket_cellwidth (Driver *drvthis);
MODULE_EXPORT int  CFontzPacket_cellheight (Driver *drvthis);
MODULE_EXPORT void CFontzPacket_clear (Driver *drvthis);
MODULE_EXPORT void CFontzPacket_flush (Driver *drvthis);
MODULE_EXPORT void CFontzPacket_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void CFontzPacket_chr (Driver *drvthis, int x, int y, char c);
MODULE_EXPORT const char *CFontzPacket_get_key (Driver *drvthis);

MODULE_EXPORT void CFontzPacket_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void CFontzPacket_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void CFontzPacket_num (Driver *drvthis, int x, int num);
MODULE_EXPORT int  CFontzPacket_icon(Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void CFontzPacket_cursor (Driver *drvthis, int x, int y, int state);

MODULE_EXPORT int  CFontzPacket_get_free_chars (Driver *drvthis);
MODULE_EXPORT void CFontzPacket_set_char (Driver *drvthis, int n, unsigned char *dat);

MODULE_EXPORT int  CFontzPacket_get_contrast (Driver *drvthis);
MODULE_EXPORT void CFontzPacket_set_contrast (Driver *drvthis, int promille);
MODULE_EXPORT int  CFontzPacket_get_brightness (Driver *drvthis, int state);
MODULE_EXPORT void CFontzPacket_set_brightness (Driver *drvthis, int state, int promille);
MODULE_EXPORT void CFontzPacket_backlight (Driver *drvthis, int on);
MODULE_EXPORT void CFontzPacket_output (Driver *drvthis, int state);
MODULE_EXPORT const char *CFontzPacket_get_info (Driver *drvthis);

#endif /* CFONTZPACKET_H */
