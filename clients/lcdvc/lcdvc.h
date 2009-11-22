#ifndef LCDVC_H
#define LCDVC_H

#include "shared/defines.h"

#define CHAIN(e,f) { if( e>=0 ) { e=(f); }}
#define CHAIN_END(e) { if( e<0 ) { report( RPT_CRIT,"Critical error, abort"); exit(e); }}

#define UNSET_INT -1
#define UNSET_STR "\01"

#ifndef TRUE
# define TRUE    1
#endif
#ifndef FALSE
# define FALSE   0
#endif

extern char *vcs_device;
extern char *vcsa_device;

extern char *keys[4];

extern char *progname;

#endif
