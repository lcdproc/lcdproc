#ifndef CFONTZ_H
#define CFONTZ_H

extern lcd_logical_driver *CFontz;

int CFontz_init (lcd_logical_driver * driver, char *device);
void CFontz_close ();
void CFontz_flush ();
void CFontz_flush_box (int lft, int top, int rgt, int bot);
void CFontz_chr (int x, int y, char c);
int CFontz_contrast (int contrast);
void CFontz_backlight (int on);
void CFontz_init_vbar ();
void CFontz_init_hbar ();
void CFontz_vbar (int x, int len);
void CFontz_hbar (int x, int y, int len);
void CFontz_init_num ();
void CFontz_num (int x, int num);
void CFontz_set_char (int n, char *dat);
void CFontz_icon (int which, char dest);
void CFontz_draw_frame (char *dat);
void CFontz_clear (void);
void CFontz_string (int x, int y, char string[]);

#define DEFAULT_CELL_WIDTH 6
#define DEFAULT_CELL_HEIGHT 8
#define DEFAULT_CONTRAST 140
#define DEFAULT_DEVICE "/dev/lcd"
#define DEFAULT_SPEED B9600
#define DEFAULT_BRIGHTNESS 60
#define DEFAULT_OFFBRIGHTNESS 0
#define DEFAULT_SIZE "20x4"

#endif
