#ifndef SOCKETS_H
#define SOCKETS_H

#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef LCDPORT
#define LCDPORT 13666
#endif

/*
  Socket functions available to server and clients...
    (ignore the rest of the comments...  I was babbling out random ideas)

  This should have stuff to read/write sockets, open/close them, etc...
 */

// Client functions...
int sock_connect (char *host, unsigned short int port);
int sock_close (int fd);
// Send/receive lines of text
int sock_send_string (int fd, char *string);
// Recv gives only one line per call...
int sock_recv_string (int fd, char *dest, size_t maxlen);
// Send/receive raw data
int sock_send (int fd, void *src, size_t size);
int sock_recv (int fd, void *dest, size_t maxlen);

// Er, ignore the rest of this file.  I'll clean it up sometime...

/*****************************************************************
  LCDproc command line interface?:  (while running)

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

#endif
