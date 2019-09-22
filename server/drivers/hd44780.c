/** \file server/drivers/hd44780.c
 * LCDd \c hd44780 "meta-driver" for Hitachi HD44780 based LCD displays.
 * This is a modular driver that readily allows support for multiple HD44780
 * connection types to be added in a flexible and maintainable manner.
 *
 * This driver also supports the aggregation of multiple displays to form
 * a single virtual display. e.g., Four 16x2 displays can be combined
 * to form a 16x8 display.
 *
 * To add support for additional HD44780 connections:
 * - Add a connection type and mapping to hd44780-drivers.h
 * - Call your initialization routine
 * - Create the low-level driver (use hd44780-ext8bit.c as a starting point)
 * - Modify the makefile
 */

/*-
 * Modular driver created and generic support for multiple displays added
 * Dec 1999, Benjamin Tse <blt@Comports.com>
 *
 * Modified July 2000 by Charles Steinkuehler to use one of 3 methods for delay
 * timing.  I/O reads, gettimeofday, and nanosleep.  Of the three, nanosleep
 * seems to work best, so that's what is set by default.
 *
 * Modified May 2001 by Joris Robijn to add Keypad support.
 *
 * Character mapping for correct display of some special ASCII chars added
 * Sep 2001, Mark Haemmerling <mail@markh.de>.
 *
 * Modified October 2001 to read the configfile.
 *
 * Modified August 2006 by Pillon Matteo <matteo.pillon@email.it> to
 * allow user selecting charmaps
 *
 * Moved the delay timing code by Charles Steinkuehler to timing.h.
 * Guillaume Filion <gfk@logidac.com>, December 2001
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c)  1995,1999,2000 Benjamin Tse <blt@Comports.com>
 *		  1997 Matthias Prinke <m.prinke@trashcan.mcnet.de>
 *		  1998 Richard Rognlie <rrognlie@gamerz.net>
 *		  1999 Andrew McMeikan <andrewm@engineer.com>
 *		  2000 Charles Steinkuehler <cstein@newtek.com>
 *		  2001 Joris Robijn <joris@robijn.net>
 *		  2001 Guillaume Filion <gfk@logidac.com>
 *		  2001 Mark Haemmerling <mail@markh.de>
 *		  2006 Matteo Pillon <matteo.pillon@email.it>
 *		  2007 Peter Marschall <peter@adpm.de>
 */

/* Default parallel port address */
#define LPTPORT	 0x378

/* Autorepeat values */
#define KEYPAD_AUTOREPEAT_DELAY 500
#define KEYPAD_AUTOREPEAT_FREQ 15


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "../elektragen.h"

#include "lcd.h"
#include "lcd_lib.h"
#include "hd44780.h"
#include "shared/report.h"
#include "adv_bignum.h"

#include "timing.h"
#include "hd44780-low.h"
#include "hd44780-drivers.h"
#include "hd44780-charmap.h"

static char *defaultKeyMapDirect[KEYPAD_MAXX] = { "A", "B", "C", "D", "E" };

static char *defaultKeyMapMatrix[KEYPAD_MAXY][KEYPAD_MAXX] = {
		{ "1", "2", "3", "A", "E" },
		{ "4", "5", "6", "B", "F" },
		{ "7", "8", "9", "C", "G" },
		{ "*", "0", "#", "D", "H" },
		{ "",   "",  "",  "",  "" },
		{ "",   "",  "",  "",  "" },
		{ "",   "",  "",  "",  "" },
		{ "",   "",  "",  "",  "" },
		{ "",   "",  "",  "",  "" },
		{ "",   "",  "",  "",  "" },
		{ "",   "",  "",  "",  "" }};


/* Vars for the server core */
MODULE_EXPORT char * api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 1;	/* yes, we have no global
						 * variables (except for
						 * constants) */
MODULE_EXPORT char *symbol_prefix = "HD44780_";


/* Internal functions */
void HD44780_position(Driver *drvthis, int x, int y);
static void uPause(PrivateData *p, int usecs);
unsigned char HD44780_scankeypad(PrivateData *p);
static int parse_span_list(int *spanListArray[], int *spLsize, int *dispOffsets[], int *dOffsize, int *dispSizeArray[], const char *spanlist);


static const struct ModelMapping {
	ElektraEnumHd44780Model elektraModel;
	int model;
} model_mapping[] = {
	{ ELEKTRA_ENUM_HD44780_MODEL_DEFAULT,      HD44780_MODEL_DEFAULT },
	{ ELEKTRA_ENUM_HD44780_MODEL_STANDARD,     HD44780_MODEL_DEFAULT },

	{ ELEKTRA_ENUM_HD44780_MODEL_EXTENDED,     HD44780_MODEL_EXTENDED },
	{ ELEKTRA_ENUM_HD44780_MODEL_KS0073,       HD44780_MODEL_EXTENDED },
	{ ELEKTRA_ENUM_HD44780_MODEL_HD66710,      HD44780_MODEL_EXTENDED },

	{ ELEKTRA_ENUM_HD44780_MODEL_WINSTAR_OLED, HD44780_MODEL_WINSTAR_OLED },
	{ ELEKTRA_ENUM_HD44780_MODEL_WEH00XXYYA,   HD44780_MODEL_WINSTAR_OLED },

	{ ELEKTRA_ENUM_HD44780_MODEL_PT6314_VFD,   HD44780_MODEL_PT6314_VFD }
};

static int map_model ( ElektraEnumHd44780Model elektraModel )
{
	int i;

	for (i=0; i<sizeof(model_mapping)/sizeof(model_mapping[0]); i++) {
		if (model_mapping[i].elektraModel == elektraModel)
			return model_mapping[i].model;
	}

	return -1;
}

static ElektraEnumHd44780Model reverse_map_model ( int type )
{
	int i;

	for (i=0; i<sizeof(model_mapping)/sizeof(model_mapping[0]); i++) {
		if (model_mapping[i].model == type)
			return model_mapping[i].elektraModel;
	}

	return -1;
}

static const struct BacklightValueMapping {
	ElektraEnumHd44780Backlightmode elektraMode;
	int value;
} bl_value_mapping[] = {
	{ELEKTRA_ENUM_HD44780_BACKLIGHTMODE_EXTERNAL,     BACKLIGHT_EXTERNAL_PIN },
	{ELEKTRA_ENUM_HD44780_BACKLIGHTMODE_INTERNAL,     BACKLIGHT_INTERNAL     },
	{ELEKTRA_ENUM_HD44780_BACKLIGHTMODE_INTERNAL_CMDS, BACKLIGHT_CONFIG_CMDS },
};

static int map_backlight_mode(ElektraEnumHd44780Backlightmode elektraMode)
{
		for (int i=0; i<sizeof(bl_value_mapping)/sizeof(bl_value_mapping[0]); i++) {
			if (elektraMode == bl_value_mapping[i].elektraMode) {
				return bl_value_mapping[i].value;
			}
		}

		return -1;
}


