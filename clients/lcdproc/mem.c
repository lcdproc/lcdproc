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

/////////////////////////////////////////////////////////////////////////
// Mem Screen displays info about memory and swap usage...
//
int
mem_screen (int rep, int display)
{
	int n;
	meminfo_type mem[2];
	static int first = 1;
	static int which_title = 0;
	float value;

	if (first) {
		first = 0;

		sock_send_string (sock, "screen_add M\n");
		sprintf (buffer, "screen_set M -name {Memory & Swap: %s}\n", get_hostname());
		sock_send_string (sock, buffer);

		if (lcd_hgt >= 4) {
			sock_send_string (sock, "widget_add M title title\n");
			sock_send_string (sock, "widget_set M title { MEM -==- SWAP}\n");
			sock_send_string (sock, "widget_add M totl string\n");
			sock_send_string (sock, "widget_add M used string\n");
			sock_send_string (sock, "widget_set M totl 9 2 Totl\n");
			sock_send_string (sock, "widget_set M used 9 3 Free\n");
			sock_send_string (sock, "widget_add M EF string\n");
			sock_send_string (sock, "widget_set M EF 1 4 {E       F  E       F}\n");
			sock_send_string (sock, "widget_add M memused string\n");
			sock_send_string (sock, "widget_add M swapused string\n");
			//sock_send_string(sock, "widget_set M memgauge 2 4 0\n");
			//sock_send_string(sock, "widget_set M swapgauge 13 4 0\n");
		} else {
			sock_send_string (sock, "widget_add M m string\n");
			sock_send_string (sock, "widget_add M s string\n");
			if (lcd_wid >= 20) {
				sock_send_string (sock, "widget_set M m 1 1 {M     [       ]}\n");
				sock_send_string (sock, "widget_set M s 1 2 {S     [       ]}\n");
			} else {
				sock_send_string (sock, "widget_set M m 1 1 {M     [   ]}\n");
				sock_send_string (sock, "widget_set M s 1 2 {S     [   ]}\n");
			}
			sock_send_string (sock, "widget_add M mem% string\n");
			sock_send_string (sock, "widget_add M swap% string\n");
			sock_send_string (sock, "widget_set M mem% 16 1 { 0.0%}\n");
			sock_send_string (sock, "widget_set M swap% 16 2 { 0.0%}\n");

			//sock_send_string(sock, "widget_set M memgauge 8 1 0\n");
			//sock_send_string(sock, "widget_set M swapgauge 8 2 0\n");
		}

		sock_send_string (sock, "widget_add M memtotl string\n");
		sock_send_string (sock, "widget_add M swaptotl string\n");

		sock_send_string (sock, "widget_add M memgauge hbar\n");
		sock_send_string (sock, "widget_add M swapgauge hbar\n");

		//sock_send_string(sock, "\n");
	}

	machine_get_meminfo(mem);

	// flip the title back and forth...
	if (lcd_hgt >= 4) {
		if (which_title & 4) {
			sprintf (buffer, "widget_set M title {%s}\n", get_hostname());
			sock_send_string (sock, buffer);
		} else
			sock_send_string (sock, "widget_set M title { MEM -==- SWAP}\n");
		which_title = (which_title + 1) & 7;

		// Total memory
		sprintf (tmp, "widget_set M memtotl 1 2 {%6dk}\n", mem[0].total);
		if (display)
			sock_send_string (sock, tmp);

		// Free memory (plus buffers and cache)
		sprintf (tmp, "widget_set M memused 1 3 {%6dk}\n", mem[0].free + mem[0].buffers + mem[0].cache);
		if (display)
			sock_send_string (sock, tmp);

		// Total swap
		sprintf (tmp, "widget_set M swaptotl 14 2 {%6dk}\n", mem[1].total);
		if (display)
			sock_send_string (sock, tmp);

		// Free swap
		sprintf (tmp, "widget_set M swapused 14 3 {%6dk}\n", mem[1].free);
		if (display)
			sock_send_string (sock, tmp);

		// Free memory graph
		n = (int) (35.0 - ((float) mem[0].free + (float) mem[0].buffers + (float) mem[0].cache)
					  / (float) mem[0].total * 35.0);
		sprintf (tmp, "widget_set M memgauge 2 4 %i\n", n);
		if (display)
			sock_send_string (sock, tmp);

		// Free swap graph
		n = (int) (35.0 - (float) mem[1].free / (float) mem[1].total * 35.0);
		sprintf (tmp, "widget_set M swapgauge 13 4 %i\n", n);
		if (display)
			sock_send_string (sock, tmp);
	} else {
		// Total memory
		sprintf (tmp, "widget_set M memtotl 2 1 {%4dM}\n", (int) (mem[0].total / 1024.0));
		if (display)
			sock_send_string (sock, tmp);

		// Total swap
		sprintf (tmp, "widget_set M swaptotl 2 2 {%4dM}\n", (int) (mem[1].total / 1024.0));
		if (display)
			sock_send_string (sock, tmp);

		// Free memory graph
		value = ((float) mem[0].free + (float) mem[0].buffers + (float) mem[0].cache)
			 / (float) mem[0].total;
		value = 1.0 - value;
		n = (int) ((lcd_cellwid * (float) (lcd_wid - 13)) * (value));
		sprintf (tmp, "widget_set M memgauge 8 1 %i\n", n);
		if (display)
			sock_send_string (sock, tmp);

		value *= 100.0;
		if (value >= 99.9) {
			sprintf (buffer, "100%%");
		} else {
			sprintf (buffer, "%4.1f%%", value);
		}
		sprintf (tmp, "widget_set M mem%% %i 1 {%s}\n", lcd_wid - 4, buffer);
		if (display)
			sock_send_string (sock, tmp);

		// Free swap graph
		if (mem[1].total == 0) {
			value = 0;
			sprintf(tmp, "widget_set M swapgauge 8 2 0\n");
		} else {
			value = ((float) mem[1].free / (float) mem[1].total);
			value = 1.0 - value;
			n = (int) ((lcd_cellwid * (float) (lcd_wid - 13)) * (value));
			sprintf(tmp, "widget_set M swapgauge 8 2 %i\n", n);
		}

		if (display)
			sock_send_string (sock, tmp);

		value *= 100.0;
		if (value >= 99.9) {
			sprintf (buffer, "100%%");
		} else {
			if (mem[1].total == 0)
				sprintf (buffer, " n/a");
			else
				sprintf (buffer, "%4.1f%%", value);
		}

		sprintf (tmp, "widget_set M swap%% %i 2 {%s}\n", lcd_wid - 4, buffer);
		if (display)
			sock_send_string (sock, tmp);

	}

	return 0;
}										  // End mem_screen()

