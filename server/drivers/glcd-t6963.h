#ifndef GLCD_T6963_H
#define GLCD_T6963_H

int glcd_t6963_init(Driver *drvthis);
void glcd_t6963_blit(PrivateData *p);
void glcd_t6963_close(PrivateData *p);

#endif
