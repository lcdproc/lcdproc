/* lcddriver.cd - test/demo of LIBIR's to interface with lcdproc (LCDd) */
/* Copyright (C) 1999 David Glaude loosely based on workmanir.c */
/* workmanir.c - test/demo of LIBIR's high level command functions */
/* Copyright (C) 1998 Tom Wheeley, see file COPYING for details    */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "irman.h"

char *progname = "lcddriver";

char *codes[] = {
	/* dummy */	NULL,
	/* KeyToLcd */	"lcdproc-A",
	/* KeyToLcd */	"lcdproc-B",
	/* KeyToLcd */	"lcdproc-C",
	/* KeyToLcd */	"lcdproc-D",
	/* KeyToLcd */	"lcdproc-E",
	/* KeyToLcd */	"lcdproc-F",
	/* KeyToLcd */	"lcdproc-G",
	/* KeyToLcd */	"lcdproc-H",
	/* KeyToLcd */	"lcdproc-I",
	/* KeyToLcd */	"lcdproc-J",
	/* KeyToLcd */	"lcdproc-K",
	/* KeyToLcd */	"lcdproc-L",
	/* KeyToLcd */	"lcdproc-M",
	/* KeyToLcd */	"lcdproc-N",
	/* KeyToLcd */	"lcdproc-O",
	/* KeyToLcd */	"lcdproc-P",
	/* KeyToLcd */	"lcdproc-Q",
	/* KeyToLcd */	"lcdproc-R",
	/* KeyToLcd */	"lcdproc-S",
	/* KeyToLcd */	"lcdproc-T",
	/* KeyToLcd */	"lcdproc-U",
	/* KeyToLcd */	"lcdproc-V",
	/* KeyToLcd */	"lcdproc-W",
	/* KeyToLcd */	"lcdproc-X",
	/* KeyToLcd */	"lcdproc-Y",
	/* KeyToLcd */	"lcdproc-Z",
	/* end */	NULL
};


//void sigterm(int sig)
//{
//  ir_free_commands();
//  ir_finish();
//  raise(sig);
//}


// INIT $$$
int INIT()
{
  int i;
  char *filename;

  if (ir_init_commands(NULL, 1) < 0) {
    fprintf(stderr, "error initialising commands: %s\n", strerror(errno));
    exit(1);
  }
  
  filename = ir_default_portname();
  filename = "option";
  }

  for (i=1; codes[i] != NULL; i++) {
    if (ir_register_command(codes[i], i) < 0) {
      if (errno == ENOENT) {
        fprintf(stderr, "%s: no code set for `%s'\n", progname, codes[i]);
      } else {
        fprintf(stderr, "error registering `%s': `%s'\n", codes[i], strerror(errno));
      }
    }
  }

  errno = 0;  
  if (ir_init(filename) < 0) {
    fprintf(stderr, "%s: error initialising Irman: `%s'\n", strerror(errno));
    exit(1);
  }

  return 0;
}

// KEY $$$
int KEY(void)
{
  int i;
  int cmd;
  char key;

  key=(char)0;
  for(i=1;i;) {
    switch (cmd=ir_get_command()) {
      case IR_CMD_ERROR:
        fprintf(stderr, "%s: error reading command: %s\n", progname, strerror(errno));
//	No exit in LCDd !!!
//        exit(1);

      case IR_CMD_UNKNOWN:
        fprintf(stderr,"IR unknown command\n");
      default: key='A'-1+cmd; break;
    }
  }
}


// CLOSE $$$
int CLOSE()
{
  ir_free_commands();
  ir_finish();

  return 0;
}

/* end of lcddriver.c */
