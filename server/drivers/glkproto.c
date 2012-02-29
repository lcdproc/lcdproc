/* glkproto
 *
 * Routines to support downloading to the GLK
 */

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <poll.h>

#include "glkproto.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define INLINE  inline
#undef MANUAL_FLOWCONTROL
#undef PAUSE_AFTER_STRINGS

/* Protocol values */
unsigned char GLKCommand = 0xfe;
unsigned char GLKConfirm = 0x01;
unsigned char GLKDeny = 0x08;

unsigned char GLKBufferFull = 0xfe;
unsigned char GLKBufferEmpty = 0xff;


/* glkopen
 *
 * Open and configure a serial port for communication with
 *   a Matrix Orbital module (GLK or otherwise)
 */
GLKDisplay *glkopen(char *name, tcflag_t speed)
{
   int fd;
   struct termios new;
   GLKDisplay *retval;

   if (name == NULL || speed == 0) {
      /* Invalid arguments */
      errno = EINVAL;
      return(NULL);
   }

   fd = open(name, O_RDWR | O_NOCTTY);
   if (fd < 0) {
      return(NULL);
   }

   /* Get current settings */
   if (tcgetattr(fd, &new) < 0) {
      int errsave = errno;

      close(fd);
      errno = errsave;
      return(NULL);
   }

   retval = malloc(sizeof(GLKDisplay));
   if (retval == NULL) {
      errno = ENOMEM;
      return(NULL);
   }
   retval->fd = fd;
   retval->saved = new;
   retval->ungetin = retval->ungetout = 0;
   retval->timeout = GLK_TIMEOUT;
   retval->flow = GLKFLOW_OK;

   /* Make new settings */

   /* We use RAW mode */
#ifdef HAVE_CFMAKERAW
   /* The easy way */
   cfmakeraw(&new);
#else
   /* The hard way */
   new.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
                     | INLCR | IGNCR | ICRNL | IXON);
   new.c_oflag &= ~OPOST;
   new.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN);
   new.c_cflag &= ~( CSIZE | PARENB | CRTSCTS);
   new.c_cflag |= CS8 | CREAD | CLOCAL;
#endif

   /* Set default MIN and TIMEOUT values */
   new.c_cc[VMIN] = 0;
   new.c_cc[VTIME] = GLK_TIMEOUT;

   /* Set the out and in speeds */
   cfsetospeed(&new, speed);
   cfsetispeed(&new, B0);  /* Input equals output speed */

   /* Configure the port */
   tcflush(fd, TCIOFLUSH);
   if (tcsetattr(fd, TCSANOW, &new) < 0) {
      int errsave = errno;

      glkclose(retval);
      errno = errsave;
      return(NULL);
   }

   return(retval);
}

/* glktimeout
 *
 * Set the intercharacter timeout.  This determines how long
 *    a read will block (glkget) waiting on data.
 */
int
glktimeout(GLKDisplay *fd, int timeout)
{
   struct termios t;

   if (timeout < 0 || timeout > 255) {
      errno = EINVAL;   /* Invalid argument */
      return(1); /* Failure */
   }

   if (tcgetattr(fd->fd, &t) < 0) {
      return(1);
   }

   fd->timeout = timeout;
   t.c_cc[VTIME] = timeout;

   if (tcsetattr(fd->fd, TCSANOW, &t) < 0) {
      return(1);
   }

   return(0);  /* Success */
}

/* glkflow
 *
 * Turn flow control processing for the port on/off and
 *    set the high/low water marks.  NOTE: This assumes
 *    the GLK12232-25SM which has a 96 byte buffer.
 */
#define GLKBUF (96)
int
glkflow(GLKDisplay *fd, int full, int empty)
{
   struct termios t;

   if ((full > GLKBUF -1)
       || (empty > GLKBUF -1)
       || (full + empty > GLKBUF -1)) {
      errno = EINVAL;
      return(1);
   }

   if (tcgetattr(fd->fd, &t) < 0) {
      return(1);  /* Failure */
   }

   if (full < 0 || empty < 0) {
      /* Turn it off */
      glkputl(fd, GLKCommand, 0x3b, EOF);
      t.c_iflag &= ~(IXON | IXANY | IXOFF);
      t.c_cc[VSTART] = GLKBufferEmpty;
      t.c_cc[VSTOP] = GLKBufferFull;
      fd->flow = GLKFLOW_DISABLE;
   }
   else {
      /* Turn it on */
      glkputl(fd, GLKCommand, 0x3a, full, empty, EOF);
      /* Control what we send */
      t.c_iflag |= IXON;
      /* Only start on VSTART (IXANY),            *
       * Don't control what we receive (IXOFF)    */
      t.c_iflag &= ~(IXANY | IXOFF);
      t.c_cc[VSTART] = GLKBufferEmpty;
      t.c_cc[VSTOP] = GLKBufferFull;
      fd->flow = GLKFLOW_OK;
   }

   if (tcsetattr(fd->fd, TCSANOW, &t) < 0) {
      return(1);
   }

   return(0);  /* Success */
}

/* glkclose
 *
 * Close a serial port and restore settings if provided.
 */
int
glkclose(GLKDisplay *fd)
{
   int retval = 0;

   if (fd->fd < 0) {
      /* Probably already closed */
      return(0);  /* Success? */
   }

   /* Trash any unread data */
   tcflush(fd->fd, TCIFLUSH);

   /* Restore settings */
   tcsetattr(fd->fd, TCSANOW, &fd->saved);

   retval = close(fd->fd);

   fd->fd = -1;
   free(fd);

   return(retval);
}


/* glkput
 *
 * Send a character to the GLK
 */
