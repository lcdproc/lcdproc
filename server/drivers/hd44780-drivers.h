/*
 * Low-level driver types, headers and names.
 *
 * To add support for a new driver in this file:
 *  1. include your header file
 *  2. Add a new connectionType
 *  3. Add an entry in the connectionMapping structure
 */

#ifndef HD44780_DRIVERS_H
#define HD44780_DRIVERS_H

// hd44780 specific header files
#include "hd44780-4bit.h"
#include "hd44780-ext8bit.h"
#include "hd44780-serialLpt.h"
#include "hd44780-winamp.h"
// add new connection type header files here

enum connectionType { HD_4bit, HD_8bit, HD_serialLpt, HD_winamp,
   // add new connection types here

   HD_unknown
};

static struct ConnectionMapping {
   enum connectionType type;
   char *connectionTypeStr;
   int (*init_fn) (HD44780_functions * hd44780_functions, lcd_logical_driver * driver, char *args, unsigned int port);
   const char *helpMsg;
} connectionMapping[] = {
   // connectionType enumerator
   // string to identify connection on command line
   // your initialisation function
   // help string for your particular connection
   {
   HD_4bit, "4bit", hd_init_4bit, "\t-e\t--extended\tEnable three or more displays\n"}, {
   HD_8bit, "8bit", hd_init_ext8bit, "\tnone\n"}, {
   HD_serialLpt, "serialLpt", hd_init_serialLpt, "\tnone\n"}, {
   HD_winamp, "winamp", hd_init_winamp, "\t-e\t--extended\tEnable three or more displays\n"},
       // add new connection types and their string specifier here
       // default, end of structure element (do not delete)
   {
   HD_unknown, "", NULL, ""}
};

#endif
