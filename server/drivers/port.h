#ifndef PORT_H
#define PORT_H
/*-----------------------------------------------------------------------------
 * port.h (by damianf@wpi.edu)
 * Low level I/O functions taken from led-stat.txt
 * 
 * Jan 22 95
 *
 * DOS part (tested only with DOS version of GCC, DJGPP)
 * by M.Prinke <m.prinke@trashcan.mcnet.de> 3/97
 */

/* #define DOS */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef DOS
static inline int
port_in (int port)
{
	unsigned char value;
	__asm__ volatile ("inb %1,%0":"=a" (value)
							:"d" ((unsigned short) port));
	return value;
}

static inline void
port_out (unsigned short int port, unsigned char val)
{
	__asm__ volatile ("outb %0,%1\n"::"a" (val), "d" (port)
		 );
}

/***
 *** Get access to a specific port
 ***/
#ifdef HAVE_IOPERM
/* Assume this is a LINUX system with ioperm */
#include <sys/io.h>

static inline int
port_access (unsigned short int port)
{
	return ioperm( port, 1, 255);
}

#else
/* Assume this is a BSD system */
#include <stdio.h>

FILE *  port_access_handle = NULL ;

static inline int
port_access (unsigned short int port)
{
	if( port_access_handle
	    || (port_access_handle = fopen("/dev/io", "rw")) != NULL ) {
		return( 0 );  /* Success */
	} else {
		return( -1 );  /* Failure */
	};
}
#endif

#else
#include <pc.h>

static inline int
port_in (int port)
{
	unsigned char value;
	value = inportb ((unsigned short) port);
	return (int) value;
}

static inline void
port_out (unsigned int port, unsigned char val)
{
	outportb ((unsigned short) port, val);
}
#endif

/**** END OF FILE ****/
#endif
