#ifndef GLCD_RENDER_H
#define GLCD_RENDER_H

int glcd_render_init(Driver *drvthis);
void glcd_render_close(Driver *drvthis);
void glcd_render_char(Driver *drvthis, int x, int y, unsigned char c);
int glcd_render_icon(Driver *drvthis, int x, int y, int icon);
void glcd_render_bignum(Driver *drvthis, int x, int num);

#ifdef HAVE_FT2
void glcd_render_char_unicode(Driver *drvthis, int x, int y, int c, int yscale, int xscale);
#endif

#endif
