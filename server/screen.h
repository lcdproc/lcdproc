#ifndef SCREEN_H
#define SCREEN_H

#include "../shared/LL.h"
#include "clients.h"

typedef struct screen {
   char *id;
   char *name;
   int wid, hgt;
   int priority;
   int duration;
   int heartbeat;
   LL *widgets;
   client *parent;
} screen;


screen *screen_create ();
int screen_destroy (screen * s);

screen *screen_find (client * c, char *id);

int screen_add (client * c, char *id);
int screen_remove (client * c, char *id);


#endif
