#ifndef MTXORB_H
#define MTXORB_H

extern lcd_logical_driver *MtxOrb;

int MtxOrb_init (lcd_logical_driver * driver, char *device);
static void MtxOrb_clear ();
static void MtxOrb_close ();
static void MtxOrb_flush ();
static void MtxOrb_flush_box (int lft, int top, int rgt, int bot);
static void MtxOrb_chr (int x, int y, char c);
static int MtxOrb_contrast (int contrast);
static void MtxOrb_backlight (int on);
static void MtxOrb_output (int on);
static void MtxOrb_init_vbar ();
static void MtxOrb_init_hbar ();
static void MtxOrb_vbar (int x, int len);
static void MtxOrb_hbar (int x, int y, int len);
static void MtxOrb_init_num ();
static void MtxOrb_num (int x, int num);
static void MtxOrb_set_char (int n, char *dat);
static void MtxOrb_icon (int which, char dest);
static void MtxOrb_draw_frame (char *dat);
static char MtxOrb_getkey ();
static char * MtxOrb_getinfo ();
static void MtxOrb_heartbeat (int type);

static int MtxOrb_ask_bar (int type);
static void MtxOrb_set_known_char (int car, int type);

// Isn't this function supposed to go away?
static void MtxOrb_init_all (int type);

#define DEFAULT_CONTRAST	120
#define DEFAULT_DEVICE		"/dev/lcd"
#define DEFAULT_SPEED		B19200
#define DEFAULT_LINEWRAP	1
#define DEFAULT_AUTOSCROLL	1
#define DEFAULT_CURSORBLINK	0
#endif

