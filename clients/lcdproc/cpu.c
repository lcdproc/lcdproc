#include <sys/types.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include "shared/sockets.h"
#include "shared/debug.h"

#include "main.h"
#include "mode.h"
#include "machine.h"
#include "cpu.h"


//////////////////////////////////////////////////////////////////////////
// CPU screen shows info about percentage of the CPU being used
//
int
cpu_screen(int rep, int display)
{
#undef CPU_BUF_SIZE
#define CPU_BUF_SIZE 4
	static int first = TRUE;
	static float cpu[CPU_BUF_SIZE + 1][5];	// last buffer is scratch

	int i, j, n;
	float value;
	load_type load;

	if(first)
	{
		first = FALSE;

		sock_send_string(sock, "screen_add C\n");
		sprintf(buffer, "screen_set C -name {CPU Use: %s}\n", get_hostname());
		sock_send_string(sock, buffer);
		if(lcd_hgt >= 4)
		{
			sock_send_string(sock, "widget_add C title title\n");
			sock_send_string(sock, "widget_set C title {CPU LOAD}\n");
			sock_send_string(sock, "widget_add C one string\n");
			sock_send_string(sock, "widget_set C one 1 2 {0        1 Sys  0.0%}\n");
			sock_send_string(sock, "widget_add C two string\n");
			sock_send_string(sock, "widget_add C three string\n");
			sock_send_string(sock, "widget_set C one 1 2 {Usr  0.0% Nice  0.0%}\n");
			sock_send_string(sock, "widget_set C two 1 3 {Sys  0.0% Idle  0.0%}\n");
			sock_send_string(sock, "widget_set C three 1 4 {0%              100%}\n");
			sock_send_string(sock, "widget_add C usr string\n");
			sock_send_string(sock, "widget_add C nice string\n");
			sock_send_string(sock, "widget_add C idle string\n");
			sock_send_string(sock, "widget_add C sys string\n");
			sock_send_string(sock, "widget_add C bar hbar\n");
			sock_send_string(sock, "widget_set C bar 3 4 0\n");
		}
		else
		{
			sock_send_string(sock, "widget_add C cpu0 string\n");
			if(lcd_wid >= 20)
				sock_send_string(sock, "widget_set C cpu0 1 1 {CPU0[         ]}\n");
			else
				sock_send_string(sock, "widget_set C cpu0 1 1 {CPU0[     ]}\n");
			sock_send_string(sock, "widget_add C cpu0% string\n");
			sprintf(buffer, "widget_set C cpu0%% 1 %d { 0.0%%}\n", lcd_wid - 4);
			sock_send_string(sock, buffer);
			sock_send_string(sock, "widget_add C usni string\n");
			sock_send_string(sock, "widget_set C usni 1 2 {    U    S    N    I}\n");
			sock_send_string(sock, "widget_add C usr hbar\n");
			sock_send_string(sock, "widget_add C sys hbar\n");
			sock_send_string(sock, "widget_add C nice hbar\n");
			sock_send_string(sock, "widget_add C idle hbar\n");
			sock_send_string(sock, "widget_add C total hbar\n");
			sock_send_string(sock, "widget_set C total 6 1 0\n");
		}

		return(0);
	}

	machine_get_load(&load);

	// Shift values over by one
	for(i = 0; i < (CPU_BUF_SIZE - 1); i++)
		for(j = 0; j < 5; j++)
			cpu[i][j] = cpu[i + 1][j];

	// Read new data
	cpu[CPU_BUF_SIZE - 1][0] = ((float) load.user / (float) load.total) * 100.0;
	cpu[CPU_BUF_SIZE - 1][1] = ((float) load.system / (float) load.total) * 100.0;
	cpu[CPU_BUF_SIZE - 1][2] = ((float) load.nice / (float) load.total) * 100.0;
	cpu[CPU_BUF_SIZE - 1][3] = ((float) load.idle / (float) load.total) * 100.0;
	cpu[CPU_BUF_SIZE - 1][4] = (((float) load.user + (float) load.system + (float) load.nice) / (float) load.total) * 100.0;

	/*
	// Only clear on first display...
	if(!rep){
		   // Make all the same, if this is the first time...
		   for(i=0; i<CPU_BUF_SIZE-1; i++)
		   for(j=0; j<5; j++)
		   cpu[i][j] = cpu[CPU_BUF_SIZE-1][j];
	}
	*/

	// Average values for final result
	for(i = 0; i < 5; i++)
	{
		value = 0;
		for(j = 0; j < CPU_BUF_SIZE; j++)
			value += cpu[j][i];
		value /= CPU_BUF_SIZE;
		cpu[CPU_BUF_SIZE][i] = value;
	}

	if(!display)
		return(0);

	if(lcd_hgt >= 4)
	{
		value = cpu[CPU_BUF_SIZE][4];
		if(value >= 99.9)
			sprintf(tmp, "100%%");
		else
			sprintf(tmp, "%4.1f%%", value);
		sprintf(buffer, "widget_set C title {CPU %s: %s}\n", tmp, get_hostname());
		sock_send_string(sock, buffer);

		value = cpu[CPU_BUF_SIZE][0];
		if(value >= 99.9)
			sprintf(tmp, " 100%%");
		else
			sprintf(tmp, "%4.1f%%", value);
		sprintf(buffer, "widget_set C usr 5 2 {%s}\n", tmp);
		sock_send_string(sock, buffer);

		value = cpu[CPU_BUF_SIZE][1];
		if(value >= 99.9)
			sprintf(tmp, " 100%%");
		else
			sprintf(tmp, "%4.1f%%", value);
		sprintf(buffer, "widget_set C sys 5 3 {%s}\n", tmp);
		sock_send_string(sock, buffer);

		value = cpu[CPU_BUF_SIZE][2];
		if(value >= 99.9)
			sprintf(tmp, " 100%%");
		else
			sprintf(tmp, "%4.1f%%", value);
		sprintf(buffer, "widget_set C nice 16 2 {%s}\n", tmp);
		sock_send_string(sock, buffer);

		value = cpu[CPU_BUF_SIZE][3];
		if(value >= 99.9)
			sprintf(tmp, " 100%%");
		else
			sprintf(tmp, "%4.1f%%", value);
		sprintf(buffer, "widget_set C idle 16 3 {%s}\n", tmp);
		sock_send_string(sock, buffer);

		value = cpu[CPU_BUF_SIZE][4];
		n = (int)(value * (lcd_cellwid * 14) / 100.0);
		sprintf(buffer, "widget_set C bar 3 4 %d\n", n);
		sock_send_string(sock, buffer);
	}
	else  // 20x2 version
	{
		value = cpu[CPU_BUF_SIZE][4];
		if(value >= 99.9)
			sprintf(tmp, "100%%");
		else
			sprintf(tmp, "%4.1f%%", value);
		sprintf(buffer, "widget_set C cpu0%% %d 1 {%s}\n", lcd_wid - 4, tmp);
		sock_send_string(sock, buffer);

		n = (float)(value * lcd_cellwid * (float) (lcd_wid - 11)) / 100.0;
		sprintf(buffer, "widget_set C total 6 1 %d\n", n);
		sock_send_string(sock, buffer);

		value = cpu[CPU_BUF_SIZE][0];
		n = (float)(value * lcd_cellwid * 4.0) / 100.0;
		sprintf(buffer, "widget_set C usr 1 2 %d\n", n);
		sock_send_string(sock, buffer);

		value = cpu[CPU_BUF_SIZE][1];
		n = (float)(value * lcd_cellwid * 3.0) / 100.0;
		sprintf(buffer, "widget_set C sys 7 2 %d\n", n);
		sock_send_string(sock, buffer);

		value = cpu[CPU_BUF_SIZE][2];
		n = (float)(value * lcd_cellwid * 3.0) / 100.0;
		sprintf(buffer, "widget_set C nice 12 2 %d\n", n);
		sock_send_string(sock, buffer);

		value = cpu[CPU_BUF_SIZE][3];
		n = (float)(value * lcd_cellwid * 3.0) / 100.0;
		sprintf(buffer, "widget_set C idle 17 2 %d\n", n);
		sock_send_string(sock, buffer);
	}

	return(0);
}										  // End cpu_screen()

