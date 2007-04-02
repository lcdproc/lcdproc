/* 	This file is part the LCDproc driver for various serial VFD Devices.

	It contains the hardwaredependent commands ach characterset.

	If you want to add a new device to the driver add a new section
	to the displaytype-switch-case in the init-function, add a new load_...
	function below and fill it with the corrrect commands for the display.
	(Try wich displaytype works best with your display, copy and modify
	it's section that is the easiest way I guess.)

	Copyright (C) 2006 Stefan Herdler

	2006-05-16 Version 0.3: everything should work (not all hardware tested!)

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
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301

*/

#include "serialVFD_displays.h"
#include "serialVFD.h"
#include "lcd.h"

void serialVFD_load_NEC_FIPC (Driver *drvthis);
void serialVFD_load_KD (Driver *drvthis);
void serialVFD_load_Noritake (Driver *drvthis);
void serialVFD_load_Futaba (Driver *drvthis);


void serialVFD_load_display_data(Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;
	switch (p->display_type) {
		case 0:
			serialVFD_load_NEC_FIPC(drvthis);
			break;
		case 1:
			serialVFD_load_KD(drvthis);
			break;
		case 2:
			serialVFD_load_Noritake(drvthis);
			break;
		case 3:
			serialVFD_load_Futaba(drvthis);
			break;
	}
}


void
serialVFD_load_NEC_FIPC (Driver *drvthis)
{		//nec_fipc
	PrivateData *p = (PrivateData*) drvthis->private_data;
	int tmp, w;

	if (p->customchars == -83)
		p->customchars = 1;	// number of custom characters the display provides
	p->vbar_cc_offset = 5;	// character offset of the bars
	p->hbar_cc_offset = 12;	// character offset of the bars
	p->predefined_hbar = 1;   // the display has predefined hbar-characters
	p->predefined_vbar = 1;   // the display has predefined vbar-characters

	// hardwarespecific commands:
	//  hw_cmd[Command][data]  = 	{{commandlength , command 1},
	//					.....
	//				 {commandlength , command N}}
	const char hw_cmd[10][4] = {{1	,0x04},  	// dark
				{1      ,0x03},
				{1	,0x02},
				{1	,0x01},  	// bright
				{1	,0x0D},  	// pos1
				{1	,0x1B},  	// move cursor
				{1	,0x0C},  	// reset
				{2	,0x14, 0x11},  	// init
				{1	,0x1A}, 	// set user char
				{1	,0x09}}; 	// tab
	for (tmp = 0; tmp < 10; tmp++)
		for (w = 0; w < 4; w++)
			p->hw_cmd[tmp][w] = hw_cmd[tmp][w];

	// Translates ISO 8859-1 to display charset.
	const unsigned char charmap[] = {
		/* #128  = 0x80 */
		128, 129, 130, 131, 132, 133, 134, 135,
		136, 137, 138, 139, 140, 141, 142, 143,
		144, 145, 146, 147, 148, 149, 150, 151,
		152, 153, 154, 155, 156, 157, 158, 159,
		/* #160 = 0xA0 */
		160,   '!', 0xF7, 0xF8,  '?',  '?', 0x7C, 0xF9,
		'"',   '?',  '?',  '?',  '?',  '-',  '?',  '?',
		0x8B, 0xF3, 0x89, 0x8A, 0x27, 0x98,  '?',  '.',
		',',   '?',  '?',  '?',  '?', 0x8C,  '?', 0xAA,
		/* #192 = 0xC0 */
		'A',   'A',  'A',  'A', 0xA2, 0xA8,  'A',  'C',
		'E',   'E',  'E', 0xB6,  'I',  'I',  'I',  'I',
		'D',   0xA9, 'O',  'O',  'O',  'O', 0xA3, 0x8D,
		'0',   'U',  'U',  'U', 0xA4,  'Y',  'p', 0x91,
		/* #224 = 0xE0 */
		'a',   'a',  'a',  'a', 0xA5,  'a',  'a',  'c',
		'e',   'e',  'e',  'e',  'i',  'i',  'i',  'i',
		'o',   'n',  'o',  'o',  'o',  'o', 0xA6, 0x8E,
		'0',   'u',  'u',  'u', 0xA7,  'y',  'p',  'y' };
	for (tmp = 0; tmp < 128; tmp++)
		p->charmap[tmp] = charmap[tmp];

	// {bytes to send, icon bit mapped to bit 0, icon bit mapped to bit 1, ...}
	const int usr_chr_dot_assignment[57] = { 7,
						 1, 2, 3, 4, 5, 0, 0, 0,
						 6, 7, 8, 9,10, 0, 0, 0,
						11,12,13,14,15, 0, 0, 0,
						16,17,18,19,20, 0, 0, 0,
						21,22,23,24,25, 0, 0, 0,
						26,27,28,29,30, 0, 0, 0,
						31,32,33,34,35, 0, 0, 0 };
	for (tmp = 0; tmp < 57; tmp++)
		p->usr_chr_dot_assignment[tmp] = usr_chr_dot_assignment[tmp];

	// Where to place the usercharacters (0..30) in the asciicode.
	// Also used to map standardcharacters in the usercharacterspace(0..30)
	// (useful for displays with less then 30 usercharacters and predefined bars)
	const unsigned int usr_chr_mapping[31] =
	{0xAF,0,0,0,0,0, 0x5F, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0, 0x5F, 0xE1, 0xE3, 0xE4};
	for (tmp = 0; tmp < 31; tmp++)
		p->usr_chr_mapping[tmp] = usr_chr_mapping[tmp];
}


