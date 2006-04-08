/* 	This is the bignumber-library used by the serialVFD driver.

	Copyright (C) 2006 Stefan Herdler

	Based on drivers.c and lcd-lib.c.
	It may contain parts of other drivers of this package too.

	2006-01-26 Version 0.1: everything should work (not all hardware tested!)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307



	Requirements to the calling driver / display:
	cellwidth == 5
	cellheight == 7 or 8 (tested only on 7)
	needed functions:
		drvthis->set_char (only if the display has customcharacters)
		drvthis->chr
	customcharacters (if available) at char 0, 1, 2, ..... ,n

	USAGE:

	Just call lib_adv_bignum(drvthis, x, num, height, do_init, customchars)
	from drvthis->num.
	The library does everything needed to show bignumbers EXCEPT checking or
	setting the CCMODE variable. The driver has to do this (see serialVFD.c
	for details).

	Example: Call of the lib_adv_bignum from a drivers_num function :

	MODULE_EXPORT void
	serialVFD_num( Driver * drvthis, int x, int num )
	{
		int do_init=0;
		PrivateData *p = drvthis->private_data;


		if (p->ccmode != CCMODE_BIGNUM) // Are the customcharacters set up correctly? If not:
		{
			do_init=1;	// Lib_adv_bignum has to set the customcharacters.
			p->ccmode = CCMODE_BIGNUM; // Switch customcharactermode to bignum.
		}
		// Lib_adv_bignum does everything needed to show the bignumbers.
		lib_adv_bignum(drvthis, x, num, p->height, do_init, p->customchars);
	}

	To enable the bignumbers on 2-line displays you have to edit "/clients/lcdproc/chrono.c"
	before compiling. In function "big_clock_screen" you have to replace the "4" by a "2" in
	this line: "if (lcd_hgt < 4)".


*/

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "adv_bignum.h"

#include "report.h"
#include "lcd_lib.h"





static void adv_bignum_num_2_0 (Driver *drvthis, int x, int num, int height, int do_init);
static void adv_bignum_num_2_1 (Driver *drvthis, int x, int num, int height, int do_init);
static void adv_bignum_num_2_2 (Driver *drvthis, int x, int num, int height, int do_init);
static void adv_bignum_num_2_5 (Driver *drvthis, int x, int num, int height, int do_init);
static void adv_bignum_num_2_28 (Driver *drvthis, int x, int num, int height, int do_init);

static void adv_bignum_num_4_0 (Driver *drvthis, int x, int num, int height, int do_init);
static void adv_bignum_num_4_3 (Driver *drvthis, int x, int num, int height, int do_init);

static void adv_bignum_write_num (Driver *drvthis, char write_num_map[][4][4], int x, int num, int height);


