/* $id$
 * Low level I/O functions taken from led-stat.txt
 * Jan 22 95 copyright damianf@wpi.edu
 *
 * DOS part (tested only with DOS version of GCC, DJGPP)
 * by M.Prinke <m.prinke@trashcan.mcnet.de> 3/97
 *
 * FreeBSD support by Guillaume Filion and Philip Pokorny, copyright 05/2001
 *
 * NetBSD port by Guillaume Filion, copyright 12/2001 and 02/2002
 */

/*
This file defines 6 static inline functions for port I/O:

Read a byte from port
	static inline int port_in (unsigned short int port);
Returns the content of the byte.

Write a char(byte) 'val' to port.
	static inline void port_out (unsigned short int port, unsigned char val);
Returns nothing.

Get access to a specific port
	static inline int port_access (unsigned short int port);
Returns 0 if successful, -1 if failed

Close access to a specific port
	static inline int port_deny (unsigned short int port);
Returns 0 if successful, -1 if failed

Get access 3 to ports: port (CONTROL), port+1 (STATUS) and port+2 (DATA)
	static inline int port_access_full (unsigned short int port);
Returns 0 if successful, -1 if failed

Close access to 3 ports: port (CONTROL), port+1 (STATUS) and port+2 (DATA)
	static inline int port_deny_full (unsigned short int port);
Returns 0 if successful, -1 if failed
*/

#ifndef PORT_H
#define PORT_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

/*  ------------------------------------------------------------- */
/*  Use ioperm, inb and outb in <sys/io.h> (Linux) */
#if defined HAVE_IOPERM && HAVE_SYS_IO_H
#include <sys/io.h>

/*  Read a byte from port */
static inline int port_in (unsigned short int port) {
	return inb(port);
}

/*  Write a byte 'val' to port */
static inline void port_out (unsigned short int port, unsigned char val) {
	outb(val, port);
}

/*  Get access to a specific port */
static inline int port_access (unsigned short int port) {
	return ioperm(port, 1, 255);
}

/*  Close access to a specific port */
static inline int port_deny (unsigned short int port) {
	return ioperm(port, 1, 0);
}

/*  Get access to 3 ports: port (CONTROL), port+1 (STATUS) and port+2 (DATA) */
static inline int port_access_full (unsigned short int port) {
	return ioperm(port, 3, 255);
}

/*  Close access to 3 ports: port (CONTROL), port+1 (STATUS) and port+2 (DATA) */
static inline int port_deny_full (unsigned short int port) {
	return ioperm(port, 3, 0);
}

/*  ------------------------------------------------------------- */
/*  Use i386_get_ioperm, i386_set_ioperm, inb and outb from <machine/pio.h> (NetBSD&OpenBSD) */
#elif defined HAVE_I386_IOPERM_NETBSD && defined HAVE_MACHINE_PIO_H && defined HAVE_MACHINE_SYSARCH_H
#include <sys/types.h>
#include <machine/pio.h>
#include <machine/sysarch.h>

/*  Read a byte from port */
static inline int port_in (unsigned short int port) {
	return inb(port);
}

/*  Write a byte 'val' to port */
static inline void port_out (unsigned short int port, unsigned char val) {
	outb(port, val);
}

static inline void setaccess(u_long * map, u_int bit, int allow) {
	u_int           word;
	u_int           shift;
	u_long          mask;

	word = bit / 32;
	shift = bit - (word * 32);

	mask = 0x000000001 << shift;
	if (allow)
		map[word] &= ~mask;
	else
		map[word] |= mask;
}

/*  Get access to a specific port */
static inline int port_access (unsigned short int port) {
	u_long          iomap[32];

	if (i386_get_ioperm(iomap) == -1) return -1;

	setaccess(iomap, port  , 1);

	if (i386_set_ioperm(iomap) == -1) return -1;

	return 0;
}

/*  Close access to a specific port */
static inline int port_deny (unsigned short int port) {
	u_long          iomap[32];

	if (i386_get_ioperm(iomap) == -1) return -1;

	setaccess(iomap, port, 0);

	if (i386_set_ioperm(iomap) == -1) return -1;

	return 0;
}

