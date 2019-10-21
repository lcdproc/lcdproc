/** \file clients/lcdproc/mem.c
 * Implements the 'Memory' and 'ProcSize' screens.
 */

/*-
 * This file is part of lcdproc, the lcdproc client.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 */

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


/**
 * Mem Screen displays info about memory and swap usage...
 *
 *\verbatim
 *
 * +--------------------+	+--------------------+
 * |##  MEM #### SWAP #@|	|M 758.3M [- ] 35.3%@|
 * | 758.3M Totl 1.884G |	|S 1.884G [  ]  0.1% |
 * | 490.8M Free 1.882G |	+--------------------+
 * |E---    F  E       F|
 * +--------------------+
 *
 *\endverbatim
 *
 * \param rep        Time since last screen update
 * \param display    1 if screen is visible or data should be updated
 * \param flags_ptr  Mode flags
 * \return  Always 0
 */
int
mem_screen(int rep, int display, int *flags_ptr)
{
	const char *title_sep = "####################################################################################################";
	static int which_title = 0;
	static int gauge_wid = 0;
	static int gauge_offs = 0;
	static int title_sep_wid = 0;
	int label_wid, label_offs;
	meminfo_type mem[2];

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		sock_send_string(sock, "screen_add M\n");
		sock_printf(sock, "screen_set M -name {Memory & Swap: %s}\n", get_hostname());

		title_sep_wid = (lcd_wid >= 16) ? lcd_wid - 16 : 0;

		if (lcd_hgt >= 4) {
			gauge_wid = lcd_wid / 2; /* Room for 2 E...F pbars */
			if ((lcd_wid % 2) == 0 && gauge_wid >= 9)
				gauge_wid--; /* Put 2 spaces between the pbars */

			label_wid = (title_sep_wid >= 4) ? 4 : title_sep_wid;
			label_offs = (lcd_wid - label_wid) / 2 + 1;

			sock_send_string(sock, "widget_add M title title\n");
			sock_printf(sock, "widget_set M title { MEM %.*s SWAP}\n", title_sep_wid, title_sep);
			sock_send_string(sock, "widget_add M totl string\n");
			sock_send_string(sock, "widget_add M free string\n");
			sock_printf(sock, "widget_set M totl %i 2 %.*s\n", label_offs, label_wid, "Totl");
			sock_printf(sock, "widget_set M free %i 3 %.*s\n", label_offs, label_wid, "Free");
			sock_send_string(sock, "widget_add M memused string\n");
			sock_send_string(sock, "widget_add M swapused string\n");
		}
		else {
			if (lcd_wid >= 20) {
				/* We have room for spaces to separate the bars and the mem / % strings */
				gauge_wid = lcd_wid - 16;
				gauge_offs = 10;
			} else if (lcd_wid >= 17) {
				/* We can fit the bars of we leave out the spaces separating them from the strings */
				gauge_wid = lcd_wid - 14;
				gauge_offs = 9;
			} else {
				/* no space for the bars */
				gauge_wid = gauge_offs = 0;
			}

			sock_send_string(sock, "widget_add M m string\n");
			sock_send_string(sock, "widget_add M s string\n");
			sock_send_string(sock, "widget_set M m 1 1 {M}\n");
			sock_send_string(sock, "widget_set M s 1 2 {S}\n");
			sock_send_string(sock, "widget_add M mem% string\n");
			sock_send_string(sock, "widget_add M swap% string\n");
		}

		sock_send_string(sock, "widget_add M memtotl string\n");
		sock_send_string(sock, "widget_add M swaptotl string\n");

		pbar_widget_add("M", "memgauge");
		pbar_widget_add("M", "swapgauge");
	}

	if (lcd_hgt >= 4) {
		/* flip the title back and forth... (every 4 updates) */
		if (which_title & 4)
			sock_printf(sock, "widget_set M title {%s}\n", get_hostname());
		else
			sock_printf(sock, "widget_set M title { MEM %.*s SWAP}\n", title_sep_wid, title_sep);
		which_title = (which_title + 1) & 7;
	}

	if (!display)
		return 0;

	machine_get_meminfo(mem);

	if (lcd_hgt >= 4) {
		char tmp[12];	/* should be sufficient */

		/* Total memory */
		sprintf_memory(tmp, mem[0].total * 1024.0, 1);
		sock_printf(sock, "widget_set M memtotl 1 2 {%7s}\n", tmp);

		/* Free memory (plus buffers and cache) */
		sprintf_memory(tmp, (mem[0].free + mem[0].buffers + mem[0].cache) * 1024.0, 1);
		sock_printf(sock, "widget_set M memused 1 3 {%7s}\n", tmp);

		/* Total swap */
		sprintf_memory(tmp, mem[1].total * 1024.0, 1);
		sock_printf(sock, "widget_set M swaptotl %i 2 {%7s}\n", lcd_wid - 7, tmp);

		/* Free swap */
		sprintf_memory(tmp, mem[1].free * 1024.0, 1);
		sock_printf(sock, "widget_set M swapused %i 3 {%7s}\n", lcd_wid - 7, tmp);

		if (gauge_wid > 0) {
			/* Free memory graph */
			if (mem[0].total > 0) {
				double value = 1.0 - (double) (mem[0].free + mem[0].buffers + mem[0].cache)
					       / (double) mem[0].total;

				pbar_widget_set("M", "memgauge", 1, 4, gauge_wid, value * 1000, "E", "F");
			}

			/* Free swap graph */
			if (mem[1].total > 0) {
				double value = 1.0 - ((double) mem[1].free / (double) mem[1].total);

				pbar_widget_set("M", "swapgauge", 1 + lcd_wid - gauge_wid, 4, gauge_wid, value * 1000, "E", "F");
			}
		}
	}
	else {
		char tmp[12];	/* should be sufficient */

		/* Total memory */
		sprintf_memory(tmp, mem[0].total * 1024.0, 1);
		sock_printf(sock, "widget_set M memtotl 3 1 {%6s}\n", tmp);

		/* Total swap */
		sprintf_memory(tmp, mem[1].total * 1024.0, 1);
		sock_printf(sock, "widget_set M swaptotl 3 2 {%6s}\n", tmp);

		/* Free memory graph */
		strcpy(tmp, "N/A");
		if (mem[0].total > 0) {
			double value = 1.0 - (double) (mem[0].free + mem[0].buffers + mem[0].cache)
					 / (double) mem[0].total;

			if (gauge_wid > 0)
				pbar_widget_set("M", "memgauge", gauge_offs, 1, gauge_wid, value * 1000, NULL, NULL);

			sprintf_percent(tmp, value * 100);
		}
		sock_printf(sock, "widget_set M mem%% %i 1 {%5s}\n", lcd_wid - 5, tmp);

		/* Free swap graph */
		strcpy(tmp, "N/A");
		if (mem[1].total > 0) {
			double value = 1.0 - ((double) mem[1].free / (double) mem[1].total);

			if (gauge_wid > 0)
				pbar_widget_set("M", "swapgauge", gauge_offs, 2, gauge_wid, value * 1000, NULL, NULL);

			sprintf_percent(tmp, value * 100);
		}
		sock_printf(sock, "widget_set M swap%% %i 2 {%5s}\n", lcd_wid - 5, tmp);
	}

	return 0;
}


