#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../server/drivers/lcd.h"

int main()
{
   int err;
   int len;
   
   srand(time(NULL));
   
   err = lcd_init("");

   err = lcd_add_driver("joy", "");
   err = lcd_add_driver("curses", "Booger");
//   err = lcd_add_driver("text", "Booger");
   err = lcd_add_driver("MtxOrb", "");

   lcd.clear();
   
   lcd.init_hbar();

   lcd.string(1,1,"Booger!");
   len = rand()%100;
   lcd.hbar(1,2,len );
   lcd.flush();

   for(err=0; err!='Z'; err=lcd.getkey())
   {
//      if(err)
      {
	 if(err == 'B') len--;
	 if(err == 'C') len++;
	 if(err == 'D') len--;
	 if(err == 'E') len-=5;
	 if(err == 'F') len+=5;
	 
	 lcd.clear();
	 lcd.string(1,1,"Booger!");
	 lcd.hbar(1,2,len );
	 lcd.chr(1,3,(char)err);
	 
	 lcd.flush();
      }
      usleep(125000);
   }
   
//   sleep(1);

   lcd_shutdown();
   
   return 0;
}
