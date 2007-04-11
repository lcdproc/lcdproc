#ifndef LCD_LINK_H
#define LCD_LINK_H

extern char *address;
extern int port;

int connect_and_setup(void);
int read_response(char *str, int maxsize);
int process_response(char *str);
int update_display(void);
int send_nop(void);

#endif