/* Reads from configuration setting for `Backlight` option, which can occur multiple times
 * Returns -1 if configuration value is not valid */
static int get_config_backlight_type(Driver *drvthis, const Hd44780DriverConfig * config, Elektra * elektra)
{
	PrivateData *p = drvthis->private_data;

	if(config->backlight == ELEKTRA_ENUM_HD44780_BACKLIGHT_DEFAULT)
	{
		return p->model == HD44780_MODEL_WINSTAR_OLED || p->model == HD44780_MODEL_PT6314_VFD ? BACKLIGHT_INTERNAL : BACKLIGHT_NONE;
	}

	if(config->backlight == ELEKTRA_ENUM_HD44780_BACKLIGHT_DISABLED)
	{
		return BACKLIGHT_NONE;
	}

	int result = BACKLIGHT_NONE;
	kdb_long_long_t size = elektraSizeV(elektra, CONF_HD44780_BACKLIGHTMODE, drvthis->index);
	for (kdb_long_long_t i = 0; i < size; ++i)
	{
		ElektraEnumHd44780Backlightmode elektraMode = elektraGetV(elektra, CONF_HD44780_BACKLIGHTMODE, drvthis->index, i);
		int mode = map_backlight_mode(elektraMode);
		if (mode < 0) {
			report(RPT_ERR, "%s ("ELEKTRA_LONG_LONG_F"): unknown Backlight mode: %d", drvthis->name, drvthis->index, elektraMode);
			return -1;
		}
		result |= mode;
	}

	return result;
}

/* reports textually setting of backlight */
static void report_backlight_type(int report_level, int backlight_type)
{
	int i;

	/* first find just whole option name */
	for (i=0; i<sizeof(bl_value_mapping)/sizeof(bl_value_mapping[0]); i++) {
		if (bl_value_mapping[i].value == backlight_type) {
			report(report_level, "HD44780: backlight: %s", ELEKTRA_TO_CONST_STRING(EnumHd44780Backlightmode)(bl_value_mapping[i].elektraMode));
			return;
		}
	}

	/* no single option value found, search for combinations */
	char *result = elektraStrDup("");

	for (i=0; i<sizeof(bl_value_mapping)/sizeof(bl_value_mapping[0]); i++) {

		if (bl_value_mapping[i].value & backlight_type) {		
			char *tmp = elektraFormat("%s%s,", result, ELEKTRA_TO_CONST_STRING(EnumHd44780Backlightmode)(bl_value_mapping[i].elektraMode));
			elektraFree(result);
			result = tmp;
			
			backlight_type &= ~bl_value_mapping[i].value;
		}
	}

	if (backlight_type != 0) {
		char *tmp = elektraFormat("%s%08x,", result, backlight_type);
		elektraFree(result);
		result = tmp;
	}

	result[strlen(result) - 1] = '\0'; // replace comma at end

	report(report_level, "HD44780: backlight: %s", result);
}

static const ConnectionMapping * map_connection_type(Hd44780ConnectionType type)
{
	for(int i = 0; connectionMapping[i].init_fn != NULL; ++i)
	{
		if(connectionMapping[i].elektraType == type)
		{
			return &connectionMapping[i];
		}
	}
	return NULL;
}

static const char * model_name(ElektraEnumHd44780Model model)
{
	return ELEKTRA_TO_CONST_STRING(EnumHd44780Model)(model);
}

