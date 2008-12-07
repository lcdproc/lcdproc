#ifndef LCD_IRMANIN__H
#define LCD_IRMANIN_H

typedef struct _codemap {
	const char *irman;
	const char *lcdproc;
} CodeMap;

/** private data for the \c irman driver */
typedef struct irmanin_private_data {
	char device[256];
	char config[256];
	char *portname;
} PrivateData;

MODULE_EXPORT int irmanin_init (Driver *drvthis);
MODULE_EXPORT void irmanin_close (Driver *drvthis);
MODULE_EXPORT const char *irmanin_get_key (Driver *drvthis);

#endif
