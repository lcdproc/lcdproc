#ifndef LCDM001_H
#define LCDM001_H

/********************************************************************
  lcdm001.h
******************************************************************/

extern lcd_logical_driver *lcdm001;

int lcdm001_init (struct lcd_logical_driver *driver, char *args);
static void lcdm001_close ();
static void lcdm001_clear ();
static void lcdm001_flush ();
static void lcdm001_string (int x, int y, char string[]);
static void lcdm001_chr (int x, int y, char c);
static int lcdm001_contrast (int contrast);
static void lcdm001_backlight (int on);
static void lcdm001_output (int on);
static void lcdm001_vbar (int x, int len);
static void lcdm001_init_vbar ();
static void lcdm001_hbar (int x, int y, int len);
static void lcdm001_init_hbar ();
static void lcdm001_num (int x, int num);
static void lcdm001_init_num ();
static void lcdm001_set_char (int n, char *dat);
static void lcdm001_icon (int which, char dest);
static void lcdm001_flush_box (int lft, int top, int rgt, int bot);
static void lcdm001_draw_frame (char *dat);
static char lcdm001_getkey ();

#define DEFAULT_DEVICE		"/dev/lcd"
#define DEFAULT_CURSORBLINK	0

/*Heartbeat workaround
  set chars to be displayed instead of "normal" icons*/

#define OPEN_HEART ' '     //This combination is at least visible
#define FILLED_HEART '*'
#define PAD '#'
#endif
