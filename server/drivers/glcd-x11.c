/** \file server/drivers/glcd-x11.c
 * This driver writes the framebuffer content to an X11 window.
 * Written to mimic X11 driver written for lcd4linux. Except this one does not
 * have graphical buttons or cell gaps.  Written to represent a graphical LCD
 * only.
 */

/*-
 * Copyright (c) 2012 Scott Meharg <ssmeharg@gmail.com>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>

#include "lcd.h"
#include "report.h"
#include "glcd-low.h"

#define X11_DEF_PIXEL_SIZE		"3+1"
#define X11_DEF_PIXEL_COLOR		0x000000
#define X11_DEF_BACKLIGHT_COLOR		0x80FF80
#define X11_DEF_BORDER			20
#define X11_DEF_INVERTED		0

/** Private data for the X11 connection type */
typedef struct glcd_x11_data {
	int pixel;		/** Size of a LCD pixel (in screen pixels) */
	int pgap;		/** Gap between each LCD pixel */
	int border;		/** Free space around LCD area */
	unsigned long bgcolor;	/** Background/backlight color */
	unsigned long fgcolor;	/** Foreground/pixel color */
	unsigned char inverted;	/** Indicate swapped background/foreground */

	Display *dp;		/** Connection to X server */
	int sc;			/** Default screen number */
	Window rw;		/** Root window for default screen */
	Window w;		/** The window the LCD area will be drawn to */
	Visual *vi;		/** Default visual style */
	GC gc;			/** Default graphics context */

	int dimx, dimy;		/** Width/height of the X window */
	Atom wmDeleteMessage;	/** Atom identifier for closing the window */

	unsigned char *backingstore;	/** Holds a copy of the LCD screen data */
} CT_x11_data;

/* Prototypes */
void glcd_x11_blit(PrivateData *p);
void glcd_x11_close(PrivateData *p);
unsigned char glcd_x11_pollkeys(PrivateData *p);
void glcd_x11_set_backlight(PrivateData *p, int state);
static void x11w_adj_contrast_brightness(unsigned long *pfgc, unsigned long *pbgc, int contrast,
					 int brightness);
static void x11w_draw_pixel(CT_x11_data * ct_data, int x, int y, unsigned long fgc,
			    unsigned long bgc);

/**
 * Draws a single LCD pixel in the X11 window.
 * \param ct_data  Connection type's private data.
 * \param x        LCD x position.
 * \param y        LCD y position.
 * \param fgc      Foreground color.
 * \param bgc      Background color.
 */
static void
x11w_draw_pixel(CT_x11_data * ct_data, int x, int y, unsigned long fgc, unsigned long bgc)
{
	int pxlsize = ct_data->pixel + ct_data->pgap;
	int xoffset = ct_data->border + (x * pxlsize);
	int yoffset = ct_data->border + (y * pxlsize);

	/* This basically erases the area beneath LCD pixel being drawn. */
	XSetForeground(ct_data->dp, ct_data->gc, bgc);
	XFillRectangle(ct_data->dp, ct_data->w, ct_data->gc, xoffset, yoffset, pxlsize, pxlsize);

	/* Only draw if foreground color is different from background color.
	 * No need to draw the same area twice. */
	if (fgc != bgc) {
		XSetForeground(ct_data->dp, ct_data->gc, fgc);
		XFillRectangle(ct_data->dp, ct_data->w, ct_data->gc, xoffset,
			       yoffset, ct_data->pixel, ct_data->pixel);
	}
}

/**
 * Adjusts the foreground color and background color for contrast and
 * brightness.
 * \param pfgc        Pointer to foreground color that will be updated.
 * \param pbgc        Pointer to background color that will be updated.
 * \param contrast    The contrast value between 0 - 1000.
 * \param brightness  The brightness value between 0 - 1000.
 */