void
serialVFD_load_KD (Driver *drvthis)
{ 		//KD Rev2.1
	PrivateData *p = (PrivateData*) drvthis->private_data;
	int tmp, w;

	if (p->customchars == -83)
		p->customchars = 31;	// number of custom characters the display provides
	p->vbar_cc_offset = 0;	// character offset of the bars
	p->hbar_cc_offset = 0;	// character offset of the bars
	p->predefined_hbar = 0;   // the display has predefined hbar-characters
	p->predefined_vbar = 0;   // the display has predefined vbar-characters

	// hardwarespecific commands:
	//  hw_cmd[Command][data]  = 	{{commandlength , command 1},
	//					.....
	//				 {commandlength , command N}}
	const char hw_cmd[10][4] = {{1	,0x04},  	// dark
				{1      ,0x03},
				{1	,0x02},
				{1	,0x01},  	// bright
				{1	,0x0D},  	// pos1
				{1	,0x1B},  	// move cursor
				{1	,0x0C},  	// reset
				{2	,0x14, 0x11},  	// init
				{1	,0x1A}, 	// set user char
				{1	,0x09}}; 	// tab
	for (tmp = 0; tmp < 10; tmp++)
		for (w = 0; w < 4; w++)
			p->hw_cmd[tmp][w] = hw_cmd[tmp][w];

	const unsigned char charmap[] = {
		/* #128  = 0x80 */
		128, 129, 130, 131, 132, 133, 134, 135,
		136, 137, 138, 139, 140, 141, 142, 143,
		144, 145, 146, 147, 148, 149, 150, 151,
		152, 153, 154, 155, 156, 157, 158, 159,
		/* #160 = 0xA0 */
		160,   '!', 0xF7, 0xF8,  '?',  '?', 0x7C, 0xF9,
		'"',   '?',  '?',  '?',  '?',  '-',  '?',  '?',
		0xA9, 0xBA,  '?',  '?', 0x27,  '?',  '?',  '.',
		',',   '?',  '?',  '?',  '?',  '?',  '?',  '?',
		/* #192 = 0xC0 */
		0xB2,  'A',  'A',  'A', 0xA2, 0xA1,  'A',  'C',
		'E',   'E', 0xA8, 0xB6, 0xAB, 0xAA, 0xAC, 0xA9,
		'D',  0xAE, 0xB1, 0xB0, 0xBF,  'O', 0xA3,  'x',
		0xAF, 0xB7, 0xB6, 0xB8, 0xA4,  'Y',  'p', 0x91,
		/* #224 = 0xE0 */
		0xD2, 0xC2, 0xC4,  'a', 0xA5, 0xC1, 0xC5, 0xC3,
		0xC7, 0xC6, 0xC8, 0xD4, 0xCC, 0xCB, 0xCD, 0xCA,
		'o',  0xCF, 0xD1, 0xD0, 0xCE,  'o', 0xA6, 0xBB,
		0xD0, 0xD7, 0xD6, 0xD8, 0xA7,  'y',  'p',  'y' };
	for (tmp = 0; tmp < 128; tmp++)
		p->charmap[tmp] = charmap[tmp];

	// {bytes to send, icon bit mapped to bit 0, icon bit mapped to bit 1, ...}
	const int usr_chr_dot_assignment[57] = { 7,
					 	 1, 2, 3, 4, 5, 0, 0, 0,
						 6, 7, 8, 9,10, 0, 0, 0,
						11,12,13,14,15, 0, 0, 0,
						16,17,18,19,20, 0, 0, 0,
						21,22,23,24,25, 0, 0, 0,
						26,27,28,29,30, 0, 0, 0,
						31,32,33,34,35, 0, 0, 0 };
	for (tmp = 0; tmp < 57; tmp++)
		p->usr_chr_dot_assignment[tmp] = usr_chr_dot_assignment[tmp];

	// Where to place the usercharacters (0..30) in the asciicode.
	// Also used to map standardcharacters in the usercharacterspace(0..30)
	// (useful for displays with less then 30 usercharacters and predefined bars)
	const unsigned int usr_chr_mapping[31]=
	{0xAF};
	for (tmp = 0; tmp < 31; tmp++)
		p->usr_chr_mapping[tmp] = usr_chr_mapping[tmp];
}