/**
 * Initialize the driver.
 * Initialize common part of drive & call sub-initialization
 * routine depending on connection type.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
HD44780_init(Driver *drvthis, Elektra * elektra)
{
	/* TODO: single point of return */
	int i = 0;
	int (*init_fn) (Driver *drvthis, const Hd44780DriverConfig *config) = NULL;
	int if_type = IF_TYPE_UNKNOWN;
	PrivateData *p;

	/* Alocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* initialize private data */
	p->cellheight = 8;	/* Do not change this !!! This is a
				 * controller property, not a display
				 * property !!! */
	p->cellwidth = 5;
	p->ccmode = standard;
	p->backlightstate = -1;	/* Init to invalid value */
	p->fd = -1;


	/* READ THE CONFIG */
	Hd44780DriverConfig config;
	elektraFillStructV(elektra, &config, CONF_HD44780, drvthis->index);

	// TODO (elektra): use hexnumber?
	p->port			= (unsigned int)strtoul(config.port, NULL, 16); /* works because, spec enforces hex string */
	p->model		= map_model(config.model);
	if (p->model < 0) {
		report(RPT_ERR, "%s ("ELEKTRA_LONG_LONG_F"): unknown Model: %s", drvthis->name, drvthis->index, model_name(config.model));
		return -1;
	}
	/* config file compability stuff */
	if (p->model == HD44780_MODEL_DEFAULT) {
		if (config.extendedmode)
			p->model = HD44780_MODEL_EXTENDED;
	}
	else {
		if (config.extendedmode != (p->model == HD44780_MODEL_EXTENDED)) {
			report(RPT_ERR, "%s ("ELEKTRA_LONG_LONG_F"): conflicting Model %s and extended mode: %d", drvthis->name, drvthis->index, model_name(config.model), config.extendedmode);
			return -1;
		}
	}

	// TODO (elektra): use hexnumber?
	p->line_address 	= (int)strtol(config.lineaddress, NULL, 16); /* works because, spec enforces hex string */
	p->have_keypad		= config.keypad;

	/* parse backlight option. Default is model specific */
	p->backlight_type	= get_config_backlight_type(drvthis, &config, elektra);
	if (p->backlight_type < 0) {
		/* error already logged in get_config_backlight_type() */
		return -1;
	}

	// TODO (elektra): use hexnumber?
	p->backlight_cmd_on	= (int)strtol(config.backlightcmdon, NULL, 16); /* works because, spec enforces hex string */
	p->backlight_cmd_off	= (int)strtol(config.backlightcmdoff, NULL, 16); /* works because, spec enforces hex string */
	if ((p->backlight_type & BACKLIGHT_CONFIG_CMDS) && (!p->backlight_cmd_on || !p->backlight_cmd_off)) {
		report(RPT_ERR, "%s ("ELEKTRA_LONG_LONG_F"): No commands for enabling or disabling backlight specified for backlight type internalCmds", drvthis->name, drvthis->index);
		return -1;
	}

	p->have_output		= config.outputport;
	p->delayMult 		= config.delaymult;
	p->delayBus 		= config.delaybus;
	p->lastline 		= config.lastline;

	p->nextrefresh		= 0;
	p->refreshdisplay 	= config.refreshdisplay;
	p->nextkeepalive	= 0;
	p->keepalivedisplay	= config.keepalivedisplay;

	/* Get and search for the connection type */
	const ConnectionMapping * connectionMapping = map_connection_type(config.connectiontype);

	if (connectionMapping == NULL) {
		report(RPT_ERR, "%s ("ELEKTRA_LONG_LONG_F"): unknown ConnectionType: %s", drvthis->name, drvthis->index, ELEKTRA_TO_CONST_STRING(EnumHd44780ConnectionType)(config.connectiontype));
		return -1;
	}

	/* set connection type */
	p->connectiontype = connectionMapping->connectiontype;

	report(RPT_INFO, "HD44780: using ConnectionType: %s", ELEKTRA_TO_CONST_STRING(EnumHd44780ConnectionType)(config.connectiontype));

	if_type = connectionMapping->if_type;
	init_fn = connectionMapping->init_fn;
	
	report(RPT_INFO, "HD44780: selecting Model: %d", reverse_map_model(p->model));
	report_backlight_type(RPT_INFO, p->backlight_type);
	if (p->backlight_type & BACKLIGHT_CONFIG_CMDS) {
		report(RPT_INFO, "HD44780: backlight config commands: on: %02x, off: %02x", p->backlight_cmd_on, p->backlight_cmd_off);
	}

	/* Get and parse vspan only when specified */
	if (strlen(config.vspan) > 0) {
		if (parse_span_list(&(p->spanList), &(p->numLines), &(p->dispVOffset), &(p->numDisplays), &(p->dispSizes), config.vspan) == -1) {
			report(RPT_ERR, "%s ("ELEKTRA_LONG_LONG_F"): invalid vspan value: %s", drvthis->name, drvthis->index, config.vspan);
			return -1;
		}
	}

	/* Get and parse size */
	if (sscanf(config.size, "%dx%d", &(p->width), &(p->height)) != 2
	    || (p->width <= 0) || (p->width > LCD_MAX_WIDTH)
	    || (p->height <= 0) || (p->height > LCD_MAX_HEIGHT)) {
		report(RPT_ERR, "%s ("ELEKTRA_LONG_LONG_F"): cannot read Size %s", drvthis->name, drvthis->index, config.size);
	}

	/* set contrast */
	p->contrast = config.contrast;

	/* set brightness */
	p->brightness = config.brightness;

	/* set backlight-off "brightness" */
	p->offbrightness = config.offbrightness;

	/* default case for when spans aren't indicated */
	if (p->numLines == 0) {
		if ((p->spanList = (int *) calloc(sizeof(int), p->height)) != NULL) {
			int i;
			for (i = 0; i < p->height; ++i) {
				p->spanList[i] = 1;
				p->numLines = p->height;
			}
		} else
			report(RPT_ERR, "%s ("ELEKTRA_LONG_LONG_F"): error allocing", drvthis->name, drvthis->index);
	}
	else {
		/* sanity check against p->height */
		if (p->numLines != p->height)
			report(RPT_ERR, "%s ("ELEKTRA_LONG_LONG_F"): height in Size does not match vSpan", drvthis->name, drvthis->index);
	}

	if (p->numDisplays == 0) {
		if (((p->dispVOffset = (int *) calloc(1, sizeof(int))) != NULL) &&
	            ((p->dispSizes = (int *) calloc(1, sizeof(int))) != NULL)) {
			p->dispVOffset[0] = 0;
			p->dispSizes[0] = p->height;
			p->numDisplays = 1;
		} else
			report(RPT_ERR, "%s ("ELEKTRA_LONG_LONG_F"): error mallocing", drvthis->name, drvthis->index);
	}

	/* Set up timing */
	if (timing_init() == -1) {
		report(RPT_ERR, "%s ("ELEKTRA_LONG_LONG_F"): timing_init() failed (%s)", drvthis->name, drvthis->index, strerror(errno));
		return -1;
	}

	/* Allocate framebuffer */
	p->framebuf = (unsigned char *) calloc(p->width * p->height, sizeof(char));
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s ("ELEKTRA_LONG_LONG_F"): unable to allocate framebuffer", drvthis->name, drvthis->index);
		//HD44780_close();
		return -1;
	}

	/* Allocate and clear the buffer for incremental updates */
	p->backingstore = (unsigned char *) calloc(p->width * p->height, sizeof(char));
	if (p->backingstore == NULL) {
		report(RPT_ERR, "%s ("ELEKTRA_LONG_LONG_F"): unable to allocate framebuffer backing store", drvthis->name, drvthis->index);
		return -1;
	}

	/* Keypad ? */
	if (p->have_keypad) {
		int x, y;

		/* Read keymap */
		for (x = 0; x < KEYPAD_MAXX; x++) {
			const char * key = elektraGetV(elektra, CONF_HD44780_KEYDIRECT, drvthis->index, x);

			/* Was a key specified in the config file ? */
			if (strlen(key) == 0) {
				key = defaultKeyMapDirect[x];
			} else {
				report(RPT_INFO, "HD44780: Direct key %d: \"%s\"", x, key);
			}

			p->keyMapDirect[x] = strdup(key);

			for (y = 0; y < KEYPAD_MAXY; y++) {
				key = elektraGetV(elektra, CONF_HD44780_KEYMATRIX, drvthis->index, x, y);
				
				/* Was a key specified in the config file ? */
				if (strlen(key) == 0) {
					key = defaultKeyMapMatrix[y][x];
				} else {
					report(RPT_INFO, "HD44780: Matrix key %d %d: \"%s\"", x+1, y+1, key);
				}

				p->keyMapMatrix[y][x] = strdup(key);
			}
		}
	}

	/* Get configured charmap */
	p->charmap = charmap_get_index(config.charmap);
	if (p->charmap == -1) {
		report(RPT_ERR, "%s ("ELEKTRA_LONG_LONG_F"): Charmap %d is unknown", drvthis->name, drvthis->index, config.charmap);
		report(RPT_ERR, "%s ("ELEKTRA_LONG_LONG_F"): Available charmaps:", drvthis->name, drvthis->index);
		for (i = 0; i < (sizeof(available_charmaps)/sizeof(struct charmap)); i++) {
			report(RPT_ERR, " %s", ELEKTRA_TO_CONST_STRING(EnumHd44780Charmap)(available_charmaps[i].elektraCharmap));
		}
		return -1;
	}
	report(RPT_INFO, "%s ("ELEKTRA_LONG_LONG_F"): Using %s charmap", drvthis->name, drvthis->index, ELEKTRA_TO_CONST_STRING(EnumHd44780Charmap)(config.charmap));

	/* Get configured font bank */
	p->font_bank = config.fontbank;

	/* Output latch state - init to a non-valid value */
	p->output_state = 999999;

	/* allocate local function pointers */
	if ((p->hd44780_functions = (HD44780_functions *) calloc(1, sizeof(HD44780_functions))) == NULL) {
		report(RPT_ERR, "%s ("ELEKTRA_LONG_LONG_F"): error mallocing", drvthis->name, drvthis->index);
		return -1;
	}
	/*
	 * pre-set local function pointers (may be overridden by local init
	 * function)
	 */
	p->hd44780_functions->uPause = uPause;
	p->hd44780_functions->drv_report = report;
	p->hd44780_functions->drv_debug = debug;
	p->hd44780_functions->senddata = NULL;
	p->hd44780_functions->backlight = NULL;
	p->hd44780_functions->set_contrast = NULL;
	p->hd44780_functions->readkeypad = NULL;
	p->hd44780_functions->scankeypad = NULL;
	p->hd44780_functions->output = NULL;
	p->hd44780_functions->close = NULL;
	p->hd44780_functions->flush = NULL;

	/* Do local (=connection type specific) display init */
	if (init_fn(drvthis, &config) != 0)
		return -1;

	/* consistency check: fail if local senddata function was not defined */
	if (p->hd44780_functions->senddata == NULL) {
		report(RPT_ERR, "%s: incomplete functions for connection type",
				drvthis->name);
		return -1;
	}

	/* set scankeypad function if local readkeypad function is defined */
	if ((p->hd44780_functions->readkeypad != NULL) &&
	    (p->hd44780_functions->scankeypad == NULL)) {
		p->hd44780_functions->scankeypad = HD44780_scankeypad;
	}

	/* consistency check: no local keypad function => no keypad */
	if (p->hd44780_functions->scankeypad == NULL)
		p->have_keypad = 0;

	/* consistency check: no local backlight function => no external backlight
	 * still backlight might be set using internal commands of display independant
	 * of connection type*/
	if (p->hd44780_functions->backlight == NULL)
		set_have_backlight_pin(p, 0);

	/* consistency check: no local output function => no output */
	if (p->hd44780_functions->output == NULL)
		p->have_output = 0;

	/* set contrast */
	HD44780_set_contrast(drvthis, p->contrast);

	HD44780_clear(drvthis);

	return 0;
}


