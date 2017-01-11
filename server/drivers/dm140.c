/*
 *  dm1400 vfd driver (c)2007 Henrik Larsson
 */

#include <stdio.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <asm/types.h>
#include <linux/hiddev.h>
#include <string.h>
#include <stdlib.h>

#include "lcd.h"
#include "dm140.h"

#include "report.h"
#include "lcd_lib.h"
#include "libvfd.h"
#include "led.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "dm140_";


MODULE_EXPORT int
dm140_init (Driver *drvthis)
{
	PrivateData *p;
	int i;

        /* Allocate and store private data */
        p = (PrivateData *) calloc(1, sizeof(PrivateData));
        if (p == NULL)
        {
                report(RPT_CRIT, "Failed to allocate memory for PrivateData\n");
                return -1;
        }
        
        if (drvthis->store_private_ptr(drvthis, p))
        {
                report(RPT_CRIT, "Failed to store Private Ptr!");
                return -1;
        }

        /* initialize private data */
	p->width = 18;
	p->height = 2;
	p->pszVendor = "040b";
	p->pszProduct = "7001";

	p->gIconMask = 0; 
	p->gLayout = VFD_STR_LAYOUT_1;
	p->gScrollTime = 1;
	p->gFlags = 0;
	p->gDisplayMode = VFD_MODE_NONE;
	
	if ((p->framebuf = (char *) calloc(1, p->height)) == NULL)
	{
	  report(RPT_CRIT, "Allocating memory for framebuffer Failed\n");
	  return -1;
	}

	for (i=0; i<p->height; i++)
	{
	  p->framebuf[i] = (char *) calloc(1, p->width+1);
	  report(RPT_INFO, "Allocating memory for framebuffer[%d]\n", i); 
	  if (p->framebuf == NULL)
	    return -1;
        }
	
	if((p->fd = OpenHID(drvthis))< 0)
	{
		report(RPT_INFO, "Device for Vendor[%s] Product[%s] was not found, exiting\n", p->pszVendor, p->pszProduct);
		return -1;
	}

	//******************************************************
	// Initialize the internal report structures
	//******************************************************
	if(ioctl(p->fd, HIDIOCINITREPORT,0)<0)
		return -1;

	//******************************************************
	// Find out what type of reports this device accepts
	//******************************************************
	//FindReports(fd);

	//******************************************************
	// Set up the display to show graphics
	//******************************************************
	VFDTurnOffIcons(drvthis);
	VFDSetDisplay(drvthis, VFD_MODE_NONE, 0, 0, 0);
	VFDGraphicsClearBuffer(drvthis, VFD_GR_PAGE_3);
        
        //******************************************************
        // Set up the display, scrolling region, scroll rate etc.
        //******************************************************
        VFDDisableDisplay(drvthis);
        VFDClearString(drvthis, VFD_STR_REGION_1);
        VFDClearString(drvthis, VFD_STR_REGION_3);
        VFDEnableString(drvthis, VFD_STR_LAYOUT_2);
//        VFDSetScrollRegion(drvthis, 0);
//        VFDSetScrollTime(drvthis, 500);
	
	VFDSetString(drvthis, VFD_STR_REGION_1, 0, "DM140 online!!!");

	return 0;

}

MODULE_EXPORT void
dm140_close (Driver *drvthis)
{
        PrivateData *p = drvthis->private_data;


        VFDClearString(drvthis, VFD_STR_REGION_1);
        VFDClearString(drvthis, VFD_STR_REGION_2);
        VFDClearString(drvthis, VFD_STR_REGION_3);
        VFDClearString(drvthis, VFD_STR_REGION_4);

        if (p != NULL) {
                if (p->fd >= 0)
                        close(p->fd);

                free(p);
        }
        drvthis->store_private_ptr(drvthis, NULL);
}

MODULE_EXPORT int
dm140_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	return p->width;
}

MODULE_EXPORT int
dm140_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	return p->height;
}

MODULE_EXPORT int
dm140_string (Driver *drvthis, int x, int y, char *buffer)
{
  PrivateData *p = drvthis->private_data;
  int i;
    
  report(RPT_INFO, "%s called with values(x,y,c): %d, %d, %s", __FUNCTION__, x, y, buffer);

  for (i=0; i<strlen(buffer); i++)
  {
    p->framebuf[y-1][x+i]=buffer[i];
  }

/*
  if (y > p->height)
    y = p->height;
    
  if (y == 2)
    y = VFD_STR_REGION_3;
  
  return VFDSetString(drvthis, y, x, buffer);
*/

  return 0;
}

MODULE_EXPORT int
dm140_char (Driver *drvthis, int x, int y, char c)
{
  PrivateData *p = drvthis->private_data;
  
  report(RPT_INFO, "%s called with values(x,y,c): %d, %d, %c", __FUNCTION__, x, y, c);

  p->framebuf[y][x] = c;
  
  return 0;
}

MODULE_EXPORT int
dm140_clear (Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;
  int i;
  
  report(RPT_INFO, "%s called with values()", __FUNCTION__);

  for (i=0; i<p->height; i++)
  {
    memset(p->framebuf[i], 0x20, p->width);
    p->framebuf[i][p->width] = 0x00;
  }

//        VFDClearString(drvthis, VFD_STR_REGION_1);
//        VFDClearString(drvthis, VFD_STR_REGION_2);
//        VFDClearString(drvthis, VFD_STR_REGION_3);
//        VFDClearString(drvthis, VFD_STR_REGION_4);

  return 0;
}

MODULE_EXPORT int
dm140_flush (Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;
  int i;
  int y;
  
  report(RPT_INFO, "%s called with values()", __FUNCTION__);

  for (i=0; i<p->height; i++)
  {
    y = VFD_STR_REGION_1;
    if(i==0) y = VFD_STR_REGION_1; 
    if(i==1) y = VFD_STR_REGION_3;
    // Do switch depening on VFD_LAYOUT
    VFDSetString(drvthis, y, 1, p->framebuf[i]);
  }
  
  // Don't know what to do
 
  return 0;
}


