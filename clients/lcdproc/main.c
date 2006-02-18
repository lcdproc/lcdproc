#include "getopt.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/utsname.h>
#include <sys/param.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "main.h"
#include "mode.h"
#include "shared/sockets.h"
#include "shared/debug.h"

// TODO: Commenting...  Everything!

int Quit = 0;
int sock;

char *version = VERSION;
char *build_date = __DATE__;

int lcd_wid = 0;
int lcd_hgt = 0;
int lcd_cellwid = 0;
int lcd_cellhgt = 0;
static struct utsname unamebuf;

/*
  Mode List:
    See below...  (default_sequence[])
*/

static void HelpScreen();
static void exit_program(int val);
static void main_loop();

static int islow = -1;

// 1/8th second is a single time unit...
#define TIME_UNIT 125000

/* list of modes to run */
#ifdef LCDPROC_MENUS
mode sequence[] =
{
	// flags default ACTIVE will run by default
	// longname    which on  off inv  timer flags  
	{ "CPU",       'C',   1,    2, 0, 0xffff, ACTIVE,},	// [C]PU
	{ "Memory",    'M',   4,   16, 0, 0xffff, ACTIVE,},	// [M]emory
	{ "Load",      'X',  64,  128, 1, 0xffff, ACTIVE,},	// [X]-load (load histogram)
	{ "TimeDate",  'T',   4,   64, 0, 0xffff, ACTIVE,},	// [T]ime/Date
	{ "About",     'A', 999, 9999, 0, 0xffff, ACTIVE,},	// [A]bout (credits)
#else
mode *sequence = NULL;

mode default_sequence[] =
{
	// longname    which on  off inv  timer visible/flags  
	{ "CPU",       'C',   1,    2, 0, 0xffff, 0,},	// [C]PU
	{ "Memory",    'M',   4,   16, 0, 0xffff, 0,},	// [M]emory
	{ "Load",      'X',  64,  128, 1, 0xffff, 0,},	// [X]-load (load histogram)
	{ "TimeDate",  'T',   4,   64, 0, 0xffff, 0,},	// [T]ime/Date
	{ "About",     'A', 999, 9999, 0, 0xffff, 0,},	// [A]bout (credits)
	{  NULL, 1, 0, 0, 0, 0, 0,},	// Modes after this line will not be run by default...
	// ... all non-default modes must be in here!
	// ... they will not show up otherwise.
#endif
	{ "SMP-CPU",   'P',   1,    2, 0, 0xffff, 0,},	// CPU_SM[P]
	{ "OldTime",   'O',   4,   64, 0, 0xffff, 0,},	// [O]ld Timescreen
	{ "BigClock",  'K',   4,   64, 0, 0xffff, 0,},	// big cloc[K] 
	{ "Uptime",    'U',   4,  128, 0, 0xffff, 0,},	// Old [U]ptime Screen
	{ "Battery",   'B',  32,  256, 1, 0xffff, 0,},	// [B]attery Status
	{ "CPUGraph",  'G',   1,    2, 0, 0xffff, 0,},	// CPU histogram [G]raph
	{ "ProcSize",  'S',  16,  256, 1, 0xffff, 0,},	// [S]ize of biggest programs
	{ "Disk",      'D', 256,  256, 1, 0xffff, 0,},	// [D]isk stats
	{ "MiniClock", 'E',   4,   64, 0, 0xffff, 0,},	// [E]ssential clock 
	{  NULL, 0, 0, 0, 0, 0,},		// No more..  all done.
};

#ifdef LCDPROC_MENUS
int seqlen = sizeof(sequence) / sizeof(mode);
#else
int seqlen = sizeof(default_sequence) / sizeof(mode);
#endif //LCDPROC_MENUS


// TODO: Config file; not just command line

const char *get_hostname()
{
	return(unamebuf.nodename);
}

const char *get_sysname()
{
	return(unamebuf.sysname);
}

const char *get_sysrelease()
{
	return(unamebuf.release);
}

#ifdef LCDPROC_MENUS //set_mode function
int set_mode(int shortname, char * longname, int state)
{
	int k;

	/* ignore already selected modes */
	for (k = 0; (k < seqlen) && (sequence[k].which != 0); k++) {
		if (((sequence[k].longname != NULL) &&
		     (0 == strcasecmp(longname, sequence[k].longname))) || 
		    (shortname == sequence[k].which)) {
			if (!state) {
				sequence[k].flags &= (~ACTIVE & ~INITIALIZED); /* both the active and inititialized bits */
				sprintf (buffer, "screen_del %c\n", shortname );
				sock_send_string (sock, buffer);
			} else
				sequence[k].flags |= ACTIVE;
			return 1; //found
		}
	}
	return 0; //not found
}

void clear_modes()
{
	int k;
	/* ignore already selected modes */
	for (k = 0; (k < seqlen) && (sequence[k].which != 0); k++) {
		sequence[k].flags &= (~ACTIVE);
	}
}
#endif //LCDPROC_MENUS

int
main(int argc, char **argv)
{
	int i, j;
	int c;
	char *server = NULL;
	int port = LCDPORT;
	int daemonize = FALSE;

	/* get uname information */
	if (uname(&unamebuf) == -1) {
		perror("uname");
		return(EXIT_FAILURE);
	}

#ifdef LCDPROC_MENUS
//	sequence = default_sequence;
#else
	/* allocate sequence */
	sequence = (mode *) malloc(seqlen * sizeof(mode));
	if (sequence == NULL) {
		perror("sequence malloc");
		return(EXIT_FAILURE);
	}

	/* copy default_sequence[] to sequence */
	for (i = 0; i < seqlen; i++) {
		//sequence[i] = default_sequence[i];
		memcpy(&sequence[i], &default_sequence[i], sizeof(mode));
	}
#endif //LCDPROC_MENUS

	/* setup error handlers */
	signal(SIGINT, exit_program);	// Ctrl-C
	signal(SIGTERM, exit_program);	// "regular" kill
	signal(SIGHUP, exit_program);	// kill -HUP
	signal(SIGKILL, exit_program);	// kill -9 [cannot be trapped; but ...]

	/* No error output from getopt */
	opterr = 0;

	/* get options */
	while ((c = getopt( argc, argv, "s:p:e:d")) > 0) {
		switch (c) {
			// s is for server
			case 's':
				if (server == NULL)
					server = optarg;
				else
					fprintf(stderr, "Ignoring additional server: %s\n", optarg);
				break;
			// p is for port
			case 'p':
				port = atoi(optarg);
				if ((port < 1) && (port > 0xFFFF)) {
					fprintf(stderr, "Warning:  Port %d outside of legal range\n", port);
					return(EXIT_FAILURE);
				}	
				break;
			case 'e':
				islow = atoi(optarg);
				break;
			case 'd':
				daemonize = TRUE;
				break;
			// otherwise...  Get help!
			case '?':	// unknown option or missing argument	
			default:
				HelpScreen();
				break;
		}
	}	

#ifdef LCDPROC_MENUS
	/* parse arguments */
	if (argc > 1 )		//user specified some modes, so clear all defaults
		clear_modes();
	for (i = (optind > 0) ? optind : 1; i < argc; i++) {
		int shortname = (strlen(argv[i]) == 1) ? toupper(argv[i][0]) : '\0';
		int found = set_mode(shortname, argv[i], 1);
#else
	/* parse arguments */
	for (i = (optind > 0) ? optind : 1, j = 0; i < argc; i++) {
		int shortname = (strlen(argv[i]) == 1) ? toupper(argv[i][0]) : '\0';
		int k, found = FALSE;

		/* skip this round if we used all allocated slots */
		if (j >= seqlen)
			continue;

		/* ignore already selected modes */
		for (k = 0; (k < j) && (sequence[k].which != 0); k++) {
			if (((sequence[k].longname != NULL) &&
			     (0 == strcasecmp(argv[i], sequence[k].longname))) || 
			    (shortname == sequence[k].which)) {
				found = TRUE;
			}
		}
		if (found) {
			fprintf(stderr, "Warning: ignoring duplicate mode\n");
			continue;
		}	

		// try to find the mode from the mode list
		for (k = 0; k < seqlen; k++) {
			if (((default_sequence[k].longname != NULL) &&
			     (0 == strcasecmp(argv[i], default_sequence[k].longname))) || 
			    (shortname == default_sequence[k].which)) {
				//sequence[j] = default_sequence[k];
				memcpy(&sequence[j], &default_sequence[k], sizeof(mode));
				j++;
				//sequence[j] = default_sequence[seqlen-1];
				memcpy(&sequence[j], &default_sequence[seqlen-1], sizeof(mode));
				found = TRUE;
				break;
			}
		}
#endif
		if (!found) {
			fprintf(stderr, "Invalid Mode: %c, ignoring\n", argv[i][0]);
			return(EXIT_FAILURE);
		}
	}

	if (server == NULL)
		server = "localhost";

	// Connect to the server...
	usleep(500000);		// wait for the server to start up
	sock = sock_connect(server, port);
	if (sock <= 0) {
		fprintf(stderr, "Error connecting to LCD server %s on port %d.\n"
		                "Check to see that the server is running and operating normally.\n",
				server, port);
		return(EXIT_FAILURE);
	}

	sock_send_string(sock, "hello\n");
	usleep(500000);			// wait for the server to say hi.

	// We grab the real values below, from the "connect" line.
	lcd_wid = 20;
	lcd_hgt = 4;
	lcd_cellwid = 5;
	lcd_cellhgt = 8;

	if (daemonize) {
		if (daemon(1,0) != 0) {
			fprintf(stderr, "Error: daemonize failed");
			return(EXIT_FAILURE);
		}
	}	

	// Init the status gatherers...
	mode_init();

	// And spew stuff!
	main_loop();

	// Clean up
	exit_program(EXIT_SUCCESS);

	return(0);
}

void
HelpScreen ()
{
	printf("LCDproc, %s\n", version);
	printf("Usage: lcdproc [-s server] [-p port] [-e islow] [-d] [modelist]\n");
	printf("\tOptions in []'s are optional.\n");
	printf("\tmodelist is \"mode [mode mode ...]\"\n");
	printf ("\tMode letters:\t[C]pu [G]raph [T]ime [M]emory [X]load [D]isk [B]attery\n"
		 	   "\t\t\tproc_[S]izes [O]ld_time big_cloc[K] [E]ssential_clock\n"
			   "\t\t\t[U]ptime CPU_SM[P] [A]bout\n");
	printf("\n");
	printf("\tislow is to slow down initial announcement of modes (in 1/100 sec)\n");
	printf("\tUse \"man lcdproc\" for more info.\n");
	printf("Example:\n");
	printf("\tlcdproc -s my.lcdproc.server.com C M X -p 13666\n");
	printf("\n");
	exit(EXIT_FAILURE);
}

///////////////////////////////////////////////////////////////////
// Called upon TERM and INTR signals...
//
void
exit_program(int val)
{
	//printf("exit program\n");
	Quit = 1;
	sock_close(sock);
	mode_close();
#ifndef LCDPROC_MENUS
	free(sequence);
#endif
	exit(val);
}

#ifdef LCDPROC_MENUS //menus_init
int
menus_init ()
{
	int k;	

	for (k = 0; sequence[k].which ; k++) {
		if (sequence[k].longname) {
			sprintf (buffer, "menu_add_item {} %c checkbox {%s} -value %s\n", sequence[k].which, sequence[k].longname, sequence[k].flags & ACTIVE ? "on" : "off");
			sock_send_string (sock, buffer);
		}
	}

#ifdef TESTMENUS
//	  # to be entered on escape from test_menu (but overwritten
//	  # for test_{checkbox,ring}
	sock_send_string (sock,  "menu_add_item {} ask menu {Leave menus?} -is_hidden true\n");
	sock_send_string (sock, "menu_add_item {ask} ask_yes action {Yes} -next _quit_\n");
	sock_send_string (sock, "menu_add_item {ask} ask_no action {No} -next _close_\n");
	sock_send_string (sock, "menu_add_item {} test menu {Test}\n");
	sock_send_string (sock, "menu_add_item {test} test_action action {Action}\n");
	sock_send_string (sock, "menu_add_item {test} test_checkbox checkbox {Checkbox}\n");
	sock_send_string (sock, "menu_add_item {test} test_ring ring {Ring} -strings {one\ttwo\tthree}\n");
	sock_send_string (sock, "menu_add_item {test} test_slider slider {Slider} -mintext < -maxtext > -value 50\n");
	sock_send_string (sock, "menu_add_item {test} test_numeric numeric {Numeric} -value 42\n");
	sock_send_string (sock, "menu_add_item {test} test_alpha alpha {Alpha} -value abc\n");
	sock_send_string (sock, "menu_add_item {test} test_ip ip {IP} -v6 false -value 192.168.1.1\n");
	sock_send_string (sock, "menu_add_item {test} test_menu menu {Menu}\n");
	sock_send_string (sock, "menu_add_item {test_menu} test_menu_action action {Submenu's action}\n");

//	  # no successor for menus. Since test_checkbox and test_ring have their
//	  # own predecessors defined the "ask" rule will not work for them
	sock_send_string (sock, "menu_set_item {} test -prev {ask}\n");

	sock_send_string (sock, "menu_set_item {test} test_action -next {test_checkbox}\n");
	sock_send_string (sock, "menu_set_item {test} test_checkbox -next {test_ring} -prev test_action\n");
	sock_send_string (sock, "menu_set_item {test} test_ring -next {test_slider} -prev {test_checkbox}\n");
	sock_send_string (sock, "menu_set_item {test} test_slider -next {test_numeric} -prev {test_ring}\n");
	sock_send_string (sock, "menu_set_item {test} test_numeric -next {test_alpha} -prev {test_slider}\n");
	sock_send_string (sock, "menu_set_item {test} test_alpha -next {test_ip} -prev {test_numeric}\n");
	sock_send_string (sock, "menu_set_item {test} test_ip -next {test_menu} -prev {test_alpha}\n");
	sock_send_string (sock, "menu_set_item {test} test_menu_action -next {_close_}\n");
#endif //TESTMENUS

	return 0;
}
#endif //LCDPROC_MENUS
///////////////////////////////////////////////////////////////////
// Main program loop...
//
void
main_loop()
{
	int i = 0, j;
	int connected = 0;
	char buf[8192];
	char *argv[256];
	int argc, newtoken;
	int len;

	// Main loop
	// Run whatever screen we want, then wait.  Woo-hoo!
	while (!Quit)
	{
		// Check for server input...
		len = sock_recv(sock, buf, 8000);

		// Handle server input...
		while (len > 0)
		{
			// Now split the string into tokens...
			//for (i=0; i<argc; i++) argv[i]=NULL; // Get rid of old tokens
			argc = 0;
			newtoken = 1;
			for (i = 0; i < len; i++)
			{
				switch (buf[i])
				{
					case ' ':
						newtoken = 1;
						buf[i] = 0;
					break;
					default:	// regular chars, keep tokenizing
						if (newtoken)
							argv[argc++] = buf + i;
						newtoken = 0;
					break;
					case '\0':
					case '\n':
						buf[i] = 0;
						if (argc > 0)
						{
							//printf("%s %s\n", argv[0], argv[1]);
							if (0 == strcmp(argv[0], "listen"))
							{
								for (j = 0; sequence[j].which; j++)
								{
									if (sequence[j].which == argv[1][0])
									{
#ifdef LCDPROC_MENUS
										sequence[j].flags |= VISIBLE;
#else
										sequence[j].visible = 1;
#endif
										//debug("Listen %s\n", argv[1]);
									}
								}
							}
							else if (0 == strcmp(argv[0], "ignore"))
							{
								for (j = 0; sequence[j].which; j++)
								{
									if (sequence[j].which == argv[1][0])
									{
#ifdef LCDPROC_MENUS
										sequence[j].flags &= ~VISIBLE;
#else
										sequence[j].visible = 0;
#endif
										//debug("Ignore %s\n", argv[1]);
									}
								}
							}
							else if (0 == strcmp(argv[0], "key"))
							{
								debug("Key %s\n", argv[1]);
							}
#ifdef LCDPROC_MENUS
							else if (0 == strcmp(argv[0], "menuevent"))
							{
								if (argc == 4 && (0 == strcmp(argv[1], "update"))) 
								{
									set_mode(argv[2][0],"", strcmp(argv[3],"off"));
								}
							}
#else
							else if (0 == strcmp(argv[0], "menu"))
							{
							}
#endif
							else if (0 == strcmp(argv[0], "connect"))
							{
								int a;
								for (a = 1; a < argc; a++)
								{
									if (0 == strcmp(argv[a], "wid"))
										lcd_wid = atoi(argv[++a]);
									else if (0 == strcmp(argv[a], "hgt"))
										lcd_hgt = atoi(argv[++a]);
									else if (0 == strcmp(argv[a], "cellwid"))
										lcd_cellwid = atoi(argv[++a]);
									else if (0 == strcmp(argv[a], "cellhgt"))
										lcd_cellhgt = atoi(argv[++a]);
								}
								connected = 1;
								{
									char buffer[8192];
									snprintf(buffer, sizeof(buffer), "client_set -name {LCDproc %s}\n", get_hostname());
									sock_send_string(sock, buffer);
								}	
#ifdef LCDPROC_MENUS
								menus_init();
#endif
							}
							else if (0 == strcmp(argv[0], "bye"))
							{
								//printf("Exiting LCDproc\n");
								exit_program(EXIT_SUCCESS);
							}
							else if (0 == strcmp(argv[0], "success"))
							{
							}
							else
							{
								//int j;
								//for (j = 0; j < argc; j++)
								//	printf("%s ", argv[j]);
								//printf("\n");
							}
						}

						// Restart tokenizing
						argc = 0;
						newtoken = 1;
					break;
				} // switch( buf[i] )
			}

			len = sock_recv(sock, buf, 8000);
			//debug("\n");
		}

		// Gather stats...
		// Update screens...
		if (connected)
		{
			for (i = 0; sequence[i].which > 0; i++)
			{
				sequence[i].timer++;
#ifdef LCDPROC_MENUS
				if ((sequence[i].flags & ACTIVE) == 0 )
					continue;
				if (sequence[i].flags & VISIBLE)
#else
				if (sequence[i].visible)
#endif
				{
					if (sequence[i].timer >= sequence[i].on_time)
					{
						sequence[i].timer = 0;
						// Now, update the screen...
						update_screen(&sequence[i], 1);
					}
				}
				else
				{
					if (sequence[i].timer >= sequence[i].off_time)
					{
						sequence[i].timer = 0;
						// Now, update the screen...
						update_screen(&sequence[i], sequence[i].show_invisible);
					}
				}
				if (islow > 0)
					usleep(islow * 10000);
			}
		}

		// Now sleep...
		usleep(TIME_UNIT);
	}
}

