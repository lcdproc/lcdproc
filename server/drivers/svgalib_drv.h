#ifndef LCD_SVGALIB_H
#define LCD_SVGALIB_H


extern lcd_logical_driver *vgalib_drv;

int svgalib_drv_init(struct lcd_logical_driver *driver, char *args);
void svgalib_drv_close();
void svgalib_drv_clear();
void svgalib_drv_flush();
void svgalib_drv_string(int x, int y, char string[]);
void svgalib_drv_chr(int x, int y, char c);
void svgalib_drv_vbar(int x, int len);
void svgalib_drv_hbar(int x, int y, int len);
void svgalib_drv_icon(int which, char dest);
void svgalib_drv_flush();
void svgalib_drv_flush_box(int lft, int top, int rgt, int bot);
void svgalib_drv_draw_frame(char *dat);
char svgalib_drv_getkey();
void svgalib_drv_init_num();
void svgalib_drv_num(int x, int num);


#endif
