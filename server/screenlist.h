#ifndef SCREENLIST_H
#define SCREENLIST_H

#include "screen.h"


#define SCR_HOLD 1
#define SCR_SKIP 2
#define SCR_BACK 3
#define RENDER_HOLD 11
#define RENDER_SKIP 12
#define RENDER_BACK 13

extern int screenlist_action;
extern int timer;

int screenlist_init();
int screenlist_shutdown();

LL * screenlist_getlist();
screen * screenlist_current();

int screenlist_add(screen *s);
screen * screenlist_next();
screen * screenlist_prev();

int screenlist_remove(screen *s);
int screenlist_remove_all(screen *s);


#endif
