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

#ifndef PORT_H
#define PORT_H
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
