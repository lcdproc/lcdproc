#ifndef VC_LINK_H
#define VC_LINK_H

extern unsigned short vc_width, vc_height;
extern unsigned short vc_cursor_x, vc_cursor_y;
extern char *vc_buf;

int open_vcs(void);
int read_vcdata(void);

#endif
