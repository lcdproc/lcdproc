#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef LINUX
#include <sys/vfs.h>
#else
#ifdef xBSD
#include <sys/param.h>
#include <sys/mount.h>
#else
#include <sys/statfs.h>
#endif

#endif


#include "../../shared/sockets.h"

#include "main.h"
#include "mode.h"
#include "disk.h"

FILE *mtab_fd;


typedef struct mounts 
{ 
  char dev[256], type[64], mpoint[256];
  long bsize, blocks, bfree, files, ffree; 
} mounts;



static int get_fs(mounts fs[])
{
  struct statfs fsinfo;
  char line[256];
  int x = 0, y;
  
  mtab_fd = fopen("/etc/mtab", "r");

  // Get rid of old, unmounted filesystems...
  memset(fs, 0, sizeof(mounts)*256);

  while (x < 256)
  {
     if(fgets(line, 256, mtab_fd) == NULL)
     {
        fclose(mtab_fd);
        return x;
     }
    
     sscanf(line, "%s %s %s", fs[x].dev, fs[x].mpoint, fs[x].type);
     
     if(strcmp(fs[x].type, "proc")
#ifndef STAT_NFS
        && strcmp(fs[x].type, "nfs")
#endif
#ifndef STAT_SMBFS
        && strcmp(fs[x].type, "smbfs")
#endif
        ) 
     {
#if LINUX || BSD
        y = statfs(fs[x].mpoint, &fsinfo); 
#else
        y = statfs(fs[x].mpoint, &fsinfo, sizeof(fsinfo), 0); 
#endif
        fs[x].blocks = fsinfo.f_blocks;
        if(fs[x].blocks > 0)
        {
           fs[x].bsize = fsinfo.f_bsize; 
           fs[x].bfree = fsinfo.f_bfree; 
           fs[x].files = fsinfo.f_files;
           fs[x].ffree = fsinfo.f_ffree; 
           x++;
        }
     }
  }

  fclose(mtab_fd);
  return x;
}

#if 0
static int get_fs(mounts fs[])
{
  struct statfs fsinfo;
  char line[256];
  int x = 0, y;
  
  mtab_fd = fopen("/etc/mtab", "r");

  // Get rid of old, unmounted filesystems...
  memset(fs, 0, sizeof(mounts)*256);
  
  while (x < 256)
  {
    if(fgets(line, 256, mtab_fd) == NULL)
      {
	fclose(mtab_fd);
	return x;
      }
    
    sscanf(line, "%s %s %s", fs[x].dev, fs[x].mpoint, fs[x].type);
    
    if(   strcmp(fs[x].type, "proc")
#ifndef STAT_NFS
       && strcmp(fs[x].type, "nfs")
#endif
#ifndef STAT_SMBFS
       && strcmp(fs[x].type, "smbfs")
#endif
      ) 
    {
#ifdef LINUX
      y = statfs(fs[x].mpoint, &fsinfo); 
#else
      y = statfs(fs[x].mpoint, &fsinfo, sizeof(fsinfo), 0); 
#endif
      fs[x].bsize = fsinfo.f_bsize; 
      fs[x].blocks = fsinfo.f_blocks;
      fs[x].bfree = fsinfo.f_bfree; 
      fs[x].files = fsinfo.f_files;
      fs[x].ffree = fsinfo.f_ffree; 
      x++;
    }
  }

  fclose(mtab_fd);
  return x;
}
#endif


int disk_init()
{

   return 0;
}

int disk_close()
{

   return 0;
}


///////////////////////////////////////////////////////////////////////////
// Gives disk stats. 
//
// Stays onscreen until it is done.
//

// TODO: Disk screen!  Requires virtual pages in the server, though...

