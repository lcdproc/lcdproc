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
#include "../../../libirman-0.4.1b/irman.h"

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

lcd_logical_driver *irmanin;

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
irmanin_init (struct lcd_logical_driver *driver, char *args)
{
	char device[256];
	char *ptrdevice;
	char config[256];
	char *ptrconfig;

	char *portname;

	char *argv[64];
	int argc, i, j;
	char *filename;

	ptrconfig = NULL;
	ptrdevice = NULL;

	irmanin = driver;

	argc = get_args (argv, args, 64);

	for (i = 0; i < argc; i++) {
		//printf("Arg(%i): %s\n", i, argv[i]);
		if (0 == strcmp (argv[i], "-d") || 0 == strcmp (argv[i], "--device")) {
			if (i + 1 > argc) {
				fprintf (stderr, "irmanin_init: %s requires an argument\n", argv[i]);
				return -1;
			}
			strcpy (device, argv[++i]);
			ptrdevice = device;
		} else if (0 == strcmp (argv[i], "-c") || 0 == strcmp (argv[i], "--config")) {
			if (i + 1 > argc) {
				fprintf (stderr, "irmanin_init: %s requires an argument\n", argv[i]);
				return -1;
			}
			strcpy (config, argv[++i]);
			ptrconfig = config;
		} else if (0 == strcmp (argv[i], "-h") || 0 == strcmp (argv[i], "--help")) {
			printf ("LCDproc IrMan input driver\n" "\t-d\t--device\tSelect the input device to use\n" "\t-d\t--config\tSelect the configuration file to use\n" "\t-h\t--help\t\tShow this help information\n");
			return -1;
		} else {
			printf ("Invalid parameter: %s\n", argv[i]);
		}
	}

	if (ir_init_commands (ptrconfig, 1) < 0) {
		fprintf (stderr, "error initialising commands: %s\n", strerror (errno));
		exit (1);
	}

	portname = ir_default_portname ();
	if (portname == NULL) {
		if (ptrdevice == NULL) {
			portname = ptrdevice;
		} else {
			fprintf (stderr, "error no device defined\n");
			exit (1);
		}
	}

	driver->getkey = irmanin_getkey;
	driver->close = irmanin_close;

	for (i = 1; codes[i] != NULL; i++) {
		if (ir_register_command (codes[i], i) < 0) {
			if (errno == ENOENT) {
				fprintf (stderr, "%s: no code set for `%s'\n", progname, codes[i]);
			} else {
				fprintf (stderr, "error registering `%s': `%s'\n", codes[i], strerror (errno));
			}
		}
	}

	errno = 0;
	if (ir_init (portname) < 0) {
		fprintf (stderr, "%s: error initialising Irman: `%s'\n", strerror (errno), portname);
		exit (1);
	}

	return 1;						  // 200 is arbitrary.  (must be 1 or more)
}

void
irmanin_close ()
{
	if (irmanin->framebuf != NULL)
		free (irmanin->framebuf);

	irmanin->framebuf = NULL;

	ir_free_commands ();
	ir_finish ();
}

//////////////////////////////////////////////////////////////////////
// Tries to read a character from an input device...
//
// Return 0 for "nothing available".
//
char
irmanin_getkey ()
{
	int i;
	int cmd;
	char key;

	key = (char) 0;
	switch (cmd = ir_poll_command ()) {
	case IR_CMD_ERROR:
		fprintf (stderr, "%s: error reading command: %s\n", progname, strerror (errno));
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
