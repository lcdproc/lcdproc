#ifndef CLIENT_FUNCTION_H
#define CLIENT_FUNCTION_H


/*
  The function list for clients is stored in a table, and the items each
  point to a function to call, defined below.
 */

typedef struct client_function {
   char *keyword;
   int (*function) (client * c, int argc, char **argv);
} client_function;

// FIXME?  Do these really need to be visible from other sources?
int test_func_func (client * c, int argc, char **argv);
int hello_func (client * c, int argc, char **argv);
int client_set_func (client * c, int argc, char **argv);
int client_add_key_func (client * c, int argc, char **argv);
int client_del_key_func (client * c, int argc, char **argv);
int screen_add_func (client * c, int argc, char **argv);
int screen_del_func (client * c, int argc, char **argv);
int screen_set_func (client * c, int argc, char **argv);
int widget_add_func (client * c, int argc, char **argv);
int widget_del_func (client * c, int argc, char **argv);
int widget_set_func (client * c, int argc, char **argv);
int menu_add_func (client * c, int argc, char **argv);
int menu_del_func (client * c, int argc, char **argv);
int menu_set_func (client * c, int argc, char **argv);
int menu_item_add_func (client * c, int argc, char **argv);
int menu_item_del_func (client * c, int argc, char **argv);
int menu_item_set_func (client * c, int argc, char **argv);
int backlight_func (client * c, int argc, char **argv);
int output_func (client * c, int argc, char **argv);

extern client_function commands[];



#endif
