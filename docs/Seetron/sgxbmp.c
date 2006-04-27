#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/termios.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define FLOW_NONE 0
#define FLOW_SOFTWARE 1
#define FLOW_HARDWARE 2

int die(char *str);
int openRawSerialLine(char *port,int speed,int flow_control,int wrmode);

char gxbits[480];

void bset(int x, int y) {
   int index;
   double theBitcom;
   int theBit;
   index = x + 120 * (y / 8);
   theBitcom = pow(2, (y % 8));
   theBit = theBitcom;
   gxbits[index] = 0+gxbits[index] | theBit;
}

int main(int ac,char **av) {
  int fd, idx, j, i, x, y;
  char bmpin[574];
  char str[1024];

  int filein;

  if (ac!=5) {
    printf("usage: %s port speed file mode\n",av[0]);
    printf("where...\n");
    printf("  port is the port device name\n");
    printf("  speed is the port speed\n");
    printf("  file is the filename of bitmap image\n");
    printf("  mode is 0 for normal and 1 for reverse\n");
    exit(0);
  }

  if (! strcmp(av[2], "-" )) {
    fd=open(av[1], O_RDWR|O_APPEND);
  } else {
    fd=openRawSerialLine(av[1],atoi(av[2]),FLOW_NONE,O_RDWR);
  }

  filein=open(av[3], O_RDWR);

  read(filein, bmpin, 62);

  read(filein, bmpin, 512);
  close(filein);

  idx=0;
  for(j=31;j>-1;j--) {
     for(i=0;i<16;i++) {
	if (bmpin[idx] & 128) bset((i * 8), j);
	if (bmpin[idx] &  64) bset(((i * 8)+ 1), j);
	if (bmpin[idx] &  32) bset(((i * 8)+ 2), j);
	if (bmpin[idx] &  16) bset(((i * 8)+ 3), j);
	if (bmpin[idx] &   8) bset(((i * 8)+ 4), j);
	if (bmpin[idx] &   4) bset(((i * 8)+ 5), j);
	if (bmpin[idx] &   2) bset(((i * 8)+ 6), j);
	if (bmpin[idx] &   1) bset(((i * 8)+ 7), j);
	idx++;
     }
  }

  sprintf(str,"%cDG", 27);

  if(atoi(av[4])==0) {
    for (idx=0;idx<480;idx++) {
       gxbits[idx]=gxbits[idx] ^ 255;
    }
  }
  write(fd,str,3); 
  write(fd,&gxbits,480);
  close(fd);
  exit(0);
}

int openRawSerialLine(char *port,int speed,int flow_control,int wrmode) {
  struct termios flags;
  speed_t tioc_speed;
  int fd, softwareFlow;

  softwareFlow= (flow_control==FLOW_SOFTWARE) ? 1 : 0;
  ((fd=open(port,wrmode|O_NDELAY))>=0)    || die("failed opening serial port");

  ((tcflush(fd, TCIFLUSH))>=0) || die("failed flushing line");

  flags.c_iflag= IGNBRK | ((flow_control==FLOW_SOFTWARE) ? (IXOFF|IXON) : 0);
  flags.c_cflag= CLOCAL | CS8 | ((flow_control==FLOW_HARDWARE) ? CRTSCTS : 0)
        | ((wrmode==O_WRONLY || wrmode==O_RDWR) ? CREAD : 0);
  flags.c_oflag= flags.c_lflag= 0;

  flags.c_cc[VMIN] = 1; flags.c_cc[VTIME] = 0;

  switch(speed) {
  case 50:   tioc_speed=B50; break;       case 75:   tioc_speed=B75; break;
  case 110:  tioc_speed=B110; break;      case 134:  tioc_speed=B134; break;
  case 150:  tioc_speed=B150; break;      case 200:  tioc_speed=B200; break;
  case 300:  tioc_speed=B300; break;      case 600:  tioc_speed=B600; break;
  case 1200: tioc_speed=B1200; break;     case 1800: tioc_speed=B1800; break;
  case 2400: tioc_speed=B2400; break;     case 4800: tioc_speed=B4800; break;
  case 9600: tioc_speed=B9600; break;     case 19200: tioc_speed=B19200; break;
  case 38400: tioc_speed=B38400; break;
  default:
    die("Unknown baud date"); break;
  }

  ((cfsetospeed(&flags,tioc_speed))>=0) || die("failed setting output speed");
  ((tcsetattr(fd,TCSANOW,&flags))>=0) || die("failed setting port attribs");

  return(fd);
}

int die(char *str) {
    perror(str);exit(1);
}