/**
 * Common initialisation sequence:
 * - set twoline mode, small (5x8) characters & 4- or 8-bit mode
 * - set display on & cursor off, not blinking,
 * - set output mode to left-to-right & turn off display scolling
 * - clear display & move cursor home
 *
 * \param p       Pointer to PrivateData structure.
 * \param if_bit  Command bits for 4- resp. 8-bit mode
 */
void
common_init(PrivateData *p, unsigned char if_bit)
{
	/* Set initial brightness according to Brightness setting.
	 * This assumes that initially backlight is on (or is not used at all) */
	int init_brightness = p->brightness;

	unsigned char cmd_funcset =  FUNCSET | if_bit | TWOLINE | SMALLCHAR;
	if (has_extended_mode(p)) {
		/* Set up extended mode */
		p->hd44780_functions->senddata(p, 0, RS_INSTR, cmd_funcset | EXTREG);
		p->hd44780_functions->uPause(p, 40);
		p->hd44780_functions->senddata(p, 0, RS_INSTR, EXTMODESET | FOURLINE);
		p->hd44780_functions->uPause(p, 40);
	}

	if (p->model == HD44780_MODEL_PT6314_VFD) {
		cmd_funcset &= ~PT6314_BRIGHT_MASK;

		if (init_brightness >= 3 * (MAX_BRIGHTNESS / 4))
			cmd_funcset |= PT6314_BRIGHT_100; /* = 0x00 */
		else if (init_brightness >= MAX_BRIGHTNESS / 2)
			cmd_funcset |= PT6314_BRIGHT_75; /* = 0x01 */
		else if (init_brightness > MAX_BRIGHTNESS / 4 && init_brightness > DEFAULT_OFFBRIGHTNESS)
			/* Idea is to set to lowest default */
			cmd_funcset |= PT6314_BRIGHT_50; /* = 0x02 */
		else
			cmd_funcset |= PT6314_BRIGHT_25; /* = 0x03 */
	}
	else {
		/* by default font_bank is zero
		 * this is ignored on most of displays, except of PT6314 VFD,
		 * which this value means completely different thing, but is handled above */
		cmd_funcset |=  p->font_bank;
	}

	/* save used cmd for FUNCSET */
	p->func_set_mode = cmd_funcset;

	/* set up standard mode.  */
	p->hd44780_functions->senddata(p, 0, RS_INSTR, cmd_funcset);
	p->hd44780_functions->uPause(p, 40);

	/* Turn off display, as manipulatimg below can cause some garbage on screen */
	p->hd44780_functions->senddata(p, 0, RS_INSTR, ONOFFCTRL | DISPOFF | CURSOROFF | CURSORNOBLINK);
	p->hd44780_functions->uPause(p, 40);

	p->hd44780_functions->senddata(p, 0, RS_INSTR, CLEAR);
	/* winstar OLEDs require 6.2ms for this command, according to spec */
	p->hd44780_functions->uPause(p, (p->model == HD44780_MODEL_WINSTAR_OLED) ? 6200 : 1600);

	if (p->model == HD44780_MODEL_WINSTAR_OLED) {
		/* For WINSTAR OLED displays need to set TEXT mode and additionally level of brigtness.
		 * It is particularly important on reinitialization without powering off it first */
		unsigned char pwr = WINST_PWROFF;
		if (init_brightness >= MAX_BRIGHTNESS / 2) {
			pwr = WINST_PWRON;
		}
		p->hd44780_functions->senddata(p, 0, RS_INSTR, WINST_MODESET | WINST_TEXTMODE | pwr);
		p->hd44780_functions->uPause(p, 500);
	}

	p->hd44780_functions->senddata(p, 0, RS_INSTR, ENTRYMODE | E_MOVERIGHT | NOSCROLL);
	p->hd44780_functions->uPause(p, 40);
	p->hd44780_functions->senddata(p, 0, RS_INSTR, HOMECURSOR);
	p->hd44780_functions->uPause(p, 1600);

	/* Turn on display again */
	p->hd44780_functions->senddata(p, 0, RS_INSTR, ONOFFCTRL | DISPON | CURSOROFF | CURSORNOBLINK);
	p->hd44780_functions->uPause(p, 40);

	if (p->hd44780_functions->flush != NULL)
		p->hd44780_functions->flush(p);
}


/**
 * Delay a number of microseconds.
 * \param p  Pointer to PrivateData structure.
 * \param usecs  Number of micro-seconds to sleep.
 */
