/*
 * SureElec.h
 */
#ifndef SUREELEC_H
#define SUREELEC_H

#define DEFAULT_DEVICE		"/dev/ttyUSB0"
#define DEFAULT_SPEED		9600
#define DEFAULT_LINEWRAP	1
#define DEFAULT_AUTOSCROLL	0
#define DEFAULT_CURSORBLINK	0
#define DEFAULT_SIZE		"16x2"
#define DEFAULT_CONTRAST	480
#define DEFAULT_BRIGHTNESS	480
#define DEFAULT_OFFBRIGHTNESS	100

#define MIN_CONTRAST		0x1
#define MAX_CONTRAST		0xFD

#define MIN_BACKLIGHT		0x1
#define MAX_BACKLIGHT		0xFD

#define CELL_WIDTH		5
#define CELL_HEIGHT		8

#define NUM_CC			8


MODULE_EXPORT int  SureElec_init (Driver *drvthis);
MODULE_EXPORT void SureElec_close (Driver *drvthis);
MODULE_EXPORT int  SureElec_width (Driver *drvthis);
MODULE_EXPORT int  SureElec_height (Driver *drvthis);
MODULE_EXPORT int  SureElec_cellwidth (Driver *drvthis);
MODULE_EXPORT int  SureElec_cellheight (Driver *drvthis);
MODULE_EXPORT void SureElec_clear (Driver *drvthis);
MODULE_EXPORT void SureElec_flush (Driver *drvthis);
MODULE_EXPORT void SureElec_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void SureElec_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void SureElec_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void SureElec_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT int  SureElec_icon (Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void SureElec_num (Driver *drvthis, int x, int num);

MODULE_EXPORT int  SureElec_get_free_chars (Driver *drvthis);
MODULE_EXPORT void SureElec_set_char (Driver *drvthis, int n, unsigned char *dat);

MODULE_EXPORT int  SureElec_get_contrast (Driver *drvthis);
MODULE_EXPORT void SureElec_set_contrast (Driver *drvthis, int promille);
MODULE_EXPORT int  SureElec_get_brightness(Driver *drvthis, int state);
MODULE_EXPORT void SureElec_set_brightness(Driver *drvthis, int state, int promille);
MODULE_EXPORT void SureElec_backlight(struct lcd_logical_driver *drvthis, int on);

#endif   // SUREELEC_H
