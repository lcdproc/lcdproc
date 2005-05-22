#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef IRIX
#include <strings.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include "shared/sockets.h"
#include "shared/LL.h"

#include "main.h"
#include "mode.h"
#include "machine.h"
#include "mem.h"
#include "util.h"


/////////////////////////////////////////////////////////////////////////
// Mem Screen displays info about memory and swap usage...
//
// +--------------------+	+--------------------+
// |##  MEM -==- SWAP #@|	|M 758.3M [- ] 35.3%@|
// | 758.3M Totl 1.884G |	|S 1.884G [  ]  0.1% |
// | 490.8M Free 1.882G |	+--------------------+
// |E---    F  E       F|
// +--------------------+
//
int
mem_screen (int rep, int display)
{
	static int first = 1;
	static int which_title = 0;
	static int gauge_wid = 0;
	static int gauge_offs = 0;
	meminfo_type mem[2];

	if (first) {
		first = 0;

		sock_send_string(sock, "screen_add M\n");
		sprintf(buffer, "screen_set M -name {Memory & Swap: %s}\n", get_hostname());
		sock_send_string(sock, buffer);

		if (lcd_hgt >= 4) {
			gauge_wid = (lcd_wid >= 18)
				    ? (lcd_wid - 6) / 2		// room for E..F pairs and 2 spaces in between
				    : (lcd_wid - 4) / 2;	// leave room for the  E...F pairs

			sock_send_string(sock, "widget_add M title title\n");
			sock_send_string(sock, "widget_set M title { MEM -==- SWAP}\n");
			sock_send_string(sock, "widget_add M totl string\n");
			sock_send_string(sock, "widget_add M used string\n");
			sprintf(buffer, "widget_set M totl %i 2 Totl\n", lcd_wid/2 - 1);
			sock_send_string(sock, buffer);
			sprintf(buffer, "widget_set M used %i 3 Free\n", lcd_wid/2 - 1);
			sock_send_string(sock, buffer);
			sock_send_string(sock, "widget_add M EFmem string\n");
			sprintf(buffer, "widget_set M EFmem 1 4 {E%*sF}\n", gauge_wid, "");
			sock_send_string(sock, buffer);
			sock_send_string(sock, "widget_add M EFswap string\n");
			sprintf(buffer, "widget_set M EFswap %i 4 {E%*sF}\n",
				lcd_wid - gauge_wid - 1, gauge_wid, "");
			sock_send_string(sock, buffer);
			sock_send_string(sock, "widget_add M memused string\n");
			sock_send_string(sock, "widget_add M swapused string\n");
		} else {
			gauge_wid = (lcd_wid >= 18) ? max(2, lcd_wid - 18) : 0;
			gauge_offs = (lcd_wid - gauge_wid) / 2 + 2;
		
			sock_send_string(sock, "widget_add M m string\n");
			sock_send_string(sock, "widget_add M s string\n");
			if (gauge_wid > 0) {
				sprintf(buffer, "widget_set M m 1 1 {M%*s[%*s]}\n",
					gauge_offs - 3, "", gauge_wid, "");
				sock_send_string(sock, buffer);
				sprintf(buffer, "widget_set M s 1 2 {S%*s[%*s]}\n",
					gauge_offs - 3, "", gauge_wid, "");
				sock_send_string(sock, buffer);
			}
			else {
				sock_send_string(sock, "widget_set M m 1 1 {M   }\n");
				sock_send_string(sock, "widget_set M s 1 2 {S   }\n");
			}
			sock_send_string(sock, "widget_add M mem% string\n");
			sock_send_string(sock, "widget_add M swap% string\n");
		}

		sock_send_string(sock, "widget_add M memtotl string\n");
		sock_send_string(sock, "widget_add M swaptotl string\n");

		sock_send_string(sock, "widget_add M memgauge hbar\n");
		sock_send_string(sock, "widget_add M swapgauge hbar\n");
	}

	machine_get_meminfo(mem);

	if (lcd_hgt >= 4) {
		// flip the title back and forth... (every 4 updates)
		if (which_title & 4) {
			sprintf(buffer, "widget_set M title {%s}\n", get_hostname());
			sock_send_string(sock, buffer);
		} else
			sock_send_string(sock, "widget_set M title { MEM -==- SWAP}\n");
		which_title = (which_title + 1) & 7;

		// Total memory
		sprintf_memory(tmp, mem[0].total * 1024, 1);
		sprintf(buffer, "widget_set M memtotl 1 2 {%7s}\n", tmp);
		if (display)
			sock_send_string(sock, buffer);

		// Free memory (plus buffers and cache)
		sprintf_memory(tmp, (mem[0].free + mem[0].buffers + mem[0].cache) * 1024, 1);
		sprintf(buffer, "widget_set M memused 1 3 {%7s}\n", tmp);
		if (display)
			sock_send_string(sock, buffer);

		// Total swap
		sprintf_memory(tmp, mem[1].total * 1024, 1);
		sprintf(buffer, "widget_set M swaptotl %i 2 {%7s}\n", lcd_wid - 7, tmp);
		if (display)
			sock_send_string(sock, buffer);

		// Free swap
		sprintf_memory(tmp, mem[1].free * 1024, 1);
		sprintf(buffer, "widget_set M swapused %i 3 {%7s}\n", lcd_wid - 7, tmp);
		if (display)
			sock_send_string(sock, buffer);

		if (gauge_wid > 0) {
			// Free memory graph
			if (mem[0].total > 0) {
				double value = 1.0 - (double) (mem[0].free + mem[0].buffers + mem[0].cache)
	                	                         / (double) mem[0].total;

				// printf(".0f", val) only prints the integer part
				sprintf(buffer, "widget_set M memgauge 2 4 %.0f\n",
						lcd_cellwid * gauge_wid * value);
				if (display)
					sock_send_string(sock, buffer);
			}	

			// Free swap graph
			if (mem[1].total > 0) {
				double value = 1.0 - ((double) mem[1].free / (double) mem[1].total);

				// printf(".0f", val) only prints the integer part
				sprintf(buffer, "widget_set M swapgauge %i 4 %.0f\n", 
					lcd_wid - gauge_wid, lcd_cellwid * gauge_wid * value);
				if (display)
					sock_send_string(sock, buffer);
			}	
		}
	}
	else {
		// Total memory
		sprintf_memory(tmp, mem[0].total * 1024, 1);
		sprintf(buffer, "widget_set M memtotl 3 1 {%6s}\n", tmp);
		if (display)
			sock_send_string(sock, buffer);

		// Total swap
		sprintf_memory(tmp, mem[1].total * 1024, 1);
		sprintf(buffer, "widget_set M swaptotl 3 2 {%6s}\n", tmp);
		if (display)
			sock_send_string(sock, buffer);

		// Free memory graph
		strcpy(tmp, "N/A");
		if (mem[0].total > 0) {
			double value = 1.0 - (double) (mem[0].free + mem[0].buffers + mem[0].cache)
					 / (double) mem[0].total;

			if (gauge_wid > 0) {
				// printf(".0f", val) only prints the integer part
				sprintf(buffer, "widget_set M memgauge %i 1 %.0f\n",
					gauge_offs, lcd_cellwid * gauge_wid * value);
				if (display)
					sock_send_string(sock, buffer);
			}	

			sprintf_percent(tmp, value * 100);
		}
		sprintf(buffer, "widget_set M mem%% %i 1 {%5s}\n", lcd_wid - 5, tmp);
		if (display)
			sock_send_string(sock, buffer);

		// Free swap graph
		strcpy(tmp, "N/A");
		if (mem[1].total > 0) {
			double value = 1.0 - ((double) mem[1].free / (double) mem[1].total);

			if (gauge_wid > 0) {
				// printf(".0f", val) only prints the integer part
				sprintf(buffer, "widget_set M swapgauge %i 2 %.0f\n", 
					gauge_offs, lcd_cellwid * gauge_wid * value);
				if (display)
					sock_send_string(sock, buffer);
			}

			sprintf_percent(tmp, value * 100);
		}
		sprintf(buffer, "widget_set M swap%% %i 2 {%5s}\n", lcd_wid - 5, tmp);
		if (display)
			sock_send_string(sock, buffer);
	}

	return 0;
}						 // End mem_screen()