static void
uPause(PrivateData *p, int usecs)
{
	timing_uPause(usecs * p->delayMult);
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
HD44780_close(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	if (p != NULL) {
		if (p->hd44780_functions->close != NULL)
			p->hd44780_functions->close(p);

		if (p->framebuf)
			free(p->framebuf);

		if (p->backingstore)
			free(p->backingstore);

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}


/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is wide.
 */
MODULE_EXPORT int
HD44780_width(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	return p->width;
}


/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is high.
 */
MODULE_EXPORT int
HD44780_height(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	return p->height;
}


/**
 * Return the width of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of pixel columns a character cell is wide.
 */
MODULE_EXPORT int
HD44780_cellwidth(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	return p->cellwidth;
}


/**
 * Return the height of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of pixel lines a character cell is high.
 */
MODULE_EXPORT int
HD44780_cellheight(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	return p->cellheight;
}


/**
 * Set position (not part of API).
 * \param drvthis  Pointer to driver structure.
 * \param x        X-coordinate to go to.
 * \param y        Y-coordinate to go to.
 */
void
HD44780_position(Driver *drvthis, int x, int y)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int dispID = p->spanList[y];
	int relY = y - p->dispVOffset[dispID - 1];
	int DDaddr;

	if (has_extended_mode(p)) {
		/* Linear addressing, each line starts 0x20 higher. */
		DDaddr = x + relY * p->line_address;
	} else {
		/*
		 * 16x1 is a special case: char 0 starts at 0x00, but char 8
		 * starts at 0x40.
		 */
		if (p->dispSizes[dispID - 1] == 1 && p->width == 16) {
			if (x >= 8) {
				x -= 8;
				relY = 1;
			}
		}

		/*
		 * Line 1 starts at 0x00, line 2 start at 0x40, line 3
		 * continues from line 1 and line 4 continues from line 2.
		 */
		DDaddr = x + (relY % 2) * 0x40;
		if ((relY % 4) >= 2)
			DDaddr += p->width;
	}
	p->hd44780_functions->senddata(p, dispID, RS_INSTR, POSITION | DDaddr);
	p->hd44780_functions->uPause(p, 40);  /* Minimum exec time for all commands */
	if (p->hd44780_functions->flush != NULL)
		p->hd44780_functions->flush(p);
}


/**
 * Flush data on screen to the LCD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
HD44780_flush(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int x, y;
	int i;
	int count;
	char refreshNow = 0;
	char keepaliveNow = 0;
	time_t now = time(NULL);

	/* force full refresh of display */
	if ((p->refreshdisplay > 0) && (now > p->nextrefresh)) {
		refreshNow = 1;
		p->nextrefresh = now + p->refreshdisplay;
	}
	/* keepalive refresh of display */
	if ((p->keepalivedisplay > 0) && (now > p->nextkeepalive)) {
		keepaliveNow = 1;
		p->nextkeepalive = now + p->keepalivedisplay;
	}

	/*
	 * LCD update algorithm: For each line skip over leading and trailing
	 * identical portions of the line. Then send everything in between.
	 * This will also update unchanged parts in the middle but is still
	 * faster than the old algorithm, especially with devices using the
	 * transmit buffer.
	 */
	count = 0;
	for (y = 0; y < p->height; y++) {
		int drawing;
		int dispID = p->spanList[y];

		/* set pointers to start of the line */
		unsigned char *sp = p->framebuf + (y * p->width);
		unsigned char *sq = p->backingstore + (y * p->width);

		/* set pointers to end of the line */
		unsigned char *ep = sp + (p->width - 1);
		unsigned char *eq = sq + (p->width - 1);

		/* On forced refresh update everything */
		if (refreshNow || keepaliveNow) {
			x = 0;
		}
		else {
			/* find begin and end of differences */
			for (x = 0; (sp <= ep) && (*sp == *sq); sp++, sq++, x++)
			  ;
			for (; (ep >= sp) && (*ep == *eq); ep--, eq--)
			  ;
		}

		/* there are differences, ... */
		if (sp <= ep) {
			for (drawing = 0; sp <= ep; x++, sp++, sq++) {
				 /* x%8 is for 16x1 displays only ! */
				if (!drawing || (p->dispSizes[dispID-1] == 1 && p->width == 16 && (x % 8 == 0))) {
					drawing = 1;
					HD44780_position(drvthis,x,y);
				}
				p->hd44780_functions->senddata(p, dispID, RS_DATA, *sp);
				p->hd44780_functions->uPause(p, 40);  /* Minimum exec time for all commands */
				*sq = *sp;	/* Update backing store */
				count++;
			}
		}
	}
	debug(RPT_DEBUG, "HD44780: flushed %d chars", count);

	/* Check which definable chars we need to update */
	count = 0;
	for (i = 0; i < NUM_CCs; i++) {
		if (!p->cc[i].clean) {
			int row;

			/* Tell the HD44780 we will redefine char number i */
			p->hd44780_functions->senddata(p, 0, RS_INSTR, SETCHAR | i * 8);
			p->hd44780_functions->uPause(p, 40);  /* Minimum exec time for all commands */

			/* Send the subsequent rows */
			for (row = 0; row < p->cellheight; row++) {
				p->hd44780_functions->senddata(p, 0, RS_DATA, p->cc[i].cache[row]);
				p->hd44780_functions->uPause(p, 40);  /* Minimum exec time for all commands */
			}
			p->cc[i].clean = 1;	/* mark as clean */
			count++;
		}
	}
	if (p->hd44780_functions->flush != NULL)
		p->hd44780_functions->flush(p);
	debug(RPT_DEBUG, "%s ("ELEKTRA_LONG_LONG_F"): flushed %d custom chars", drvthis->name, drvthis->index, count);
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
HD44780_clear(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
	p->ccmode = standard;
}


/**
 * Print a character on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param ch       Character that gets written.
 */
MODULE_EXPORT void
HD44780_chr(Driver *drvthis, int x, int y, char ch)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	y--;
	x--;

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[(y * p->width) + x] =
			available_charmaps[p->charmap].charmap[(unsigned char) ch];
}


/**
 * Print a string on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param string   String that gets written.
 */
MODULE_EXPORT void
HD44780_string(Driver *drvthis, int x, int y, const char string[])
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int i;

	x--;			/* Convert 1-based coords to 0-based */
	y--;

	if ((y < 0) || (y >= p->height))
		return;

	for (i = 0; (string[i] != '\0') && (x < p->width); i++, x++) {
		if (x >= 0)	/* no write left of left border */
			p->framebuf[(y * p->width) + x] =
				available_charmaps[p->charmap].charmap[(unsigned char) string[i]];
	}
}


/**
 * Get current LCD contrast.
 * This is only the locally stored contrast, the contrast value
 * cannot be retrieved from the LCD.
 * \param drvthis  Pointer to driver structure.
 * \return  Stored contrast in promille.
 */
MODULE_EXPORT int
HD44780_get_contrast(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->contrast;
}


/**
 * Change LCD contrast.
 * \param drvthis  Pointer to driver structure.
 * \param promille New contrast value in promille.
 */
MODULE_EXPORT void
HD44780_set_contrast (Driver *drvthis, int promille)
{
	PrivateData *p = drvthis->private_data;
	unsigned char contrast_byte;

	/* Check it */
	if (promille < 0 || promille > 1000)
		return;

	/* store the software value since there is not get */
	p->contrast = promille;

	/* map range [0, 1000] -> [0, 255] (for one byte)) */
	contrast_byte = (255 * promille) / 1000;

	/* call local function */
	if (p->hd44780_functions->set_contrast != NULL)
		p->hd44780_functions->set_contrast(p, contrast_byte);
}


