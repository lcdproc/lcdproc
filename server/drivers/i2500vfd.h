#ifndef I2500VFD_H
#define I2500VFD_H

MODULE_EXPORT int i2500vfd_init (Driver *drvthis);
MODULE_EXPORT void i2500vfd_close (Driver *drvthis);
MODULE_EXPORT int i2500vfd_width (Driver *drvthis);
MODULE_EXPORT int i2500vfd_height (Driver *drvthis);
MODULE_EXPORT int i2500vfd_cellwidth (Driver *drvthis);
MODULE_EXPORT int i2500vfd_cellheight (Driver *drvthis);
MODULE_EXPORT void i2500vfd_clear (Driver *drvthis);
MODULE_EXPORT void i2500vfd_flush (Driver *drvthis);
MODULE_EXPORT void i2500vfd_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void i2500vfd_chr (Driver *drvthis, int x, int y, char c);

MODULE_EXPORT void i2500vfd_vbar(Driver *drvthis, int x, int y, int len, int promille, int pattern);
MODULE_EXPORT void i2500vfd_hbar(Driver *drvthis, int x, int y, int len, int promille, int pattern);
MODULE_EXPORT int  i2500vfd_icon (Driver *drvthis, int x, int y, int icon);

MODULE_EXPORT void i2500vfd_set_char (Driver *drvthis, int n, char *dat);

#endif
