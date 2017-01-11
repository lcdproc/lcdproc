/* File modified by Henrik Larsson 2007 to interface with LCDproc API
 */

/* vfd demo application
 * Copyright (C) 2006, Advanced Micro Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/* This is an example application that turns on the icons on the front panel
 * and also displays text on the VFD screen
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
#include "lcd.h"
#include "libvfd.h"
#include "report.h"
#include "dm140.h"


//**************************************************************
//  HID devices exchange data with the host computer using data bundles called 
//  reports.  Each report is divided into "fields", each of which can have one
//  or more "usages".  In the hid-core each one of these usages has a single
//  signed 32 bit value.
//
//  read():
//  This is the event interface.  When the HID device's state changes, it
//  performs an interrupt transfer containing a report which contains the 
//  changed value.  The hid-core.c module parses the report, and returns to
//  hiddev.c the individual usages that have changed within the report.  In
//  its basic mode, the hiddev will make these individual usage changes
//  available to the reader using a struct hiddev_event:
//    struct hiddev_event { unsigned hid;
//  containing the HID usage identifier for the status that changed, and the
//  value that it was changed to.  Note that the structure is defined within
//  <linux/hiddev.h>, along with some other useful #defines and structures.  
//  The HID usage identifier is a composite of the HID usage page shifed to 
//  the 16 high order bits ORed with the usage code.  The behavior of the read()
//  function can be modified using the HIDIOCSFLAG ioctl described below.
//
//  ioctl():
//  Instructs the kernel to retrieve all input and feature report values from
//  the device.  At this point, all the usage structures will contain current
//  values for the device, and will maintain it as the device changes.  Note 
//  that the use of this ioctl is unnecessary in general, since later kernels
//  automatically initialize the reports from the device at attach time.
//**************************************************************

/* hack - fix improper signed char handling - it's seeing 0x80 as a negative value*/
#define VALUE_FILTER(_value)  (_value>0x7F)?(__s32)(0xFFFFFF00 | _value):(_value)

#define VFD_PACKET_SIZE(s) (s*8)

unsigned char amd_logo[VFD_PAGE_SIZE] = {
  0x7f,0xfe,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x3f,0xfe,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x1f,0xfe,0x07,0x83,0xc0,0xe3,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x0f,0xfe,0x07,0x83,0xe1,0xe3,0xfe,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x07,0xfe,0x07,0xc3,0xe1,0xe3,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x10,0x1e,0x0f,0xc3,0xf3,0xe3,0x87,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
  0x30,0x1e,0x0e,0xe3,0xb3,0x63,0x83,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
  0x30,0x1e,0x1c,0xe3,0xb7,0x63,0x83,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
  0xf0,0x1e,0x1c,0xe3,0x9e,0x63,0x83,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
  0xf0,0x1e,0x1c,0x73,0x9e,0x63,0x83,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
  0xf0,0x1e,0x3f,0xf3,0x9c,0x63,0x83,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
  0xff,0xce,0x3f,0xf3,0x8c,0x63,0x8f,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
  0xff,0x86,0x38,0x3b,0x80,0x63,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0xff,0x02,0x70,0x3b,0x80,0x63,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0xfe,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};


char icon_bits[] = 
{
  15, 14, 13, 12, 11, 10,  9,  8,
  18, 17, 16, 19, 23, 27, 31, 35,
  39, 43, 47, 51, 55, 59, 63
};

//**************************************************************
// FUNCTION: Compare
//
// INPUT: 
//  const char *pszValue - string to compare
//  short sValue - numberic value to compare
//
// OUTPUT:
//   int - Boolean value, 0 on non match, 1 on success.
//
// DESCRIPTION: Compare a character value to a numeric value.
//**************************************************************
int Compare(const char *pszValue, short sValue)
{
  int iValue;
  // convert the pszValue to a number
  sscanf( pszValue, "%4x", &iValue);
  return( iValue == sValue );
}