/////////////////////////////////////////////////////////////////////////////////////////////
// This function determines the best possible bignumbertype for the display.
// The called bignumberfunction is depending on the display's height and the
// number of customcharacters.
void
lib_adv_bignum(Driver *drvthis, int x, int num, int height, int do_init, int customchars)
{

	switch (height) { // Display heigth:
		case 2: // 2 lines
		case 3: // are 3 line displays really existing?
			if (customchars == 0){// 2 lines and customchars = 0
				adv_bignum_num_2_0 (drvthis, x, num, height, do_init);
				}
			else if (customchars ==1 ){// 2 lines and customchars = 1
				adv_bignum_num_2_1 (drvthis, x, num, height, do_init);
				}
			else if (customchars < 5){// 2 lines and customchars = 2 ... 4
				adv_bignum_num_2_2 (drvthis, x, num, height, do_init);
				}
			else if (customchars < 28){ // 2 lines and customchars = 5 ... 27
				adv_bignum_num_2_5 (drvthis, x, num, height, do_init);
				}
			else { // 2 lines and customchars >= 28
				adv_bignum_num_2_28 (drvthis, x, num, height, do_init);
				}
			break;
		case 4: // 4 lines
			if (customchars < 3){// 4 lines and customchars < 3
				adv_bignum_num_4_0 (drvthis, x, num, height, do_init);
				}
			else { // 4 lines and customchars >= 3
				adv_bignum_num_4_3 (drvthis, x, num, height, do_init);
				}
			break;
		default:
			return;

	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// This function writes the selected type of bignumber by calling the drivers_char function.
// It is called by displaydepending bignumber function to write the numbers.
static void adv_bignum_write_num (Driver *drvthis, char write_num_map[][4][4], int x, int num, int height)
{
	int y, dx;
	for (y = 0; y < height; y++)
	{
		if (num == 10) // ":" is only 1 character wide.
				drvthis->chr (drvthis, x, y+1, write_num_map[num][y][0]);
		else
			for (dx = 0; dx <3; dx++)
				drvthis->chr (drvthis, x + dx, y+1, write_num_map[num][y][dx]);
	}


}

//
//
//
//	Display dependent functions.
//
//
//

/////////////////////////////////////////////////////////////////////////////////////////////
// This function is called for a 2 line display without custom characters.
// (pretty ugly looking)
static void adv_bignum_num_2_0 (Driver *drvthis, int x, int num, int height, int do_init)
{
	static char num_map[][4][4] = {
		{ /* 0 */
			" ||",
			" ||",
			"   ",
			"   " },
		{ /* 1 */
			"  |",
			"  |",
			"   ",
			"   " },
		{ /* 2 */
			"  ]",
			" [ ",
			"   ",
			"   " },
		{ /* 3 */
			"  ]",
			"  ]",
			"   ",
			"   " },
		{ /* 4 */
			" L|",
			"  |",
			"   ",
			"   " },
		{ /* 5 */
			" [ ",
			"  ]",
			"   ",
			"   " },
		{ /* 6 */
			" [ ",
			" []",
			"   ",
			"   " },
		{ /* 7 */
			"  7",
			"  |",
			"   ",
			"   " },
		{ /* 8 */
			" []",
			" []",
			"   ",
			"   " },
		{ /* 9 */
			" []",
			"  ]",
			"   ",
			"   " },
		{ /* colon */
			".",
			".",
			" ",
			" " }
		}; // Defines the character placing inside the bignumber.

		adv_bignum_write_num (drvthis, num_map, x, num, height); // write the number

}


/////////////////////////////////////////////////////////////////////////////////////////////
// This function is usable for a 2 line display with 1 or more custom characters.
// (not a beauty, but useable)
static void adv_bignum_num_2_1 (Driver *drvthis, int x, int num, int height, int do_init)
{
	static char num_map[][4][4] = {
		{ /* 0 */
			{'|' , 0 , '|',0},
			{"|_|"},
			{"   "},
			{"   "} },
		{ /* 1 */
			{"  |"},
			{"  |"},
			{"   "},
			{"   "} },
		{ /* 2 */
			{' ',0,']',0},
			{" [_"},
			{"   "},
			{"   "} },
		{ /* 3 */
			{' ',0,']',0},
			{" _]"},
			{"   "},
			{"   "} },
		{ /* 4 */
			{" L|"},
			{"  |"},
			{"   "},
			{"   "} },
		{ /* 5 */
			{' ','[',0,0},
			{" _]"},
			{"   "},
			{"   " }},
		{ /* 6 */
			{' ','[',0,0},
			{" []"},
			{"   "},
			{"   "} },
		{ /* 7 */
			{' ',0,'|',0},
			{"  |"},
			{"   "},
			{"   "} },
		{ /* 8 */
			{" []"},
			{" []"},
			{"   "},
			{"   "} },
		{ /* 9 */
			{" []"},
			{" _]"},
			{"   "},
			{"   "} },
		{ /* colon */
			{"."},
			{"."},
			{" "},
			{" "} }
		}; // Defines the character placing inside the bignumber.
	if (do_init) // Set customcharacters if needed.
		{
		static char bignum[1][5 * 8] = { // stored customcharacter
		{
			1, 1, 1, 1, 1,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
		}};
		drvthis->set_char (drvthis, 0, &bignum[0][0]); // put the customcharacter into the display
		}
	adv_bignum_write_num (drvthis, num_map, x, num, height); // write the number

}


/////////////////////////////////////////////////////////////////////////////////////////////
// This function is usable for a 2 line display with 2 or more custom characters.
// (o.k.)
static void adv_bignum_num_2_2 (Driver *drvthis, int x, int num, int height, int do_init)
{
	static char num_map[][4][4] ={
		{ /* 0 */
			{'|',0,'|'},
			"|_|",
			"   ",
			"   " },
		{ /* 1 */
			"  |",
			"  |",
			"   ",
			"   " },
		{ /* 2 */
			{' ',1,'|'},
			"|_ ",
			"   ",
			"   " },
		{ /* 3 */
			{' ',1,'|'},
			" _|",
			"   ",
			"   " },
		{ /* 4 */
			"|_|",
			"  |",
			"   ",
			"   " },
		{ /* 5 */
			{'|',1,' '},
			" _|",
			"   ",
			"   " },
		{ /* 6 */
			{'|',0,' '},
			{'|',1,'|'},
			"   ",
			"   " },
		{ /* 7 */
			{' ',0,'|'},
			"  |",
			"   ",
			"   " },
		{ /* 8 */
			{'|',1,'|'},
			"|_|",
			"   ",
			"   " },
		{ /* 9 */
			{'|',1,'|'},
			" _|",
			"   ",
			"   " },
		{ /* colon */
			".",
			".",
			" ",
			" " }
		}; // Defines the character placing inside the bignumber.
	if (do_init) // Set customcharacters if needed.
		{
		static char bignum[2][5 * 8] = { // stored customcharacters
		{
			1, 1, 1, 1, 1,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
		},
			{
			1, 1, 1, 1, 1,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			1, 1, 1, 1, 1,
			1, 1, 1, 1, 1,
		}};
		int i;
		for (i = 0; i <= 2 ; i++) { // put the customcharacters into the display
			drvthis->set_char (drvthis, i, &bignum[i][0]);
			}
		}
	adv_bignum_write_num (drvthis, num_map, x, num, height); // write the number
}


/////////////////////////////////////////////////////////////////////////////////////////////
// This function is usable for a 2 line display with 5 or more custom characters.
// (nice bignumbers)
static void adv_bignum_num_2_5 (Driver *drvthis, int x, int num, int height, int do_init)
{
	static char num_map[][4][4] =
		{{{3	,0	,2}, /*0*/
		{3	,1	,2},
		{"   "},
		{"   "}},
		{{' '	,' ',    2},/*1*/
		{' '	,' ',    2},
		{"   "},
		{"   "}},
		{{' '	,4	,2},/*2*/
		{3	,1	,' '},
		{"   "},
		{"   "}},
		{{' '	,4	,2},/*3*/
		{' '	,1	,2},
		{"   "},
		{"   "}},
		{{3	,1	,2},/*4*/
		{' '	,' '	,2},
		{"   "},
		{"   "}},
		{{3	,4	,' '},/*5*/
		{' '	,1	,2},
		{"   "},
		{"   "}},
		{{3	,0	,' '},/*6*/
		{3	,4	,2},
		{"   "},
		{"   "}},
		{{' '	,0	,2},/*7*/
		{' '	,' '	,2},
		{"   "},
		{"   "}},
		{{3	,4	,2},/*8*/
		{3	,1	,2},
		{"   "},
		{"   "}},
		{{3	,4	,2},/*9*/
		{' '	,1	,2},
		{"   "},
		{"   "}},
		{{'.'},/*:*/
		{'.'},
		{"   "},
		{"   "}}}; // Defines the character placing inside the bignumber.
	if (do_init) // Set customcharacters if needed.
		{
		static char bignum[5][5 * 8] = { // stored customcharacters
		{
			1, 1, 1, 1, 1,
			1, 1, 1, 1, 1,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
		},
			{
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			1, 1, 1, 1, 1,
			1, 1, 1, 1, 1,
			1, 1, 1, 1, 1,
		},
			{
			1, 1, 1, 0, 0,
			1, 1, 1, 0, 0,
			1, 1, 1, 0, 0,
			1, 1, 1, 0, 0,
			1, 1, 1, 0, 0,
			1, 1, 1, 0, 0,
			1, 1, 1, 0, 0,
			1, 1, 1, 0, 0,
		},
			{
			0, 0, 1, 1, 1,
			0, 0, 1, 1, 1,
			0, 0, 1, 1, 1,
			0, 0, 1, 1, 1,
			0, 0, 1, 1, 1,
			0, 0, 1, 1, 1,
			0, 0, 1, 1, 1,
			0, 0, 1, 1, 1,
		},
		{
			1, 1, 1, 1, 1,
			1, 1, 1, 1, 1,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			1, 1, 1, 1, 1,
			1, 1, 1, 1, 1,
			1, 1, 1, 1, 1,
		}};
		int i;
		for (i = 0; i <= 5 ; i++) { // put the customcharacters into the display
			drvthis->set_char (drvthis, i, &bignum[i][0]);
			}
		}
		adv_bignum_write_num (drvthis, num_map, x, num, height); // write the number
}


/////////////////////////////////////////////////////////////////////////////////////////////
// This function is usable for a 2 line display with 28 or more custom characters.
// (Wow, allmost graphical)
static void adv_bignum_num_2_28 (Driver *drvthis, int x, int num, int height, int do_init)
{
	static char num_map[][4][4] =
		{{{15	,6	,2}, /*0*/
		{14	,4	,5},
		{"   "},
		{"   "}},
		{{' '	,26	,' '},/*1*/
		{' '	,10	,' '},
		{"   "},
		{"   "}},
		{{1	,6	,2},/*2*/
		{7	,8	,9},
		{"   "},
		{"   "}},
		{{0	,11	,2},/*3*/
		{3	,13	,5},
		{"   "},
		{"   "}},
		{{25	,21	,23},/*4*/
		{17	,22	,24},
		{"   "},
		{"   "}},
		{{10	,11	,12},/*5*/
		{3	,13	,5},
		{"   "},
		{"   "}},
		{{15	,11	,16},/*6*/
		{14	,13	,5},
		{"   "},
		{"   "}},
		{{17	,18	,19},/*7*/
		{' '	,20	,' '},
		{"   "},
		{"   "}},
		{{15	,11	,2},/*8*/
		{14	,13	,5},
		{"   "},
		{"   "}},
		{{15	,11	,2},/*9*/
		{3	,13	,5},
		{"   "},
		{"   "}},
		{{27},/*:*/
		{27},
		{"   "},
		{"   "}}}; // Defines the character placing inside the bignumber.

	if (do_init) // Set customcharacters if needed.
		{

		static char bignum[28][5 * 8] = { // stored customcharacters
			{			//0
				0, 0, 0, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
			},
			{			//1
				0, 0, 0, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
			},
			{			//2
				1, 1, 0, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0
			},
			{			//3
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 0, 1, 1,
				0, 0, 0, 1, 1,
			},
			{			//4
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
			},
			{			//5
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 0, 0, 0,
				1, 0, 0, 0, 0,
			},
			{			//6
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
			},
			{			//7
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
			},
			{			//8
				0, 0, 1, 1, 1,
				0, 1, 1, 1, 1,
				1, 1, 1, 1, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
			},
			{			//9
				1, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
			},
			{			//10
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
			},
			{			//11
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
			},
			{			//12
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
			},
			{			//13
				1, 1, 1, 1, 1,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
			},
			{			//14
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 0, 1, 1,
				0, 0, 0, 1, 1,
			},
			{			//15
				0, 0, 0, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
			},
			{			//16
				1, 1, 0, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
			},
			{			//17
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
			},
			{			//18
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				0, 0, 0, 0, 0,
				0, 0, 0, 1, 1,
				0, 0, 0, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
			},
			{			//19
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 0, 0, 0,
				1, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
			},
			{			//20
				0, 1, 1, 1, 0,
				0, 1, 1, 1, 0,
				1, 1, 1, 1, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
			},
			{			//21
				0, 0, 0, 0, 1,
				0, 0, 0, 1, 1,
				0, 0, 1, 1, 1,
				0, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				1, 1, 1, 0, 1,
				1, 1, 0, 0, 1,
				1, 1, 0, 0, 1,
			},
			{			//22
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				0, 0, 0, 0, 1,
				0, 0, 0, 0, 1,
				0, 0, 0, 0, 1,
				0, 0, 0, 0, 1,
				0, 0, 0, 0, 1,
			},
			{			//23
				1, 0, 0, 0, 0,
				1, 0, 0, 0, 0,
				1, 0, 0, 0, 0,
				1, 0, 0, 0, 0,
				1, 0, 0, 0, 0,
				1, 0, 0, 0, 0,
				1, 0, 0, 0, 0,
				1, 0, 0, 0, 0,
			},
			{			//24
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 1, 1, 0, 0,
				1, 0, 0, 0, 0,
				1, 0, 0, 0, 0,
				1, 0, 0, 0, 0,
				1, 0, 0, 0, 0,
				1, 0, 0, 0, 0,
			},
			{			//25
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 1,
				0, 0, 0, 0, 1,
			},
			{			//26
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 1, 1, 1, 1,
				1, 1, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
				0, 0, 1, 1, 1,
			},
			{			//27
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 1, 1, 0,
				0, 0, 1, 1, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
			}
			};
		int i;
		for (i = 0; i <= 28 ; i++) { // put the customcharacters into the display
			drvthis->set_char (drvthis, i, &bignum[i][0]);
			}
		}
	adv_bignum_write_num (drvthis, num_map, x, num, height); // write the number
}


/////////////////////////////////////////////////////////////////////////////////////////////
// This function is usable for a 4 line display without custom characters.
// (o.k.)
static void adv_bignum_num_4_0 (Driver *drvthis, int x, int num, int height, int do_init)
{
	/* Ugly code extracted by David GLAUDE from lcdm001.c ;)*/
	/* Moved to driver.c by Joris Robijn */
	// Moved to adv_bignum.c by Stefan Herdler
static char num_map[][4][4] = {
	{ /* 0 */
		" _ ",
		"| |",
		"|_|",
		"   " },
	{ /* 1 */
		"   ",
		"  |",
		"  |",
		"   " },
	{ /* 2 */
		" _ ",
		" _|",
		"|_ ",
		"   " },
	{ /* 3 */
		" _ ",
		" _|",
		" _|",
		"   " },
	{ /* 4 */
		"   ",
		"|_|",
		"  |",
		"   " },
	{ /* 5 */
		" _ ",
		"|_ ",
		" _|",
		"   " },
	{ /* 6 */
		" _ ",
		"|_ ",
		"|_|",
		"   " },
	{ /* 7 */
		" _ ",
		"  |",
		"  |",
		"   " },
	{ /* 8 */
		" _ ",
		"|_|",
		"|_|",
		"   " },
	{ /* 9 */
		" _ ",
		"|_|",
		" _|",
		"   " },
	{ /* colon */
		" ",
		".",
		".",
		" " }
	}; // Defines the character placing inside the bignumber.

	adv_bignum_write_num (drvthis, num_map, x, num, height); // write the number
}


/////////////////////////////////////////////////////////////////////////////////////////////
// This function is usable for a 4 line display with 5 or more custom characters.
// (nice bignumbers)
static void adv_bignum_num_4_3 (Driver *drvthis, int x, int num, int height, int do_init)
{
static char num_map[][4][4] =
	{{{3	,1	,3}, /*0*/
	{3	,' '	,3},
	{3	,' '	,3},
	{3	,2	,3}},
	{{' '	,' '	,3},/*1*/
	{' '	,' '	,3},
	{' '	,' '	,3},
	{' '	,' '	,3}},
	{{' '	,1	,3},/*2*/
	{' '	,2	,3},
	{3	,' '	,3},
	{3	,2	,3}},
	{{' '	,1	,3},/*3*/
	{' '	,' '	,3},
	{' '	,2	,3},
	{' '	,' '	,3}},
	{{3	,2	,3},/*4*/
	{3	,2	,3},
	{' '	,' '	,3},
	{' '	,' '	,3}},
	{{3	,1	,3},/*5*/
	{3	,2	,3},
	{' '	,' '	,3},
	{' '	,2	,3}},
	{{3	,1	,3},/*6*/
	{3	,' '	,3},
	{3	,1	,3},
	{3	,2	,3}},
	{{' '	,1	,3},/*7*/
	{' '	,' '	,3},
	{' '	,' '	,3},
	{' '	,' '	,3}},
	{{3	,1	,3},/*8*/
	{3	,2	,3},
	{3	,' '	,3},
	{3	,2	,3}},
	{{3	,1	,3},/*9*/
	{3	,2	,3},
	{' '	,' '	,3},
	{' '	,2	,3}},
	{{" "},/*:*/
	{'.'},
	{'.'},
	{" "}}}; // Defines the character placing inside the bignumber.
if (do_init) // Set customcharacters if needed.
	{
	static char bignum[5][5 * 8] = { // stored customcharacters
	{
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
	},
		{
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	},
		{
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
	}
	};
		int i;
	for (i = 0; i <= 3 ; i++) { // put the customcharacters into the display
		drvthis->set_char (drvthis, i, &bignum[i][0]);
	}
}
	adv_bignum_write_num (drvthis, num_map, x, num, height); // write the number


}
