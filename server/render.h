#ifndef RENDER_H
#define RENDER_H

#include "screen.h"

#define HEART_OFF 0
#define HEART_ON 1
#define HEART_OPEN 2

#define BACKLIGHT_OFF 0
#define BACKLIGHT_ON 1
#define BACKLIGHT_OPEN 2
#define BACKLIGHT_LOAD 3
#define BACKLIGHT_VIS 4

#define BACKLIGHT_BLINK 0x100
#define BACKLIGHT_FLASH 0x200

extern int heartbeat;
extern int backlight;
extern int backlight_state;
extern int backlight_brightness;
extern int backlight_off_brightness;
extern int output_state;
int draw_screen (screen * s, int timer);

#endif
