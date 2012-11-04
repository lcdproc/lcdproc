/** \file server/drivers/glcdlib.c
 * LCDd \c glcdlib "meta-driver" for all displays supported by graphlcd-base.
 */

/*-
 * This so-called "meta-driver" extends LCDproc's supported drivers by
 * all the drivers supported by graphlcd-base, which you can get from
 * http://projects.vdr-developer.org/projects/graphlcd/ .
 * In order to be able to use it, you have to get and install the glcdprocdriver
 * from http://lucianm.github.com/GLCDprocDriver/ .
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c)  2005 Lucian Muresan <lucianm AT users.sourceforge.net>
 */


#include <glcdproclib/glcdprocdriver.h>


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "report.h"

#include "glcdlib.h"


/** private data for the \c glcdlib driver */
typedef struct glcdlib_private_data {
	GlcdDriver *glcdDriver;		/**< GlcdDriver handle */
	char info[255];			/**< info string contents */
} glcdlibPD;


// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "glcdlib_";


MODULE_EXPORT int glcdlib_init (Driver *drvthis)
{
	glcdlibPD * pPD;

	// Alocate, initialize and store private p
	pPD = (glcdlibPD *) malloc(sizeof(glcdlibPD));
	if (pPD == NULL) {
		report(RPT_ERR, "%s: failed to allocate private data", drvthis->name);
		return -1;
	}

	pPD->glcdDriver = NULL;
	memset(pPD->info, '\0', sizeof(pPD->info));

	if (drvthis->store_private_ptr(drvthis, pPD)) {
		report(RPT_ERR, "%s: failed to store private data pointer", drvthis->name);
		return -1;
	}

	debug(RPT_INFO, "glcdlib_init: reading config file...");
	//##################################################################
	//	Read config file
	//##################################################################


	//##################################################################
	//	mandatory entries:

	// which driver
	char strCfgDriver[30];
	strncpy(strCfgDriver,
		drvthis->config_get_string(drvthis->name, "Driver", 0, "image"),
		sizeof(strCfgDriver));
	strCfgDriver[sizeof(strCfgDriver)-1] = '\0';

	// use or not FreeType2
	bool bCfgUseFT2 = drvthis->config_get_bool(drvthis->name, "UseFT2", 0, true);

	// which text resolution
	const char strTextResDefault[] = "16x4";
	char strTextRes[7];
	int nCfgTextWidth = 0;
	int nCfgTextRows = 0;
	strncpy(strTextRes,
		drvthis->config_get_string(drvthis->name, "TextResolution", 0, strTextResDefault),
		sizeof(strTextRes));
	strTextRes[sizeof(strTextRes)-1] = '\0';
	if ((sscanf(strTextRes, "%dx%d", &nCfgTextWidth, &nCfgTextRows) != 2)
	    || (nCfgTextWidth <= 0) || (nCfgTextWidth > LCD_MAX_WIDTH)
	    || (nCfgTextRows <= 0) || (nCfgTextRows > LCD_MAX_HEIGHT))
	{
		report(RPT_WARNING,
			"%s: cannot read or invalid TextResolution: %s; using default %s",
			drvthis->name, strTextRes, strTextResDefault);
		sscanf(strTextResDefault, "%dx%d", &nCfgTextWidth, &nCfgTextRows);
	}

	// which font file
	const char strCfgFontFileDef[] = "/usr/share/fonts/corefonts/courbd.ttf";
	char strCfgFontFile[256];
	strncpy(strCfgFontFile,
		drvthis->config_get_string(drvthis->name, "FontFile", 0, strCfgFontFileDef),
		sizeof(strCfgFontFile));
	strCfgFontFile[sizeof(strCfgFontFile)-1] = '\0';

	//##################################################################
	//	these only apply if bCfgUseFT2 = yes:

	// character encoding
	char strCfgEncoding[25];
	strncpy(strCfgEncoding,
		drvthis->config_get_string(drvthis->name, "CharEncoding", 0, "ISO8859-1"),
		sizeof(strCfgEncoding));
	strCfgEncoding[sizeof(strCfgEncoding)-1] = '\0';

	// minimum font face pixel resolution
	const char strMinFaceSizeDef[] = "6x8";
	char strMinFontFaceSize[7];
	int nCfgMinFontFaceWidth = 0;
	int nCfgMinFontFaceHeight = 0;
	strncpy(strMinFontFaceSize,
		drvthis->config_get_string(drvthis->name, "MinFontFaceSize", 0, strMinFaceSizeDef),
		sizeof(strMinFontFaceSize));
	strMinFontFaceSize[sizeof(strMinFontFaceSize)-1] = '\0';
	if ((sscanf(strMinFontFaceSize, "%dx%d", &nCfgMinFontFaceWidth, &nCfgMinFontFaceHeight) != 2)
	    || (nCfgMinFontFaceWidth <= 0) || (nCfgMinFontFaceWidth > LCD_MAX_WIDTH)
	    || (nCfgMinFontFaceHeight <= 0) || (nCfgMinFontFaceHeight > LCD_MAX_HEIGHT))
	{
		report(RPT_WARNING,
			"%s: cannot read or invalid MinFontFaceSize: %s; using default %s",
			drvthis->name, strMinFontFaceSize, strMinFaceSizeDef);
		sscanf(strMinFaceSizeDef, "%dx%d", &nCfgMinFontFaceWidth, &nCfgMinFontFaceHeight);
	}

	// show debugging frame?
	bool bShowDbgFrame = drvthis->config_get_bool(drvthis->name, "ShowDebugFrame", 0, true);
	// show big border?
	bool bShowBigBorder = drvthis->config_get_bool(drvthis->name, "ShowBigBorder", 0, true);
	// show thin border?
	bool bShowThinBorder = drvthis->config_get_bool(drvthis->name, "ShowThinBorder", 0, true);

	// pixel shift
	int nPixShiftX = drvthis->config_get_int(drvthis->name, "PixelShiftX", 0, 0);
	int nPixShiftY = drvthis->config_get_int(drvthis->name, "PixelShiftY", 0, 0);

	//##################################################################
	// these are optional and override graphlcd-base library's own settings
	// before on startup only:

	int nContrast = drvthis->config_get_int(drvthis->name, "Contrast", 0, 50);
	bool bBacklight = drvthis->config_get_bool(drvthis->name, "Backlight", 0, false);
	bool bUpsideDown = drvthis->config_get_bool(drvthis->name, "UpsideDown", 0, false);

	// instantiate driver in the wrapper library
	pPD->glcdDriver = glcddriverCreate();

	if (pPD->glcdDriver == NULL) {
		report(RPT_ERR, "%s: failed to instantiate glcdlib wrapper", drvthis->name);
		return -1;
	}

	glcddriverInit(	pPD->glcdDriver,
			strCfgDriver,
			strCfgFontFile,
			bCfgUseFT2,
			strCfgEncoding,
			nCfgMinFontFaceHeight,
			bShowBigBorder,
			bShowThinBorder,
			bShowDbgFrame,
			nPixShiftX,
			nPixShiftY,
			bUpsideDown,
			bBacklight,
			nContrast);

	// apply supplemental settings
	int nBrightness = drvthis->config_get_int(drvthis->name, "Brightness", 0, 50);
	glcddriverSetBrightness(pPD->glcdDriver, nBrightness);
	bool bInvert = drvthis->config_get_bool(drvthis->name, "Invert", 0, false);
	glcddriverInvert(pPD->glcdDriver, bInvert);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}


