#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>

#include "main.h"
#include "lcd.h"
#include "mode.h"
//#include "lock.h"
#include "sockets.h"


// TODO: Commenting...  Everything!


char version[] = "v0.3.4";
char build_date[] = "1998-06-20";
int Quit = 0;

/*
  Mode List:
    See below...  (default_sequence[])
*/

typedef struct mode {
  char which;
  int num_times;
  int delay_time;
} mode;

void HelpScreen();
void exit_program(int val);
void main_loop(mode *sequence);

#define MAX_SEQUENCE 256
// 1/8th second is a single time unit...
#define TIME_UNIT 125000


// Contains a list of modes to run
mode default_sequence[] = 
{
  { 'C', 32,  1,  },// [C]PU
  { 'M', 8,   4,  },// [M]emory
  { 'X', 1,   32, },// [X]-load (load histogram)
  { 'T', 8,   4,  },// [T]ime/Date
  { 'D', 32,  1,  },// [D]isk stats
  { 'A', 1,   16, },// [A]bout (credits)

  {  1 , 0,   0,  },// Modes after this line will not be run by default...
                    // ... all non-default modes must be in here!
                    // ... they will not show up otherwise.
  { 'O', 8,   4,  },// [O]ld Timescreen
  { 'U', 8,   4,  },// Old [U]ptime Screen
  { 'B', 32,  1,  },// [B]attery Status
  { 'G', 32,  1,  },// Cpu histogram [G]raph
  { 0,	0,    0,  },// No more..  all done.
};

// TODO: Clean up main()...  It is still way too big.

// TODO: Socket language, client handling...
// TODO: Config file; not just command line


int main(int argc, char **argv)
{
  char device[256] = "/dev/lcd";
  char cfgfile[256] = "/etc/lcdproc.cf";
  mode sequence[MAX_SEQUENCE];
  char driver[256] = "MtxOrb";
  int i, j, k;
  int already_running = 0;
  int contrast = 140;
  int tmp;

  memset(sequence, 0, sizeof(mode) * MAX_SEQUENCE);

  // Ctrl-C will cause a clean exit...
  signal(SIGINT, exit_program);
  // and "kill"...
  signal(SIGTERM, exit_program);
  // and "kill -HUP" (hangup)...
  signal(SIGHUP, exit_program);
  // and just in case, "kill -KILL" (which cannot be trapped; but oh well)
  signal(SIGKILL, exit_program);
  
  // Check to see if we are already running...
//  already_running = CheckForLock();
  already_running = PingLCDport();
  if(already_running < 0)
    {
      printf("Error checking for another LCDproc.\n");
      return -1;
    }
  
  
  // Communicate with the previously running LCDproc
  if(already_running > 0)
  {
    // "already_running" holds the pid of the LCDproc we want to talk to...

    // Do the command line  ("lcd pet status", or "lcd contrast 50")
    // And stuff...
    // ...Then exit
    printf("Detected another LCDproc.  (%i)  Exiting...\n", already_running);
    // Remove me ^^^^
    return 0;
  }

  tmp = StartSocketServer();
  if (tmp <= 0)
    {
      printf("Error starting socket server.\n");
      return 0;
    }
  
  
  
  // Command line
  memcpy(sequence, default_sequence, sizeof(default_sequence));
  for(i=1, j=0; i<argc; i++)
  {
    if(argv[i][0] == '-') switch(argv[i][1])
    {
      // "C is for cookie (erm, contrast), and that is good enough for me..."
      case 'C': 
      case 'c': if(argc < i+1) HelpScreen();
                contrast = atoi(argv[++i]);
                if(contrast <= 0) HelpScreen();
                break;
      // D for Device...
      case 'D': 
      case 'd': if(argc < i+1) HelpScreen();
                strcpy(device, argv[++i]);
                break;
      case 'L':
      case 'l': if(argc < i+1) HelpScreen();
	        strcpy(driver, argv[++i]);
		break;
      
      // otherwise...  Get help!
      default: HelpScreen(); break;
    }
    // Parse command line here...  read the man page.
    else if(strlen(argv[i]) == 1)
    {
      // Grab the mode letter...
      sequence[j].which = argv[i][0];

      // If we have just a letter with no numbers...
      // Set the defaults...
      for(tmp=0, k=0; default_sequence[k].which; k++)
      {
        if(toupper(sequence[j].which) == default_sequence[k].which)
        {
           memcpy(&sequence[j], &default_sequence[k], sizeof(mode));
           tmp=1;
           break;
        }
      }
      if(!tmp) { printf("Invalid Mode: %c\n", argv[i][0]); exit(0); }

      j++;
      // Set the last element to 0...
      memset(sequence + j, 0, sizeof(mode));
    } // End if(strlen(argv == 1))
    else
    {
      // A multicharacter parameter by itself is assumed to be a config file..
      strcpy(cfgfile, argv[i]);
      printf("Ignoring config file: %s\n", cfgfile);
    }
  }

  // Init the com port
  if(lcd_init(device, driver) < 1) 
  {
    printf("Cannot initialize %s.\n", device);
    exit(1);
  }
  mode_init();
  lcd.contrast(contrast);

  main_loop(sequence);
  
  // Clean up
  exit_program(0);
  return 0;
}