void
serialVFD_load_Noritake (Driver *drvthis)
{ 		//Noritake
	PrivateData *p = (PrivateData*) drvthis->private_data;
	int tmp, w;

	if (p->customchars == -83)
		p->customchars = 16;	// number of custom characters the display provides
	p->vbar_cc_offset = 0;	// character offset of the bars
	p->hbar_cc_offset = 0;	// character offset of the bars
	p->predefined_hbar = 0;   // the display has predefined hbar-characters
	p->predefined_vbar = 0;   // the display has predefined vbar-characters

	// hardwarespecific commands:
	//  hw_cmd[Command][data]  = 	{{commandlength , command 1},
	//					.....
	//				 {commandlength , command N}}
	const char hw_cmd[10][4] = {{3	,0x1B, 0x4C, 0x00},  // dark
				{3      ,0x1B, 0x4C, 0x50},
				{3	,0x1B, 0x4C, 0x90},
				{3	,0x1B, 0x4C, 0xFF},  // bright
				{1	,0x0C},		// pos1
				{2	,0x1B, 0x48},	// move cursor
				{2	,0x1B, 0x49},	// reset
				{2	,0x14, 0x11},  	// init
				{2	,0x1B, 0x43}, 	// set user char
				{1	,0x09}}; 	// tab
	for (tmp = 0; tmp < 10; tmp++)
		for (w = 0; w < 4; w++)
			p->hw_cmd[tmp][w] = hw_cmd[tmp][w];

	// no charmap needed
	for (tmp = 128; tmp <= 255; tmp++)
		p->charmap[tmp] = tmp;

	// {bytes to send, icon bit mapped to bit 0, icon bit mapped to bit 1, ...}
	const int usr_chr_dot_assignment[57] = { 5,
						 1, 2, 3, 4, 5, 6, 7, 8,
						 9,10,11,12,13,14,15,16,
						17,18,19,20,21,22,23,24,
						25,26,27,28,29,30,31,32,
						33,34,35, 0, 0, 0, 0, 0 };
	for (tmp = 0; tmp < 57; tmp++)
		p->usr_chr_dot_assignment[tmp] = usr_chr_dot_assignment[tmp];

	// Where to place the usercharacters (0..30) in the asciicode.
	// Also used to map standardcharacters in the usercharacterspace(0..30)
	// (useful for displays with less then 30 usercharacters and predefined bars)
	const unsigned int usr_chr_mapping[31]=
	{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,\
 		0x0F, 0x10, 0x13 , 0x14, 0x1C, 0x1D, 0x1E, 0x1F};
	for (tmp = 0; tmp < 31; tmp++)
		p->usr_chr_mapping[tmp] = usr_chr_mapping[tmp];
}


