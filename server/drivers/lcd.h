#ifndef LCD_H
#define LCD_H

// Maximum supported sizes
#define LCD_MAX_WID 256
#define LCD_MAX_WIDTH 256
#define LCD_MAX_HGT 256
#define LCD_MAX_HEIGHT 256

int lcd_init (char *args);
int lcd_add_driver (char *driver, char *args);
int lcd_shutdown ();

/////////////////////////////////////////////////////////////////
// Driver functions / info held here...
//
// Feel free to override any/all functions here with real driver
// functions...
//
typedef struct lcd_logical_driver {
	// Size in cells of the LCD
	int wid, hgt;
	// Size of each LCD cell, in pixels
	int cellwid, cellhgt;
	// Frame buffer...
	char *framebuf;

	// Functions which might be the same for all drivers...
	void (*clear) ();
	void (*string) (int x, int y, char lcd[]);

	void (*chr) (int x, int y, char c);
	void (*vbar) (int x, int len);
	void (*hbar) (int x, int y, int len);
	void (*init_num) ();
	void (*num) (int x, int num);

	// Functions which should probably be implemented in each driver...
	int (*init) (struct lcd_logical_driver * driver, char *args);
	void (*close) ();
	void (*flush) ();
	void (*flush_box) (int lft, int top, int rgt, int bot);
	int (*contrast) (int contrast);
	void (*backlight) (int on);
	void (*output) (int on);
	void (*set_char) (int n, char *dat);
	void (*icon) (int which, char dest);
	void (*init_vbar) ();
	void (*init_hbar) ();
	void (*draw_frame) ();

	// Returns 0 for "no key pressed", or (A-Z).
	char (*getkey) ();

	// Returns pointer to static string.
	char * (*getinfo) ();

	// more?

} lcd_logical_driver;

typedef struct lcd_physical_driver {
	char *name;
	int (*init) (struct lcd_logical_driver * driver, char *device);
} lcd_physical_driver;

extern lcd_logical_driver lcd;

int lcd_drv_init (lcd_logical_driver * driver, char *args);
void lcd_drv_close ();
void lcd_drv_clear ();
void lcd_drv_flush ();
void lcd_drv_string (int x, int y, char string[]);
void lcd_drv_chr (int x, int y, char c);
int lcd_drv_contrast (int contrast);
void lcd_drv_backlight (int on);
void lcd_drv_output (int on);
void lcd_drv_init_vbar ();
void lcd_drv_init_hbar ();
void lcd_drv_init_num ();
void lcd_drv_num (int x, int num);
void lcd_drv_set_char (int n, char *dat);
void lcd_drv_vbar (int x, int len);
void lcd_drv_hbar (int x, int y, int len);
void lcd_drv_icon (int which, char dest);
void lcd_drv_flush_box (int lft, int top, int rgt, int bot);
void lcd_drv_draw_frame ();
char lcd_drv_getkey ();
char *lcd_drv_getinfo ();

#endif