static void
x11w_adj_contrast_brightness(unsigned long *pfgc, unsigned long *pbgc, int contrast, int brightness)
{
	float adjf = 0;

	unsigned long fgc = *pfgc;
	unsigned long bgc = *pbgc;

	unsigned char fgr = (fgc >> 16);
	unsigned char fgg = ((fgc << 8) >> 16);
	unsigned char fgb = ((fgc << 16) >> 16);

	unsigned char bgr = (bgc >> 16);
	unsigned char bgg = ((bgc << 8) >> 16);
	unsigned char bgb = ((bgc << 16) >> 16);

	/* Adjust contrast */
	adjf = ((1000 - contrast) / (float) 1000);

	fgr += ((bgr - fgr) * adjf);
	fgg += ((bgg - fgg) * adjf);
	fgb += ((bgb - fgb) * adjf);


	/* Adjust the brightness */
	adjf = ((1000 - brightness) / (float) 1000);

	bgr -= (bgr * adjf);
	bgg -= (bgg * adjf);
	bgb -= (bgb * adjf);

	fgr -= (fgr * adjf);
	fgg -= (fgg * adjf);
	fgb -= (fgb * adjf);

	fgc = (fgr << 16) + (fgg << 8) + fgb;
	bgc = (bgr << 16) + (bgg << 8) + bgb;

	*pfgc = fgc;
	*pbgc = bgc;
}