/**
 * Retrieve brightness.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want the value.
 * \return Stored brightness in promille.
 */
MODULE_EXPORT int
HD44780_get_brightness(Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;

	return (state == BACKLIGHT_ON) ? p->brightness : p->offbrightness;
}


/**
 * Set on/off brightness.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want to store the value.
 * \param promille New brightness in promille.
 */
MODULE_EXPORT void
HD44780_set_brightness(Driver *drvthis, int state, int promille)
{
	PrivateData *p = drvthis->private_data;

	/* Check it */
	if (promille < 0 || promille > MAX_BRIGHTNESS)
		return;

	/* store the software value since there is not get */
	if (state == BACKLIGHT_ON) {
		p->brightness = promille;
	}
	else {
		p->offbrightness = promille;
	}

	/*
	 * Make last backlight state invalid to force update on next
	 * rendering
	 */
	p->backlightstate = -1;
}

/** Sets internal backlight state using internal commands (based on configured model) */
static void
hd44780_set_backlight_internal(PrivateData *p, int state)
{
	unsigned char cmd = 0;
	int brightness = state ? p->brightness : p->offbrightness;
	switch (p->model) {
		case HD44780_MODEL_WINSTAR_OLED:
			cmd = WINST_MODESET | WINST_TEXTMODE \
			    | (brightness >= MAX_BRIGHTNESS/2 ? WINST_PWRON : WINST_PWROFF);
			p->hd44780_functions->senddata(p, 0, RS_INSTR, cmd);
			report(RPT_DEBUG, "hd44780: setting BL %s using winstar_oled internal cmd: %02x", state ? "on" : "off", cmd);
			break;

		case HD44780_MODEL_PT6314_VFD:
			cmd =  p->func_set_mode & ~PT6314_BRIGHT_MASK;
			if (brightness >= 3 * MAX_BRIGHTNESS / 4)
				cmd |= PT6314_BRIGHT_100; /* = 0x00 */
			else if (brightness >= MAX_BRIGHTNESS / 2)
				cmd |= PT6314_BRIGHT_75; /* = 0x01 */
			else if (brightness > MAX_BRIGHTNESS / 4)
				cmd |= PT6314_BRIGHT_50; /* = 0x02 */
			else
				cmd |= PT6314_BRIGHT_25; /* = 0x03 */
			p->hd44780_functions->senddata(p, 0, RS_INSTR, cmd);
			report(RPT_DEBUG, "hd44780: setting BL %s using pt6314_vfd internal cmd: %02x", state ? "on" : "off", cmd);
			break;

		default:
			/* do nothing for other models */
			break;
	}
}

static void
hd44780_set_backlight_config_cmds(PrivateData *p, int state)
{
	int brightness = state ? p->brightness : p->offbrightness;
	int i, shift;
	unsigned char cmd;

	/* Assume two levels of brightness */
	if (brightness >= MAX_BRIGHTNESS/2) {
		for (i=0; i<sizeof(p->backlight_cmd_on); i++) {
			shift = (sizeof(p->backlight_cmd_on) - i - 1)*8;
			cmd =  (unsigned char)(p->backlight_cmd_on >> shift) & 0xff;
			if (cmd) {
				report(RPT_DEBUG, "hd44780: setting BL on using cmd %02x", cmd);
				p->hd44780_functions->senddata(p, 0, RS_INSTR, cmd);
			}
		}
	}
	else {
		for (i=0; i<sizeof(p->backlight_cmd_off); i++) {
			shift = (sizeof(p->backlight_cmd_on) - i - 1)*8;
			cmd =  (unsigned char)(p->backlight_cmd_off >> shift) & 0xff;
			if (cmd) {
				report(RPT_DEBUG, "hd44780: setting BL off using cmd %02x", cmd);
				p->hd44780_functions->senddata(p, 0, RS_INSTR, cmd);
			}
		}
	}
}

/**
 * Turn the LCD backlight on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
HD44780_backlight(Driver *drvthis, int on)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	/* Immediately return if no way of setting backlight is available
           or no change is necessary */
	if (!p->backlight_type|| p->backlightstate == on)
		return;

	if (p->hd44780_functions->backlight != NULL)
		p->hd44780_functions->backlight(p, on);

	if (p->backlight_type & BACKLIGHT_INTERNAL)
		hd44780_set_backlight_internal(p, on);

	if (p->backlight_type & BACKLIGHT_CONFIG_CMDS)
		hd44780_set_backlight_config_cmds(p, on);

	p->backlightstate = on;
}


/**
 * Draw a vertical bar bottom-up.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is high at 100%
 * \param promille Current height level of the bar in promille.
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void
HD44780_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	if (p->ccmode != vbar) {
		unsigned char vBar[p->cellheight];
		int i;

		if (p->ccmode != standard) {
			/* Not supported(yet) */
			report(RPT_WARNING, "%s: vbar: cannot combine two modes using user-defined characters",
				drvthis->name);
			return;
		}
		p->ccmode = vbar;

		memset(vBar, 0x00, sizeof(vBar));

		for (i = 1; i < p->cellheight; i++) {
			/* add pixel line per pixel line ... */
			vBar[p->cellheight - i] = 0xFF;
			HD44780_set_char(drvthis, i, vBar);
		}
	}

	lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0);
}


/**
 * Draw a horizontal bar to the right.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is long at 100%
 * \param promille Current length level of the bar in promille.
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void
HD44780_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	if (p->ccmode != hbar) {
		unsigned char hBar[p->cellheight];
		int i;

		if (p->ccmode != standard) {
			/* Not supported(yet) */
			report(RPT_WARNING, "%s: hbar: cannot combine two modes using user-defined characters",
			      drvthis->name);
			return;
		}

		p->ccmode = hbar;

		for (i = 1; i <= p->cellwidth; i++) {
			/* fill pixel columns from left to right. */
			memset(hBar, 0xFF & ~((1 << (p->cellwidth - i)) - 1), sizeof(hBar));
			HD44780_set_char(drvthis, i, hBar);
		}
	}

	lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
}


/**
 * Write a big number to the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
MODULE_EXPORT void
HD44780_num(Driver *drvthis, int x, int num)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int do_init = 0;

	if ((num < 0) || (num > 10))
		return;

	if (p->ccmode != bignum) {
		if (p->ccmode != standard) {
			/* Not supported (yet) */
			report(RPT_WARNING, "%s: num: cannot combine two modes using user-defined characters",
					drvthis->name);
			return;
		}

		p->ccmode = bignum;

		do_init = 1;
	}

	/* Lib_adv_bignum does everything needed to show the bignumbers. */
	lib_adv_bignum(drvthis, x, num, 0, do_init);
}


/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of custom characters (always NUM_CCs).
 */
MODULE_EXPORT int
HD44780_get_free_chars(Driver *drvthis)
{
	return NUM_CCs;
}


