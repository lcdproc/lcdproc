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
#include "config.h"
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

int lcd_wid;
int lcd_hgt;
int lcd_cellwid;
int lcd_cellhgt;
static struct utsname unamebuf;

/*
  Mode List:
    See below...  (default_sequence[])
*/

static void HelpScreen();
static void exit_program(int val);
static void main_loop();

static mode *sequence = NULL;
static int islow = -1;

// 1/8th second is a single time unit...
#define TIME_UNIT 125000

// Contains a list of modes to run
static mode default_sequence[] =
{
// which on  off  inv  timer/visible  
	{'C', 1, 2, 0, 0xffff, 0,},			// [C]PU
	{'M', 4, 16, 0, 0xffff, 0,},		// [M]emory
	{'X', 64, 128, 1, 0xffff, 0,},		// [X]-load (load histogram)
	{'T', 4, 64, 0, 0xffff, 0,},		// [T]ime/Date
	{'A', 999, 9999, 0, 0xffff, 0,},	// [A]bout (credits)

	{1, 0, 0, 0, 0, 0,},	// Modes after this line will not be run by default...
	// ... all non-default modes must be in here!
	// ... they will not show up otherwise.
	{'P', 1, 2, 0, 0xffff, 0,},			// [O]ld Timescreen
	{'O', 4, 64, 0, 0xffff, 0,},		// [O]ld Timescreen
	{'K', 4, 64, 0, 0xffff, 0,},		// big cloc[K] 
	{'U', 4, 128, 0, 0xffff, 0,},		// Old [U]ptime Screen
	{'B', 32, 256, 1, 0xffff, 0,},		// [B]attery Status
	{'G', 1, 2, 0, 0xffff, 0,},			// Cpu histogram [G]raph
	{'S', 16, 256, 1, 0xffff, 0,},		// [S]ize of biggest programs
	{'D', 256, 256, 1, 0xffff, 0,},		// [D]isk stats
	{0, 0, 0, 0, 0,},					// No more..  all done.
};

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

int
main(int argc, char **argv)
{
	char *server = NULL;
	int i, j, seqlen;
	int port = LCDPORT;
	int daemonize = FALSE;

	if(uname(&unamebuf) == -1)
	{
		perror("uname");
		return(EXIT_FAILURE);
	}

	seqlen = 0;
	while(default_sequence[seqlen].which != 0)
		seqlen++;
	seqlen++;
	sequence = (mode*)malloc(seqlen*sizeof(mode));
	if(sequence == NULL)
	{
		perror("sequence malloc");
		return(EXIT_FAILURE);
	}
	for(i = 0; i < seqlen; i++)
	{
		//sequence[i] = default_sequence[i];
		memcpy(&sequence[i], &default_sequence[i], sizeof(mode));
	}

	// Ctrl-C will cause a clean exit...
	signal(SIGINT, exit_program);
	// and "kill"...
	signal(SIGTERM, exit_program);
	// and "kill -HUP" (hangup)...
	signal(SIGHUP, exit_program);
	// and just in case, "kill -KILL" (which cannot be trapped; but oh well)
	signal(SIGKILL, exit_program);

	// Command line
	for(i = 1, j = 0; i < argc; i++)
	{
		if(argv[i][0] == '-')
		{
			switch(argv[i][1])
			{
				// s is for server
				case 'S':
				case 's':
					if(argc < i + 1)
						HelpScreen();
					if(server == NULL)
						server = argv[++i];
					else
						fprintf(stderr, "Ignoring additional server: %s\n", argv[++i]);
					break;
					// p is for port
				case 'P':
				case 'p':
					if(argc < i + 1)
						HelpScreen();
					port = atoi(argv[++i]);
					if(port < 1 && port > 0xffff)
						fprintf(stderr, "Warning:  Port %d outside of standard range\n", port);
					break;
				case 'e':
				case 'E':
					if(argc < i + 1)
						HelpScreen();
					islow = atoi(argv[++i]);
					break;
				case 'd':
				case 'D':
					daemonize = TRUE;
					break;
					// otherwise...  Get help!
				default:
					HelpScreen();
					break;
			}
		}
		else if(strlen(argv[i]) == 1)
		{
			int k, found = FALSE;

			if(j >= seqlen)
				continue;

			// Grab the mode letter...
			sequence[j].which = argv[i][0];

			for(k = 0; k < seqlen; k++)
			{
				if(toupper(sequence[j].which) == default_sequence[k].which)
				{
					//sequence[j] = default_sequence[k];
					memcpy(&sequence[j], &default_sequence[k], sizeof(mode));
					found = TRUE;
					break;
				}
			}

			if(!found)
			{
				fprintf(stderr, "Invalid Mode: %c, ignoring\n", argv[i][0]);
			}
			else
			{
				j++;
				memcpy(&sequence[j], &default_sequence[seqlen-1], sizeof(mode));
				//sequence[j] = default_sequence[seqlen-1];
			}
		}
	}

	if(server == NULL)
		server = "localhost";

	// Connect to the server...
	usleep(500000);		// wait for the server to start up
	sock = sock_connect(server, port);
	if(sock <= 0)
	{
		printf("Error connecting to LCD server %s on port %i.\nCheck to see that the server is running and operating normally.\n", server, port);
		return 0;
	}
	sock_send_string(sock, "hello\n");
	usleep(500000);			// wait for the server to say hi.

	// We grab the real values below, from the "connect" line.
	lcd_wid = 20;
	lcd_hgt = 4;
	lcd_cellwid = 5;
	lcd_cellhgt = 8;

	if(daemonize)
		if(daemon(1,0) != 0)
			fprintf(stderr, "Error: daemonize failed");

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
	printf("\tMode letters: \t[C]pu [G]raph [T]ime [M]emory [X]load [D]isk [B]attery\n\t\t\tproc_[S]izes [O]ld_time big_cloc[K] [U]ptime CPU_SM[P]\n\t\t\t[A]bout\n");
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
	Quit = 1;
	sock_close(sock);
	mode_close();
	free(sequence);
	exit(val);
}

