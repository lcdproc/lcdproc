/*
  render.c

  Draws screens on the LCD.

  This needs to be greatly expanded and redone for greater flexibility.
  For example, it should support multiple screen sizes, more flexible
  widgets, and multiple simultaneous screens.

  This will probably take a while to do.  :(


  THIS FILE IS MESSY!  Anyone care to rewrite it nicely?  Please??  :)
  
 */


#include <string.h>
#include <stdio.h>

#include "../shared/debug.h"
#include "../shared/LL.h"

#include "drivers/lcd.h"

#include "screen.h"
#include "screenlist.h"
#include "widget.h"
#include "render.h"



int heartbeat=HEART_OPEN;
int backlight=BACKLIGHT_OPEN;
int backlight_state = BACKLIGHT_OPEN;
int backlight_brightness=255;
int backlight_off_brightness=0;
int output_state = 0;

static int reset;

#define BUFSIZE 1024

static int draw_frame(LL *list, char fscroll,
		      int left, int top, int right, int bottom,
		      int fwid, int fhgt,
		      int fspeed, int timer);


int draw_screen(screen *s, int timer)
{
   static screen *old_s=NULL;
   int tmp=0;
   
   //debug("Render...\n");
   //return 0;

   reset = 1;

   //debug("draw_screen: %8x, %i\n", (int)s, timer);
   
   if(!s) return -1;

   if(s == old_s) reset = 0;
   old_s = s;



   lcd.clear();


   switch(backlight_state)
   {
      case BACKLIGHT_OFF:
	 lcd.backlight(backlight_off_brightness);
	 break;
      case BACKLIGHT_ON:
	 lcd.backlight(backlight_brightness);
	 break;
      default:
	 if(backlight_state & BACKLIGHT_FLASH)
	 {
	    tmp = (!((timer&7) == 7));
	    if(backlight_state & 1)
	       lcd.backlight(tmp?backlight_brightness:backlight_off_brightness);
	       //lcd.backlight(backlight_brightness * (!((timer&7) == 7)));
	    else
	       lcd.backlight(!tmp?backlight_brightness:backlight_off_brightness);
	       //lcd.backlight(backlight_brightness * ((timer&7) == 7));
	 }
	 else if(backlight_state & BACKLIGHT_BLINK)
	 {
	    tmp = (!((timer&14) == 14));
	    if(backlight_state & 1)
	       lcd.backlight(tmp?backlight_brightness:backlight_off_brightness);
	       //lcd.backlight(backlight_brightness * (!((timer&14) == 14)));
	    else
	       lcd.backlight(!tmp?backlight_brightness:backlight_off_brightness);
	       //lcd.backlight(backlight_brightness * ((timer&14) == 14));
	 }
	 break;
   }

   lcd.output(output_state);

   draw_frame(s->widgets, 'v',
	      0, 0, lcd.wid, lcd.hgt,
	      s->wid, s->hgt,
	      s->duration/s->hgt, timer);
   
   
   //debug("draw_screen done\n");

   
   if(heartbeat)
   {
      if(s->heartbeat  ||  heartbeat == HEART_ON)
      {
	 // Set this to pulsate like a real heart beat...
	 // (binary is fun...  :)
	 lcd.icon(!((timer+4)&5), 0);
	 lcd.chr(lcd.wid, 1, 0);
      }
   }
   
   lcd.flush();
   
   //debug("draw_screen: %8x, %i\n", s, timer);

   return 0;
   
}

