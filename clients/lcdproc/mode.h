#ifndef MODE_H
#define MODE_H

#include "main.h"

//TODO: Net stats screen...
//TODO: "Who"
//TODO: biff / mail checking, etc...

extern char tmp[];
extern char buffer[];
extern char host[];

// Character to use for padding title bars, etc...
extern int PAD;
// Character for the "..." symbol.
extern int ELLIPSIS;

int mode_init (mode * sequence);
void mode_close ();

int update_screen (mode * m, int display);

int credit_screen (int rep, int display);

// These are for the modescreens' use only..
void reread (int f, char *errmsg);
int getentry (const char *tag, const char *bufptr);

#endif