MODULE_EXPORT void
glcdlib_close (Driver *drvthis)
{
	debug(RPT_INFO, "Shutting down!");
	glcdlibPD * pPD = drvthis->private_data;

	if (pPD != NULL) {
		if (pPD->glcdDriver != NULL)
			glcddriverDestroy(pPD->glcdDriver);
		pPD->glcdDriver = NULL;

		free(pPD);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
glcdlib_width (Driver *drvthis)
{
	glcdlibPD * pPD = drvthis->private_data;
	return glcddriverTextWidth(pPD->glcdDriver);
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
glcdlib_height (Driver *drvthis)
{
	glcdlibPD * pPD = drvthis->private_data;
	return glcddriverTextHeight(pPD->glcdDriver);
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
glcdlib_clear (Driver *drvthis)
{
	debug(RPT_DEBUG, "Clearing screen");
	glcdlibPD * pPD = drvthis->private_data;
	glcddriverClear(pPD->glcdDriver);
}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
MODULE_EXPORT void
glcdlib_flush (Driver *drvthis)
{
	debug(RPT_DEBUG, "glcdlib_flush: Flushing pixbuffer(s)");
	glcdlibPD * pPD = drvthis->private_data;
	glcddriverRefresh(pPD->glcdDriver, false);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,6).
//
MODULE_EXPORT void
glcdlib_string (Driver *drvthis, int x, int y, const char string[])
{
	debug(RPT_DEBUG, "String out");
	y--;
	x--;
	glcdlibPD * pPD = drvthis->private_data;
	glcddriverDrawText(pPD->glcdDriver, x, y, string);
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,6).
//
MODULE_EXPORT void
glcdlib_chr (Driver *drvthis, int x, int y, char c)
{
	debug(RPT_DEBUG, "Char out");
	y--;
	x--;
	glcdlibPD * pPD = drvthis->private_data;
	glcddriverDrawCharacter(pPD->glcdDriver, x, y, c);
}

/////////////////////////////////////////////////////////////////
// Sets an icon...
//
MODULE_EXPORT int
glcdlib_icon (Driver *drvthis, int x, int y, int icon)
{
	debug(RPT_DEBUG, "GLCDLIB: set icon %d", icon);
	y--;
	x--;
	glcdlibPD * pPD = drvthis->private_data;
	return glcddriverDrawIcon(pPD->glcdDriver, x, y, icon);
}

/////////////////////////////////////////////////////////////////
// provides some info about this driver
//
MODULE_EXPORT const char *
glcdlib_get_info (Driver *drvthis)
{
	glcdlibPD * pPD = drvthis->private_data;
	strcpy(pPD->info, "Meta driver which adds support for displays supported by graphlcd-base");
	return pPD->info;
}

/////////////////////////////////////////////////////////////////
// Returns the display's cell width
//
MODULE_EXPORT int
glcdlib_cellwidth (Driver *drvthis)
{
	glcdlibPD * pPD = drvthis->private_data;
	return glcddriverCellWidth(pPD->glcdDriver);
}

/////////////////////////////////////////////////////////////////
// Returns the display's cell height
//
MODULE_EXPORT int
glcdlib_cellheight (Driver *drvthis)
{
	glcdlibPD * pPD = drvthis->private_data;
	return glcddriverCellHeight(pPD->glcdDriver);
}

/////////////////////////////////////////////////////////////////
// Returns the maximum number of custom char slots (not how many
// are free at a moment, maybe this isn't even needed...
//
MODULE_EXPORT int
glcdlib_get_free_chars (Driver *drvthis)
{
	glcdlibPD * pPD = drvthis->private_data;
	return glcddriverMaxCustomChars(pPD->glcdDriver);
}

/////////////////////////////////////////////////////////////////
// Retrieves brightness (in promille)
//
MODULE_EXPORT int
glcdlib_get_brightness(Driver *drvthis, int state)
{
	glcdlibPD * pPD = drvthis->private_data;
	if (state == BACKLIGHT_ON)
		return 10 * glcddriverGetBrightness(pPD->glcdDriver);
	else
		return 0;
}

/////////////////////////////////////////////////////////////////
// Sets brightness (in promille)
//
MODULE_EXPORT void
glcdlib_set_brightness(Driver *drvthis, int state, int promille)
{
	glcdlibPD * pPD = drvthis->private_data;
	if (state == BACKLIGHT_ON)
		glcddriverSetBrightness(pPD->glcdDriver, promille/10);
}

/////////////////////////////////////////////////////////////////
// Sets a custom character from 0 to glcdlib_get_free_chars - 1
// The input is an array of characters [cellheight x cellwidth]
// (0 & 1 according to pixels) ordered by rows
MODULE_EXPORT void
glcdlib_set_char (Driver *drvthis, int n, char *dat)
{
	glcdlibPD * pPD = drvthis->private_data;
	glcddriverSetCustomChar(pPD->glcdDriver, n, dat);

}

//////////////////////////////////////////////////////////////////////
// Draws a big (4-row) number.
//
MODULE_EXPORT void
glcdlib_num (Driver *drvthis, int x, int num)
{
	glcdlibPD * pPD = drvthis->private_data;
	glcddriverDrawBigNum(pPD->glcdDriver, x - 1, num);
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
glcdlib_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	y--;
	x--;
	glcdlibPD * pPD = drvthis->private_data;
	glcddriverDrawBarH(pPD->glcdDriver, x, y, len, promille, options);
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar, from the bottom of the screen up.
//
MODULE_EXPORT void
glcdlib_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	y--;
	x--;
	glcdlibPD * pPD = drvthis->private_data;
	glcddriverDrawBarV(pPD->glcdDriver, x, y, len, promille, options);
}

/*
/////////////////////////////////////////////////////////////////
// Turns backlight on/off
//
MODULE_EXPORT void
glcdlib_backlight(Driver *drvthis, int on)
{
	glcdlibPD * pPD = drvthis->private_data;
}
*/