INLINE int
glkput(GLKDisplay *fd, int c)
{
   unsigned char val;
   ssize_t retval;

   /* Output the byte */
   val = c;
   retval = write(fd->fd, &val, 1);

   /* retval <= 0  =>  FAILURE  =>  1
    * retval == 1  =>  SUCCESS  =>  0
    */
   return(retval <= 0);
}

/* glkunget
 *
 * Put a read character "back" into the input queue.
 *    NOTE:  This queue holds characters between the
 *          low level glkget interface and the higher
 *          glkgetc interface.  In this case, the queue
 *          holds any characters read that were not
 *          flow control signals
 */
int
glkunget(GLKDisplay *fd, int c)
{
   /* Is buffer already full? */
   if (((fd->ungetin + 1) & (~UNGETBUFSIZE)) == fd->ungetout) {
      return(1);  /* Failure */
   }

   fd->ungetbuf[fd->ungetin] = c;
   fd->ungetin = (fd->ungetin + 1) & (~UNGETBUFSIZE);

   return(0);
}

/* glkget
 *
 * Read a character from the GLK
 */
INLINE int
glkget(GLKDisplay *fd)
{
   unsigned char c;
   ssize_t retval;

   retval = read(fd->fd, &c, 1);
   if (retval <= 0) {
      return(-1);
   } else {
      return((int) c);
   }
}

/* glkpoll
 *
 * Test to see if data is availble to read from the
 *    GLK.
 */
int
glkpoll(GLKDisplay *fd, int timeout)
{
   struct pollfd fds;
   int  retval;

   fds.fd = fd->fd;
   fds.events = POLLIN;
   fds.revents = 0;
   retval = poll(&fds, 1, timeout);
   if (retval < 0) {
      /* Some error.  Ignore it, and return "no data" */
      retval = 0;
   }

   return(retval);
}

/* glkgetc
 *
 * Read a character from the GLK.  This will generally be used
 *    to read keypad characters from the GLK.  RAW protocol
 *    implementations may want to use glkget.  glkget will return
 *    GLKBufferFull and GLKBufferEmpty whereas glkgetc wil NOT.
 */
int
glkgetc(GLKDisplay *fd)
{
   int  c;

   /* Are there characters in the unget buffer? */
   if (fd->ungetin == fd->ungetout) {
      /* Nope, so read one */
      for ( ; ; ) {  /* loop until we get a non-flow control value */
         c = glkget(fd);

         if (fd->flow != GLKFLOW_DISABLE) {
            if (c == GLKBufferFull) {
               fd->flow = GLKFLOW_STOPPED;
               continue;
            } else if (c == GLKBufferEmpty) {
               fd->flow = GLKFLOW_OK;
               continue;
            }
         }

         /* Must be what we're looking for */
         break;
      }
   }
   else {
      /* Get one of the "ungotten" char's */
      c = fd->ungetbuf[fd->ungetout];
      fd->ungetout = (fd->ungetout + 1) & (~UNGETBUFSIZE);

   }

   return(c);
}

/* glkput_confirm
 *
 * Send a character to the GLK on fd with echo and
 *    confirmation of echo
 */
int
glkput_confirm(GLKDisplay *fd, int c)
{
   int  echo;

   /* Output the byte */
   if (glkput(fd, c)) {
      return(1);  /* Failure */
   }

   /* Look for the echo */
   echo = glkget(fd);
   if (echo < 0) {
      return(1);  /* Failure */
   }

   if (echo == c) {
      glkput(fd, GLKConfirm);
      return(0); /* Success */
   } else {
      glkput(fd, GLKDeny);
      return(1);
   }
}

/* glkputa_confirm
 *
 * Send an array of characters all with echo and
 *    confirmation.
 */
int
glkputa_confirm(GLKDisplay *fd, int len, unsigned char *str)
{
   int i;
   int retval;

   retval = 0;  /* Assume Success */
   for (i = len; !retval && i; ++str, --i) {
      retval = glkput_confirm(fd, *str);
   }

   return(retval);
}

/* glkput_echo
 *
 * Send a character to the GLK on fd with echo expected
 */
int
glkput_echo(GLKDisplay *fd, int c)
{
   int echo;

   /* Output the byte */
   if (glkput(fd, c)) {
      return(1);  /* Failure */
   }

   /* Look for the echo */
   echo = glkget(fd);
   if (echo < 0) {
      return(1);  /* Failure */
   }

   if (echo == c) {
      return(0);  /* Success */
   } else {
      return(1);  /* Failure */
   }
}

/* glkputl
 *
 * Send a list of characters to the GLK.  The list is
 *    terminated by an EOF valued argument.
 */
int
glkputl(GLKDisplay *fd, ...)
{
   va_list ap;
   int value;
   int retval;

   va_start(ap, fd);
   retval = 0;  /* Success */
   value = va_arg(ap, int);

   while (!retval && value != EOF) {
      retval = glkput(fd, value);
      value = va_arg(ap, int);
   }
   va_end(ap);

   return(retval);
}

/* glkputs
 *
 * Send a null terminated string of characters to the GLK.
 */
int
glkputs(GLKDisplay *fd, char *str)
{
   register char *p = str;
   int retval;

   retval = 0;  /* Success */
   for (p = str; !retval && *p; ++p) {
      retval = glkput(fd, *p);
   }

   return(retval);
}

/* glkputa
 *
 * Send an array of characters.
 */
int
glkputa(GLKDisplay *fd, int len, unsigned char *str)
{
   register unsigned char *p = str;
   int i;
   int retval;

   retval = 0;  /* Assume Success */
   for (i = len; !retval && i; ++p, --i) {
      retval = glkput(fd, *p);
   }

   return(retval);
}