/**
 * Define a custom character and write it to the LCD.
 * \param drvthis  Pointer to driver structure.
 * \param n        Custom character to define [0 - (NUM_CCs-1)].
 * \param dat      Array of 8 (=cellheight) bytes, each representing a pixel row
 *                 starting from the top to bottom.
 *                 The bits in each byte represent the pixels where the LSB
 *                 (least significant bit) is the rightmost pixel in each pixel row.
 */
MODULE_EXPORT void
HD44780_set_char(Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	unsigned char mask = (1 << p->cellwidth) - 1;
	int row;

	if ((n < 0) || (n >= NUM_CCs))
		return;
	if (!dat)
		return;

	for (row = 0; row < p->cellheight; row++) {
		int letter = 0;

		if (p->lastline || (row < p->cellheight - 1))
			letter = dat[row] & mask;

		if (p->cc[n].cache[row] != letter)
			p->cc[n].clean = 0;	/* only mark dirty if really different */
		p->cc[n].cache[row] = letter;
	}
}


/**
 * Place an icon on the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param icon     synbolic value representing the icon.
 * \retval 0       Icon has been successfully defined/written.
 * \retval <0      Server core shall define/write the icon.
 */
MODULE_EXPORT int
HD44780_icon(Driver *drvthis, int x, int y, int icon)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	static unsigned char heart_open[] =
		{ b__XXXXX,
		  b__X_X_X,
		  b_______,
		  b_______,
		  b_______,
		  b__X___X,
		  b__XX_XX,
		  b__XXXXX };
	static unsigned char heart_filled[] =
		{ b__XXXXX,
		  b__X_X_X,
		  b___X_X_,
		  b___XXX_,
		  b___XXX_,
		  b__X_X_X,
		  b__XX_XX,
		  b__XXXXX };
	static unsigned char arrow_up[] =
		{ b____X__,
		  b___XXX_,
		  b__X_X_X,
		  b____X__,
		  b____X__,
		  b____X__,
		  b____X__,
		  b_______ };
	static unsigned char arrow_down[] =
		{ b____X__,
		  b____X__,
		  b____X__,
		  b____X__,
		  b__X_X_X,
		  b___XXX_,
		  b____X__,
		  b_______ };
	static unsigned char checkbox_off[] =
		{ b_______,
		  b_______,
		  b__XXXXX,
		  b__X___X,
		  b__X___X,
		  b__X___X,
		  b__XXXXX,
		  b_______ };
	static unsigned char checkbox_on[] =
		{ b____X__,
		  b____X__,
		  b__XXX_X,
		  b__X_XX_,
		  b__X_X_X,
		  b__X___X,
		  b__XXXXX,
		  b_______ };
	static unsigned char checkbox_gray[] =
		{ b_______,
		  b_______,
		  b__XXXXX,
		  b__X_X_X,
		  b__XX_XX,
		  b__X_X_X,
		  b__XXXXX,
		  b_______ };
	static unsigned char block_filled[] =
		{ b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX };

	/* Icons from CGROM will always work */
	switch (icon) {
	    case ICON_ARROW_LEFT:
		HD44780_chr(drvthis, x, y, 0x1B);
		return 0;
	    case ICON_ARROW_RIGHT:
		HD44780_chr(drvthis, x, y, 0x1A);
		return 0;
	}

	/* The full block works except if ccmode=bignum */
	if (icon == ICON_BLOCK_FILLED) {
		if (p->ccmode != bignum) {
			HD44780_set_char(drvthis, 0, block_filled);
			HD44780_chr(drvthis, x, y, 0);
			return 0;
		}
		else {
			return -1;
		}
	}

	/* The heartbeat icons do not work in bignum and vbar mode */
	if ((icon == ICON_HEART_FILLED) || (icon == ICON_HEART_OPEN)) {
		if ((p->ccmode != bignum) && (p->ccmode != vbar)) {
			switch (icon) {
			    case ICON_HEART_FILLED:
				HD44780_set_char(drvthis, 7, heart_filled);
				HD44780_chr(drvthis, x, y, 7);
				return 0;
			    case ICON_HEART_OPEN:
				HD44780_set_char(drvthis, 7, heart_open);
				HD44780_chr(drvthis, x, y, 7);
				return 0;
			}
		}
		else {
			return -1;
		}
	}

	/* All other icons work only in the standard or icon ccmode */
	if (p->ccmode != icons) {
		if (p->ccmode != standard) {
			/* Not supported (yet) */
			report(RPT_WARNING, "%s: num: cannot combine two modes using user-defined characters",
					drvthis->name);
			return -1;
		}
		p->ccmode = icons;
	}

	switch (icon) {
		case ICON_ARROW_UP:
			HD44780_set_char(drvthis, 1, arrow_up);
			HD44780_chr(drvthis, x, y, 1);
			break;
		case ICON_ARROW_DOWN:
			HD44780_set_char(drvthis, 2, arrow_down);
			HD44780_chr(drvthis, x, y, 2);
			break;
		case ICON_CHECKBOX_OFF:
			HD44780_set_char(drvthis, 3, checkbox_off);
			HD44780_chr(drvthis, x, y, 3);
			break;
		case ICON_CHECKBOX_ON:
			HD44780_set_char(drvthis, 4, checkbox_on);
			HD44780_chr(drvthis, x, y, 4);
			break;
		case ICON_CHECKBOX_GRAY:
			HD44780_set_char(drvthis, 5, checkbox_gray);
			HD44780_chr(drvthis, x, y, 5);
			break;
		default:
			return -1;	/* Let the core do other icons */
	}
	return 0;
}


/**
 * Get key from the key panel connected to the display.
 * \param drvthis  Pointer to driver structure.
 * \return         String representation of the key;
 *                 \c NULL if nothing available / unmapped key.
 */
MODULE_EXPORT const char *
HD44780_get_key(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	unsigned char scancode;
	char *keystr = NULL;
	struct timeval curr_time, time_diff;

	/* return "no key pressed" if required function is missing or input disabled */
	if ((!p->have_keypad) || (p->hd44780_functions->scankeypad == NULL))
		return NULL;

	gettimeofday(&curr_time, NULL);

	scancode = p->hd44780_functions->scankeypad(p);
	if (scancode != '\0') {
		/* Check if arrays are large enough */
		if ((scancode&0x0F) > KEYPAD_MAXX || ((scancode&0xF0)>>4) > KEYPAD_MAXY) {
			report(RPT_WARNING, "HD44780_get_key: Scancode out of range: %d",
				scancode);
			return NULL;
		}

		keystr = (scancode & 0xF0)
			 ? p->keyMapMatrix[((scancode&0xF0)>>4)-1][(scancode&0x0F)-1]
			 : p->keyMapDirect[scancode - 1];
	}

	if (keystr != NULL) {
		if (keystr == p->pressed_key) {
			timersub(&curr_time, &(p->pressed_key_time), &time_diff);
			if (((time_diff.tv_usec / 1000 + time_diff.tv_sec * 1000) - KEYPAD_AUTOREPEAT_DELAY) < 1000 * p->pressed_key_repetitions / KEYPAD_AUTOREPEAT_FREQ) {
				/*
				 * The key is already pressed quite some time
				 * but it's not yet time to return a repeated
				 * keypress
				 */
				return NULL;
			}
			/* Otherwise a keypress will be returned */
			p->pressed_key_repetitions++;
		}
		else {
			/* It's a new keypress */
			p->pressed_key_time = curr_time;
			p->pressed_key_repetitions = 0;
			report(RPT_INFO, "HD44780_get_key: Key pressed: %s (%d,%d)",
					keystr, scancode&0x0F, (scancode&0xF0)>>4);
		}
	}

	/* Store the key for the next round */
	p->pressed_key = keystr;

	return keystr;
}