void HelpScreen()
{
  printf("LCDproc, %s\n", version);
  printf("Usage: lcdproc [-d device] [-c contrast] [modelist]\n");
  printf("\tOptions in []'s are optional.\n");
  printf("\t-l driver is the output driver to use:\n");
  printf("\t\tMtxOrb, curses, text, debug\n");
  printf("\t-d device is what the lcd display is hooked to. (/dev/cua0?)\n");
  printf("\t-c contrast sets the screen contrast (0 - 255)\n");
  printf("\tmodelist is \"mode [mode mode ...]\"\n");
  printf("\tMode letters: [C]pu [G]raph [T]ime [M]emory [X]load [D]isk [B]attery [O]ld Time screen [U]ptime [A]bout\n");
  printf("\n");
  printf("\tUse \"man lcdproc\" for more info.\n");
  printf("Example:\n");
  printf("\tlcdproc -d /dev/cua1 C M X -l MtxOrb\n");
  printf("\n");
  exit(0);
}


///////////////////////////////////////////////////////////////////
// Called upon TERM and INTR signals...
//
void exit_program(int val)
{
  Quit = 1;
  //unlock();  // Not needed any more...
  CloseAllConnections();
  goodbye_screen(0);
  lcd.flush();
  lcd.backlight(1);
  lcd.close();
  mode_close();
  exit(0);
}


///////////////////////////////////////////////////////////////////
// Main program loop...
//
void main_loop(mode *sequence)
{
  int i, j, k;
  int Quit=0;
  int status=0;
  int timer=0;

  int blink=0;
  int hold=0;
  int heartbeat=1;

  // Main loop
  // Run whatever screen we want, then wait.  Woo-hoo!
  for(i=0; !Quit; )
  {
    timer=0;
    for(j=0; hold || (j<sequence[i].num_times && !Quit); j++)
    {
      switch(sequence[i].which)
      {
	case 'g':
	case 'G': status = cpu_graph_screen(j); break;
        case 'c':
        case 'C': status = cpu_screen(j); break;
        case 'o':
        case 'O': status = clock_screen(j); break;
        case 'm':
        case 'M': status = mem_screen(j); break;
        case 'u':
        case 'U': status = uptime_screen(j); break;
        case 't':
        case 'T': status = time_screen(j); break;
        case 'd':
        case 'D': status = disk_screen(j); break;
        case 'x':
        case 'X': status = xload_screen(j); break;
        case 'b':
        case 'B': status = battery_screen(j); break;
        case 'a':
        case 'A': status = credit_screen(j); break;
        default: status = dumbass_screen(j); break;
      }

      
     
      
      for(k=0; k<sequence[i].delay_time; k++)
      {
        usleep(TIME_UNIT);  timer++;
        // Modify lcd status here...  (blinking, for example)
        switch(status)
        {
          case BLINK_ON : blink=1; break;
          case BLINK_OFF: blink=0; lcd.backlight(1); break;
          case BACKLIGHT_OFF: blink=0; lcd.backlight(0); break;
          case BACKLIGHT_ON : blink=0; lcd.backlight(1); break;
	  case HOLD_SCREEN : hold=1; break;
	  case CONTINUE    : hold=0; break;
        }
	status=0;
	
        if(blink) lcd.backlight(! ((timer&15) == 15));

	if(heartbeat)
	{
	  // Set this to pulsate like a real heart beat...
	  // (binary is fun...  :)
	  lcd.icon(!((timer+4)&5), 0);
	  lcd.chr(lcd.wid, 1, 0);
	}

	lcd.flush();
	
	PollSockets();
      }
    }
    i++;
    if(sequence[i].which < 2) i=0;
  }
}