int disk_screen(int rep, int display)
{
   static mounts mnt[256]; 
   static int count=0;

   // Holds info to display (avoid recalculating it)
   struct disp  
   { 
	 char dev[8]; 
	 char cap[8];
	 int full; 
   } table[256];
   int i;
   static int num_disks=0;
   static int first=1;  // First line to display, sort of.

#define huge long long int
   huge size;


   if(first)
   {
      first = 0;

      sock_send_string(sock, "screen_add D\n");
      sprintf(buffer, "screen_set D name {Disk Use: %s}\n", host);
      sock_send_string(sock, buffer);
      sock_send_string(sock, "widget_add D title title\n");
      sprintf(buffer, "widget_set D title {DISKS: %s}\n", host);
      sock_send_string(sock, buffer);
      sock_send_string(sock, "widget_add D f frame\n");
      //sock_send_string(sock, "widget_set D f 1 2 20 4 20 3 v 8\n");
      sprintf(buffer, "widget_set D f 1 2 %i %i %i %i v 12\n",
	      lcd_wid, lcd_hgt, lcd_wid, lcd_hgt-1);
      sock_send_string(sock, buffer);
      sock_send_string(sock, "widget_add D err1 string\n");
      sock_send_string(sock, "widget_add D err2 string\n");
      sock_send_string(sock, "widget_set D err1 5 2 {  Reading  }\n");
      sock_send_string(sock, "widget_set D err2 5 3 {Filesystems}\n");
   }


   
   // Grab disk stats on first display, and fill "table".
   // Get rid of old, unmounted filesystems...
   memset(table, 0, sizeof(struct disp)*256);
    
   count = get_fs(mnt);
   first = 0;
    
   // Fill the display structure...
   if(count)
   {
      sock_send_string(sock, "widget_set D err1 30 5 .\n");
      sock_send_string(sock, "widget_set D err2 30 5 .\n");
      for(i=0; i<count; i++)
      {
	 if(strlen(mnt[i].mpoint) > 6)
	 {
	    sprintf(table[i].dev, "%c%s", ELLIPSIS,
		    (mnt[i].mpoint)+(strlen(mnt[i].mpoint)-5));
	 }
	 else
	 {
	    sprintf(table[i].dev, "%s", mnt[i].mpoint);
	 }
	 
	 //table[i].full = (lcd.cellwid * 4)
	 table[i].full = (huge)(lcd_cellwid * 4)
	    * (huge)(mnt[i].blocks - mnt[i].bfree)
	    / (huge)mnt[i].blocks;

	 size = (huge)mnt[i].bsize * (huge)mnt[i].blocks;
	 memset(table[i].cap, 0, 8);

	 // Kilobytes
	 if(size > 0  &&  size < (huge)1000*(huge)1000)
	    sprintf(table[i].cap, "%3.1fk", 
		    (double)(size)/1024.0);
	 // Megabytes
	 else if(size >= (huge)1000*(huge)1000
		 &&  size < (huge)1000*(huge)1000*(huge)1000)
	    sprintf(table[i].cap, "%3.1fM", 
		    (float)(size/(huge)1024)/1024.0);
	 // Gigabytes
	 else if(size >= (huge)1000*(huge)1000*(huge)1000  
		 &&  size < (huge)1000*(huge)1000*(huge)1000*(huge)1000)
	    sprintf(table[i].cap, "%3.1fG", 
		    (float)(size/((huge)1024*(huge)1024))/1024.0);
	 // Terabytes
	 else if(size >= (huge)1000*(huge)1000*(huge)1000*(huge)1000  
		 &&  size < (huge)1000*(huge)1000*(huge)1000*(huge)1000*(huge)1000)
	    sprintf(table[i].cap, "%3.1fT", 
		    (float)(size/((huge)1024*(huge)1024*(huge)1024))/1024.0);
	 
	 // PectaBytes -- Yeah!  I want some!
	 else if(size >= (huge)1000*(huge)1000*(huge)1000*(huge)1000*(huge)1000
		 &&  size < (huge)1000*(huge)1000*(huge)1000*(huge)1000*(huge)1000*(huge)1000)
	    sprintf(table[i].cap, "%3.1fP", 
		    (float)(size/((huge)1024*(huge)1024*(huge)1024*(huge)1024))/1024.0);
	 
      }
   }

   
   
   if (!count) 
   { 
      sock_send_string(sock, "widget_set D err1 1 2 {Error Retrieving}\n");
      sock_send_string(sock, "widget_set D err2 1 3 {Filesystem Stats}\n");
      return 0;
   }


   // Display stuff...  (show for two seconds, then scroll once per
   //  second, then hold at the end for two seconds)
   sprintf(buffer, "widget_set D f 1 2 %i %i %i %i v 12\n",
	   lcd_wid, lcd_hgt, lcd_wid, count);
   sock_send_string(sock, buffer);
   //sprintf(tmp, "widget_set D f 1 2 20 4 20 %i v 8\n", count);
   //sock_send_string(sock, tmp);
   for(i=0; i<count; i++)
   {
      if(table[i].dev[0] == 0) continue;
      if(i >= num_disks) // Make sure we have enough lines...
      {
	 sprintf(tmp, "widget_add D s%i string -in f\n", i);
	 sock_send_string(sock, tmp);
	 sprintf(tmp, "widget_add D h%i hbar -in f\n", i);
	 sock_send_string(sock, tmp);
	 
      }
      sprintf(tmp, "widget_set D s%i 1 %i {%-6s %6s E    F}\n",
	      i, i+1, table[i].dev, table[i].cap);
      sock_send_string(sock, tmp);
      sprintf(tmp, "widget_set D h%i 16 %i %i\n",
	      i, i+1, table[i].full);
      sock_send_string(sock, tmp);
   }

   // Now remove extra widgets...
   for(; i < num_disks; i++)
   {
	 sprintf(tmp, "widget_del D s%i\n", i);
	 sock_send_string(sock, tmp);
	 sprintf(tmp, "widget_del D h%i\n", i);
	 sock_send_string(sock, tmp);
   }
   

   num_disks = count;

  #undef huge

/*    
// ** FILESYSTEMS *****
// /      543.2M E----F
// /dos/c   2.1G E----F
// /stuff   4.3G E----F
*/
  return 0;
}