static int
sort_procs (void *a, void *b)
{
	procinfo_type *one, *two;

	if (!a)
		return 0;
	if (!b)
		return 0;

	one = (procinfo_type *) a;
	two = (procinfo_type *) b;

	return (two->totl > one->totl);
}

int
mem_top_screen (int rep, int display)
{
	int i;
	procinfo_type *p;
	LinkedList *procs;
	static int first = 1;

	if (first) {
		first = 0;
		sock_send_string (sock, "screen_add S\n");
		sprintf (buffer, "screen_set S -name {Top Memory Use: %s}\n", get_hostname());
		sock_send_string (sock, buffer);
		sock_send_string (sock, "widget_add S title title\n");
		sprintf (buffer, "widget_set S title {TOP MEM: %s}\n", get_hostname());
		sock_send_string (sock, buffer);
		sock_send_string (sock, "widget_add S f frame\n");
		if (lcd_hgt >= 4)
			sock_send_string (sock, "widget_set S f 1 2 20 4 20 5 v 8\n");
		else
			sock_send_string (sock, "widget_set S f 1 2 20 2 20 5 v 16\n");
		for (i = 1; i <= 5; i++) {
			sprintf (buffer, "widget_add S %i string -in f\n", i);
			sock_send_string (sock, buffer);
		}
		sock_send_string (sock, "widget_set S 1 1 1 Checking...\n");
	}

	procs = LL_new ();
	if (!procs) {
		fprintf (stderr, "mem_top_screen: Error allocating list\n");
		return -1;
	}

	machine_get_procs(procs);

	// Now, print some info...
	LL_Rewind (procs);
	LL_Sort (procs, sort_procs);
	LL_Rewind (procs);
	for (i = 1; i <= 5; i++) {
		p = LL_Get (procs);
		if (p) {
			//printf("Mem hog: %s: %ik\n", p->name, p->size);
			if (p->number > 1)
				sprintf (buffer, "widget_set S %i 1 %i {%i%6ik %s(%i)}\n", i, i, i, p->totl, p->name, p->number);
			else
				sprintf (buffer, "widget_set S %i 1 %i {%i%6ik %s}\n", i, i, i, p->totl, p->name);
			if (display)
				sock_send_string (sock, buffer);
		} else {
			//printf("Mem hog: none?\n");
			//sprintf (buffer, "widget_set S %i 1 %i {}\n", i, i);
			sprintf (buffer, "widget_set S %i 1 %i \" \"\n", i, i);
			if (display)
				sock_send_string (sock, buffer);
		}

		LL_Next (procs);
	}

	// Now clean it all up...
	LL_Rewind (procs);
	do {
		p = (procinfo_type *) LL_Get (procs);
		if (p) {
			//printf("Proc: %6ik %s\n", p->size, p->name);
			free (p);
		}
	} while (LL_Next (procs) == 0);
	LL_Destroy (procs);

	return 0;

}

