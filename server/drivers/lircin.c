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
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <lirc/lirc_client.h>

#define __u32 unsigned int
#define __u8 unsigned char

#include "shared/str.h"

#define NAME_LENGTH 128

#include "lcd.h"
#include "lircin.h"

char *progname = "lircin";

int fd;
char buf[256];
struct sockaddr_un addr;

static struct lirc_config *config;

// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "LB216_";


//////////////////////////////////////////////////////////////////////////
////////////////////// Base "class" to derive from ///////////////////////
//////////////////////////////////////////////////////////////////////////

//void sigterm(int sig)
//{
//  ir_free_commands();
//  ir_finish();
//  raise(sig);
//}

MODULE_EXPORT void
lircin_close (Driver * drvthis)
{
	lirc_freeconfig (config);
	lirc_deinit ();
}

//////////////////////////////////////////////////////////////////////
// Tries to read a character from an input device...
//
// Return 0 for "nothing available".
//
MODULE_EXPORT char
lircin_getkey (Driver * drvthis)
{
	char key;
	char *ir, *cmd;

	if (!(ir = lirc_nextir ())) {
		return 0;
	} else {
		if (!(cmd = lirc_ir2char (config, ir)))
			return 0;

		printf ("lirc: \"%s\"\n", cmd);
		sscanf (cmd, "%c", &key);
		printf ("\n%c\n", key);
		free (ir);
		return key;
	}

	return 0;
}

////////////////////////////////////////////////////////////
// init() should set up any device-specific stuff, and
// point all the function pointers.
MODULE_EXPORT int
lircin_init (Driver * drvthis, char *args)
{

/* assign funktions */

/* open socket to lirc */

	if (-1 == (fd = lirc_init ("lcdd", 1))) {
		fprintf (stderr, "no infrared remote support available\n");
		return -1;
	}

	if (0 != lirc_readconfig (NULL, &config, NULL)) {
		lirc_deinit ();
		return -1;
	}
	fcntl (fd, F_SETFL, O_NONBLOCK);
	fcntl (fd, F_SETFD, FD_CLOEXEC);

/* socket shouldn block lcdd */

	fcntl (fd, F_SETFL, O_NONBLOCK);
	fcntl (fd, F_SETFD, FD_CLOEXEC);

	return 0;
}
