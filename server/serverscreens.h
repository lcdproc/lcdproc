#ifndef SRVSTATS_H
#define SRVSTATS_H

#include "screen.h"


extern screen *server_screen;

int server_screen_init();
int update_server_screen(int timer);
int no_screen_screen(int timer);
int goodbye_screen();

#endif