void
serialVFD_load_Futaba (Driver *drvthis)
{ 		//Futaba
	PrivateData *p = (PrivateData*) drvthis->private_data;
	int tmp, w;

	if (p->customchars == -83)
		p->customchars = 3;	// number of custom characters the display provides
	p->vbar_cc_offset = 0;	// character offset of the bars
	p->hbar_cc_offset = 0;	// character offset of the bars
	p->predefined_hbar = 0;   // the display has predefined hbar-characters
	p->predefined_vbar = 0;   // the display has predefined vbar-characters

	// hardwarespecific commands:
	//  hw_cmd[Command][data]  = 	{{commandlength , command 1},
	//					.....
	//				 {commandlength , command N}}
	const char hw_cmd[10][4] = {{2    ,0x04, 0x20},	// dark
				{2      ,0x04, 0x40},
				{2	,0x04, 0x60},
				{2	,0x04, 0xFF},	// bright
				{2	,0x10, 0x00},	// pos1
				{1	,0x10,},	// move cursor
				{1	,0x1F},		// reset
				{2	,0x11,0x14},  	// init
				{1	,0x03}, 	// set user char
				{1	,0x09}}; 	// tab
	for (tmp = 0; tmp < 10; tmp++)
		for (w = 0; w < 4; w++)
			p->hw_cmd[tmp][w] = hw_cmd[tmp][w];

	// Translates ISO 8859-1 to display charset.
	const unsigned char charmap[] = {
		/* #128  = 0x80 */
		128, 129, 130, 131, 132, 133, 134, 135,
		136, 137, 138, 139, 140, 141, 142, 143,
		144, 145, 146, 147, 148, 149, 150, 151,
		152, 153, 154, 155, 156, 157, 158, 159,
		/* #160 = 0xA0 */
		160,  0xAD, 0x9B, 0x9C, 0xC8, 0x9D, 0x7C, 0xC0,
		'"',   '?', 0xA6, 0xAE, 0xAA,  '-',  '?',  '?',
		0xEF, 0xCA, 0xC6, 0xC7, 0x27, 0xB8,  '?',  '.',
		',',   '?', 0xA7, 0xAF, 0xAC, 0xAB,  '?', 0xA8,
		/* #192 = 0xC0 */
		0xD0,  'A', 0xD5,  'A', 0x8E, 0x8F, 0x92, 0x80,
		0xD1, 0x90, 0xD6, 0xD3, 'I',   'I', 0xD7, 0xD4,
		'D',  0xA5,  'O',  'O', 0xD8,  'O', 0x99,  'x',
		'0',  0xD2,  'U', 0xD9, 0x9A,  'Y',  'p', 0xB1,
		/* #224 = 0xE0 */
		0x85, 0xA0, 0x83,  'a', 0x84, 0x86, 0x91, 0x87,
		0x8A, 0x82, 0x88, 0x89, 0x8D, 0xA1, 0x8C, 0x8B,
		'o',  0xA4, 0x95, 0xA9, 0x93,  'o', 0x94,  '/',
		'0',  0x97, 0xA3, 0x96, 0x81,  'y',  'p', 0x89 };
	for (tmp = 0; tmp < 128; tmp++)
		p->charmap[tmp] = charmap[tmp];

	// {bytes to send, icon bit mapped to bit 0, icon bit mapped to bit 1, ...}
	const int usr_chr_dot_assignment[57] = { 5,
						 8, 7, 6, 5, 4, 3, 2, 1,
						16,15,14,13,12,11,10, 9,
						24,23,22,21,20,19,18,17,
						32,31,30,29,28,27,26,25,
						 0, 0, 0, 0, 0,35,34,33 };
	for (tmp = 0; tmp < 57; tmp++)
		p->usr_chr_dot_assignment[tmp] = usr_chr_dot_assignment[tmp];

	// Where to place the usercharacters (0..30) in the asciicode.
	// Also used to map standardcharacters in the usercharacterspace(0..30)
	// (useful for displays with less then 30 usercharacters and predefined bars)
	const unsigned int usr_chr_mapping[31] = { 0xCD, 0xCE, 0xCF };
	for (tmp = 0; tmp < 31; tmp++)
		p->usr_chr_mapping[tmp] = usr_chr_mapping[tmp];
}
