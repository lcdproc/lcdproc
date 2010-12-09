/** \file server/drivers/serialVFD_displays.c
 * Setup routines for the inidividual displays supported by the \c serialVFD driver.
 * 
 * Here are the routines that set up the data as well as function pointers
 * for the individual displays supported by the \c serialVFD driver.
 *
 * If you want to add a new device to the driver add a new section
 * to the displaytype-switch-case in the init-function, add a new load_...
 * function below and fill it with the corrrect commands for the display.
 * (Try wich displaytype works best with your display, copy and modify
 * it's section that is the easiest way I guess.) 
 *
 * For each display the following setting have to be set in
 * serialVFD_private_data:
 * 
 * \li customchars       Number of possible custom characters
 * \li predefined_hbar   The display has predefined hbar-characters
 * \li hbar_cc_offset    Offset of the predefined hbar-characters
 * \li predefined_vbar   The display has predefined vbar-characters
 * \li vbar_cc_offset    Offset of the predefined vbar-characters
 * \li hw_cmd            Table with hardware commands (see below)
 * \li charmap           Table mapping characters 127...255 to other chars
 * \li usr_chr_dot_assignment  Mapping of icon bits (see below)
 * \li usr_chr_mapping   Map indicating which chars may be overwritten in RAM
 * \li usr_chr_load_mapping  Location byte (see below)
 *
 * \b hw_cmd:
 * The hw_cmd is a two-dimensional array used as a table to store hardware-
 * dependant command. 10 commands are supported. For each command a byte
 * array is set with the 1st byte defining the command length (which may be
 * zero) and up to 9 bytes of command. The following commands exist:
 * 
 * \li Set brightness level 0
 * \li Set brightness level 1
 * \li Set brightness level 2
 * \li Set brightness level 3
 * \li Move cursor to Pos1
 * \li Move cursor to position X
 * \li Reset
 * \li Initialize
 * \li Set user character
 * \li Horizontal tab
 * \li Move cursor to next line
 *
 *\verbatim
 * hw_cmd[Command][data] = {{commandlength , command 1},
 *                          .....
 *                          {commandlength , command N}}
 *\endverbatim
 *
 * \b usr_chr_dot_assignment:
 * usr_chr_dot_assignment is an array specifying how our 5x7 matrix of custom
 * character bits are mapped into user defined characters (UDC). The very
 * first byte of usr_chr_dot_assignment gives the number of bytes this VFD
 * uses for one UDC. The remaining numbers give which byte/bit if a UDC hold
 * which bit of the 5x7 matrix.
 *
 * Note: LSB of an icon line is the leftmost entry in the array!
 *
 * \b usr_chr_load_mapping:
 * Usually any character may be overwritten in RAM. Some VFD however use
 * an index number for user defined characters. The usr_chr_load_mapping
 * array specifies which byte of usr_chr_mapping is assigned which index.
 *
 * For displays not using index numbers usr_chr_load_mapping is a copy of
 * usr_chr_mapping.
 */

/*-
 * Copyright (C) 2006 Stefan Herdler
 *
 * This driver is based on wirz-sli.c, hd44780.c, drv_base.c and NoritakeVFD
 * driver. It may contain parts of other drivers of this package too.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

/*
 * 2006-05-16 Version 0.3: everything should work (not all hardware tested!)
 */

#include "lcd.h"
#include "serialVFD_displays.h"
#include "serialVFD.h"

void serialVFD_load_NEC_FIPC (Driver *drvthis);
void serialVFD_load_KD (Driver *drvthis);
void serialVFD_load_Noritake (Driver *drvthis);
void serialVFD_load_Futaba (Driver *drvthis);
void serialVFD_load_IEE_95B (Driver *drvthis);
void serialVFD_load_IEE_96 (Driver *drvthis);
void serialVFD_load_Futaba_NA202SD08FA(Driver *drvthis);
void serialVFD_load_Samsung (Driver *drvthis);
void serialVFD_load_Nixdorf_BA6x (Driver *drvthis);

/**
 * Load display specific settings.
 * \param  drvthis  Pointer to driver
 * \return  0 on success; -1 if unknown display type selected in config
 */
