#include <curses.h>

void main()
{
  char string[64];
  
  int done=0;
  int key=0;
  

  initscr();
  cbreak();
  noecho();
  nonl();
  intrflush(stdscr, FALSE);
  keypad(stdscr, TRUE);


  while(!done)
    {
      key=getch();
      if(key == KEY_BACKSPACE) done=1;
      sprintf(string, "Key: %i", key);
      
      mvaddstr(0,0,string);
      
      
    }
  
}
