/*
  main.c for LCDd

  Contains main(), plus signal callback functions and a help screen.

  Program init, command-line handling, and the main loop are
  implemented here.  Also, minimal data about the program such as
  the revision number.

  Some of this stuff should probably be move elsewhere eventually,
  such as command-line handling and the main loop.  main() is supposed
  to be "dumb".
  
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "../shared/debug.h"

#include "drivers/lcd.h"
#include "sock.h"
#include "clients.h"
#include "screenlist.h"
#include "screen.h"
#include "parse.h"
#include "render.h"
#include "serverscreens.h"
#include "input.h"
#include "main.h"


char *version = VERSION;
char *protocol_version = PROTOCOL_VERSION;
char *build_date = __DATE__;


/* no longer needed
static screen_size sizes[] =
{
   {"16x2", 16, 2},
   {"16x4", 16, 4},
   {"20x2", 20, 2},
   {"20x4", 20, 4},
   {"40x2", 40, 2},
   {"40x4", 40, 4},
   {NULL  ,  0, 0},
};
*/

// This is currently only a list of available arguments, but doesn't
// really *do* anything.  It just helps to figure out which parameters
// go to the server, and which ones go to individual drivers...
static parameter args[] = {
   {"-h", "--help"},
   {"-d", "--driver"},
   {"-t", "--type"},
   {"-f", "--foreground"},
   {"-b", "--backlight"},
   {NULL, NULL},
};


void exit_program (int val);
void HelpScreen ();


int
main (int argc, char **argv)
{
   // TODO:  Use a config file!
   //char cfgfile[256] = "/etc/LCDd.cf";
   int i, err, tmp;
   int daemon_mode = 1;
   int disable_server_screen = 1;
   int child;
   screen *s = NULL;
   char *str, *ing;		// strings for commandline handling
//   screen_size *size = &sizes[0]; // No longer needed



   // Ctrl-C will cause a clean exit...
   signal (SIGINT, exit_program);
   // and "kill"...
   signal (SIGTERM, exit_program);
   // and "kill -HUP" (hangup)...
   signal (SIGHUP, exit_program);
   // and just in case, "kill -KILL" (which cannot be trapped; but oh well)
   signal (SIGKILL, exit_program);



   // If no paramaters given, give the help screen.
   if (argc == 1)
      HelpScreen ();

   // Don't spawn a child if we're using the curses driver
   for (i = 1; i < argc; i++) {
      if (0 == strcmp (argv[i], "-f") || 0 == strcmp (argv[i], "--foreground") || 0 == strcmp (argv[i], "curses"))
	 daemon_mode = 0;
   }

   // Now, go into daemon mode...
#ifndef DEBUG
   if (daemon_mode) {
      if ((child = fork ()) != 0) {
	 usleep (1500000);	// Wait for child to initialize
	 exit (0);		/* PARENT EXITS */
      }
      // This line removed because it eats error messages...
      //setsid();                                       /* RELEASE TTY */
   }
#endif



   // Set up lcd driver base system
   lcd_init ("");


   // Parse the command line now...
   // TODO:  Move this to a separate function?
   for (i = 1; i < argc; i++) {
      if (0 == strcmp (argv[i], "-d") || 0 == strcmp (argv[i], "--driver")) {
	 if (argc <= i + 1)
	    HelpScreen ();
	 str = argv[++i];
	 ing = NULL;

	 // Check to see if the next parameter is intended for LCDd,
	 // or if it should be passed to the driver...
	 if (argc > i + 1) {
	    int j, skip = 0;
	    for (j = 1; args[j].lg; j++)	// check each option except "help"
	       if (!strcmp (argv[i + 1], args[j].sh) || !strcmp (argv[i + 1], args[j].lg))
		  skip = 1;

	    if (!skip) {
	       ing = argv[++i];
	    }
	    //else i++;
	 }
	 err = lcd_add_driver (str, ing);
	 if (err <= 0) {
	    printf ("Error loading driver %s.  Continuing anyway...\n", str);
	 }
	 if ((err > 0) && (0 == strcmp (str, "curses")))
	    daemon_mode = 0;
      } else if (0 == strcmp (argv[i], "-h") || 0 == strcmp (argv[i], "--help")) {
	 HelpScreen ();
      }
      // Redundant, but it prevents errors...
      else if (0 == strcmp (argv[i], "-f") || 0 == strcmp (argv[i], "--foreground")) {
	 daemon_mode = 0;
      } else if (0 == strcmp (argv[i], "-b") || 0 == strcmp (argv[i], "--backlight")) {
	 if (argc <= i + 1)
	    HelpScreen ();
	 str = argv[++i];
	 if (0 == strcmp (str, "off"))
	    backlight_state = backlight = BACKLIGHT_OFF;
	 else if (0 == strcmp (str, "on"))
	    backlight_state = backlight = BACKLIGHT_ON;
	 else if (0 == strcmp (str, "open"))
	    backlight = BACKLIGHT_OPEN;
	 else
	    HelpScreen ();
      } else if (0 == strcmp (argv[i], "-t") || 0 == strcmp (argv[i], "--type")) {
	 if (i + 1 > argc)
	    HelpScreen ();
	 else {
	    int wid, hgt;

	    i++;
	    sscanf (argv[i], "%ix%i", &wid, &hgt);
	    if (wid > 80 || wid < 16 || hgt > 25 || hgt < 2) {
	       fprintf (stderr, "LCDd: Invalid lcd size \"%s\".  Using 20x4.\n", argv[i]);
	       lcd.wid = 20;
	       lcd.hgt = 4;
	    } else {
	       lcd.wid = wid;
	       lcd.hgt = hgt;
	    }

/* no longer needed
	    int j=0, valid=0;
	    i++;
	    for(j=0; sizes[j].size; j++)
	    {
	       if(0 == strcmp(sizes[j].size, argv[i]))
	       {
		  valid = 1;
		  size = &sizes[j];
		  lcd.wid = size->wid;
		  lcd.hgt = size->hgt;
	       }
	    }
	    if(!valid)
	    {
	       fprintf(stderr, "LCDd: Invalid lcd size \"%s\".  Using 20x4.\n", argv[i]);
	    }
*/
	 }
      } else if (0 == strcmp (argv[i], "-i") || 0 == strcmp (argv[i], "--serverinfo")) {
	 if (i + 1 > argc)
	    HelpScreen ();
	 else {
	    i++;
	    if (0 == strcmp (argv[i], "off"))
	       disable_server_screen = 1;
	    if (0 == strcmp (argv[i], "on"))
	       disable_server_screen = 0;
	 }
      } else {
	 // otherwise...  Get help!
	 printf ("Invalid parameter: %s\n", argv[i]);
	 HelpScreen ();
      }
   }



   // Now init a bunch of required stuff...

   if (sock_create_server () <= 0) {
      printf ("Error opening socket.\n");
      return 1;
   }

   if (client_init () < 0) {
      printf ("Error initializing client list\n");
      return 1;
   }

   if (screenlist_init () < 0) {
      printf ("Error initializing screen list\n");
      return 1;
   }
   // Make sure the server screen shows up every once in a while..
   if (server_screen_init () < 0) {
      printf ("Error initializing server screens\n");
      return 1;
   } else if (disable_server_screen) {
      server_screen->priority = 256;
   }




   // Main loop...
   while (1) {
      sock_poll_clients ();
      parse_all_client_messages ();
      handle_input ();

      // TODO:  Move this code to screenlist.c...
      // ... it should just say "handle_screens();"
      // Timer gets reset by screenlist_next()
      timer++;
      // this line's here because s was getting overwritten at one time...
      //s = screenlist_current();
      if (s && (timer >= s->duration)) {
	 screenlist_next ();
      }
      // Just in case it gets out of hand...
      if (timer >= 0x10000)
	 timer = 0;
      update_server_screen (timer);
      s = screenlist_current ();

      // render something here...
      if (s)
	 draw_screen (s, timer);
      else
	 no_screen_screen (timer);

      usleep (TIME_UNIT);
   }


   // Quit!
   exit_program (0);

   return 0;
}


