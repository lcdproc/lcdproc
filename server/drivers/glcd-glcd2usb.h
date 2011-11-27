/** \file server/drivers/glcd-glcd2usb.h
 * glcd2usb interface definitions (flags, report IDs and data structures).
 */

#ifndef GLCD2USB_H
#define GLCD2USB_H

#define FLAG_SIX_BIT          (1<<0)
#define FLAG_VERTICAL_UNITS   (1<<1)
#define FLAG_BOTTOM_START     (1<<2)
#define FLAG_VERTICAL_INC     (1<<3)
#define FLAG_BACKLIGHT        (1<<4)

#define GLCD2USB_RID_GET_INFO      1	/**< get display info */
#define GLCD2USB_RID_SET_ALLOC     2	/**< allocate/free display */
#define GLCD2USB_RID_GET_BUTTONS   3	/**< get state of the four buttons */
#define GLCD2USB_RID_SET_BL        4	/**< set backlight brightness */
#define GLCD2USB_RID_GET_IR        5	/**< get last ir message (unused) */
#define GLCD2USB_RID_WRITE         8	/**< write some bitmap data to the display */
#define GLCD2USB_RID_WRITE_4       (GLCD2USB_RID_WRITE+0)
#define GLCD2USB_RID_WRITE_8       (GLCD2USB_RID_WRITE+1)
#define GLCD2USB_RID_WRITE_16      (GLCD2USB_RID_WRITE+2)
#define GLCD2USB_RID_WRITE_32      (GLCD2USB_RID_WRITE+3)
#define GLCD2USB_RID_WRITE_64      (GLCD2USB_RID_WRITE+4)
#define GLCD2USB_RID_WRITE_128     (GLCD2USB_RID_WRITE+5)

typedef struct {
	unsigned char report_id;
	char name[32];
	unsigned short width, height;
	unsigned char flags;
} __attribute__((packed)) display_info_t;

#endif
