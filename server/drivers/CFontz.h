#ifndef CFONTZ_H
#define CFONTZ_H

#define DEFAULT_CELL_WIDTH	6
#define DEFAULT_CELL_HEIGHT	8
#define DEFAULT_CONTRAST	560
#define DEFAULT_DEVICE		"/dev/lcd"
#define DEFAULT_SPEED		B9600
#define DEFAULT_BRIGHTNESS	1000
#define DEFAULT_OFFBRIGHTNESS	0
#define DEFAULT_SIZE		"20x4"

#define CFONTZ_Cursor_Home			0x01
#define CFONTZ_Hide_Display			0x02
#define CFONTZ_Restore_Display			0x03
#define CFONTZ_Hide_Cursor			0x04
#define CFONTZ_Show_Underline_Cursor		0x05
#define CFONTZ_Show_Block_Cursor		0x06
#define CFONTZ_Show_Inverting_Block_Cursor	0x07
#define CFONTZ_Backspace			0x08
#define CFONTZ_Control_Boot_Screen		0x09
#define CFONTZ_LineFeed				0x0A
#define CFONTZ_Delete_InPlace			0x0B
#define CFONTZ_FormFeed				0x0C
#define CFONTZ_CarriageReturn			0x0D
#define CFONTZ_Backlight_Control		0x0E
#define CFONTZ_Contrast_Control			0x0F
#define CFONTZ_Set_Cursor_Position		0x11
#define CFONTZ_Horizontal_Bar_Graph		0x12
#define CFONTZ_Scroll_On			0x13
#define CFONTZ_Scroll_Off			0x14
#define CFONTZ_Set_Scrolling_Marquee		0x15
#define CFONTZ_Enable_Scrolling_Marquee		0x16
#define CFONTZ_Wrap_On				0x17
#define CFONTZ_Wrap_Off				0x18
#define CFONTZ_Set_Custom_Char			0x19
#define CFONTZ_Reboot				0x1A
#define CFONTZ_Escape_Sequence_Prefix		0x1B
#define CFONTZ_Large_Block_Number		0x1C
#define CFONTZ_Send_Data_Directly_To_LCD	0x1E
#define CFONTZ_Show_Information_Screen		0x1F


MODULE_EXPORT int  CFontz_init(Driver *drvthis);
MODULE_EXPORT void CFontz_close(Driver *drvthis);
MODULE_EXPORT int  CFontz_width(Driver *drvthis);
MODULE_EXPORT int  CFontz_height(Driver *drvthis);
MODULE_EXPORT int  CFontz_cellwidth(Driver *drvthis);
MODULE_EXPORT int  CFontz_cellheight(Driver *drvthis);
MODULE_EXPORT void CFontz_clear(Driver *drvthis);
MODULE_EXPORT void CFontz_flush(Driver *drvthis);
MODULE_EXPORT void CFontz_string(Driver *drvthis, int x, int y, unsigned char string[]);
MODULE_EXPORT void CFontz_chr(Driver *drvthis, int x, int y, unsigned char c);

MODULE_EXPORT void CFontz_vbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void CFontz_hbar(Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void CFontz_num(Driver *drvthis, int x, int num);
MODULE_EXPORT int  CFontz_icon(Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void CFontz_cursor(Driver *drvthis, int x, int y, int state);

MODULE_EXPORT int  CFontz_get_free_chars(Driver *drvthis);
MODULE_EXPORT void CFontz_set_char(Driver *drvthis, int n, unsigned char *dat);

MODULE_EXPORT int  CFontz_get_contrast(Driver *drvthis);
MODULE_EXPORT void CFontz_set_contrast(Driver *drvthis, int contrast);
MODULE_EXPORT int  CFontz_get_brightness(Driver *drvthis, int state);
MODULE_EXPORT void CFontz_set_brightness(Driver *drvthis, int state, int promille);
MODULE_EXPORT void CFontz_backlight(Driver *drvthis, int on);

#endif
