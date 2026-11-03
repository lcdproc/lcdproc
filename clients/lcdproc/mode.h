#ifndef MODE_H
#define MODE_H

int mode_init(void);
void mode_close(void);
int update_screen(ScreenMode *m, int display);
void update_screen_reset(void);
int credit_screen(int rep, int display, int *flags_ptr);

#endif