static int
sort_procs (void *a, void *b)
{
	procinfo_type *one, *two;

	if ((a == NULL) || (b == NULL))
		return 0;

	one = (procinfo_type *) a;
	two = (procinfo_type *) b;

	return (two->totl > one->totl);
}


/////////////////////////////////////////////////////////////////////////
// Mem Top Screen displays info about top 5 memory hogs...
//
// +--------------------+	+--------------------+
// |## TOP MEM: myhos #@|	|## TOP MEM: myhos #@|
// |1 110.4M mysqld     |	|1 110.4M mysqld     |
// |2 35.38M konqueror(2|	+--------------------+
// |3 29.21M XFree86    |
// +--------------------+
//
int
mem_top_screen (int rep, int display)
{
	static int first = 1;
	LinkedList *procs;
	int i;

	if (first) {
		first = 0;
		
		sock_send_string(sock, "screen_add S\n");
		sprintf(buffer, "screen_set S -name {Top Memory Use: %s}\n", get_hostname());
		sock_send_string(sock, buffer);
		sock_send_string(sock, "widget_add S title title\n");
		sprintf(buffer, "widget_set S title {TOP MEM: %s}\n", get_hostname());
		sock_send_string(sock, buffer);
		
		// frame from (2nd line, left) to (last line, right)
		sock_send_string(sock, "widget_add S f frame\n");
		sprintf(buffer, "widget_set S f 1 2 %i %i %i 5 v %i\n",
			lcd_wid, lcd_hgt, lcd_wid, ((lcd_hgt >= 4) ? 8 : 12));
		sock_send_string(sock, buffer);
		
		// frame contents
		for (i = 1; i <= 5; i++) {
			sprintf(buffer, "widget_add S %i string -in f\n", i);
			sock_send_string(sock, buffer);
		}
		sock_send_string(sock, "widget_set S 1 1 1 Checking...\n");
	}

	procs = LL_new();
	if (procs == NULL) {
		fprintf (stderr, "mem_top_screen: Error allocating list\n");
		return -1;
	}

	machine_get_procs(procs);

	// Now, print some info...
	LL_Rewind(procs);
	LL_Sort(procs, sort_procs);
	LL_Rewind(procs);
	for (i = 1; i <= 5; i++) {
		procinfo_type *p = LL_Get(procs);

		if (p != NULL) {
			char mem[10];

			sprintf_memory(mem, (double) p->totl * 1024, 1);

			//printf("Mem hog: %s: %s\n", p->name, mem);
			if (p->number > 1)
				sprintf(tmp, "%i %5s %s(%i)", i, mem, p->name, p->number);
			else
				sprintf(tmp, "%i %5s %s", i, mem, p->name);
			sprintf(buffer, "widget_set S %i 1 %i {%s}\n", i, i, tmp);
			if (display)
				sock_send_string(sock, buffer);
		} else {
			// printf("Mem hog: none?\n");
			// sprintf(buffer, "widget_set S %i 1 %i {}\n", i, i);
			sprintf(buffer, "widget_set S %i 1 %i { }\n", i, i);
			if (display)
				sock_send_string(sock, buffer);
		}

		LL_Next (procs);
	}

	// Now clean it all up...
	LL_Rewind(procs);
	do {
		procinfo_type *p = (procinfo_type *) LL_Get(procs);
		if (p != NULL) {
			//printf("Proc: %6ik %s\n", p->size, p->name);
			free(p);
		}
	} while (LL_Next(procs) == 0);
	LL_Destroy(procs);

	return 0;
}