/**
 * API: Initialize the connection type driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
int
glcd_x11_init(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	CT_x11_data *ct_data;
	char buf[256];

	report(RPT_INFO, "GLCD/x11: initializing");

	/* Set up connection type low-level functions */
	p->glcd_functions->blit = glcd_x11_blit;
	p->glcd_functions->close = glcd_x11_close;
	p->glcd_functions->poll_keys = glcd_x11_pollkeys;
	p->glcd_functions->set_backlight = glcd_x11_set_backlight;

	/* Allocate memory structures */
	ct_data = (CT_x11_data *) calloc(1, sizeof(CT_x11_data));
	if (ct_data == NULL) {
		report(RPT_ERR, "GLCD/x11: error allocating connection data");
		return -1;
	}
	p->ct_data = ct_data;

	ct_data->backingstore = malloc(p->framebuf.size);
	if (ct_data->backingstore == NULL) {
		report(RPT_ERR, "GLCD/x11: unable to allocate backing store");
		return -1;
	}
	memset(ct_data->backingstore, 0x00, p->framebuf.size);

	/* Get and parse pixel size */
	strncpy(buf, drvthis->config_get_string(drvthis->name, "x11_PixelSize",
						0, X11_DEF_PIXEL_SIZE), sizeof(buf));
	buf[sizeof(buf) - 1] = '\0';

	if ((sscanf(buf, "%d+%d", &ct_data->pixel, &ct_data->pgap) != 2) ||
	    (ct_data->pixel <= 0) || (ct_data->pgap < 0)) {
		report(RPT_WARNING, "GLCD/x11: Cannot read/use PixelSize: %s; using default %s",
		       buf, X11_DEF_PIXEL_SIZE);
		strncpy(buf, X11_DEF_PIXEL_SIZE, sizeof(buf));
		buf[sizeof(buf) - 1] = '\0';
		/* this assumes X11_DEF_PIXEL_SIZE is usable */
		sscanf(buf, "%d+%d", &ct_data->pixel, &ct_data->pgap);
	}

	/* Get Pixel Color (pixel on color) */
	ct_data->fgcolor = drvthis->config_get_int(drvthis->name,
						   "x11_PixelColor", 0, X11_DEF_PIXEL_COLOR);

	/* Get Backlight Color (pixel off color) */
	ct_data->bgcolor = drvthis->config_get_int(drvthis->name,
						   "x11_BacklightColor", 0,
						   X11_DEF_BACKLIGHT_COLOR);

	/* Get Border size in pixels */
	ct_data->border = drvthis->config_get_int(drvthis->name, "x11_Border", 0, X11_DEF_BORDER);

	/* Get inverted setting */
	ct_data->inverted =
		drvthis->config_get_bool(drvthis->name, "x11_Inverted", 0, X11_DEF_INVERTED);

	/* Setup X11-Window */
	XSetWindowAttributes wa;
	XSizeHints sh;
	XEvent ev;

	if ((ct_data->dp = XOpenDisplay(NULL)) == NULL) {
		report(RPT_ERR, "GLCD/x11: can't open display");
		return -1;
	}

	ct_data->sc = DefaultScreen(ct_data->dp);
	ct_data->gc = DefaultGC(ct_data->dp, ct_data->sc);
	ct_data->vi = DefaultVisual(ct_data->dp, ct_data->sc);
	ct_data->rw = DefaultRootWindow(ct_data->dp);

	ct_data->dimx = (p->framebuf.px_width * (ct_data->pixel + ct_data->pgap))
		+ (ct_data->border * 2);
	ct_data->dimy = (p->framebuf.px_height * (ct_data->pixel + ct_data->pgap))
		+ (ct_data->border * 2);

	wa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPressMask | ButtonReleaseMask;

	sh.min_width = sh.max_width = ct_data->dimx;
	sh.min_height = sh.max_height = ct_data->dimy;
	sh.flags = PPosition | PSize | PMinSize | PMaxSize;

	if (sh.min_width > DisplayWidth(ct_data->dp, ct_data->sc) ||
	    sh.min_height > DisplayHeight(ct_data->dp, ct_data->sc)) {
		report(RPT_WARNING,
		       "GLCD/x11: Warning: X11-Window with dimensions (%d,%d) is greater than display (%d,%d)!",
		       sh.min_width, sh.min_height, DisplayWidth(ct_data->dp, ct_data->sc),
		       DisplayHeight(ct_data->dp, ct_data->sc));
		if (sh.min_width > 32767 || sh.min_height > 32676) {
			report(RPT_ERR, "GLCD/x11: XProtocol data size exceeded");
			return -1;
		}
	}

	ct_data->w = XCreateWindow(ct_data->dp, ct_data->rw, 0, 0, sh.min_width,
				   sh.min_height, 0, 0, InputOutput, ct_data->vi, CWEventMask, &wa);

	XSetWMProperties(ct_data->dp, ct_data->w, NULL, NULL, NULL, 0, &sh, NULL, NULL);
	ct_data->wmDeleteMessage = XInternAtom(ct_data->dp, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(ct_data->dp, ct_data->w, &ct_data->wmDeleteMessage, 1);

	XSetWindowBackground(ct_data->dp, ct_data->w, ct_data->bgcolor);
	XClearWindow(ct_data->dp, ct_data->w);

	XStoreName(ct_data->dp, ct_data->w, "GLCD/x11");
	XMapWindow(ct_data->dp, ct_data->w);

	XFlush(ct_data->dp);

	while (1) {
		XNextEvent(ct_data->dp, &ev);
		if (ev.type == Expose && ev.xexpose.count == 0)
			break;
	}

	debug(RPT_DEBUG, "GLCD/x11: init() done");

	return 0;
}

/**
 * API: Write the frame buffer to the display
 * \param p  Pointer to glcd driver's private date structure.
 */
void
glcd_x11_blit(PrivateData *p)
{
	CT_x11_data *ct_data = (CT_x11_data *) p->ct_data;

	/* Check if frame buffer has changed. If not there's nothing to do */
	if (memcmp(p->framebuf.data, ct_data->backingstore, p->framebuf.size)
	    == 0)
		return;

	unsigned long fgc = ct_data->fgcolor;
	unsigned long bgc = ct_data->bgcolor;
	int y;
	int x;

	/* Adjust colors for contrast and brightness */
	if (p->backlightstate == 0) {
		x11w_adj_contrast_brightness(&fgc, &bgc, p->contrast, p->offbrightness);
	}
	else {
		x11w_adj_contrast_brightness(&fgc, &bgc, p->contrast, p->brightness);
	}

	/* Draw each LCD pixel on the X11 window. */
	for (y = 0; y < p->framebuf.px_height; y++) {
		for (x = 0; x < p->framebuf.px_width; x++) {
			if ((fb_get_pixel(&p->framebuf, x, y) ^ ct_data->inverted) == FB_BLACK)
				x11w_draw_pixel(ct_data, x, y, fgc, bgc);
			else
				x11w_draw_pixel(ct_data, x, y, bgc, bgc);
		}
	}

	XFlush(ct_data->dp);
	memcpy(ct_data->backingstore, p->framebuf.data, p->framebuf.size);

}