//**************************************************************
// FUNCTION: OpenHID
//
// INPUT: 
//  const char *pszVendor - Vendor ID of device to open
//  const char *pszProduct - Product ID of device to open
//
// OUTPUT:
//   int fd - file descriptor to the opened HID device or -1 if err.
//
// DESCRIPTION: This function will open all the HID's on the
//  system until we find a match or we've exhausted our search.
//**************************************************************
int OpenHID(Driver *drvthis)
{
  int i, fd=-1;
  char port[32];
  char name[256];
  const char *hiddev_prefix = "/dev/usb/hiddev"; /* in devfs */
  int version;
  struct hiddev_devinfo device_info;
  PrivateData *p = drvthis->private_data;
      

  //******************************************************
  // Loop through all the 16 HID ports/devices looking for 
  // one that matches our device.
  //******************************************************
  for(i=0; i<16;i++)
  {
    sprintf(port, "%s%d", hiddev_prefix, i);
    if((fd = open(port,O_WRONLY))>=0)
    {
      ioctl(fd, HIDIOCGDEVINFO, &device_info);
      ioctl(fd, HIDIOCGNAME(sizeof(name)), name);

      // If we've found our device, no need to look further, time to stop searching
      if( Compare(p->pszVendor, device_info.vendor) && Compare(p->pszProduct, device_info.product))
      {
        break; // stop the for loop
      }
      close(fd); // Added by HL
    }
  }

  //******************************************************
  // If we've found our device, print out some information about it.
  //******************************************************
  if(fd != -1)
  {
    int appl; 
    report(RPT_INFO, "Found Device - Name is %s\n", name);
    report(RPT_INFO, "Vendor[0x%04hx] Product[0x%04hx] Version[0x%04hx]\n\t",
          device_info.vendor, device_info.product, device_info.version);
    switch(device_info.bustype)
    {
      default: 
    	  report(RPT_INFO, " an unknown bus type: 0x%04hx ", device_info.bustype);
    	  report(RPT_INFO, "bus[%d], devnum[%d] ifnum[%d]\n", device_info.busnum,
    		  device_info.devnum, device_info.ifnum); 
	      break;
    }
    //******************************************************
    // Read the version - it's a packed 32 field, so 
    // unpack it in order to display
    //******************************************************
    ioctl(fd, HIDIOCGVERSION, &version);
    report(RPT_INFO, "HIDdev Driver Version is %d.%d.%d\n", 
    	  version >>16, (version>>8)&0xff,version &0xff);

    report(RPT_INFO, "There are %d applications for this device\n", device_info.num_applications);

    for(i=0;i<device_info.num_applications; i++)
    {
			appl = ioctl(fd, HIDIOCAPPLICATION,	i);
			report(RPT_INFO, "Application[%i]	is 0x%x	", i,	appl);
      //******************************************************
			// The magic values	come from	various	usage	table	specs	
      //******************************************************
			switch(appl	>> 16)
			{
				case 0x01: report(RPT_INFO, "(Generic	Desktop	Page)\n");break;
				case 0x02: report(RPT_INFO, "(Simulation Controls)\n");	break;
				case 0x03: report(RPT_INFO, "(VR Controls)\n");					break;
				case 0x04: report(RPT_INFO, "(Sport	Controls)\n");			break;
				case 0x05: report(RPT_INFO, "(Game Controls)\n");				break;
				case 0x06: report(RPT_INFO, "(Generic	Device Controls)\n");break;
				case 0x07: report(RPT_INFO, "(Keyboard/Keypad)\n");			break;
				case 0x08: report(RPT_INFO, "(LEDs)\n");								break;
				case 0x09: report(RPT_INFO, "(Button)\n");							break;
				case 0x0A: report(RPT_INFO, "(Ordinal)\n");							break;
				case 0x0B: report(RPT_INFO, "(Telphony)\n");						break;
				case 0x0C: report(RPT_INFO, "(Consumer Product Page)\n");	break;
				case 0x0D: report(RPT_INFO, "(Digitizer)\n");						break;
				case 0x0E: report(RPT_INFO, "(Reserved)\n");						break;
				case 0x0F: report(RPT_INFO, "(PID	Page)\n");						break;
				case 0x14: report(RPT_INFO, "(Alphanumeric Display)\n"); break;
				case 0x15:
				case 0x3f: report(RPT_INFO, "(Reserved)\n");						break;
				case 0x40: report(RPT_INFO, "(Medical	Instruments)\n");	break;
				case 0x80: report(RPT_INFO, "(USB	Monitor	Page)\n");		break;
				case 0x81: report(RPT_INFO, "(USB	Enumerated Values	Page)\n"); break;
				case 0x82: report(RPT_INFO, "(VESA Virtual Controls	Page)\n"); break;
				case 0x83: report(RPT_INFO, "(Reserved Monitor Page)\n");	break;
				case 0x84: report(RPT_INFO, "(Power	Device Page)\n");		break;
				case 0x85: report(RPT_INFO, "(Battery	System Page)\n");	break;
				case 0x86: 
				case 0x87: report(RPT_INFO, "(Reserved Power Device	Page)\n"); break;
				case 0x8C: report(RPT_INFO, "(Bar	Code Scanner Page)\n");	break;
				case 0x8D: report(RPT_INFO, "(Scale	Page)\n");					break;
				case 0x8E: report(RPT_INFO, "(Magnetic Stripe	Reading	Device)\n"); break;
				case 0x8F: report(RPT_INFO, "(Point	of Sale	pages)\n");	break;
				case 0x90: report(RPT_INFO, "(Camera Control Page)\n");	break;
				case 0x91: report(RPT_INFO, "(Arcade Page)\n");					break;
				default: 
				{
					int page	=	(appl	>> 16) & 0x0000FFFF;
					if((page >=	0xFF00)	&& (page <=	0xFFFF))
						report(RPT_INFO, "(Vendor	Defined	-	0x%04X)\n",page);	
					else
						report(RPT_INFO, "(Unknown page	-	needs	to be	added	0x%04X)\n",(appl>>16));	
					break;
				}
			}
		}
  }
  return fd;
}

