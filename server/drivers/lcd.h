#ifndef LCD_H
#define LCD_H

// Maximum supported sizes
#define LCD_MAX_WIDTH 256
#define LCD_MAX_HEIGHT 256

// Standard supported sizes
#define LCD_STD_WIDTH 20
#define LCD_STD_HEIGHT 4
#define LCD_STD_CELL_WIDTH 5
#define LCD_STD_CELL_HEIGHT 8

void lcd_list_drivers (void);
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

	// Daemonizable?  Usually yes...
	int daemonize;

	// Pointer to next input function...
	//lcd_logical_driver *nextkey;
	void *nextkey;

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

	// Puts up a heartbeat...
	void (*heartbeat) (int type);
	// more?

	// Config file functions, filled by server
	char (*config_get_bool)	(char * sectionname, char * keyname,
				int skip, char default_value);
	int (*config_get_int)	(char * sectionname, char * keyname,
				int skip, int default_value);
	double (*config_get_float) (char * sectionname, char * keyname,
				int skip, double default_value);
	char *(*config_get_string) (char * sectionname, char * keyname,
				int skip, char * default_value);
				// Returns a string in server memory space.
				// Copy this string.
	int (*config_has_section) (char *sectionname);
	int (*config_has_key)	(char *sectionname, char *keyname);

	// Driver private data
	int (*store_private_ptr) (struct lcd_logical_driver * driver, void * private_data);
	void * private_data;	// Filled by server by calling store_private_ptr()
				// Driver should cast this to it's own
				// private structure pointer

} lcd_logical_driver;

typedef struct lcd_physical_driver {
	char *name;
	int (*init) (struct lcd_logical_driver * driver, char *device);
} lcd_physical_driver;

//extern lcd_logical_driver lcd;
extern lcd_logical_driver *lcd_ptr;

#endif
