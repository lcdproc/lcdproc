#ifndef LCDVC_H
#define LCDVC_H

#define CHAIN(e,f) { if( e>=0 ) { e=(f); }}
#define CHAIN_END(e) { if( e<0 ) { report( RPT_CRIT,"Critical error, abort"); exit(e); }}

#define min(a,b) ((a)<(b))?(a):(b)

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

int split(char * str, char delim, char * parts[], int maxparts);

#endif
