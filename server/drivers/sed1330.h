/*
 * Driver for SED1330 graphical displays
 * Header file
 */

#ifndef SED1330_H
#define SED1330_H

#include "lcd.h"

int sed1330_init( lcd_logical_driver * driver, char *args );
void sed1330_close();
void sed1330_clear();
void sed1330_string( int x, int y, char lcd[] );
void sed1330_chr( int x, int y, char c );
void sed1330_flush();
void sed1330_cursor( int x, int y, char state );
void sed1330_backlight( int on );
void sed1330_vbar( int x, int len );
void sed1330_hbar( int x, int y, int len );
void sed1330_num( int x, int num );
void sed1330_heartbeat( int type );

#endif
