#ifndef LED_H
#define LED_H

// led.c functions
int OpenHID(Driver *drvthis);
int VFDShowIcons(Driver *drvthis, int mask);
int VFDIconSet(Driver *drvthis, int icon, int state);
int VFDIconOn(Driver *drvthis, int icon);
int VFDIconOff(Driver *drvthis, int icon);
int VFDTurnOffIcons(Driver *drvthis);
int VFDSetVolume(Driver *drvthis, int level);
int VFDSetString(Driver *drvthis, int region, int offset, char *buffer);
int VFDClearString(Driver *drvthis, int region);
int VFDSetDisplay(Driver *drvthis, int mode, char layout, char time, char flags);
int VFDUpdateDisplay(Driver *drvthis, int mode, char layout, char time, char flags);
int VFDGlobalUpdateDisplay(Driver *drvthis);
int VFDSetScrollRegion(Driver *drvthis, int region);
int VFDSetScrollTime(Driver *drvthis, int time);
int VFDEnableDisplay(Driver *drvthis, int mode, char layout, char time, char flags);
int VFDDisableDisplay(Driver *drvthis);
int VFDEnableString(Driver *drvthis, char ucLayout);
int VFDGraphicsClearBuffer(Driver *drvthis, int page);
int VFDGraphicsCopyPage(Driver *drvthis, int page, char *buffer);
int VFDSetGraphics(Driver *drvthis, char region, char *buf);
int VFDGraphicsSendPage(Driver *drvthis, int page);
int VFDGraphicsShowPage(Driver *drvthis, int page);
int VFDGraphicsRect(Driver *drvthis, int page, char color, int srcx, int srcy, int width, int height);

#endif
