#ifndef MODE_H
#define MODE_H

#include <elektra.h>

int mode_init(void);
void mode_close(void);
int update_screen(ScreenMode *m, int display, Elektra * elektra);
int credit_screen(int rep, int display, int *flags_ptr, Elektra * elektra);

#endif