/**
 * Scan the keypad (not part of the API).
 *
 * This function uses the readkeypad function of the connection type (if
 * available) to read a key from the keypad. It calls readkeypad with a bit
 * mask of keypad rows to enable and reads the bitmap of keys pressed on these
 * rows.
 *
 * The returned scancode consists of 4 bits for the row and 4 bits for the
 * column of the keypad. This allows for a 16x16 matrix keypad in theory, but
 * on a parallel port we are limited to a 11x5 matrix (8 data + 3 control
 * pins - excluding one for EN1, 5 status pins).
 *
 * Note that a connection type may support even less rows/columns!
 *
 * \param p  Pointer to PrivateData structure.
 * \return   Scancode of the key.
 */
unsigned char HD44780_scankeypad(PrivateData *p)
{
	unsigned int keybits;
	unsigned int shiftcount;
	unsigned int shiftingbit;
	unsigned int Ypattern;
	unsigned int Yval;
	signed char exp;

	unsigned char scancode = 0;

	/* return 'no key pressed' if no keypad reading function defined */
	if (p->hd44780_functions->readkeypad == NULL)
		return('\0');

	/* Step 1: Check if a directly connected key is pressed */
	/* Put all zeros on Y of keypad */
	keybits = p->hd44780_functions->readkeypad(p, 0);

	if (keybits) {
		/*
		 * A directly connected key was pressed
		 * Which key was it ?
		 */
		shiftingbit = 1;
		for (shiftcount = 0; shiftcount < KEYPAD_MAXX && !scancode; shiftcount++) {
			if (keybits & shiftingbit) {
				/* Found ! */
				scancode = shiftcount+1;
			}
			shiftingbit <<= 1;
		}
	}
	else {
		/* Now check the matrix */
		/* Step 2: Check with all 1's to see if a key is pressed at all */
		Ypattern = (1 << KEYPAD_MAXY) - 1;
		if (p->hd44780_functions->readkeypad(p, Ypattern)) {
			/*
			 * Yes, a key on the matrix is pressed
			 *
			 * Step 3: Determine the row
			 * Do a 'binary search' to minimize I/O
			 * Requires 4 I/O reads
			 */
			Ypattern = 0;
			Yval = 0;
			for (exp=3; exp>=0; exp--) {
				Ypattern = ((1 << (1 << exp)) - 1) << Yval;
				/*-
				 * The above line generates the line mask for
				 * the binary search. Example if a key is
				 * pressed on row 3:
				 * exp  bitmap
				 *   3  00000000 11111111
				 *   2  00000000 00001111
				 *   1  00000000 00000011
				 *   0  00000000 00000100
				 */
				keybits = p->hd44780_functions->readkeypad(p, Ypattern);
				if (!keybits) {
					Yval += (1 << exp);
				}
			}

			/* Step 4: Final read */
			keybits = p->hd44780_functions->readkeypad(p, 1<<Yval);
			/* Step 5: Which key is pressed in that row ? */
			shiftingbit = 1;
			for (shiftcount = 0; shiftcount < KEYPAD_MAXX && !scancode; shiftcount++) {
				if (keybits & shiftingbit) {
					/* Found ! */
					scancode = (Yval+1) << 4 | (shiftcount+1);
				}
				shiftingbit <<= 1;
			}
		}
	}
	return scancode;
}


/**
 * Output to the optional output latch(es).
 * \param drvthis  Pointer to driver structure.
 * \param on       Integer with bits representing LED states.
 */
MODULE_EXPORT void
HD44780_output(Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;

	/* return immediately if output is disabled */
	if (!p->have_output)
		return;

	/*
	 * perhaps it is better just to do this every time in case of a glitch
	 * but leaving this in does make sure that any latch-enable line glitches
	 * are more easily seen.
	 */
	if (p->output_state == on)
		return;

	p->output_state = on;

	/* call output function only if it is defined for the commenction type */
	if (p->hd44780_functions->output != NULL)
		p->hd44780_functions->output(p, on);
}


/**
 * Parse a span list, a comma separated list of numbers (e.g. "1,4,2").
 * \param spanListArray  Array to store vertical spans.
 * \param spLsize	 Size of spanListArray.
 * \param dispOffsets    Array to store display offsets.
 * \param dOffsize	 Size of dispOffsets.
 * \param dispSizeArray  Array of display vertical sizes (= spanlist).
 * \param spanlist	 '\\0'-terminated input span list in comma delimited format.
 * \return  Number of span elements, -1 on parse error.
 */
static int
parse_span_list(int *spanListArray[], int *spLsize, int *dispOffsets[], int *dOffsize, int *dispSizeArray[], const char *spanlist)
{
	int j = 0, retVal = 0;

	*spLsize = 0;
	*dOffsize = 0;

	while (j < strlen(spanlist)) {
		if (spanlist[j] >= '1' && spanlist[j] <= '9') {
			int spansize = spanlist[j] - '0';

			/*
			 * add spansize lines to the span list, note the offset
			 * to the previous display and the size of the display
			 */
			if ((*spanListArray = (int *) realloc(*spanListArray, sizeof(int) * (*spLsize + spansize)))
				 && (*dispOffsets = (int *) realloc(*dispOffsets, sizeof(int) * (*dOffsize + 1)))
				 && (*dispSizeArray = (int *) realloc(*dispSizeArray, sizeof(int) * (*dOffsize + 1)))) {
				int k;
				for (k = 0; k < spansize; ++k)
					(*spanListArray)[*spLsize + k] = *dOffsize + 1;

				(*dispOffsets)[*dOffsize] = *spLsize;
				(*dispSizeArray)[*dOffsize] = spansize;
				*spLsize += spansize;
				++(*dOffsize);
				retVal = *dOffsize;

				/* find the next number (\0 is also outside this range) */
				for (++j; spanlist[j] < '1' || spanlist[j] > '9'; ++j);
			} else {
				retVal = -1;
			}
		} else {
			retVal = -1;
		}
	}
	return retVal;
}

/* EOF */