//**************************************************************
// FUNCTION: FindReports
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//
// OUTPUT:
//  none
//
// DESCRIPTION: This function will print out the type of reports
//  this HID accepts.
//**************************************************************
void FindReports(Driver *drvthis)
{
  int iFields, iUsage;
  int iReportType;
  struct hiddev_report_info repInfo;
  struct hiddev_field_info fieldInfo;
  struct hiddev_usage_ref usageRef;
  PrivateData *p = drvthis->private_data;
  int fd = p->fd;
  
  //******************************************************
  // For each type of report (Input, Output, Feature)
  // find out which is accepted for this device.
  //******************************************************
  for(iReportType=HID_REPORT_TYPE_MIN; 
      iReportType<=HID_REPORT_TYPE_MAX; 
  	  iReportType ++)
  {
    repInfo.report_type = iReportType;
    repInfo.report_id = HID_REPORT_ID_FIRST;

    //******************************************************
    // HIDIOCGREPORTINFO - struct hiddev_report_info (read/write)
    // Obtain the usage information if it is found
    //******************************************************
    while(ioctl(fd, HIDIOCGREPORTINFO, &repInfo)>=0)
    {
      report(RPT_INFO, "  Report id: %d (%s) (%d fields)\n",
      	      repInfo.report_id, 
	            (iReportType == HID_REPORT_TYPE_INPUT) ? "Input" : 
	            (iReportType == HID_REPORT_TYPE_OUTPUT) ? "Output" : "Feature/Other",
	            repInfo.num_fields);
     
      //******************************************************
      // HIDIOCGFIELDINFO - struct hiddev_field_info (read/write)
      // Returns the field information associated with a report 
      // in a hiddev_field_info structure.  The user must fill 
      // in report_id and report_type in this structure.  The 
      // field_index should also be filled in, which should be 
      // a number between 0 and maxfield-1.
      //******************************************************
      for(iFields = 0; iFields<repInfo.num_fields; iFields++)
      {
        memset(&fieldInfo, 0, sizeof(fieldInfo));
	      fieldInfo.report_type = repInfo.report_type;
        fieldInfo.report_id   = repInfo.report_id;
        fieldInfo.field_index = iFields;
        ioctl(fd, HIDIOCGFIELDINFO, &fieldInfo);

        // Print out information about this field
        report(RPT_INFO, "    Field: %d(id=%d): app: %04x phys %04x flags %x "
	              "(%d usages) unit %x exp %d\n",
	        iFields, fieldInfo.report_id, fieldInfo.application, 
	        fieldInfo.physical, fieldInfo.flags, fieldInfo.maxusage, 
          fieldInfo.unit, fieldInfo.unit_exponent);

        //******************************************************
        // HIDIOCGUCODE - struct hiddev_usage_ref(read/write)
        // Fill in the structure with report_type, report_id,
        // field_index, and usage_index to obtain the usage_code.
        //
        // HIDIOCGUSAGE - struct hiddev_usage_ref(read/write)
        // Obtain the value for the usage_code.
        //******************************************************
        for(iUsage=0; iUsage<fieldInfo.maxusage; iUsage++)
        {
          memset(&usageRef, 0, sizeof(usageRef));
	        usageRef.report_type = fieldInfo.report_type;
	        usageRef.report_id = fieldInfo.report_id;
	        usageRef.field_index = iFields;
	        usageRef.usage_index = iUsage;
	        ioctl(fd, HIDIOCGUCODE, &usageRef);
	        ioctl(fd, HIDIOCGUSAGE, &usageRef);
	        report(RPT_INFO, "      Usage: %04x val %d idx %x\n", usageRef.usage_code,
	          usageRef.value, usageRef.usage_index);
        }
      }

      //******************************************************
      // Go to the next report if there is one.
      //******************************************************
      repInfo.report_id |= HID_REPORT_ID_NEXT;
    }
  }
}

