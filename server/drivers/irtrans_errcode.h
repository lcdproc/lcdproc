/*
 * irtrans driver
 *
 * Displays LCD screens, one after another; suitable for hard-copy
 * terminals.
 *
 * Copyright (C) 1998-2007 <info@irtrans.de>
 *
 * Copyright (C) 2007 Phant0m <phantom@netkeke.com>
 * porting the LCDproc 0.4.3 code to LCDproc 0.5.1
 *
 * Inspired by:
 *  TextMode driver (LCDproc authors)
 *  irtrans driver (Irtrans)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

#define ERR_OPEN			1
#define ERR_RESET			2
#define ERR_VERSION			3
#define ERR_TIMEOUT			4
#define	ERR_READVERSION		5

#define ERR_DBOPENINPUT		11
#define ERR_REMOTENOTFOUND	12
#define ERR_COMMANDNOTFOUND	13
#define ERR_TIMINGNOTFOUND	14
#define ERR_OPENASCII		15
#define ERR_NODATABASE		16
#define ERR_OPENUSB			17
#define ERR_RESEND			18
#define ERR_TOGGLE_DUP		19
#define ERR_DBOPENINCLUDE	20
#define ERR_NOFILEOPEN		21
#define	ERR_FLOCK			22
#define ERR_STTY			23
#define ERR_HOTCODE			24
#define ERR_NOTIMING		25

#define ERR_OPENSOCKET		100
#define ERR_BINDSOCKET		101
#define ERR_FINDHOST		103
#define ERR_CONNECT			104
#define ERR_SEND			105
#define ERR_RECV			106


#define FATAL				1
#define IR					2
#define	IRTIMEOUT			3

void GetError(int res, char st[]);
