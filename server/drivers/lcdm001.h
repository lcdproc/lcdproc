#ifndef LCDM001_H
#define LCDM001_H

/********************************************************************
  lcdm001.h
******************************************************************/

// REMOVE: I don't thing this is actualy needed.
// extern lcd_logical_driver *lcdm001;

int lcdm001_init (struct lcd_logical_driver *driver, char *args);

#define DEFAULT_DEVICE		"/dev/lcd"
#define DEFAULT_CURSORBLINK	0

/*Heartbeat workaround
  set chars to be displayed instead of "normal" icons*/

#define OPEN_HEART ' '     //This combination is at least visible
#define FILLED_HEART '*'
#define PAD 255
#endif