//**************************************************************
// FUNCTION: SendReport
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//	const char *buf - Message to write
//	size_t size - size of buf.
//
// OUTPUT:
//  int err - result of the ioctl call (On success 0, On error -1)
//
// DESCRIPTION: This function will write the 'buf' to the opened
//	HID device.  Specifically, it updates the device's usage 
//	reference with the data and then sends a report to the HID.
//**************************************************************
int SendReport(Driver *drvthis, const char *buf, size_t size)
{
	struct hiddev_report_info rinfo;
	struct hiddev_usage_ref	uref;
	int	i, err;
	PrivateData *p = drvthis->private_data;
	int fd = p->fd;

  //******************************************************
	// Initialize the usage Reference and mark it for OUTPUT
  //******************************************************
	memset(&uref, 0, sizeof(uref));
	uref.report_type = HID_REPORT_TYPE_OUTPUT;
	uref.report_id	 = 0;
	uref.field_index = 0;

	//**************************************************************
	// Fill in the information that we wish to set
	//**************************************************************
	uref.usage_code  = 0xffa10005; //	unused?
	for(i=0;i<size;i++)
	{
		uref.usage_index = i;
		uref.value	 = VALUE_FILTER(buf[i]);

		//**************************************************************
		// HIDIOCSUSAGE - struct hiddev_usage_ref (write)
		// Sets the value of a usage in an output report.  The user fills
		// in the hiddev_usage_ref structure as above, but additionally 
		// fills in the value field.
		//**************************************************************
		if((err	= ioctl(fd, HIDIOCSUSAGE, &uref)) < 0)
		{
			report(RPT_INFO, "Error with sending the USAGE ioctl %d\n", err);
			return err;
		}
		uref.usage_code  = 0xffa10006; //	unused?
	}

	//**************************************************************
	// HIDIOCSREPORT - struct hiddev_report_info (write)
	// Instructs the kernel to SEND a report to the device.  This 
	// report can be filled in by the user through HIDIOCSUSAGE calls 
	// (below) to fill in individual usage values in the report before
	// sending the report in full to the device.
	//**************************************************************
	memset(&rinfo, 0,	sizeof(rinfo));
	rinfo.report_type	= HID_REPORT_TYPE_OUTPUT;
	rinfo.report_id		= 0;
	rinfo.num_fields	= 0;
	if((err	= ioctl(fd, HIDIOCSREPORT, &rinfo)) < 0)
	{
		report(RPT_INFO, "Error with sending the REPORT ioctl %d\n", errno);
	}

  //******************************************************
	// All done, let's return what we did.
  //******************************************************
	return err;
}


