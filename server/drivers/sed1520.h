#ifndef SED1520_H
#define SED1520_H

/* Commands for SED1520 */
#define DISP_OFF	0xAE
#define DISP_ON		0xAF
#define DISP_START_LINE	0xC0
#define PAGE_ADR	0xB8
#define COLUMN_ADR	0x00
#define ADC_NORM	0xA0	/* left-to-right */
#define ADC_INV		0xA1	/* right-to-left */
#define STATIC_DR_OFF	0xA4	/* normal operation */
#define STATIC_DR_ON	0xA5	/* power safe */
#define DUTY_1_16	0xA8
#define DUTY_1_32	0xA9
#define SOFT_RESET	0xE2

MODULE_EXPORT int sed1520_init (Driver *drvthis);
MODULE_EXPORT void sed1520_close (Driver *drvthis);
MODULE_EXPORT int sed1520_width (Driver *drvthis);
MODULE_EXPORT int sed1520_height (Driver *drvthis);
MODULE_EXPORT int sed1520_cellwidth (Driver *drvthis);
MODULE_EXPORT int sed1520_cellheight (Driver *drvthis);
MODULE_EXPORT void sed1520_clear (Driver *drvthis);
MODULE_EXPORT void sed1520_flush (Driver *drvthis);
MODULE_EXPORT void sed1520_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void sed1520_chr (Driver *drvthis, int x, int y, char c);
MODULE_EXPORT void sed1520_vbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void sed1520_hbar (Driver *drvthis, int x, int y, int len, int promille, int options);
MODULE_EXPORT void sed1520_num (Driver *drvthis, int x, int num);
MODULE_EXPORT int  sed1520_icon (Driver *drvthis, int x, int y, int icon);
MODULE_EXPORT void sed1520_set_char (Driver *drvthis, int n, char *dat);

#endif
