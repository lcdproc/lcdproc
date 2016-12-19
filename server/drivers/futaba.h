/*
 * Definitions for the Futaba TSOD-5711BB
 *
 * As found on the Black Artisan media center case.
 * This is a 7 digit VFD plus a load of symbols
 */
#ifndef FUTABA_TSOD_5711BB_INCLUDED
#define FUTABA_TSOD_5711BB_INCLUDED

//---------------------------INCLUDES------------------------------------------
#include <stdint.h>

//---------------------------DEFINITIONS---------------------------------------
#define FUTABA_REPORT_LENGTH 64
#define FUTABA_STR_LEN 60
#define FUTABA_SYM_LEN 30

//---------------------------OPCODES-------------------------------------------
#define	FUTABA_OPCODE_SYMBOL 0x85
#define	FUTABA_OPCODE_STRING 0x8b

#define FUTABA_SYM_ON 0x01
#define FUTABA_SYM_OFF 0x00

//---------------------------SYMBOL DEFINITIONS--------------------------------
#define FUTABA_VOLUME_START      0x02
#define FUTABA_VOLUME_BARS       11
#define FUTABA_ICON_ARRAY_LENGTH 39
//--------------------------REPORT STRUCTURE-----------------------------------

// The report definition for a string
typedef struct
{
	uint8_t	startPos;
	uint8_t len;
	char	string[FUTABA_STR_LEN];
}futabaStringRep_t;

// The symbol report inner part looks like this
typedef struct
{
	uint8_t	symName;
	uint8_t	state;
}futabaSymAttr_t;

// The complete symbol report
typedef struct
{
	uint8_t	count;
	futabaSymAttr_t	symbol[FUTABA_SYM_LEN];
}futabaSymbolRep_t;


// Put it all together to get the overall structure of a report
typedef struct
{
	uint8_t	opcode;
	uint8_t	param1;
	union
	{
		futabaStringRep_t	str;
		futabaSymbolRep_t	sym;
	}type;

}futabaReport_t;


// The driver object definition
typedef struct
{
	libusb_device_handle	*my_handle;
	libusb_context 		*ctx;
}futabaDriver_t;

//---------------------------Function declarations-----------------------------
int futabaSendReport(libusb_device_handle *my_handle, futabaReport_t *report);

int futabaSetVolume(libusb_device_handle *my_handle, int volPercent);

int futabaInitDriver( futabaDriver_t *my_driver, Driver *drvthis);

int futaba_set_icon (Driver *drvthis);

void futaba_shutdown(Driver *drvthis);

MODULE_EXPORT int  futaba_init (Driver *drvthis);
MODULE_EXPORT void futaba_close (Driver *drvthis);
MODULE_EXPORT int  futaba_width (Driver *drvthis);
MODULE_EXPORT int  futaba_height (Driver *drvthis);
MODULE_EXPORT void futaba_clear (Driver *drvthis);
MODULE_EXPORT void futaba_flush (Driver *drvthis);
MODULE_EXPORT void futaba_string (Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void futaba_chr (Driver *drvthis, int x, int y, char c);
MODULE_EXPORT void futaba_set_contrast (Driver *drvthis, int promille);
MODULE_EXPORT void futaba_backlight (Driver *drvthis, int on);
MODULE_EXPORT void futaba_output (Driver *drvthis, int icon_map);
MODULE_EXPORT const char * futaba_get_info (Driver *drvthis);

#endif		//#ifndef FUTABA_TSOD_5711BB_INCLUDED