/**
 * Compares memory usage two procinfo structures and returns 1 (true) if the
 * second one's is larger than the first one's, 0 (false) otherwise.
 */
static int
sort_procs(void *a, void *b)
{
	procinfo_type *one, *two;

	if ((a == NULL) || (b == NULL))
		return 0;

	one = (procinfo_type *) a;
	two = (procinfo_type *) b;

	return (two->totl > one->totl);
}


/**
 * Mem Top Screen displays info about top memory hogs...
 *
 *\verbatim
 *
 * +--------------------+	+--------------------+
 * |## TOP MEM: myhos #@|	|## TOP MEM: myhos #@|
 * |1 110.4M mysqld     |	|1 110.4M mysqld     |
 * |2 35.38M konqueror(2|	+--------------------+
 * |3 29.21M XFree86    |
 * +--------------------+
 *
 *\endverbatim
 *
 * \param rep        Time since last screen update
 * \param display    1 if screen is visible or data should be updated
 * \param flags_ptr  Mode flags
 * \return  Always 0
 */
int
mem_top_screen(int rep, int display, int *flags_ptr)
{
	LinkedList *procs;
	int lines;
	int i;

	/* On screen <= 4 lines show info for 5 processes and use scrolling */
	if (lcd_hgt <= 4)
		lines = 5;
	/*
	 * On 5 lines displays we have 4 free lines, using 5 lines with
	 * scrolling here makes things look like we scroll 1 line down and
	 * then 1 line back up again, which looks wrong. So we use as many
	 * lines as fit (4) instead of 5 on a 5 lines display, so that we do
	 * not get the weird scrolling.
	 *
	 * Likewise for > 5 lines we also want to show as many lines as fit
	 * so that we use the entire screen.
	 */
	else
		lines = lcd_hgt - 1;

	if ((*flags_ptr & INITIALIZED) == 0) {
		*flags_ptr |= INITIALIZED;

		sock_send_string(sock, "screen_add S\n");
		sock_printf(sock, "screen_set S -name {Top Memory Use: %s}\n", get_hostname());
		sock_send_string(sock, "widget_add S title title\n");
		sock_printf(sock, "widget_set S title {TOP MEM: %s}\n", get_hostname());

		/* frame from (2nd line, left) to (last line, right) */
		sock_send_string(sock, "widget_add S f frame\n");

		/* scroll rate: 1 line every X ticks (= 1/8 sec) */
		sock_printf(sock, "widget_set S f 1 2 %i %i %i %i v %i\n",
			    lcd_wid, lcd_hgt, lcd_wid, lines,
			    ((lcd_hgt >= 4) ? 8 : 12));

		/* frame contents */
		for (i = 1; i <= lines; i++) {
			sock_printf(sock, "widget_add S %i string -in f\n", i);
		}
		sock_send_string(sock, "widget_set S 1 1 1 Checking...\n");
	}

	if (!display)
		return 0;

	/* Create a new process list */
	procs = LL_new();
	if (procs == NULL) {
		fprintf(stderr, "mem_top_screen: Error allocating list\n");
		return -1;
	}

	machine_get_procs(procs);
	/*
	 * Ignore if machine_get_procs returns an errror. The list will be
	 * empty then and all process info will be shown empty, too.
	 */

	/* Now, print some info... */
	LL_Rewind(procs);
	LL_Sort(procs, sort_procs);
	LL_Rewind(procs);
	for (i = 1; i <= lines; i++) {
		procinfo_type *p = LL_Get(procs);

		if (p != NULL) {
			char mem[10];

			sprintf_memory(mem, (double) p->totl * 1024.0, 1);

			if (p->number > 1)
				sock_printf(sock, "widget_set S %i 1 %i {%i %5s %s(%i)}\n",
					    i, i, i, mem, p->name, p->number);
			else
				sock_printf(sock, "widget_set S %i 1 %i {%i %5s %s}\n",
					    i, i, i, mem, p->name);
		}
		else {
			sock_printf(sock, "widget_set S %i 1 %i { }\n", i, i);
		}

		LL_Next(procs);
	}

	/* Delete the process list */
	LL_Rewind(procs);
	do {
		procinfo_type *p = (procinfo_type *) LL_Get(procs);
		if (p != NULL) {
			free(p);
		}
	} while (LL_Next(procs) == 0);
	LL_Destroy(procs);

	return 0;
}