///////////////////////////////////////////////////////////////////
// Main program loop...
//
void
main_loop()
{
	int i = 0, j;
	//int status=0;
	char buf[8192];
	char *argv[256];
	int argc, newtoken;
	int len;

	// Main loop
	// Run whatever screen we want, then wait.  Woo-hoo!
	while(!Quit)
	{
		// Check for server input...
		len = sock_recv(sock, buf, 8000);

		// Handle server input...
		while(len > 0)
		{
			// Now split the string into tokens...
			//for(i=0; i<argc; i++) argv[i]=NULL; // Get rid of old tokens
			argc = 0;
			newtoken = 1;
			for(i = 0; i < len; i++)
			{
				switch(buf[i])
				{
					case ' ':
						newtoken = 1;
						buf[i] = 0;
					break;
					default:	// regular chars, keep tokenizing
						if(newtoken)
							argv[argc++] = buf + i;
						newtoken = 0;
					break;
					case '\0':
					case '\n':
						buf[i] = 0;
						if(argc > 0)
						{
							//printf("%s %s\n", argv[0], argv[1]);
							if(0 == strcmp(argv[0], "listen"))
							{
								for(j = 0; sequence[j].which; j++)
								{
									if(sequence[j].which == argv[1][0])
									{
										sequence[j].visible = 1;
										//debug("Listen %s\n", argv[1]);
									}
								}
							}
							else if(0 == strcmp(argv[0], "ignore"))
							{
								for(j = 0; sequence[j].which; j++)
								{
									if(sequence[j].which == argv[1][0])
									{
										sequence[j].visible = 0;
										//debug("Ignore %s\n", argv[1]);
									}
								}
							}
							else if(0 == strcmp(argv[0], "key"))
							{
								debug("Key %s\n", argv[1]);
							}
							else if(0 == strcmp(argv[0], "menu"))
							{
							}
							else if(0 == strcmp(argv[0], "connect"))
							{
								int a;
								for(a = 1; a < argc; a++)
								{
									if(0 == strcmp(argv[a], "wid"))
										lcd_wid = atoi(argv[++a]);
									else if(0 == strcmp(argv[a], "hgt"))
										lcd_hgt = atoi(argv[++a]);
									else if(0 == strcmp(argv[a], "cellwid"))
										lcd_cellwid = atoi(argv[++a]);
									else if(0 == strcmp(argv[a], "cellhgt"))
										lcd_cellhgt = atoi(argv[++a]);
								}
								sock_send_string(sock, "client_set -name LCDproc\n");
							}
							else if(0 == strcmp(argv[0], "bye"))
							{
								//printf("Exiting LCDproc\n");
								exit_program(EXIT_SUCCESS);
							}
							else if(0 == strcmp(argv[0], "success"))
							{
							}
							else
							{
								int j;
								for(j = 0; j < argc; j++)
									printf("%s ", argv[j]);
								printf("\n");
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
		for(i = 0; sequence[i].which > 1; i++)
		{
			sequence[i].timer++;
			if(sequence[i].visible)
			{
				if(sequence[i].timer >= sequence[i].on_time)
				{
					sequence[i].timer = 0;
					// Now, update the screen...
					update_screen(&sequence[i], 1);
				}
			}
			else
			{
				if(sequence[i].timer >= sequence[i].off_time)
				{
					sequence[i].timer = 0;
					// Now, update the screen...
					update_screen(&sequence[i], sequence[i].show_invisible);
				}
			}
			if(islow > 0)
				usleep(islow * 10000);
		}

		// Now sleep...
		usleep(TIME_UNIT);
	}
}