static int draw_frame(LL *list, char fscroll,
		      int left, int top, int right, int bottom,
		      int fwid, int fhgt,
		      int fspeed, int timer)
{
   char str[BUFSIZE];  // scratch buffer
   widget *w;
   
   int wid, hgt;  // Width and height of visible frame area
   int x, y;
   int fx, fy;  // Scrolling offset for the frame...
   int length, speed;
   //int lines;
   
   int reset = 1;

   wid = right - left;  // This is the size of the visible frame area
   hgt = bottom - top;

   fx = 0;
   fy = 0;
   if(fscroll == 'v')
   {
      
      if(fspeed > 0)
	 fy = (timer-fspeed) / fspeed;
      if(fspeed < 0)
	 fy = (-fspeed) * timer;
      if(fy < 0) fy = 0;

      // Make sure the whole frame gets displayed, at least...
      if(!screenlist_action) screenlist_action = RENDER_HOLD;
      if((fy) > fhgt - 1)
      {
	 // Release hold after it has been displayed
	 if(!screenlist_action  ||  screenlist_action == RENDER_HOLD)
	    screenlist_action = 0;
      }

      fy %= fhgt;
      if(fy > fhgt - hgt) fy = fhgt - hgt;
   }
   else if (fscroll == 'h')
   { // TODO:  Frames don't scroll horizontally yet!
      
   }
   
   //debug("draw_screen: %8x, %i\n", s, timer);
   
   if(!list) return -1;
   
   //debug("draw_frame: %8x, %i\n", frame, timer);

   
   
   LL_Rewind(list);
   do {
      w = (widget *)LL_Get(list);
      if(!w) return -1;

      // TODO:  Make this cleaner and more flexible!
      switch(w->type)
      {
	 case WID_STRING:
	    if((w->x > 0) && (w->y > 0) && (w->text))
	    {
	       if((w->y <= hgt + fy)  &&
		  (w->y > fy))
	       {
		  strncpy(str, w->text, wid - w->x + 1);
		  str[wid - w->x + 1] = 0;
		  lcd.string(w->x + left,
			     w->y + top - fy,
			     str);
	       }
	    }
	    break;
	 case WID_HBAR:
	    if(reset)
	    {
	       lcd.init_hbar();
	       reset = 0;
	    }
	    if((w->x > 0) && (w->y > 0))
	    {
	       if((w->y <= hgt + fy)  &&
		  (w->y > fy))
	       {
		  if(w->length > 0)
		  {
		     if((w->length / lcd.cellwid) < wid - w->x + 1)
			lcd.hbar(w->x + left,
				 w->y + top - fy,
				 w->length);
		     else
			lcd.hbar(w->x + left,
				 w->y + top - fy,
				 wid * lcd.cellwid);
		  }
		  else if(w->length < 0)
		  {
		     // TODO:  Rearrange stuff to get left-extending
		     // hbars to draw correctly...
		     // .. er, this'll require driver modifications,
		     // so I'll leave it out for now.
		  }
	       }
	    }
	    break;
	 case WID_VBAR:  // FIXME:  Vbars don't work in frames!
	    if(reset)
	    {
	       lcd.init_vbar();
	       reset = 0;
	    }
	    if((w->x > 0) && (w->y > 0))
	    {
	       if(w->length > 0)
	       {
		  lcd.vbar(w->x, w->length);
	       }
	       else if(w->length < 0)
	       {
		  // TODO:  Rearrange stuff to get down-extending
		  // vbars to draw correctly...
		  // .. er, this'll require driver modifications,
		  // so I'll leave it out for now.
	       }
	    }
	    break;
	 case WID_ICON:  // FIXME:  Not implemented
	    break;
	 case WID_TITLE: // FIXME:  Doesn't work quite right in frames...
	    if(!w->text) break;
	    if(wid < 8) break;
	    
	    memset(str, 255, wid);
	    str[2] = ' ';
	    length = strlen(w->text);
	    if(length <= wid-6)
	    {
	       memcpy(str+3, w->text, length);
	       str[length+3] = ' ';
	    }
	    else // Scroll the title, if it doesn't fit...
	    {
	       speed = 1;
	       x = timer / speed;
	       y = x / length;

	       // Make sure the whole title gets displayed, at least...
	       if(!screenlist_action) screenlist_action = RENDER_HOLD;
	       if(x > length - 6)
	       {
		  // Release hold after it has been displayed
		  if(!screenlist_action  ||  screenlist_action == RENDER_HOLD)
		     screenlist_action = 0;
	       }
	       x %= (length);
	       x -= 3;
	       if(x < 0) x = 0;
	       if(x > length - (wid-6)) x = length - (wid-6);
	       
	       if(y&1) // Scrolling backwards...
	       {
		  x = (length-(wid-6)) - x;
	       }
	       strncpy(str+3, w->text+x, (wid-6));
	       str[wid-3] = ' ';
	    }
	    str[wid] = 0;
	    
	    lcd.string(1 + left, 1 + top, str);
	    break;
	 case WID_SCROLLER: // FIXME: doesn't work in frames...
	 {
	      int offset;
	      int screen_width;
	      if (!w->text) break;
	      if (w->right < w->left) break;
	      //printf("rendering: %s %d\n",w->text,timer);
	      screen_width = w->right - w->left + 1;
	      switch (w->length)
	      { // actually, direction...
		 // FIXED:  Horz scrollers don't show the
		 // last letter in the string...  (1-off error?)
		 case 'h':
		    length = strlen(w->text) + 1;
		    if (length <= screen_width)
		    {
		       /* it fits within the box, just render it */
		       lcd.string(w->left,w->top,w->text);
		    }
		    else
		    {
		       int effLength = length - screen_width;
		       int necessaryTimeUnits = 0;
		       if (!screenlist_action) screenlist_action = RENDER_HOLD;
		       if (w->speed > 0) {
			  necessaryTimeUnits = effLength * w->speed;
			  if (((timer / (effLength*w->speed)) % 2) == 0) {
			     //wiggle one way
			     offset = (timer % (effLength*w->speed))
				/ w->speed;
			  }
			  else
			  {
			     //wiggle the other
			     offset = (((timer % (effLength*w->speed))
					- (effLength*w->speed) + 1)
				       / w->speed) * -1;
			  }
		       }
		       else if (w->speed < 0)
		       {
			  necessaryTimeUnits = effLength / (w->speed * -1);
			  if (((timer / (effLength/(w->speed*-1))) % 2) == 0)
			  {
			     offset = (timer % (effLength/(w->speed*-1)))
				* w->speed * -1;
			  }
			  else
			  {
			     offset = (((timer % (effLength/(w->speed*-1)))
					* w->speed * -1) - effLength + 1) * -1;
			  }
		       }
		       else
		       {
			  offset = 0;
			  if(screenlist_action == RENDER_HOLD)
			     screenlist_action = 0;
		       }
		       if (timer > necessaryTimeUnits)
		       {
			  if(screenlist_action == RENDER_HOLD)
			     screenlist_action = 0;
		       }
		       if (offset <= length)
		       {
			  strncpy(str,&((w->text)[offset]),screen_width);
			  str[screen_width] = '\0';
			  //printf("%s : %d\n",str,length-offset);
		       }
		       else
		       {
			  str[0] = '\0';
		       }
		       lcd.string(w->left,w->top,str);
		    }
		    break;
		 // FIXME:  Vert scrollers don't always seem to scroll
                 // back up after hitting the bottom.  They jump back to
                 // the top instead...  (nevermind?)
		 case 'v':
		 {
		    int i=0;
		    length = strlen(w->text);
		    if (length <= screen_width)
		    {
		       /* no scrolling required...*/
		       lcd.string(w->left,w->top,w->text);
		    }
		    else
		    {
		       int lines_required =
			  (length / screen_width)
			  + (length % screen_width ? 1 : 0);
		       int available_lines = (w->bottom - w->top + 1);
		       if (lines_required <= available_lines)
		       {
			  // easy...
			  for(i=0;i<lines_required;i++)
			  {
			     strncpy(str,
				     &((w->text)[i*screen_width]),
				     screen_width);
			     str[screen_width] = '\0';
			     lcd.string(w->left,w->top + i,str);
			  }
		       }
		       else
		       {
			  int necessaryTimeUnits = 0;
			  int effLines = lines_required - available_lines + 1;
			  int begin = 0;
			  if (!screenlist_action)
			     screenlist_action = RENDER_HOLD;
			  //printf("length: %d sw: %d lines req: %d  avail lines: %d  effLines: %d \n",length,screen_width,lines_required,available_lines,effLines);
			  if (w->speed > 0)
			  {
			     necessaryTimeUnits = effLines * w->speed;
			     if (((timer / (effLines*w->speed)) % 2) == 0)
			     {
				//printf("up ");
				begin = (timer % (effLines*w->speed))
				   / w->speed;
			     }
			     else
			     {
				//printf("down ");
				begin = (((timer % (effLines*w->speed))
					  - (effLines*w->speed) + 1)/w->speed)
				   * -1;
			     }
			  }
			  else if (w->speed < 0)
			  {
			     necessaryTimeUnits = effLines / (w->speed * -1);
			     if (((timer / (effLines/(w->speed*-1))) % 2) == 0)
			     {
				begin = (timer % (effLines/(w->speed*-1)))
				   * w->speed * -1;
			     }
			     else
			     {
				begin = (((timer % (effLines/(w->speed*-1)))
					  * w->speed * -1) - effLines + 1)
				   * -1;
			     }
			  }
			  else
			  {
			     begin = 0;
			  }
			  //printf("rendering begin: %d  timer: %d effLines: %d\n",begin,timer,effLines);
			  for(i=begin;i<begin+available_lines;i++)
			  {
			     strncpy(str,
				     &((w->text)[i*(screen_width)]),
				     screen_width);
			     str[screen_width] = '\0';
			     //printf("rendering: '%s' of %s\n",
			     //str,w->text);
			     lcd.string(w->left,w->top + (i-begin),str);
			  }
			  if (timer > necessaryTimeUnits)
			  {
			     if(screenlist_action == RENDER_HOLD)
				screenlist_action = 0;
			  }
		       }
		    }
		    break;
		 }
	      }
	      break;
	 }
	 case WID_FRAME:
	 {
	    // FIXME: doesn't handle nested frames quite right!
	    // doesn't handle scrolling in nested frames at all...
	    int new_left, new_top, new_right, new_bottom;
	    new_left = left + w->left - 1;
	    new_top = top + w->top - 1;
	    new_right = left + w->right;
	    new_bottom = top + w->bottom;
	    if(new_right > right) new_right = right;
	    if(new_bottom > bottom) new_bottom = bottom;
	    if(new_left >= right  ||
	       new_top >= bottom)
	    {  // Do nothing if it's invisible...
	    }
	    else
	    {
	       draw_frame(w->kids, w->length,
			  new_left, new_top, new_right, new_bottom,
			  w->wid, w->hgt,
			  w->speed, timer);
	    }
	 }
	 break;
	 case WID_NUM: // FIXME: doesn't work in frames...
	    if((w->x > 0) && (w->y >= 0) && (w->y <= 9))
	    {
	       if(reset)
	       {
	          lcd.init_num();
	          reset = 0;
	       }
	       lcd.num(w->x + left, w->y);
	    }
	    break;
	 case WID_NONE:
	 default:
	    break;
      }
   } while(LL_Next(list) == 0);

   
   return 0;
}

