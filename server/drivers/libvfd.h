/* Header for libvfd
 * Copyright (C) 2006, Advanced Micro Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef LIBVFD_H_
#define LIBVFD_H_

/* Offsets into the VFD command packet */

#define VFD_CLC_OFFSET          0x00
#define VFD_FID_OFFSET          0x01

/* VFD FID (Function IDs) */

#define VFD_FID_PANEL_BANK      0x01
#define VFD_FID_ICON_CONTROL    0x02
#define VFD_FID_SET_STRING      0x03
#define VFD_FID_STRING_CLEAR    0x04
#define VFD_FID_SET_DISPLAY     0x05
#define VFD_FID_SET_GRAPHICS    0x06
#define VFD_FID_GRAPHIC_AREA    0x07

/* Command specific offsets */

/* Panel Blank */
#define VFD_PANEL_BLANK_BL      0x02

/* Icon Control */
#define VFD_ICON_CONTROL_BITMAP 0x08

/* Set String */
#define VFD_SET_STRING_RN       0x02
#define VFD_SET_STRING_SL       0x03
#define VFD_SET_STRING_XP       0x04
#define VFD_SET_STRING_DATA     0x08

/* String Clear */
#define VFD_STRING_CLEAR_MD     0x02
#define      VFD_CLEAR_STR      0x01
#define      VFD_CLEAR_GRAPHICS 0x02
#define VFD_STRING_CLEAR_RN     0x03

/* Set Display */
#define VFD_SET_DISPLAY_MD      0x02
#define       VFD_MODE_NONE     0x00
#define       VFD_MODE_STR      0x02
#define       VFD_MODE_GRAPHICS 0x03
#define VFD_SET_DISPLAY_DM      0x03
#define VFD_SET_DISPLAY_ST      0x04
#define VFD_SET_DISPLAY_SF      0x06

/* Set Graphics */
#define VFD_SET_GRAPHICS_GP     0x02
#define VFD_SET_GRAPHICS_DATA   0x08

/* Graphic Area */
#define VFD_GRAPHIC_AREA_OOF    0x02

/* LAYOUT 1
  /-----------------------\
  |                       |
  |      Region 1         |
  |                       |
  \-----------------------/
*/

/* LAYOUT 2
  /-----------------------\
  |        Region 1       |
  |---------------------- |
  |        Region 3       |
  \-----------------------/
*/

/* LAYOUT 3
  /-----------------------\
  | Region 1 | Region 2   |
  |---------------------- |
  | Region 3 | Region 4   |
  \-----------------------/
*/

#define VFD_GR_PAGE_1 0x0
#define VFD_GR_PAGE_2 0x1
#define VFD_GR_PAGE_3 0x2
#define VFD_GR_PAGE_4 0x3

#define VFD_STR_LAYOUT_1 0x01
#define VFD_STR_LAYOUT_2 0x02
#define VFD_STR_LAYOUT_3 0x03

#define VFD_STR_REGION_1 0x01
#define VFD_STR_REGION_2 0x02
#define VFD_STR_REGION_3 0x03
#define VFD_STR_REGION_4 0x04

#define VFD_SCROLL_REGION1 (1 << 0)
#define VFD_SCROLL_REGION2 (1 << 1)
#define VFD_SCROLL_REGION3 (1 << 2)
#define VFD_SCROLL_REGION4 (1 << 3)

#define VFD_SCROLL_ENABLE  (1 << 7)

/* Graphics defintions */
/* The graphics area has a static resolution */

#define VFD_WIDTH  112
#define VFD_HEIGHT 16
#define VFD_PITCH  (112 / 8)
#define VFD_PAGE_SIZE  (VFD_PITCH * VFD_HEIGHT)
#define VFD_PAGE_COUNT   4

#define VFD_ICON_VIDEO   0
#define VFD_ICON_CD      1   
#define VFD_ICON_PLAY    2   
#define VFD_ICON_RWND    3   
#define VFD_ICON_PAUSE   4   
#define VFD_ICON_FFWD    5 
#define VFD_ICON_SPEAKER 6   
#define VFD_ICON_REC     7   
#define VFD_ICON_VOLUME  8   
#define VFD_ICON_RADIO   9   
#define VFD_ICON_DVD     10  

#define VFD_VOLUME_1     11  
#define VFD_VOLUME_2     12
#define VFD_VOLUME_3     13
#define VFD_VOLUME_4     14
#define VFD_VOLUME_5     15
#define VFD_VOLUME_6     16
#define VFD_VOLUME_7     17
#define VFD_VOLUME_8     18
#define VFD_VOLUME_9     19
#define VFD_VOLUME_10    20
#define VFD_VOLUME_11    21
#define VFD_VOLUME_12    22

#define VFD_ICON_COUNT   23

int vfd_cmd_graphic_area_state(unsigned char);
int vfd_cmd_panel_bank(unsigned char);
int vfd_cmd_icon_control(unsigned char *);
int vfd_cmd_set_string(unsigned char, unsigned char, unsigned char *, int);
int vfd_cmd_clear(unsigned char, unsigned char);
int vfd_cmd_set_display(unsigned char, unsigned char, unsigned char, unsigned char);
int vfd_cmd_set_graphics(unsigned char, unsigned char *);

int vfd_enable_display(int, unsigned char, unsigned char, unsigned char);
int vfd_update_display(int, unsigned char, unsigned char, unsigned char);
int vfd_disable_display(int);

void vfd_str_set_scroll_time(int);
void vfd_str_set_scroll_speed(unsigned char);
void vfd_str_set_scroll_regions(unsigned char);
int vfd_str_set_string(unsigned char, unsigned char, const unsigned char *);
int vfd_str_clear_string(unsigned char);
int vfd_str_enable(unsigned char);
int vfd_str_disable(void);

int vfd_gr_clear_buffer(unsigned char);
int vfd_gr_show_page(int);
int vfd_gr_disable(void);
int vfd_gr_send_page(int);
int vfd_gr_clear_page(int);
int vfd_gr_copy_page(int page, char *src);

int vfd_icon_set(int, int);
int vfd_icon_on(int, int);
int vfd_icon_off(int, int);

void vfr_gr_bitmap(int, char *, int, int, int, int, int, int);
int vfr_gr_rect(int, unsigned char, int, int, int, int);

int vfd_init(void);
int vfd_send_command(unsigned char, int, unsigned char *);
void vfd_close(void);

#endif