//**************************************************************
// FUNCTION: VFDShowIcons
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//	int mask - Icon mask
//
// OUTPUT:
//  int err - result of the SendReport call (On success 0, On error -1)
//
// DESCRIPTION: This function will turn on/off various icons
//  depending upon what was selected.
//**************************************************************
int VFDShowIcons(Driver *drvthis, int mask)
{
  int i, err;
  char bitmap[8];
  const char panelCmd[]  = {0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00};
  const char iconCmd[]   = {0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                  
  memset(bitmap, 0, sizeof(bitmap));
  for(i=0; i< VFD_ICON_COUNT; i++)
  {
    if(!(mask & (1<<i)))
      continue;
    bitmap[icon_bits[i]/8] |= 1 << (icon_bits[i] % 8);
    //report(RPT_INFO, "Bitmap[%d]=%x\n",icon_bits[i]/8,bitmap[icon_bits[i]/8]);
  }

  err = SendReport(drvthis, panelCmd, sizeof(panelCmd));
  err = SendReport(drvthis, iconCmd, sizeof(iconCmd));
  err = SendReport(drvthis, bitmap, sizeof(bitmap));
  return err;
}

//**************************************************************
// FUNCTION: VFDIconSet
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//	int icon - Icon to manipulate
//	int state - 1 for on, 0 for off.
//
// OUTPUT:
//  int err - result of the SendReport call (On success 0, On error -1)
//
// DESCRIPTION: This function will set up the icon mask so that
//  the selected icons are marked to be on/off as desired.
//**************************************************************
int VFDIconSet(Driver *drvthis, int icon, int state)
{
  PrivateData *p = drvthis->private_data;
    
  if(icon > VFD_ICON_DVD)
    return -1;

  if(state)
    p->gIconMask |= 1<<icon;
  else
    p->gIconMask &= ~(1<<icon);

  return VFDShowIcons(drvthis, p->gIconMask);
}

//**************************************************************
// FUNCTION: VFDIconOn
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//	int icon - icon to turn on
//
// OUTPUT:
//  int err - result of the VFDIconSet call (On success 0, On error -1)
//
// DESCRIPTION: This function will turn on one specific icon.
//**************************************************************
int VFDIconOn(Driver *drvthis, int icon)
{
#ifdef DEBUG
	report(RPT_INFO, "VFDIconOn %d\n",icon);
#endif
  return VFDIconSet(drvthis, icon, 1);
}

//**************************************************************
// FUNCTION: VFDIconOff
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//	int icon - icon to turn off
//
// OUTPUT:
//  int err - result of the VFDIconSet call (On success 0, On error -1)
//
// DESCRIPTION: This function will turn off one specific icon.
//**************************************************************
int VFDIconOff(Driver *drvthis, int icon)
{
#ifdef DEBUG
	report(RPT_INFO, "VFDIconOff %d\n",icon);
#endif
  return VFDIconSet(drvthis, icon, 0);
}

//**************************************************************
// FUNCTION: VFDTurnOffIcons
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//
// OUTPUT:
//  int err - result of the SendReport call (On success 0, On error -1)
//
// DESCRIPTION: This function will turn off all the icons
//**************************************************************
int VFDTurnOffIcons(Driver *drvthis)
{
#ifdef DEBUG
	report(RPT_INFO, "VFDTurnOffIcons\n");
#endif
  int err;
  const char panelCmd[]  = {0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00};
  const char iconCmd[]   = {0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  //icon command
  const char iconoff[]   = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  //icon data
    
  err = SendReport(drvthis, panelCmd, sizeof(panelCmd));
  err = SendReport(drvthis, iconCmd, sizeof(iconCmd));
  err = SendReport(drvthis, iconoff, sizeof(iconoff));
  return err;
}

//**************************************************************
// FUNCTION: VFDSetVolume
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//  int level - volume level (between 0 and 12)
//
// OUTPUT:
//  int err - result of the SendReport call (On success 0, On error -1)
//
// DESCRIPTION: This function will raise/lower the volume indicator.
//**************************************************************
int VFDSetVolume(Driver *drvthis, int level)
{
  int i;
  PrivateData *p = drvthis->private_data;
  
  if(level>12)
    return -1;

  // Clear all of the volume values
  p->gIconMask &= ~0xFFFFF800;

  for(i=0;i<level;i++)
    p->gIconMask |= 1 << (VFD_VOLUME_1 + i);

  return VFDShowIcons(drvthis, p->gIconMask);
}

//**************************************************************
// FUNCTION: VFDSetString
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//  int region - region to display the string
//  int offset - location to display the string
//  char *buffer - string to display
//
// OUTPUT:
//  int err - result of the SendReport call (On success 0, On error -1)
//
// DESCRIPTION: This function will display a string in the 
//  specified region.
//**************************************************************
int VFDSetString(Driver *drvthis, int region, int offset, char *buffer)
{
#ifdef DEBUG
	report(RPT_INFO, "VFDSetString region %d offset %d buffer %s\n",region,offset,buffer);
#endif

  int i,size;
  int len = strlen(buffer) + 1;// make sure we make room for the NULL
  char stringCmd[8];

  if( region > VFD_STR_REGION_4)
    return -1;

  if( offset > 111)
    offset = 111;

  if( len > 128) 
  {
    len = 128;
    buffer[127] = 0;
  }

  //******************************************************
  // Figure out how many 8 character lines we'll be sending
  //******************************************************
  size = ((len + 7) / 8) + 1;

  //******************************************************
  //  Setup the string command packet
  //******************************************************
  memset(stringCmd, 0, 8);
  stringCmd[VFD_CLC_OFFSET] = size;
  stringCmd[VFD_FID_OFFSET] = VFD_FID_SET_STRING;
  stringCmd[VFD_SET_STRING_RN] = region;
  stringCmd[VFD_SET_STRING_SL] = len;
  stringCmd[VFD_SET_STRING_XP] = offset;
  SendReport(drvthis, stringCmd, sizeof(stringCmd));

  //******************************************************
  // Now send the string for display
  //******************************************************
  for(i=0; i<len;i+=8)
  {
    // make sure we only send 8 bytes at a time
    size = (len-i);
    size = (size > 8) ? 8 : size; 
    SendReport(drvthis, &buffer[i], size);
  }
  return VFDGlobalUpdateDisplay(drvthis);
}

//**************************************************************
// FUNCTION: VFDClearString
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//  int region - region to display the string based upon the chosen layout
//
// OUTPUT:
//  int err - result of the SendReport call (On success 0, On error -1)
//
// DESCRIPTION: This function will clear a string in the 
//  specified region.
//**************************************************************
int VFDClearString(Driver *drvthis, int region)
{
  char pszClearStr[8];

  if( region > VFD_STR_REGION_4)
    return -1;

  memset(pszClearStr, 0, 8);
  pszClearStr[VFD_CLC_OFFSET] = 1;
  pszClearStr[VFD_FID_OFFSET] = VFD_FID_STRING_CLEAR;
  pszClearStr[VFD_STRING_CLEAR_MD] = VFD_CLEAR_STR;
  pszClearStr[VFD_STRING_CLEAR_RN] = region;
  return SendReport(drvthis, pszClearStr, sizeof(pszClearStr));
}

//**************************************************************
// FUNCTION: VFDSetDisplay
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//  int mode - VFD_MODE_NONE, VFD_MODE_STR, VFD_MODE_GRAPHICS
//  char layout - VFD_STR_LAYOUT_1, VFD_STR_LAYOUT_2, VFD_STR_LAYOUT_3
//  char time - Scrolling Time in 50 ms units
//  char flags - String Scrolling Enable Flag 
//
// OUTPUT:
//  int err - result of the SendReport call (On success 0, On error -1)
//
// DESCRIPTION: This function will turn on scrolling in the specified region.
//**************************************************************
int VFDSetDisplay(Driver *drvthis, int mode, char layout, char time, char flags)
{
  char packet[VFD_PACKET_SIZE(1)];

  memset(packet, 0, 8);

  packet[VFD_CLC_OFFSET]     = 1;
  packet[VFD_FID_OFFSET]     = VFD_FID_SET_DISPLAY;
  packet[VFD_SET_DISPLAY_MD] = mode;
  packet[VFD_SET_DISPLAY_DM] = layout;
  packet[VFD_SET_DISPLAY_ST] = time;
  packet[VFD_SET_DISPLAY_SF] = flags;
  return SendReport(drvthis, packet, sizeof(packet));
}

//**************************************************************
// FUNCTION: VFDUpdateDisplay
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//  int mode - VFD_MODE_NONE, VFD_MODE_STR, VFD_MODE_GRAPHICS
//  char layout - VFD_STR_LAYOUT_1, VFD_STR_LAYOUT_2, VFD_STR_LAYOUT_3
//  char time - Scrolling Time in 50 ms units
//  char flags - String Scrolling Enable Flag 
//
// OUTPUT:
//  int err - result of the SendReport call (On success 0, On error -1)
//
// DESCRIPTION: This function will update the display mode/layout/scrolltime.
//**************************************************************
int VFDUpdateDisplay(Driver *drvthis, int mode, char layout, char time, char flags)
{
  PrivateData *p = drvthis->private_data;

  if(mode != p->gDisplayMode)
    return 0;

  return VFDSetDisplay(drvthis, mode, layout, time, flags);
}

//**************************************************************
// FUNCTION: VFDGlobalUpdateDisplay
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//
// OUTPUT:
//  int err - result of the SendReport call (On success 0, On error -1)
//
// DESCRIPTION: Update the display based upon the global configuration values.
//**************************************************************
int VFDGlobalUpdateDisplay(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;
  return VFDUpdateDisplay(drvthis, p->gDisplayMode, p->gLayout, p->gScrollTime, p->gFlags);
}

//**************************************************************
// FUNCTION: VFDSetScrollRegion
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//  int region - region to scroll
//
// OUTPUT:
//  int err - result of the SendReport call (On success 0, On error -1)
//
// DESCRIPTION: This function will turn on scrolling in the specified region.
//**************************************************************
int VFDSetScrollRegion(Driver *drvthis, int region)
{
  PrivateData *p = drvthis->private_data;

#ifdef DEBUG
	report(RPT_INFO, "VFDSetScrollRegion region %d\n",region);
#endif
  p->gFlags &= ~0x0F;
  p->gFlags |= (region & 0x0F);
  return VFDUpdateDisplay(drvthis, VFD_MODE_STR, p->gLayout, p->gScrollTime, p->gFlags | VFD_SCROLL_ENABLE);
}

//**************************************************************
// FUNCTION: VFDSetScrollTime
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//  int time - time in ms to scroll
//
// OUTPUT:
//  int err - result of the SendReport call (On success 0, On error -1)
//
// DESCRIPTION: This function will set the rate at which to scroll
//**************************************************************
int VFDSetScrollTime(Driver *drvthis, int time)
{
  PrivateData *p = drvthis->private_data;
  
  p->gScrollTime = time / 50;
  return VFDUpdateDisplay(drvthis, VFD_MODE_STR, p->gLayout, p->gScrollTime, p->gFlags | VFD_SCROLL_ENABLE);
}

//**************************************************************
// FUNCTION: VFDEnableDisplay
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//  int mode - VFD_MODE_NONE, VFD_MODE_STR, VFD_MODE_GRAPHICS
//  char layout - VFD_STR_LAYOUT_1, VFD_STR_LAYOUT_2, VFD_STR_LAYOUT_3
//  char time - Scrolling Time in 50 ms units
//  char flags - String Scrolling Enable Flag 
//
// OUTPUT:
//  int err - result of the SendReport call (On success 0, On error -1)
//
// DESCRIPTION: This function specifies how the VFD is segmented.
//**************************************************************
int VFDEnableDisplay(Driver *drvthis, int mode, char layout, char time, char flags)
{
  PrivateData *p = drvthis->private_data;
  
  p->gDisplayMode = mode;
  return VFDSetDisplay(drvthis, mode, layout, time, flags);
}

//**************************************************************
// FUNCTION: VFDDisableDisplay
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//
// OUTPUT:
//  int err - result of the SendReport call (On success 0, On error -1)
//
// DESCRIPTION: This function will disable any updates to the display
//**************************************************************
int VFDDisableDisplay(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;
  
  p->gDisplayMode = VFD_MODE_NONE; 
  return VFDSetDisplay(drvthis, VFD_MODE_NONE, 0, 0, 0);
}


//**************************************************************
// FUNCTION: VFDEnableString
//
// INPUT: 
//	int fd - file descriptor to the opened HID device
//  char ucLayout - specify how the VFD is segmented
//
// OUTPUT:
//  int err - result of the SendReport call (On success 0, On error -1)
//
// DESCRIPTION: This function specifies how the VFD is laid out.
//         LAYOUT 1          LAYOUT 2             LAYOUT 3
//   /----------------\  /---------------\  /--------------------\
//   |                |  |   Region 1    |  |Region 1 | Region 2 |
//   |    Region 1    |  |---------------|  |---------+----------|
//   |                |  |   Region 3    |  |Region 3 | Region 4 |
//   \----------------/  \---------------/  \--------------------/
//**************************************************************
int VFDEnableString(Driver *drvthis, char ucLayout)
{
  PrivateData *p = drvthis->private_data;
  
  if(ucLayout < VFD_STR_LAYOUT_1 || ucLayout > VFD_STR_LAYOUT_3)
    return -1;

  p->gLayout = ucLayout;
  return VFDEnableDisplay(drvthis, VFD_MODE_STR, p->gLayout, p->gScrollTime, p->gFlags | VFD_SCROLL_ENABLE);
}


//**************************************************************
// FUNCTION: _set_pixel
//
// INPUT: 
//	int page - page to draw on(VFD_GR_PAGE_1 ... VFD_GR_PAGE_4)
//  int x - coordinate
//  int y - coordinate
//  int color - 0 or 1 - turn on/off the LED
//
// OUTPUT:
//  Nothing
//
// DESCRIPTION: This function turns on/off pixels in the graphic page.
//**************************************************************
static void _set_pixel(Driver *drvthis, int page, int x, int y, int color)
{
  PrivateData *p = drvthis->private_data;
  
  char dst = ((y/8) * 112) + x;
  char mask = (1 << (7 - (y%8)));

  // Turn the LED on/off based upon the color setting
  if(color) 
    p->gPages[page][dst] |= mask;
  else
    p->gPages[page][dst] &= ~mask;
}


//**************************************************************
// FUNCTION: VFDGraphicsClearBuffer
//
// INPUT: 
//	int page - page to clear (VFD_GR_PAGE_1 ... VFD_GR_PAGE_4)
//
// OUTPUT:
//  0 on success, -1 on error
//
// DESCRIPTION: This function clears the specified graphic page.
//**************************************************************
int VFDGraphicsClearBuffer(Driver *drvthis, int page)
{
  PrivateData *p = drvthis->private_data;
  
  if(page >= VFD_PAGE_COUNT)
    return -1;

  // Clear the page to all off
  memset(p->gPages[page], 0, VFD_PAGE_SIZE);
  return 0;
}

//**************************************************************
// FUNCTION: VFDGraphicsCopyPage
//
// INPUT: 
//  int page - Graphic page (dst) (VFD_GR_PAGE_1 ... VFD_GR_PAGE_4)
//  char *buffer - buffer worth of data (src)
//
// OUTPUT:
//  Nothing
//
// DESCRIPTION: This function turns on/off pixels in the graphic page.
//**************************************************************
int VFDGraphicsCopyPage(Driver *drvthis, int page, char *buffer)
{
  int x, y;
  int shift;

  if(page >= VFD_PAGE_COUNT)
    return -1;

  for(y=0; y < VFD_HEIGHT; y++)
  {
    for(x=0; x < VFD_WIDTH; x++)
    {
      char src = (y*14)+(x/8);
      shift = 7 - (x%8);
      if(shift == 0)
        shift = 1;
      else
        shift = 1 << shift;
      _set_pixel(drvthis, page, x, y, buffer[src] & shift);
    }
  }
  return 0;
}

//**************************************************************
// FUNCTION: VFDSetGraphics
//
// INPUT: 
//  int fd - file descriptor to the opened HID device
//  char region - Graphic Page index
//  char *buf - data to display
//
// OUTPUT:
//
// DESCRIPTION: 
//**************************************************************
int VFDSetGraphics(Driver *drvthis, char region, char *buf)
{
  int i, size;
  char packet[8];

  // Send the Command to Set Graphics
  memset(packet, 0, 8);

  packet[VFD_CLC_OFFSET] = 29;
  packet[VFD_FID_OFFSET] = VFD_FID_SET_GRAPHICS;
  packet[VFD_SET_GRAPHICS_GP]=region;
  SendReport(drvthis, packet, sizeof(packet));

  // Send the actual graphics
  for(i=0; i<VFD_PAGE_SIZE;i+=8)
  {
    // make sure we only send 8 bytes at a time
    size = (VFD_PAGE_SIZE - i);
    size = (size > 8) ? 8 : size; 
    SendReport(drvthis, &buf[i], size);
  }
  return 0;
}

//**************************************************************
// FUNCTION: VFDGraphicsSendPage
//
// INPUT: 
//  int fd - file descriptor to the opened HID device
//  int page - page to display (VFD_GR_PAGE_1 ... VFD_GR_PAGE_4)
//
// OUTPUT:
//  -1 on error, or results of VFDSetGraphics
//
// DESCRIPTION:  Sends the graphics page to the VFD to be displayed.
//**************************************************************
int VFDGraphicsSendPage(Driver *drvthis, int page)
{
  PrivateData *p = drvthis->private_data;
  
  if(page >= VFD_PAGE_COUNT)
    return -1;
  return VFDSetGraphics(drvthis, page+1, p->gPages[page]);
}

//**************************************************************
// FUNCTION: VFDGraphicsShowPage
//
// INPUT: 
//  int fd - file descriptor to the opened HID device
//  int page - page to get ready to show (VFD_GR_PAGE_1 ... VFD_GR_PAGE_4)
//
// OUTPUT:
//  -1 on error
//
// DESCRIPTION: Sets the VFD into Graphic mode
//**************************************************************
int VFDGraphicsShowPage(Driver *drvthis, int page)
{
  if(page >= VFD_PAGE_COUNT)
    return -1;
  return VFDEnableDisplay(drvthis, VFD_MODE_GRAPHICS, page+1, 0, 0);
}

//**************************************************************
// FUNCTION: VFDGraphicsRect
//
// INPUT: 
//  int page - page to get ready to show (VFD_GR_PAGE_1 ... VFD_GR_PAGE_4)
//  char color - 0 or 1
//  int srcx - starting X Coordinate
//  int srcy - starting Y Coordinate
//  int width - how wide to make the box
//  int height - how tall to make the box
//
// OUTPUT:
//  -1 on error
//
// DESCRIPTION: Draws a box at (srcx,srcy) - (srcx+width, srcy+height)
//**************************************************************
int VFDGraphicsRect(Driver *drvthis, int page, char color, int srcx, int srcy, int width, int height)
{
  PrivateData *p = drvthis->private_data;
  
  char *b;
  int h, w, x, y;

  if( page >= VFD_PAGE_COUNT)
    return -1;

  if (srcx > VFD_WIDTH || srcy > VFD_HEIGHT)
    return 0;

  h = (srcy + height > VFD_HEIGHT) ? VFD_HEIGHT - srcy : srcy + height;
  w = (srcx + width > VFD_WIDTH) ? VFD_WIDTH - srcx : srcx + width;
  b = p->gPages[page];

  for( y = srcy; y < h; y++ )
    for( x = srcx; x < w; x++)
      _set_pixel(drvthis, page, x, y, color);

  return 0;
}
