#ifndef GLKPROTO_H
#define GLKPROTO_H

/* Necessary pre-defines */
#include <termios.h>

/* Protocol values */
extern unsigned char  GLKCommand ;
extern unsigned char  GLKConfirm ;
extern unsigned char  GLKDeny ;

extern unsigned char  GLKBufferFull ;
extern unsigned char  GLKBufferEmpty ;

#define GLKFLOW_OK  (0)
#define GLKFLOW_STOPPED  (1)
#define GLKFLOW_DISABLE  (-1)

/* Structure to hold various information about a
 *    device
 */
#define UNGETBUFSIZE  (16)
typedef struct {
      int  fd ;
      struct termios  saved ;
      int  flow ;
      int  timeout ;
      int  ungetin, ungetout ;
      unsigned char  ungetbuf[UNGETBUFSIZE] ;
   } GLKDisplay ;

/* Quote from Matrix Orbital GLK users manual:
 *
 *    ... during file downloads, the modules may need
 *    to spend considerable time moving files to make
 *    room for the new file.  This delay during download
 *    can be as much as a minute, but generally it will
 *    not exceed 10 seconds.
 */

/* Default timeout = 25.4 seconds */
#define GLK_TIMEOUT  (254)

/* Functions */
GLKDisplay * glkopen( char * name, tcflag_t speed );
int glktimeout( GLKDisplay * fd, int timeout );
int glkclose( GLKDisplay * );
int glkput_confirm( GLKDisplay * fd, int c );
int glkputa_confirm( GLKDisplay * fd, int len, unsigned char * str );
int glkput_echo( GLKDisplay * fd, int c );
int glkputl( GLKDisplay * fd, ... );
int glkputa( GLKDisplay * fd, int len, unsigned char * str );
int glkputs( GLKDisplay * fd, char * str );
int glkput( GLKDisplay * fd, int c );
int glkget( GLKDisplay * fd );
int glkgetc( GLKDisplay * fd );
int glkpoll( GLKDisplay * fd, int timeout );
int glkflow( GLKDisplay * fd, int full, int empty );

int glkunget( GLKDisplay * fd, int c );

#endif
