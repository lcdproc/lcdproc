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

#include "../../shared/debug.h"
#include "../../shared/str.h"

#define NAME_LENGTH 128

#include "lcd.h"
#include "lircin.h"

char *progname = "lircin";

int fd;
char buf[256];
struct sockaddr_un addr;

static struct lirc_config *config;

//////////////////////////////////////////////////////////////////////////
////////////////////// Base "class" to derive from ///////////////////////
//////////////////////////////////////////////////////////////////////////

lcd_logical_driver *lircin;

//void sigterm(int sig)
//{
//  ir_free_commands();
//  ir_finish();
//  raise(sig);
//}

void
lircin_close ()
{
   lirc_freeconfig (config);
   lirc_deinit ();
}

//////////////////////////////////////////////////////////////////////
// Tries to read a character from an input device...
//
// Return 0 for "nothing available".
//
char
lircin_getkey ()
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
int
lircin_init (struct lcd_logical_driver *driver, char *args)
{

/* assign funktions */

   lircin = driver;

   driver->getkey = lircin_getkey;
   driver->close = lircin_close;

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

   return 1;			// 200 is arbitrary.  (must be 1 or more)
}