/**
 * API: Release low-level resources.
 * \param p  Pointer to glcd driver's private date structure.
 */
void
glcd_x11_close(PrivateData *p)
{
	if (p->ct_data != NULL) {
		CT_x11_data *ct_data = (CT_x11_data *) p->ct_data;

		if (ct_data->dp != NULL) {
			XCloseDisplay(ct_data->dp);
		}

		if (ct_data->backingstore != NULL)
			free(ct_data->backingstore);

		free(p->ct_data);
		p->ct_data = NULL;
	}
}

/**
 * API: Poll for key presses.
 * \param p  Pointer to glcd driver's private data structure.
 */
unsigned char
glcd_x11_pollkeys(PrivateData *p)
{
	CT_x11_data *ct_data = (CT_x11_data *) p->ct_data;
	unsigned char rv = 0;

	XEvent ev;
	KeySym key;

	if (XCheckWindowEvent(ct_data->dp, ct_data->w, KeyPressMask |
			      KeyReleaseMask | ButtonPressMask | ButtonReleaseMask, &ev) == 0
	    && XCheckTypedWindowEvent(ct_data->dp, ct_data->w, ClientMessage, &ev) == 0)
		return 0;

	switch (ev.type) {
	case KeyPress:
		key = XLookupKeysym(&ev.xkey, 0);

		switch (key) {
		case XK_Up:
			rv = 1;
			break;
		case XK_Down:
			rv = 2;
			break;
		case XK_Left:
			rv = 3;
			break;
		case XK_Right:
			rv = 4;
			break;
		case XK_Return:
			rv = 5;
			break;
		case XK_Escape:
			rv = 6;
			break;
		}

		break;

	case ClientMessage:
		if ((Atom) (ev.xclient.data.l[0]) == ct_data->wmDeleteMessage) {
			p->glcd_functions->drv_report(RPT_INFO,
						      "GLCD/x11: Window closed by WindowManager");

			if (raise(SIGTERM) != 0) {
				p->glcd_functions->drv_report(RPT_ERR,
							      "GLCD/x11: Error raising SIGTERM");
			}
		}
		else {
			p->glcd_functions->drv_report(RPT_DEBUG,
						      "GLCD/x11: Get XClient message 0x!lx %lx %lx %lx %lx",
						      ev.xclient.data.l[0], ev.xclient.data.l[1],
						      ev.xclient.data.l[2], ev.xclient.data.l[3],
						      ev.xclient.data.l[4]);
		}
		break;
	}

	return rv;
}

/**
 * API: Switch backlight on and off.
 * \param p      Pointer to glcd driver's private data structure.
 * \param state  State of backlight.
 */
void
glcd_x11_set_backlight(PrivateData *p, int state)
{
	CT_x11_data *ct_data = p->ct_data;

	unsigned long fgc = ct_data->fgcolor;
	unsigned long bgc = ct_data->bgcolor;

	if (state == 0) {
		x11w_adj_contrast_brightness(&fgc, &bgc, p->contrast, p->offbrightness);
		XSetWindowBackground(ct_data->dp, ct_data->w, bgc);
	}
	else {
		x11w_adj_contrast_brightness(&fgc, &bgc, p->contrast, p->brightness);
		XSetWindowBackground(ct_data->dp, ct_data->w, bgc);
	}

	XClearWindow(ct_data->dp, ct_data->w);
	memset(ct_data->backingstore, 0, p->framebuf.size);
}
