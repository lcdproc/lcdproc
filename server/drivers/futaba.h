/*- Copyright Blackeagle gm(dot)blackeagle(at)gmail(dot)com
 *-           Alex Wood <thetewood(at)gmail(dot)com 2016
 * Definitions for the Futaba TSOD-5711BB
 *
 * As found on the Black Artisan media center case.
 * This is a 7 digit VFD plus a load of symbols
 */
#ifndef FUTABA_H
#define FUTABA_H

/* INCLUDES-------------------------------------- */
#include <stdint.h>
#ifdef HAVE_LIBUSB_1_0
#include <libusb.h>
#define USB_DEVICE_HANDLE    libusb_device_handle
#define USB_RESET_DEVICE     libusb_reset_device
#define USB_CLOSE_DEVICE     libusb_close
#define USB_CONTROL_TRANSFER libusb_control_transfer
#else
#include <usb.h>
#define USB_DEVICE_HANDLE    usb_dev_handle
#define USB_RESET_DEVICE     usb_reset
#define USB_CLOSE_DEVICE     usb_close
#define USB_CONTROL_TRANSFER usb_control_msg
#endif

/* DEFINITIONS----------------------------------- */
#define FUTABA_REPORT_LENGTH 64
#define FUTABA_STR_LEN 60
#define FUTABA_SYM_LEN 30

/* OPCODES--------------------------------------- */
#define	FUTABA_OPCODE_SYMBOL 0x85
#define	FUTABA_OPCODE_STRING 0x8b

#define FUTABA_SYM_ON 0x01
#define FUTABA_SYM_OFF 0x00

/* SYMBOL DEFINITIONS---------------------------- */
#define FUTABA_VOLUME_START      0x02
#define FUTABA_VOLUME_BARS       11
#define FUTABA_ICON_ARRAY_LENGTH 40
/* REPORT STRUCTURE------------------------------ */

// The report definition for a string
typedef struct {
	uint8_t startPos;
	uint8_t len;
	char string[FUTABA_STR_LEN];
} futabaStringRep_t;

// The symbol report inner part looks like this
typedef struct {
	uint8_t symName;
	uint8_t state;
} futabaSymAttr_t;

// The complete symbol report
typedef struct {
	uint8_t count;
	futabaSymAttr_t symbol[FUTABA_SYM_LEN];
} futabaSymbolRep_t;

// Put it all together to get the overall structure of a report
typedef struct {
	uint8_t opcode;
	uint8_t param1;
	union {
		futabaStringRep_t str;
		futabaSymbolRep_t sym;
	} type;

} futabaReport_t;

// The driver object definition
typedef struct {
	USB_DEVICE_HANDLE *my_handle;
#ifdef HAVE_LIBUSB_1_0
	libusb_context *ctx;
#endif
} futabaDriver_t;

/* Function declarations------------------------- */
int futaba_send_report(USB_DEVICE_HANDLE * my_handle, futabaReport_t * report);

int futaba_set_volume(USB_DEVICE_HANDLE * my_handle, int volPercent);

int futaba_init_driver(Driver *drvthis);

int futaba_set_icon(Driver *drvthis);

void futaba_shutdown(Driver *drvthis);

MODULE_EXPORT int futaba_init(Driver *drvthis);
MODULE_EXPORT void futaba_close(Driver *drvthis);
MODULE_EXPORT int futaba_width(Driver *drvthis);
MODULE_EXPORT int futaba_height(Driver *drvthis);
MODULE_EXPORT void futaba_clear(Driver *drvthis);
MODULE_EXPORT void futaba_flush(Driver *drvthis);
MODULE_EXPORT void futaba_string(Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void futaba_chr(Driver *drvthis, int x, int y, char c);
MODULE_EXPORT void futaba_output(Driver *drvthis, uint64_t icon_map);
MODULE_EXPORT const char *futaba_get_info(Driver *drvthis);

#endif // #ifndef FUTABA_H
