#ifndef GLCD_PNG_H
#define GLCD_PNG_H

int glcd_png_init(Driver *drvthis);
void glcd_png_blit(PrivateData *p);
void glcd_png_close(PrivateData *p);

#endif
