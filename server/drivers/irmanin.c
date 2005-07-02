/* irmanin.c - test/demo of LIBIR's to interface with lcdproc (LCDd) */
/* Copyright (C) 1999 David Glaude loosely based on workmanir.c */
/* workmanir.c - test/demo of LIBIR's high level command functions */
/* Copyright (C) 1998 Tom Wheeley, see file COPYING for details    */
#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>

#define __u32 unsigned int
#define __u8 unsigned char

#include "shared/debug.h"
#include "shared/str.h"

#define NAME_LENGTH 128

#include "lcd.h"
#include "irmanin.h"
#include "report.h"
#include "irman.h"

char *progname = "irmanin";

char *codes[] = {
	/* dummy */ NULL,
	/* KeyToLcd */ "lcdproc-A",
	/* KeyToLcd */ "lcdproc-B",
	/* KeyToLcd */ "lcdproc-C",
	/* KeyToLcd */ "lcdproc-D",
	/* KeyToLcd */ "lcdproc-E",
	/* KeyToLcd */ "lcdproc-F",
	/* KeyToLcd */ "lcdproc-G",
	/* KeyToLcd */ "lcdproc-H",
	/* KeyToLcd */ "lcdproc-I",
	/* KeyToLcd */ "lcdproc-J",
	/* KeyToLcd */ "lcdproc-K",
	/* KeyToLcd */ "lcdproc-L",
	/* KeyToLcd */ "lcdproc-M",
	/* KeyToLcd */ "lcdproc-N",
	/* KeyToLcd */ "lcdproc-O",
	/* KeyToLcd */ "lcdproc-P",
	/* KeyToLcd */ "lcdproc-Q",
	/* KeyToLcd */ "lcdproc-R",
	/* KeyToLcd */ "lcdproc-S",
	/* KeyToLcd */ "lcdproc-T",
	/* KeyToLcd */ "lcdproc-U",
	/* KeyToLcd */ "lcdproc-V",
	/* KeyToLcd */ "lcdproc-W",
	/* KeyToLcd */ "lcdproc-X",
	/* KeyToLcd */ "lcdproc-Y",
	/* KeyToLcd */ "lcdproc-Z",
	/* end */ NULL
};

//////////////////////////////////////////////////////////////////////////
////////////////////// Base "class" to derive from ///////////////////////
//////////////////////////////////////////////////////////////////////////


//void sigterm(int sig)
//{
//  ir_free_commands();
//  ir_finish();
//  raise(sig);
//}

////////////////////////////////////////////////////////////
// init() should set up any device-specific stuff, and
// point all the function pointers.
MODULE_EXPORT int
irmanin_init (Driver *drvthis)
{
	char device[256];
	char *ptrdevice;
	char config[256];
	char *ptrconfig;

	char *portname;

	int i;
	char *filename;

	ptrconfig = NULL;
	ptrdevice = NULL;

	/* Read config file */

	/* What device should be used */
	strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0,
						   ""), sizeof(device));
	device[sizeof(device)-1] = '\0';
	if (*device != '\0')  ptrdevice = device;

	/* What config file should be used */
	strncpy(config, drvthis->config_get_string(drvthis->name, "Config", 0,
						   ""), sizeof(config));
	config[sizeof(config)-1] = '\0';
	if (*config != '\0')  ptrconfig = config;

	/* End of config file parsing */

	if (ir_init_commands (ptrconfig, 1) < 0) {
		report(RPT_ERR, "error initialising commands: %s\n", strerror (errno));
		exit (1);
	}

	portname = ir_default_portname ();
	if (portname == NULL) {
		if (ptrdevice != NULL) {
			portname = ptrdevice;
		} else {
			report(RPT_ERR, "error no device defined\n");
			exit (1);
		}
	}

	drvthis->getkey = irmanin_getkey;
	drvthis->close = irmanin_close;

	for (i = 1; codes[i] != NULL; i++) {
		if (ir_register_command (codes[i], i) < 0) {
			if (errno == ENOENT) {
				report(RPT_WARNING, "%s: no code set for `%s'\n", progname, codes[i]);
			} else {
				report(RPT_WARNING, "error registering `%s': `%s'\n", codes[i], strerror (errno));
			}
		}
	}

	errno = 0;
	if (ir_init (portname) < 0) {
		report(RPT_ERR, "%s: error initialising Irman: `%s'\n", strerror (errno), portname);
		exit (1);
	}

	return 1;						  // 200 is arbitrary.  (must be 1 or more)
}

void
irmanin_close (Driver *drvthis)
{
	//if (drvthis->framebuf != NULL)
	//	free (drvthis->framebuf);
	//drvthis->framebuf = NULL;

	ir_free_commands ();
	ir_finish ();
}

//////////////////////////////////////////////////////////////////////
// Tries to read a character from an input device...
//
// Return 0 for "nothing available".
//
char
irmanin_getkey (Driver *drvthis)
{
	int i;
	int cmd;
	char key;

	key = (char) 0;
	switch (cmd = ir_poll_command ()) {
	case IR_CMD_ERROR:
		report(RPT_WARNING, "%s: error reading command: %s\n", progname, strerror (errno));
		break;
	case IR_CMD_UNKNOWN:
		break;
	default:
		key = (char) ('A' - 1 + cmd);
		break;
	}

	return key;
}

/* end of irmanin.c */
