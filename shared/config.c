#include "config.h"


typedef struct command
{
      char *text;
      struct command *child;
      int (*func)(char *line);
};

command main_list[] = {
   "Key",           NULL,           Key_func,
   "Driver",        Driver_commands,NULL,
   NULL,            NULL,           NULL,
};

command Driver_commands[] = {
   "MtxOrb",        NULL,           MtxOrb_drv_init,
   "curses",        NULL,           curses_drv_init,
   "hd44780",       NULL,           hd44780_drv_init,
//   "X11",           NULL,           X11_drv_init,
//   "debug",         NULL,           debug_drv_init,
   "text",          NULL,           text_drv_init,
};



// Interpret a line...
int parse_line(char *line);
// Interpret the rest of the line...
int parse_rest(char *line, command *commands);


////////// Helper functions for line parsing /////////////////
// Pops off the first word of the string.
char *shift(char *string);


////////////// Config Functions /////////////////////////////////
int Key_func(char *line)
{
   // uh, interpret the command...
   return 0;
}


int parse_line(char *line, command *commands)
{
   int i, j;
   int newtoken, inquote;
   char *str;
//   char *tok;
   int argc;
   char *argv[256];
   char delimiters[] = " \0";
   char leftquote[]  = "\0\"'`([{\0";
   char rightquote[] = "\0\"'`)]}\0";
   char errmsg[256];
   int invalid=0;
   int err=0;
   
   // And parse all its messages...
   //debug("parse: Getting messages...\n");
   str = strdup(line);
   debug("parse: ...%s\n", str);
   // Now, split up the string...
   //len = strlen(str);
   argc=0;
   newtoken=1;
   inquote=0;
   for(i=0; str[i]; i++)
   {
      if(inquote)  // Scan for the end of the quote
      {
	 if(str[i] == rightquote[inquote])
	 {  // Found the end of the quote
	    inquote=0;
	    str[i] = 0;
	    newtoken=1;
	 }
      }
      else // Normal operation; split at delimiters
      {
	 for(j=1; leftquote[j]; j++)
	 {
	    // Found the beginning of a new quote...
	    if(str[i] == leftquote[j])
	    {
	       inquote = j;
	       str[i] = 0;
	       continue;
	    }
	 }
	 for(j=0; delimiters[j]; j++)
	 {
	    // Break into a new string...
	    if(str[i] == delimiters[j])
	    {
	       str[i] = 0;
	       newtoken = 1;
	       continue;
	    }
	 }
      }
      if(newtoken && str[i])
      {
	 newtoken=0;
	 argv[argc] = str + i;
	 argc++;
      }
      else
      {
      }
   }
   if(inquote)
   {
      sprintf(errmsg, "huh? Unterminated string: missing %c\n",
	      rightquote[inquote]);
      sock_send_string(c->sock, errmsg);
      continue;
   }
/*
  for(tok = strtok(str, delimiters);
  tok;
  tok=strtok(NULL, delimiters))
  {
  argv[argc] = tok;
  argc++;
  }
*/
   argv[argc] = NULL;
   if(argc < 1) continue;
   
   // Now find and call the appropriate function...
//	    debug("parse: Finding function...\n");
   invalid = 1;
   for(i=0; commands[i].keyword; i++)
   {
//	       debug("(checking %s)\n", commands[i].keyword);
      if(0 == strcmp(argv[0], commands[i].keyword))
      {
//	          debug("(FOUND %s)\n", commands[i].keyword);
	 invalid = commands[i].function(c, argc, argv);
//	          debug("parse: Returned %i...\n", err);
      }
   }
   if(invalid)
   {
      // FIXME:  Check for buffer overflows here...
      err = 1;
      //sprintf(errmsg, "huh? Invalid command \"%s\"\n", argv[0]);
      //sock_send_string(c->sock, errmsg);
   }
   
   free(str);  // Don't want to forget this...  :)
   
   return err;
}
