#ifndef DM140_H
#define DM140_H

#include "libvfd.h"

typedef struct driver_private_data {
        char device[256];
        int fd;

        int width;
        int height;

        char *pszVendor;
        char *pszProduct;

        int gIconMask;
        char gLayout;
        char gScrollTime;
        char gFlags;
        int gDisplayMode;
        char gPages[VFD_PAGE_SIZE][VFD_PAGE_COUNT];

        char **framebuf;

} PrivateData;


/* dm140 */
MODULE_EXPORT int  dm140_init (Driver *drvthis);
MODULE_EXPORT void dm140_close (Driver *drvthis);
MODULE_EXPORT int  dm140_width (Driver *drvthis);
MODULE_EXPORT int  dm140_height (Driver *drvthis);
MODULE_EXPORT int  dm140_string (Driver *drvthis, int y, int x, char *buffer);
MODULE_EXPORT int  dm140_char (Driver *drvthis, int y, int x, char c);
MODULE_EXPORT int  dm140_clear (Driver *drvthis);
MODULE_EXPORT int  dm140_flush (Driver *drvthis);

#endif
