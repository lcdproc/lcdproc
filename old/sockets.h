#define LCDPORT 13666

/**********************************************************************
  LCDproc socket interface grammar:

  RawText     Print whatever you send, at whatever the current position is.

        or...

  0xFE        Command prefix, followed by...

  0x00        EOT.  End of transmission, where applicicable.
  0x01        Ping.  Will respond with a "pong".
  'k',x       Send "keypad" input, character x (A-Z)

  ?,x,...,EOT Send new mode sequence (x,...)...
  ?,x         Switch to new mode x immediately
  ?,xxxx      Pause in current mode for x frames/seconds (0=infinite)
  ?           Play -- continue mode cycle.
  ?,x,y,...,EOT
              Send raw text to be displayed.  x is style (raw, wrapped),
	      and y is num frames between lines displayed.

  ?,...       Send MtxOrb-like control commands (bargraphs, etc)
  
  ... more to come later ...
**********************************************************************/

/*****************************************************************
  LCDproc command line interface:  (while running)

  -command
      Tells LCDproc to interpret stdin as raw commands to send through
      the socket.  Input must be formatted as above, in socket interface.
  -function f
      Runs LCDproc external function f, where f is one of the predefined
      functions which can be assigned to keypad keys.  (like NEXTMODE, etc)
  -key x
      Simulates keypad press of key 'x', where 'x' is (A-Z).
  -print [time]
      Prints stdin on LCD one line at a time, with no line-wrapping (raw),
      with [time] frames between updates (lines).
  -wrap [time]
      Prints stdin as with "-print", but with line wrapping when possible.
  -contrast xxx
      Sets contrast to xxx (decimal)
  -backlight [on/off]
      Turns backlight [on/off/auto], or toggles it.
      If [off], stays off.
      If [on], stays on.
      If [auto], LCDproc controls backlight based on load, etc...
  -exit
  -quit
      Duh...  :)

******************************************************************/

/*****************************************************************
  LCDproc stuff supported in config file (loose approximation):

  Grammar is tcl-style.  I.e., "command arg1 arg2 ...".
  Spaces are used as argument separators, *until* it thinks it has the final
  argument.  So, "function thing shell myprogram arg1 arg2 arg3" would be
  split into "function", "thing", "shell", and "myprogram arg1 arg2 arg3".

  User-definable functions (use built-in's to create new ones?):
    Function mp3NextSong Shell /usr/local/bin/mp3player -next
    Function MySequence Sequence cpu mem xload
    Function OtherSequence Sequence time cd xload

  Keypad keys can be bound to any _function_:
    Key A mp3NextSong
    Key B HaltSystem
    Key C Menu
    Key D Next/+
    Key E OtherSequence


******************************************************************/

typedef struct sockaddr_in sockaddr_in;

int init_sockaddr (sockaddr_in *name,
		   const char *hostname,
		   unsigned short int port);

// Creates a socket in internet space
int CreateInetSocket(unsigned short int port);

// Checks the LCDproc port for a response...
int PingLCDport();

int StartSocketServer();

int PollSockets();

int read_from_client (int filedes);

int CloseAllConnections();