void
exit_program (int val)
{
   // TODO: These things shouldn't be so interdependent.  The order
   // things are shut down in shouldn't matter...

   // Say goodbye!
   goodbye_screen ();
   // Can go anywhere...
   lcd_shutdown ();

   // Must come before screenlist_shutdown
   client_shutdown ();
   // Must come after client_shutdown
   screenlist_shutdown ();
   // Should come after client_shutdown
   sock_close_all ();

   exit (0);

}


void
HelpScreen ()
{
   printf ("LCDproc server daemon, %s\n", version);
   printf ("Copyright (c) 1999 Scott Scriven, William Ferrell, and misc contributors\n");
   printf ("This program is freely redistributable under the terms of the GNU Public License\n\n");
   printf ("Usage: LCDd [options]\n");
   printf ("\tOptions in []'s are optional.  Available options are:\n");
   printf ("\t-h\t--help\n\t\t\tDisplay this help screen\n");
   printf ("\t-t\t--type <size>\n\t\t\tSelect an LCD size (20x4, 16x2, etc...)\n");
   printf ("\t-d\t--driver <driver> [args]\n\t\t\tAdd a driver to use:\n");
   printf ("\t\t\tCFontz, curses, HD44780, irmanin, joy,\n\t\t\tMtxOrb, text\n");
   printf ("\t\t\t(args will be passed to the driver for init)\n");
   printf ("\t-f\t--foreground\n\t\t\tRun in the foreground (no daemon)\n");
   printf ("\t-b\t--backlight <mode>\n\t\t\tSet backlight mode (on, off, open)\n");
   printf ("\t-i\t--serverinfo off\n\t\t\tSet the server screen to low priority\n");
   printf ("\n");
   printf ("\tUse \"man LCDd\" for more info.\n");
   printf ("\tHelp on each driver's parameters are obtained upon request:\n\t\t\"LCDd -d driver --help\"\n");
   printf ("Example:\n");
   printf ("\tLCDd -d MtxOrb \"--device /dev/lcd --contrast 200\" -d joy\n");
   printf ("\n");
   exit (0);
}