//////////////////////////////////////////////////////////////////////////
// Cpu Graph Screen shows a quick-moving histogram of CPU use.
//
int
cpu_graph_screen(int rep, int display)
{
#undef CPU_BUF_SIZE
#define CPU_BUF_SIZE 2
	static int first = TRUE;
	static float cpu[CPU_BUF_SIZE];
	static int cpu_past[LCD_MAX_WIDTH];

	int i, j, n = 0;
	float value ;
	load_type load;

	if(first)
	{
		first = FALSE;

		sock_send_string(sock, "screen_add G\n");
		sprintf(buffer, "screen_set G -name {CPU Graph: %s}\n", get_hostname());
		sock_send_string(sock, buffer);
		if(lcd_hgt >= 4)
		{
			sock_send_string(sock, "widget_add G title title\n");
			sprintf(buffer, "widget_set G title {CPU: %s}\n", get_hostname());
		}
		else
		{
			sock_send_string(sock, "widget_add G title string\n");
			sprintf(buffer, "widget_set G title 1 1 {CPU: %s}\n", get_hostname());
		}
		sock_send_string(sock, buffer);

		for(i = 1; i <= lcd_wid; i++)
		{
			sprintf(buffer, "widget_add G %d vbar\n", i);
			sock_send_string(sock, buffer);
			sprintf(buffer, "widget_set G %d %d %d 0\n", i, i, lcd_hgt);
			sock_send_string(sock, buffer);
			cpu_past[i-1] = 0;
		};

		// Clear out CPU averaging array
		for(i = 0; i < CPU_BUF_SIZE ; i++)
			cpu[i] = 0.;
	}

	// Shift values over by one
	for(i = 0; i < (CPU_BUF_SIZE-1); i++)
		cpu[i] = cpu[i+1];

	// Read and save new data
	machine_get_load(&load);
	cpu[CPU_BUF_SIZE-1] = ((float) load.user + (float) load.system + (float) load.nice) / (float) load.total;

	// Average values for final result
	value = 0;
	for(j = 0; j < CPU_BUF_SIZE; j++)
		value += cpu[j];
	value /= (float) CPU_BUF_SIZE;

	// Scale result to available height
	if(lcd_hgt > 2)
		n = (int)(value * (float)(lcd_cellhgt) * (float)(lcd_hgt - 1));
	else
		n = (int)(value * (float)(lcd_cellhgt) * (float)(lcd_hgt));

	// Shift and update display the graph
	for(i = 0; i < lcd_wid - 1; i++)
	{
		cpu_past[i] = cpu_past[i + 1];

		if(display)
		{
			sprintf(buffer, "widget_set G %d %d %d %d\n",
			              i + 1, i + 1, lcd_hgt, cpu_past[i]);
			sock_send_string(sock, buffer);
		}
	}

	// Save the newest entry and display it
	cpu_past[lcd_wid - 1] = n;
	if(display)
	{
		sprintf(buffer, "widget_set G %d %d %d %d\n", lcd_wid, lcd_wid, lcd_hgt, n);
		sock_send_string(sock, buffer);
	}

	return(0);
}  // End cpu_graph_screen()