int serialVFD_load_display_data(Driver *drvthis)
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
		case 4:
			serialVFD_load_IEE_95B(drvthis);
			break;
		case 5:
			serialVFD_load_IEE_96(drvthis);
			break;
		case 6:
			serialVFD_load_Futaba_NA202SD08FA(drvthis);
			break;
		case 7:
			serialVFD_load_Samsung(drvthis);
			break;
		case 8:
			serialVFD_load_Nixdorf_BA6x(drvthis);
			break;
		default:
			return -1;
			break;
	}
	return 0;
}

/* Init data for NEC FIPS based VFD */
void
serialVFD_load_NEC_FIPC (Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;
	int tmp, w;

	if (p->customchars == CC_UNSET)
		p->customchars = 1;
	p->vbar_cc_offset = 5;
	p->hbar_cc_offset = 12;
	p->predefined_hbar = 1;
	p->predefined_vbar = 1;

	/*-
	 * hardwarespecific commands:
	 *   hw_cmd[Command][data] =	{{commandlength , command 1},
	 *				.....
	 *				 {commandlength , command N}};
	 */
	const char hw_cmd[11][4] = {{1	,0x04},  	// dark
				{1      ,0x03},
				{1	,0x02},
				{1	,0x01},  	// bright
				{1	,0x0D},  	// pos1
			// pos1 command is only used (and needed), when mv_cursor command is not supported
				{1	,0x1B},  	// move cursor (set to 0 if not supported)
				{1	,0x0C},  	// reset
				{2	,0x14, 0x11},  	// init
				{1	,0x1A}, 	// set user char
				{1	,0x09}, 	// tab
				{0	}};	// next_line (only used in line mode)
			// Line mode will be used if Next_line command is set!!!
			// Do not set if the display supports normal mode (most displays should do this).

	for (tmp = 0; tmp < 11; tmp++)
		for (w = 0; w < 4; w++)
			p->hw_cmd[tmp][w] = hw_cmd[tmp][w];

	/* Translates ISO 8859-1 to display charset. */
	const unsigned char charmap[] = {
		127,		/* the "filled-block"-character usually 127 */
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
	for (tmp = 0; tmp < 129; tmp++)
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

	// The following is only needet to set if the display needs a different setting
	// for loading the usercharacters.
	// Example: The character loaded to 0x00 will be shown at 0xFD.
	// usr_chr_load_mapping[0] or usr_chr_load_mapping[1] has to be != 0
	//const unsigned int usr_chr_load_mapping[31]=
	//{ 0x02, 0x01, 0x00};
	//for (tmp = 0; tmp < 31; tmp++)
	//	p->usr_chr_load_mapping[tmp] = usr_chr_load_mapping[tmp];
}


/* Init data for KD Rev2.1 */
void
serialVFD_load_KD (Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;
	int tmp, w;

	if (p->customchars == CC_UNSET)
		p->customchars = 31;
	p->vbar_cc_offset = 0;
	p->hbar_cc_offset = 0;
	p->predefined_hbar = 0;
	p->predefined_vbar = 0;

	/*-
	 * hardwarespecific commands:
	 *   hw_cmd[Command][data] =	{{commandlength , command 1},
	 *				.....
	 *				 {commandlength , command N}};
	 */
	const char hw_cmd[10][4] = {{1	,0x04},  	// dark
				{1      ,0x03},
				{1	,0x02},
				{1	,0x01},  	// bright
				{1	,0x0D},  	// pos1
				{1	,0x1B},  	// move cursor (set to 0 if not supported)
				{1	,0x0C},  	// reset
				{2	,0x14, 0x11},  	// init
				{1	,0x1A}, 	// set user char
				{1	,0x09}}; 	// tab
	for (tmp = 0; tmp < 10; tmp++)
		for (w = 0; w < 4; w++)
			p->hw_cmd[tmp][w] = hw_cmd[tmp][w];

	/* Translates ISO 8859-1 to display charset. */
	const unsigned char charmap[] = {
		127,		/* the "filled-block"-character usually 127 */
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
	for (tmp = 0; tmp < 129; tmp++)
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

/* Init data for Noritake VFD */
void
serialVFD_load_Noritake (Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;
	int tmp, w;

	if (p->customchars == CC_UNSET)
		p->customchars = 16;
	p->vbar_cc_offset = 0;
	p->hbar_cc_offset = 0;
	p->predefined_hbar = 0;
	p->predefined_vbar = 0;

	/*-
	 * hardwarespecific commands:
	 *   hw_cmd[Command][data] =	{{commandlength , command 1},
	 *				.....
	 *				 {commandlength , command N}};
	 */
	const char hw_cmd[10][4] = {{3	,0x1B, 0x4C, 0x00},  // dark
				{3      ,0x1B, 0x4C, 0x50},
				{3	,0x1B, 0x4C, 0x90},
				{3	,0x1B, 0x4C, 0xFF},  // bright
				{1	,0x0C},		// pos1
				{2	,0x1B, 0x48},	// move cursor (set to 0 if not supported)
				{2	,0x1B, 0x49},	// reset
				{2	,0x14, 0x11},  	// init
				{2	,0x1B, 0x43}, 	// set user char
				{1	,0x09}}; 	// tab
	for (tmp = 0; tmp < 10; tmp++)
		for (w = 0; w < 4; w++)
			p->hw_cmd[tmp][w] = hw_cmd[tmp][w];

	/* no charmap needed */
	for (tmp = 0; tmp < 129; tmp++)
		p->charmap[tmp] = tmp+127;

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


/* Init data for Futaba VFD */
void
serialVFD_load_Futaba (Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;
	int tmp, w;

	if (p->customchars == CC_UNSET)
		p->customchars = 3;
	p->vbar_cc_offset = 0;
	p->hbar_cc_offset = 0;
	p->predefined_hbar = 0;
	p->predefined_vbar = 0;

	/*-
	 * hardwarespecific commands:
	 *   hw_cmd[Command][data] =	{{commandlength , command 1},
	 *				.....
	 *				 {commandlength , command N}};
	 */
	const char hw_cmd[10][4] = {{2    ,0x04, 0x20},	// dark
				{2      ,0x04, 0x40},
				{2	,0x04, 0x60},
				{2	,0x04, 0xFF},	// bright
				{2	,0x10, 0x00},	// pos1
				{1	,0x10},	// move cursor (set to 0 if not supported)
				{1	,0x1F},		// reset
				{2	,0x11,0x14},  	// init
				{1	,0x03}, 	// set user char
				{1	,0x09}}; 	// tab
	for (tmp = 0; tmp < 10; tmp++)
		for (w = 0; w < 4; w++)
			p->hw_cmd[tmp][w] = hw_cmd[tmp][w];

	/* Translates ISO 8859-1 to display charset. */
	const unsigned char charmap[] = {
		127,		/* the "filled-block"-character usually 127 */
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
	for (tmp = 0; tmp < 129; tmp++)
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


/* Init data for IEE_03601-95B_2x40_VFD */
void
serialVFD_load_IEE_95B (Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;
	int tmp, w;

	if (p->customchars == CC_UNSET)
		p->customchars = 10;
	p->vbar_cc_offset = 0;
	p->hbar_cc_offset = 0;
	p->predefined_hbar = 0;
	p->predefined_vbar = 0;
	p->para_wait = 60;	// the display needs more delay in the parallelport mode

	/*-
	 * hardwarespecific commands:
	 *   hw_cmd[Command][data] =	{{commandlength , command 1},
	 *				.....
	 *				 {commandlength , command N}};
	 */
	const char hw_cmd[10][4] = {{1	,0x1C},  // dark
				{1      ,0x1D},
				{1	,0x1E},
				{1	,0x1F},  	// bright
				{1	,0x16},		// pos1
				{1	,0x1B},		// move cursor (set to 0 if not supported)
				{1	,0x15},		// reset
				{2	,0x0E, 0x11},  	// init
				{1	,0x18}, 	// set user char
				{1	,0x09}}; 	// tab
	for (tmp = 0; tmp < 10; tmp++)
		for (w = 0; w < 4; w++)
			p->hw_cmd[tmp][w] = hw_cmd[tmp][w];

	/* Translates ISO 8859-1 to display charset. */
	const unsigned char charmap[] = {
		0xEF,		/* the "filled-block"-character usually 127 */
		/* #128  = 0x80 */
		0x7F, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
		0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
		0xE8, 0xE9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
		152, 153, 154, 155, 156, 157, 158, 159,
		/* #160 = 0xA0 */
		160,   '!', 0xD3,  '?',  '?', 0x5C, 0x7C,  '?',
		'"',   '?',  '?',  '?',  '?',  '-',  '?',  '?',
		0xB9, 0xBA,  '?',  '?', 0x27,  '?',  '?',  '.',
		',',   '?',  '?',  '?',  '?',  '?',  '?',  '?',
		/* #192 = 0xC0 */
		0xB2, 0xA2, 0xA4,  'A', 0xA0, 0xA1, 0xA5, 0xA3,
		0xA7, 0xA6, 0xA8, 0xB4, 0xAB, 0xAA, 0xAC, 0xA9,
		'D',  0xAE, 0xB1, 0xB0, 0xBF,  'O', 0xAD,  'x',
		0xAF, 0xB7, 0xB6, 0xB8, 0xB5,  'Y',  'p', 0xB3,
		/* #224 = 0xE0 */
		0xD2, 0xC2, 0xC4,  'a', 0xC0, 0xC1, 0xC5, 0xC3,
		0xC7, 0xC6, 0xC8, 0xD4, 0xCB, 0xCA, 0xCC, 0xC9,
		'o',  0xCE, 0xD1, 0xD0, 0xDF,  'o', 0xAD, 0xBB,
		0xCF, 0xD7, 0xD6, 0xD8, 0xD5,  'y',  'p',  'y' };
	for (tmp = 0; tmp < 129; tmp++)
		p->charmap[tmp] = charmap[tmp];


	// {bytes to send, icon bit mapped to bit 0, icon bit mapped to bit 1, ...}
	const int usr_chr_dot_assignment[57] = { 6,
						 0, 0, 0, 0,34,17, 3, 0,
						33,14, 4,23,30,13, 7, 0,
						29,10, 8,27,26, 9,11, 0,
						25, 6,12,31,22, 5,15, 0,
						21, 2,16,35,18, 1,19, 0,
						20,24,28,32, 0, 0, 0, 0 };
	for (tmp = 0; tmp < 57; tmp++)
		p->usr_chr_dot_assignment[tmp] = usr_chr_dot_assignment[tmp];

	// Where to place the usercharacters (0..30) in the asciicode.
	// Also used to map standardcharacters in the usercharacterspace(0..30)
	// (useful for displays with less then 30 usercharacters and predefined bars)
	const unsigned int usr_chr_mapping[31]=
	{0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8, 0xF7, 0xF6};
	for (tmp = 0; tmp < 31; tmp++)
		p->usr_chr_mapping[tmp] = usr_chr_mapping[tmp];

	// The following is only needet to set if the display needs a different setting
	// for loading the usercharacters.
	// Example: The character loaded to 0x00 will be shown at 0xFD.
	// usr_chr_load_mapping[0] or usr_chr_load_mapping[1] has to be != 0
	const unsigned int usr_chr_load_mapping[31]=
	{0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00};
	for (tmp = 0; tmp < 31; tmp++)
		p->usr_chr_load_mapping[tmp] = usr_chr_load_mapping[tmp];

}


/* Init data for IEE_03601-96_2x40_VFD */
void
serialVFD_load_IEE_96 (Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;
	int tmp, w;

	if (p->customchars == CC_UNSET)
		p->customchars = 3;
	p->vbar_cc_offset = 0;
	p->hbar_cc_offset = 0;
	p->predefined_hbar = 0;
	p->predefined_vbar = 0;
	p->para_wait = 60;	// the display needs more delay in the parallelport mode

	/*-
	 * hardwarespecific commands:
	 *   hw_cmd[Command][data] =	{{commandlength , command 1},
	 *				.....
	 *				 {commandlength , command N}};
	 */
	const char hw_cmd[10][4] = {{1	,0x1C},  // dark
				{1      ,0x1D},
				{1	,0x1E},
				{1	,0x1F},  // bright
				{1	,0x16},	// pos1
				{0	},	// move cursor (set to 0 if not supported)
				{1	,0x15},	// reset
				{2	,0x0E, 0x11},  	// init
				{1	,0x18}, 	// set user char
				{1	,0x09}}; 	// tab
	for (tmp = 0; tmp < 10; tmp++)
		for (w = 0; w < 4; w++)
			p->hw_cmd[tmp][w] = hw_cmd[tmp][w];

	/* Translates ISO 8859-1 to display charset. */
	const unsigned char charmap[] = {
		0xEF,		/* the "filled-block"-character usually 127 */
		/* #128  = 0x80 */
		0x7F, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
		0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
		0xE8, 0xE9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
		152, 153, 154, 155, 156, 157, 158, 159,
		/* #160 = 0xA0 */
		160,   '!', 0xD3,  '?',  '?', 0x5C, 0x7C,  '?',
		'"',   '?',  '?',  '?',  '?',  '-',  '?',  '?',
		0xB9, 0xBA,  '?',  '?', 0x27,  '?',  '?',  '.',
		',',   '?',  '?',  '?',  '?',  '?',  '?',  '?',
		/* #192 = 0xC0 */
		0xB2, 0xA2, 0xA4,  'A', 0xA0, 0xA1, 0xA5, 0xA3,
		0xA7, 0xA6, 0xA8, 0xB4, 0xAB, 0xAA, 0xAC, 0xA9,
		'D',  0xAE, 0xB1, 0xB0, 0xBF,  'O', 0xAD,  'x',
		0xAF, 0xB7, 0xB6, 0xB8, 0xB5,  'Y',  'p', 0xB3,
		/* #224 = 0xE0 */
		0xD2, 0xC2, 0xC4,  'a', 0xC0, 0xC1, 0xC5, 0xC3,
		0xC7, 0xC6, 0xC8, 0xD4, 0xCB, 0xCA, 0xCC, 0xC9,
		'o',  0xCE, 0xD1, 0xD0, 0xDF,  'o', 0xAD, 0xBB,
		0xCF, 0xD7, 0xD6, 0xD8, 0xD5,  'y',  'p',  'y' };
	for (tmp = 0; tmp < 129; tmp++)
		p->charmap[tmp] = charmap[tmp];


	// {bytes to send, icon bit mapped to bit 0, icon bit mapped to bit 1, ...}
	const int usr_chr_dot_assignment[57] = { 6,
						 0, 7, 0,26, 0, 8,17, 0,
						 0, 9,18,28, 0,10,19, 0,
						 1,11,20,30, 2,12,21, 0,
						 3,13,22,32, 4,14,23, 0,
						 5,15,24,34, 6,16,25, 0,
						27,29,31,33,35, 0, 0, 0 };
	for (tmp = 0; tmp < 57; tmp++)
		p->usr_chr_dot_assignment[tmp] = usr_chr_dot_assignment[tmp];

	// Where to place the usercharacters (0..30) in the asciicode.
	// Also used to map standardcharacters in the usercharacterspace(0..30)
	// (useful for displays with less then 30 usercharacters and predefined bars)
	const unsigned int usr_chr_mapping[31]=
	{0xFF, 0xFE, 0xFD};
	for (tmp = 0; tmp < 31; tmp++)
		p->usr_chr_mapping[tmp] = usr_chr_mapping[tmp];

	// The following is only needet to set if the display needs a different setting
	// for loading the usercharacters.
	// Example: The character loaded to 0x00 will be shown at 0xFD.
	// usr_chr_load_mapping[0] or usr_chr_load_mapping[1] has to be != 0
	const unsigned int usr_chr_load_mapping[31]=
	{ 0x02, 0x01, 0x00};
	for (tmp = 0; tmp < 31; tmp++)
		p->usr_chr_load_mapping[tmp] = usr_chr_load_mapping[tmp];
}


/* Init data for Futaba NA202SD08FA */
void
serialVFD_load_Futaba_NA202SD08FA (Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;
	int tmp, w;

	p->customchars = 0;	/* display doesn't support custom characters */
	p->vbar_cc_offset = 5;
	p->hbar_cc_offset = 12;
	p->predefined_hbar = 1;
	p->predefined_vbar = 1;
	p->para_wait = 25;	// the display needs more delay in the parallelport mode

	/*-
	 * hardwarespecific commands:
	 *   hw_cmd[Command][data] =	{{commandlength , command 1},
	 *				.....
	 *				 {commandlength , command N}};
	 */
	const char hw_cmd[10][4] ={{2    ,0x04, 0x20},	// dark
				{2      ,0x04, 0x40},
				{2	,0x04, 0x60},
				{2	,0x04, 0xFF},	// bright
				{1	,0x16},	// pos1
				{1	,0x1B},	// move cursor (set to 0 if not supported)
				{1	,0x15},	// reset
				{2	,0x0E, 0x11},  	// init
				{1	,0x18}, 	// set user char
				{1	,0x09}}; 	// tab
	for (tmp = 0; tmp < 10; tmp++)
		for (w = 0; w < 4; w++)
			p->hw_cmd[tmp][w] = hw_cmd[tmp][w];

	/* Translates ISO 8859-1 to display charset. */
	const unsigned char charmap[] = {
		127,		/* the "filled-block"-character usually 127 */
		/* #128  = 0x80 */
		128, 129, 130, 131, 132, 133, 134, 135,
		136, 137, 138, 139, 140, 141, 142, 143,
		144, 145, 146, 147, 148, 149, 150, 151,
		152, 153, 154, 155, 156, 157, 158, 159,
		/* #160 = 0xA0 */
		160,   '!', 0xD3, 0x90, 0x9A,  'Y', 0x7C, 0x91,
		'"',   '?',  '?',  '?',  '?',  '-',  '?',  '?',
		0xB9, 0xBA, 0x98, 0x99, 0x27, 0x88,  '?',  '.',
		',',   '?',  '?',  '?',  '?', 0x9B,  '?',  '?',
		/* #192 = 0xC0 */
		0xB2, 0xA2, 0xA4,  'A', 0xA0, 0xA1, 0xA5, 0xA3,
		0xA7, 0xA6, 0xA8, 0xB4, 0xAB, 0xAA, 0xAC, 0xA9,
		'D',  0xAE, 0xB1, 0xB0, 0xBF,  'O', 0xAD,  'x',
		0xAF, 0xB7, 0xB6, 0xB8, 0xA5,  'Y',  'p', 0xB3,
		/* #224 = 0xE0 */
		0xD2, 0xC2, 0xC4,  'a', 0xC0, 0xC1, 0xC5, 0xC3,
		0xC7, 0xC6, 0xC8, 0xD4, 0xCB, 0xCA, 0xCC, 0xC9,
		'o',  0xCE, 0xD1, 0xD0, 0xDF,  'o', 0xAD, 0xBB,
		0xCF, 0xD7, 0xD6, 0xD8, 0xD5,  'y',  'p',  'y' };
	for (tmp = 0; tmp < 129; tmp++)
		p->charmap[tmp] = charmap[tmp];


	// {bytes to send, icon bit mapped to bit 0, icon bit mapped to bit 1, ...}
	const int usr_chr_dot_assignment[57] = { 0, 0 }; 	// no usercharacters
	for (tmp = 0; tmp < 57; tmp++)
		p->usr_chr_dot_assignment[tmp] = usr_chr_dot_assignment[tmp];

	// Where to place the usercharacters (0..30) in the asciicode.
	// Also used to map standardcharacters in the usercharacterspace(0..30)
	// (useful for displays with less then 30 usercharacters and predefined bars)
	const unsigned int usr_chr_mapping[31]=
	{0,0,0,0,0,0, 0xF4, 0xF4, 0xF5, 0xF6, 0xF6, 0xF7, 0, 0xF0, 0xF1, 0xF2, 0xF3};
	for (tmp = 0; tmp < 31; tmp++)
		p->usr_chr_mapping[tmp] = usr_chr_mapping[tmp];
}


/* Init data for Samsung 20S207DA4 & 20S207DA6 */
void
serialVFD_load_Samsung (Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;
	int tmp, w;

	if (p->customchars == CC_UNSET)
		p->customchars = 16;
	p->vbar_cc_offset = 0;
	p->hbar_cc_offset = 0;
	p->predefined_hbar = 0;
	p->predefined_vbar = 0;

	/*-
	 * hardwarespecific commands:
	 *   hw_cmd[Command][data] =	{{commandlength , command 1},
	 *				.....
	 *				 {commandlength , command N}};
	 */
	const char hw_cmd[10][4] = {{2    ,0x04, 0x20},	// dark
				{2      ,0x04, 0x40},
				{2	,0x04, 0x60},
				{2	,0x04, 0xFF},	// bright
				{2	,0x10, 0x00},	// pos1
				{1	,0x10,},	// move cursor
				{1	,0x1F},		// reset
				{2	,0x11,0x14},  	// init
				{1	,0x1B}, 	// set user char
				{1	,0x09}}; 	// tab
	for (tmp = 0; tmp < 10; tmp++)
		for (w = 0; w < 4; w++)
			p->hw_cmd[tmp][w] = hw_cmd[tmp][w];

	/* Translates ISO 8859-1 to display charset. */
	const unsigned char charmap[] = {
		127,		/* the "filled-block"-character usually 127 */
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
	for (tmp = 0; tmp < 129; tmp++)
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
	const unsigned int usr_chr_mapping[31] = { 0xCD, 0xCE, 0xCF, 0xEE,
						   0xFF, 0x05, 0x06, 0x07,
						   0x0B, 0x0C, 0x0E, 0x0F,
						   0x17, 0x1A, 0x1C, 0x1D };
	for (tmp = 0; tmp < 31; tmp++)
		p->usr_chr_mapping[tmp] = usr_chr_mapping[tmp];
}


/* Init data for Nixdorf BA63 & BA66 */
void
serialVFD_load_Nixdorf_BA6x (Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;
	int tmp, w;

	p->customchars = 0;	/* display doesn't support custom characters */
	p->vbar_cc_offset = 5;
	p->hbar_cc_offset = 12;
	p->predefined_hbar = 1;
	p->predefined_vbar = 1;

	/*-
	 * hardwarespecific commands:
	 *   hw_cmd[Command][data] =	{{commandlength , command 1},
	 *				.....
	 *				 {commandlength , command N}};
	 */
	const char hw_cmd[11][10] = {{0    },	// dark
				{0      },
				{0	},
				{0	},	// bright
				{6 ,0x1B, 0x5B, '1', 0x3B, '1', 0x48},	// pos1
		// pos1 command is only used (and needed), when mv_cursor command is not supported
				{0	},	// move cursor
				{4 ,0x1B, 0x5B, 0x32, 0x4A},	// reset
				{3 ,0x1B, 0x52, 0x00},  		// init
				{0	}, 	// set user char
				{0	}, 	// tab
				{2 ,0x0D, 0x0A}	// next_line
		// Next_line command is only used in line mode.
		// Line mode will be used if Next_line command is set!!!
		// Set to "0" if display supports normal_mode!
				};
	for (tmp = 0; tmp < 11; tmp++)
		for (w = 0; w < 10; w++)
			p->hw_cmd[tmp][w] = hw_cmd[tmp][w];

	/* Translates ISO 8859-1 to display charset. */
	const unsigned char charmap[] = {
		0xDB,		/* the "filled-block"-character usually 127 */
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
	for (tmp = 0; tmp < 129; tmp++)
		p->charmap[tmp] = charmap[tmp];


	// Where to place the usercharacters (0..30) in the asciicode.
	// Also used to map standardcharacters in the usercharacterspace(0..30)
	// (useful for displays with less then 30 usercharacters and predefined bars)
	const unsigned int usr_chr_mapping[31]=
	{0,0,0,0,0,0, ' ', ' ', 0xDC, 0xDC, 0xDC, 0xDB, 0, ' ', 0xDD, 0xDD, 0xDB};
	for (tmp = 0; tmp < 31; tmp++)
		p->usr_chr_mapping[tmp] = usr_chr_mapping[tmp];
}
