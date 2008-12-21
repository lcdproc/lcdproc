#ifndef LCD_IRMANIN__H
#define LCD_IRMANIN_H

/** mapping between IrMan code names and LCDd key names */
typedef struct _codemap {
	const char *irman;	/**< IrMan code name */
	const char *lcdproc;	/**< LCDpüroc key name */
} CodeMap;

/** private data for the \c irman driver */
typedef struct irmanin_private_data {
	char device[256];	/**< IrMan device name */
	char config[256];	/**< IrMan config file */
	char *portname;		/**< IrMan port name */
} PrivateData;

MODULE_EXPORT int irmanin_init (Driver *drvthis);
MODULE_EXPORT void irmanin_close (Driver *drvthis);
MODULE_EXPORT const char *irmanin_get_key (Driver *drvthis);

#endif