/*  Get access to 3 ports: port (CONTROL), port+1 (STATUS) and port+2 (DATA) */
static inline int port_access_full (unsigned short int port) {
	u_long          iomap[32];

	if (i386_get_ioperm(iomap) == -1) return -1;

	setaccess(iomap, port  , 1);
	setaccess(iomap, port+1, 1);
	setaccess(iomap, port+2, 1);

	if (i386_set_ioperm(iomap) == -1) return -1;

	return 0;
}

/*  Close access to 3 ports: port (CONTROL), port+1 (STATUS) and port+2 (DATA) */
static inline int port_deny_full (unsigned short int port) {
	u_long          iomap[32];

	if (i386_get_ioperm(iomap) == -1) return -1;

	setaccess(iomap, port  , 0);
	setaccess(iomap, port+1, 0);
	setaccess(iomap, port+2, 0);

	if (i386_set_ioperm(iomap) == -1) return -1;

	return 0;
}

/*#endif // defined HAVE_I386_IOPERM_NETBSD && defined HAVE_MACHINE_PIO_H && defined HAVE_MACHINE_SYSARCH_H
-------------------------------------------------------------
Use i386_get_ioperm, i386_set_ioperm and ASM inb and outb from <machine/sysarch.h> (FreeBSD) */
#elif defined HAVE_I386_IOPERM_FREEBSD && defined HAVE_MACHINE_SYSARCH_H
#include <machine/sysarch.h>  
        
/* Read a byte from port */
static inline int port_in (unsigned short int port) {
        unsigned char value;
        __asm__ volatile ("inb %1,%0":"=a" (value)
                        :"d" ((unsigned short) port));
        return value;
}
        
/* Write a byte 'val' to port */
static inline void port_out (unsigned short int port, unsigned char val) {
        __asm__ volatile ("outb %0,%1\n"::"a" (val), "d" (port)
                 );
}
        
/* Get access to a specific port */
static inline int port_access (unsigned short int port) {
        return i386_set_ioperm(port, 1, 1);
}

/* Get access 3 to ports: port (CONTROL), port+1 (STATUS) and port+2 (DATA) */
static inline int port_access_full (unsigned short int port) {
        return i386_set_ioperm(port, 3, 1);
}

/* Close access to a specific port */
static inline int port_deny (unsigned short int port) {
        return i386_set_ioperm(port, 1, 0);
}

/* Close access to 3 ports: port (CONTROL), port+1 (STATUS) and port+2 (DATA) */
static inline int port_deny_full (unsigned short int port) {
        return i386_set_ioperm(port, 3, 0);
}

/*  ------------------------------------------------------------- */
/*  Last chance! Use /dev/io and i386 ASM code (BSD4.3 ?) */
#else

/*  Read a byte from port */
static inline int port_in (unsigned short int port) {
	unsigned char value;
	__asm__ volatile ("inb %1,%0":"=a" (value)
							:"d" ((unsigned short) port));
	return value;
}

/*  Write a byte 'val' to port */
static inline void port_out (unsigned short int port, unsigned char val) {
	__asm__ volatile ("outb %0,%1\n"::"a" (val), "d" (port)
		 );
}

/*  Get access to 3 ports: port (CONTROL), port+1 (STATUS) and port+2 (DATA) */
static inline int port_access_full (unsigned short int port) {
	static FILE *  port_access_handle = NULL ;

	if( port_access_handle
	    || (port_access_handle = fopen("/dev/io", "rw")) != NULL ) {
		return( 0 );  /*  Success */
	} else {
		return( -1 );  /*  Failure */
	};

	return -1;
}

/*  Get access to a specific port */
static inline int port_access (unsigned short int port) {
	return port_access_full(port); /*  /dev/io gives you access to all ports. */
}

/*  Close access to 3 ports: port (CONTROL), port+1 (STATUS) and port+2 (DATA) */
static inline int port_deny_full (unsigned short int port) {
	/*  Can't close /dev/io... */
}

/* Close access to a specific port */
static inline int port_deny (unsigned short int port) {
	/* Can't close /dev/io... */
}

#endif

/*
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
*/

#endif /*  PORT_H */
