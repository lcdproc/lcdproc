#ifndef SCREEN_H
#define SCREEN_H

#include "shared/LL.h"
#include "clients.h"

typedef struct screen {
	char *id;
	char *name;
	int wid, hgt;
	int priority;
	int duration;
	int heartbeat;
	int timeout;
	int backlight_state;
	char *keys;
	LinkedList *widgets;
	client *parent;
} screen;

extern int  default_duration ;
extern int  default_priority ;

screen *screen_create ();
int screen_destroy (screen * s);

screen *screen_find (client * c, char *id);

int screen_add (client * c, char *id);
int screen_remove (client * c, char *id);

#endif
