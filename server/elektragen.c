// clang-format off


// clang-format on
/**
 * @file
 *
 * This file was automatically generated using `kdb gen elektra`.
 * Any changes will be overwritten, when the file is regenerated.
 *
 * @copyright BSD Zero Clause License
 *
 *     Copyright (C) 2019 Elektra Initiative (https://libelektra.org)
 *
 *     Permission to use, copy, modify, and/or distribute this software for any
 *     purpose with or without fee is hereby granted.
 *
 *     THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 *     REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 *     FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 *     INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 *     LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 *     OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 *     PERFORMANCE OF THIS SOFTWARE.
 */

#include "elektragen.h"



#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <kdbhelper.h>
#include <kdbinvoke.h>
#include <kdbopts.h>

#include <elektra/conversion.h>

static Key * helpKey = NULL;


/**
 * Initializes an instance of Elektra for the application '/sw/lcdproc/lcdd/#0/current'.
 *
 * This can be invoked as many times as you want, however it is not a cheap operation,
 * so you should try to reuse the Elektra handle as much as possible.
 *
 * @param elektra A reference to where the Elektra instance shall be stored.
 *                Has to be disposed of with elektraClose().
 * @param error   A reference to an ElektraError pointer. Will be passed to elektraOpen().
 *
 * @retval 0  on success, @p elektra will be set, @p error will be unchanged
 * @retval -1 on error, @p elektra will be unchanged, @p error will be set
 * @retval 1  specload mode, exit as soon as possible and must DO NOT write anything to stdout,
 *            @p elektra and @p error are both unchanged
 * @retval 2  help mode, '-h' or '--help' was specified call printHelpMessage and exit
 *            @p elektra and @p error are both unchanged
 *            IMPORTANT: there will be memory leaks, if you don't call printHelpMessage !!
 *
 * @see elektraOpen
 */// 
int loadConfiguration (Elektra ** elektra, ElektraError ** error)
{
	KeySet * defaults = ksNew (595,
	keyNew("", KEY_META, "infos/plugins", "ini validation type", KEY_META, "mountpoint", "LCDd.conf", KEY_END),
	keyNew ("/bayrad/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a bayrad driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "BayradDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/bayrad/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/bayrad/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/bayrad/#/device", KEY_VALUE, "/dev/lcd", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/bayrad/#/file", KEY_VALUE, "bayrad", KEY_META, "default", "bayrad", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/bayrad/#/offbrightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/bayrad/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/bayrad/#/speed", KEY_VALUE, "9600", KEY_META, "check/range", "1200, 2400, 9600, 19200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "9600", KEY_META, "description", "Set the communication speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/cfontz/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a CFontz driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "CFontzDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/cfontz/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial brightness", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/cfontz/#/contrast", KEY_VALUE, "350", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "350", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/cfontz/#/device", KEY_VALUE, "/dev/lcd", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/cfontz/#/file", KEY_VALUE, "CFontz", KEY_META, "default", "CFontz", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/cfontz/#/newfirmware", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Set the firmware version (New means >= 2.0)", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/cfontz/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Set the initial off-brightness. This value is used when the display is normally switched off in case LCDd is inactive", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/cfontz/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/cfontz/#/size", KEY_VALUE, "20x4", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "Select the LCD size", KEY_META, "type", "string", KEY_END),
	keyNew ("/cfontz/#/speed", KEY_VALUE, "9600", KEY_META, "check/range", "1200, 2400, 9600, 19200, 115200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "9600", KEY_META, "description", "Set the communication speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/cfontzpacket/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a CFontzPacket driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "CFontzPacketDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/cfontzpacket/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial brightness", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/cfontzpacket/#/contrast", KEY_VALUE, "350", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "350", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/cfontzpacket/#/device", KEY_VALUE, "/dev/lcd", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "type", "string", KEY_END),
	keyNew ("/cfontzpacket/#/file", KEY_VALUE, "CFontzPacket", KEY_META, "default", "CFontzPacket", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/cfontzpacket/#/model", KEY_VALUE, "633", KEY_META, "check/enum", "#3", KEY_META, "check/enum/#0", "533", KEY_META, "check/enum/#1", "631", KEY_META, "check/enum/#2", "633", KEY_META, "check/enum/#3", "635", KEY_META, "check/type", "enum", KEY_META, "default", "633", KEY_META, "description", "Select the LCD model", KEY_META, "gen/enum/type", "CFontzPacketModel", KEY_META, "type", "enum", KEY_END),
	keyNew ("/cfontzpacket/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Set the initial off-brightness. This value is used when the display is normally switched off in case LCDd is inactive", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/cfontzpacket/#/oldfirmware", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Very old 633 firmware versions do not support partial screen updates using \'Send Data to LCD\' command (31). For those devices it may be necessary to enable this flag", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/cfontzpacket/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/cfontzpacket/#/size", KEY_VALUE, "20x4", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "Override the LCD size known for the selected model. Usually setting this value should not be necessary.", KEY_META, "type", "string", KEY_END),
	keyNew ("/cfontzpacket/#/speed", KEY_VALUE, "115200", KEY_META, "check/range", "19200, 115200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "115200", KEY_META, "description", "Override the default communication speed known for the selected model. Default value depends on model.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/cfontzpacket/#/usb", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Enable the USB flag if the device is connected to an USB port. For serial ports leave it disabled.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/curses/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a curses driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "CursesDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/curses/#/background", KEY_VALUE, "cyan", KEY_META, "check/enum", "#7", KEY_META, "check/enum/#0", "red", KEY_META, "check/enum/#1", "black", KEY_META, "check/enum/#2", "green", KEY_META, "check/enum/#3", "yellow", KEY_META, "check/enum/#4", "blue", KEY_META, "check/enum/#5", "magenta", KEY_META, "check/enum/#6", "cyan", KEY_META, "check/enum/#7", "white", KEY_META, "check/type", "enum", KEY_META, "default", "cyan", KEY_META, "description", "background color when \"backlight\" is off", KEY_META, "gen/enum/type", "CursesColor", KEY_META, "type", "enum", KEY_END),
	keyNew ("/curses/#/backlight", KEY_VALUE, "red", KEY_META, "check/enum", "#7", KEY_META, "check/enum/#0", "red", KEY_META, "check/enum/#1", "black", KEY_META, "check/enum/#2", "green", KEY_META, "check/enum/#3", "yellow", KEY_META, "check/enum/#4", "blue", KEY_META, "check/enum/#5", "magenta", KEY_META, "check/enum/#6", "cyan", KEY_META, "check/enum/#7", "white", KEY_META, "check/type", "enum", KEY_META, "default", "red", KEY_META, "description", "background color when \"backlight\" is on", KEY_META, "gen/enum/type", "CursesColor", KEY_META, "type", "enum", KEY_END),
	keyNew ("/curses/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/curses/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/curses/#/drawborder", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "draw Border", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/curses/#/file", KEY_VALUE, "curses", KEY_META, "default", "curses", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/curses/#/foreground", KEY_VALUE, "blue", KEY_META, "check/enum", "#7", KEY_META, "check/enum/#0", "red", KEY_META, "check/enum/#1", "black", KEY_META, "check/enum/#2", "green", KEY_META, "check/enum/#3", "yellow", KEY_META, "check/enum/#4", "blue", KEY_META, "check/enum/#5", "magenta", KEY_META, "check/enum/#6", "cyan", KEY_META, "check/enum/#7", "white", KEY_META, "check/type", "enum", KEY_META, "default", "blue", KEY_META, "description", "Color settings", KEY_META, "gen/enum/type", "CursesColor", KEY_META, "type", "enum", KEY_END),
	keyNew ("/curses/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/curses/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/curses/#/size", KEY_VALUE, "20x4", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "display size", KEY_META, "type", "string", KEY_END),
	keyNew ("/curses/#/topleftx", KEY_VALUE, "7", KEY_META, "check/type", "unsigned_short", KEY_META, "check/validation", "([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Must be a positive number", KEY_META, "default", "7", KEY_META, "description", "What position (X,Y) to start the left top corner at...", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/curses/#/toplefty", KEY_VALUE, "7", KEY_META, "check/type", "unsigned_short", KEY_META, "check/validation", "([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Must be a positive number", KEY_META, "default", "7", KEY_META, "description", "What position (X,Y) to start the left top corner at...", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/curses/#/useacs", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "use ASC symbols for icons & bars", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/cwlnx/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a CwLnx driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "CwLnxDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/cwlnx/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/cwlnx/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/cwlnx/#/device", KEY_VALUE, "/dev/lcd", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/cwlnx/#/file", KEY_VALUE, "CwLnx", KEY_META, "default", "CwLnx", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/cwlnx/#/keymap_a", KEY_VALUE, "Up", KEY_META, "check/type", "string", KEY_META, "default", "Up", KEY_META, "description", "If you have a non-standard keypad you can associate any keystrings to keys.\nThere are 6 input keys in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n\nThe built-in default mapping hardcoded in the driver.\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("/cwlnx/#/keymap_b", KEY_VALUE, "Down", KEY_META, "check/type", "string", KEY_META, "default", "Down", KEY_META, "description", "If you have a non-standard keypad you can associate any keystrings to keys.\nThere are 6 input keys in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n\nThe built-in default mapping hardcoded in the driver.\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("/cwlnx/#/keymap_c", KEY_VALUE, "Left", KEY_META, "check/type", "string", KEY_META, "default", "Left", KEY_META, "description", "If you have a non-standard keypad you can associate any keystrings to keys.\nThere are 6 input keys in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n\nThe built-in default mapping hardcoded in the driver.\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("/cwlnx/#/keymap_d", KEY_VALUE, "Right", KEY_META, "check/type", "string", KEY_META, "default", "Right", KEY_META, "description", "If you have a non-standard keypad you can associate any keystrings to keys.\nThere are 6 input keys in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n\nThe built-in default mapping hardcoded in the driver.\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("/cwlnx/#/keymap_e", KEY_VALUE, "Enter", KEY_META, "check/type", "string", KEY_META, "default", "Enter", KEY_META, "description", "If you have a non-standard keypad you can associate any keystrings to keys.\nThere are 6 input keys in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n\nThe built-in default mapping hardcoded in the driver.\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("/cwlnx/#/keymap_f", KEY_VALUE, "Escape", KEY_META, "check/type", "string", KEY_META, "default", "Escape", KEY_META, "description", "If you have a non-standard keypad you can associate any keystrings to keys.\nThere are 6 input keys in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n\nThe built-in default mapping hardcoded in the driver.\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("/cwlnx/#/keypad", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If you have a keypad connected. Keypad layout is currently not configureable from the config file.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/cwlnx/#/keypad_test_mode", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "permits one to test keypad assignment", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/cwlnx/#/model", KEY_VALUE, "12232", KEY_META, "check/enum", "#2", KEY_META, "check/enum/#0", "12232", KEY_META, "check/enum/#1", "12832", KEY_META, "check/enum/#2", "1602", KEY_META, "check/type", "enum", KEY_META, "default", "12232", KEY_META, "description", "Select the LCD model", KEY_META, "gen/enum/type", "CwLnxModel", KEY_META, "type", "enum", KEY_END),
	keyNew ("/cwlnx/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/cwlnx/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/cwlnx/#/size", KEY_VALUE, "20x4", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "Select the LCD size. Default depends on model: \n12232: 20x4 \n12832: 21x4 \n1602: 16x2", KEY_META, "type", "string", KEY_END),
	keyNew ("/cwlnx/#/speed", KEY_VALUE, "19200", KEY_META, "check/range", "9600, 19200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "19200", KEY_META, "description", "Set the communication speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/ea65/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a ea65 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Ea65DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/ea65/#/brightness", KEY_VALUE, "500", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "500", KEY_META, "description", "As the VFD is self luminescent we don\'t have a backlight\nBut we can use the backlight functions to control the front LEDs\nBrightness 0 to 299 -> LEDs off\nBrightness 300 to 699 -> LEDs half bright\nBrightness 700 to 1000 -> LEDs full bright", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/ea65/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/ea65/#/file", KEY_VALUE, "ea65", KEY_META, "default", "ea65", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/ea65/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "OffBrightness is the the value used for the \'backlight off\' state", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/ea65/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/eyeboxone/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a EyeboxOne driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "EyeboxOneDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/eyeboxone/#/backlight", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Switch on the backlight\?", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/eyeboxone/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/eyeboxone/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/eyeboxone/#/cursor", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Switch on the cursor\?", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/eyeboxone/#/device", KEY_VALUE, "/dev/ttyS1", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyS1", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/eyeboxone/#/downkey", KEY_VALUE, "B", KEY_META, "check/type", "string", KEY_META, "default", "B", KEY_META, "description", "Enter Key is a \r character, so it\'s hardcoded in the driver", KEY_META, "type", "string", KEY_END),
	keyNew ("/eyeboxone/#/escapekey", KEY_VALUE, "P", KEY_META, "check/type", "string", KEY_META, "default", "P", KEY_META, "description", "Enter Key is a \r character, so it\'s hardcoded in the driver", KEY_META, "type", "string", KEY_END),
	keyNew ("/eyeboxone/#/file", KEY_VALUE, "EyeboxOne", KEY_META, "default", "EyeboxOne", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/eyeboxone/#/keypad_test_mode", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "You can find out which key of your display sends which\ncharacter by setting keypad_test_mode to yes and running\nLCDd. LCDd will output all characters it receives.\nAfterwards you can modify the settings above and set\nkeypad_set_mode to no again.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/eyeboxone/#/leftkey", KEY_VALUE, "D", KEY_META, "check/type", "string", KEY_META, "default", "D", KEY_META, "description", "Enter Key is a \r character, so it\'s hardcoded in the driver", KEY_META, "type", "string", KEY_END),
	keyNew ("/eyeboxone/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/eyeboxone/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/eyeboxone/#/rightkey", KEY_VALUE, "C", KEY_META, "check/type", "string", KEY_META, "default", "C", KEY_META, "description", "Enter Key is a \r character, so it\'s hardcoded in the driver", KEY_META, "type", "string", KEY_END),
	keyNew ("/eyeboxone/#/size", KEY_VALUE, "20x4", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "Set the display size", KEY_META, "type", "string", KEY_END),
	keyNew ("/eyeboxone/#/speed", KEY_VALUE, "19200", KEY_META, "check/range", "1200, 2400, 9600, 19200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "19200", KEY_META, "description", "Set the communication speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/eyeboxone/#/upkey", KEY_VALUE, "A", KEY_META, "check/type", "string", KEY_META, "default", "A", KEY_META, "description", "Enter Key is a \r character, so it\'s hardcoded in the driver", KEY_META, "type", "string", KEY_END),
	keyNew ("/futaba/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a futaba driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "FutabaDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/futaba/#/file", KEY_VALUE, "futaba", KEY_META, "default", "futaba", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/g15/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a g15 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "G15DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/g15/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/g15/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/g15/#/file", KEY_VALUE, "g15", KEY_META, "default", "g15", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/g15/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/g15/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/g15/#/size", KEY_VALUE, "20x5", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x5", KEY_META, "description", "Display size (currently unused)", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a glcd driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "GlcdDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/glcd/#/bidirectional", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Use LPT port in bi-directional mode. This should work on most LPT port and is required for proper timing!", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/glcd/#/brightness", KEY_VALUE, "800", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "800", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/glcd/#/cellsize", KEY_VALUE, "6x8", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "6x8", KEY_META, "description", "Width and height of a character cell in pixels. This value is only used the driver has been compiled with FreeType and it is enabled. Otherwise the default 6x8 cell is used.", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#/connectiontype", KEY_VALUE, "t6963", KEY_META, "check/enum", "#6", KEY_META, "check/enum/#0", "t6963", KEY_META, "check/enum/#1", "png", KEY_META, "check/enum/#2", "serdisplib", KEY_META, "check/enum/#3", "glcd2usb", KEY_META, "check/enum/#4", "x11", KEY_META, "check/enum/#5", "picolcdgfx", KEY_META, "check/enum/#6", "xyz", KEY_META, "check/type", "enum", KEY_META, "default", "t6963", KEY_META, "description", "Select what type of connection. See documentation for types.", KEY_META, "gen/enum/type", "GlcdConnectionType", KEY_META, "type", "enum", KEY_END),
	keyNew ("/glcd/#/contrast", KEY_VALUE, "600", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "600", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/glcd/#/delaybus", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Insert additional delays into reads / writes.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/glcd/#/file", KEY_VALUE, "glcd", KEY_META, "default", "glcd", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#/fonthasicons", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Some fonts miss the Unicode characters used to represent icons. In this case the built-in 5x8 font can used if this option is turned off.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/glcd/#/keymap_a", KEY_VALUE, "Up", KEY_META, "check/type", "string", KEY_META, "default", "Up", KEY_META, "description", "Assign key strings to keys. There may be up to 16 keys numbered \'A\' to \'Z\'.\nBy default keys \'A\' to \'F\' are assigned Up, Down, Left, Right, Enter, Escape.", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#/keymap_b", KEY_VALUE, "Down", KEY_META, "check/type", "string", KEY_META, "default", "Down", KEY_META, "description", "Assign key strings to keys. There may be up to 16 keys numbered \'A\' to \'Z\'.\nBy default keys \'A\' to \'F\' are assigned Up, Down, Left, Right, Enter, Escape.", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#/keymap_c", KEY_VALUE, "Left", KEY_META, "check/type", "string", KEY_META, "default", "Left", KEY_META, "description", "Assign key strings to keys. There may be up to 16 keys numbered \'A\' to \'Z\'.\nBy default keys \'A\' to \'F\' are assigned Up, Down, Left, Right, Enter, Escape.", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#/keymap_d", KEY_VALUE, "Right", KEY_META, "check/type", "string", KEY_META, "default", "Right", KEY_META, "description", "Assign key strings to keys. There may be up to 16 keys numbered \'A\' to \'Z\'.\nBy default keys \'A\' to \'F\' are assigned Up, Down, Left, Right, Enter, Escape.", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#/keymap_e", KEY_VALUE, "Enter", KEY_META, "check/type", "string", KEY_META, "default", "Enter", KEY_META, "description", "Assign key strings to keys. There may be up to 16 keys numbered \'A\' to \'Z\'.\nBy default keys \'A\' to \'F\' are assigned Up, Down, Left, Right, Enter, Escape.", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#/keymap_f", KEY_VALUE, "Escape", KEY_META, "check/type", "string", KEY_META, "default", "Escape", KEY_META, "description", "Assign key strings to keys. There may be up to 16 keys numbered \'A\' to \'Z\'.\nBy default keys \'A\' to \'F\' are assigned Up, Down, Left, Right, Enter, Escape.", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#/keyrepeatdelay", KEY_VALUE, "500", KEY_META, "check/range", "0-3000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "500", KEY_META, "description", "Time (ms) from first key report to first repeat. Set to 0 to disable repeated key reports.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/glcd/#/keyrepeatinterval", KEY_VALUE, "500", KEY_META, "check/range", "0-3000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "500", KEY_META, "description", "Time (ms) between repeated key reports. Ignored if KeyRepeatDelay is disabled (set to zero).", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/glcd/#/normal_font", KEY_VALUE, "/usr/local/lib/X11/fonts/TTF/andalemo.ttf", KEY_META, "check/type", "string", KEY_META, "default", "/usr/local/lib/X11/fonts/TTF/andalemo.ttf", KEY_META, "description", "Path to font file to use for FreeType rendering. This font must be monospace and should contain some special Unicode characters like arrows (Andale Mono is recommended and can be fetched at http://corefonts.sf.net).", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#/offbrightness", KEY_VALUE, "100", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "100", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/glcd/#/picolcdgfx_inverted", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Inverts the pixels.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/glcd/#/picolcdgfx_keytimeout", KEY_VALUE, "125", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9][0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Number must be any positive integer >0", KEY_META, "default", "125", KEY_META, "description", "Time in ms for usb_read to wait on a key press.", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#/port", KEY_VALUE, "0x378", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([2-3][0-9A-F]{2}|400)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid port declaration. legal: 0x200 - 0x400", KEY_META, "default", "0x378", KEY_META, "description", "Parallel port to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/glcd/#/serdisp_device", KEY_VALUE, "/dev/ppi0", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ppi0", KEY_META, "description", "The display device to use, e.g. serraw:/dev/ttyS0, parport:/dev/parport0 or USB:07c0/1501", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#/serdisp_name", KEY_VALUE, "t6963", KEY_META, "check/type", "string", KEY_META, "default", "t6963", KEY_META, "description", "Name of the underlying serdisplib driver, e.g. ctinclud. See", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#/serdisp_options", KEY_VALUE, "INVERT=1", KEY_META, "check/type", "string", KEY_META, "default", "INVERT=1", KEY_META, "description", "Options string to pass to serdisplib during initialization. Use\nthis to set any display related options (e.g. wiring). The display size is\nalways set based on the Size configured above! By default, no options are\nset.\nImportant: The value must be quoted as it contains equal signs!", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#/size", KEY_VALUE, "128x64", KEY_META, "check/type", "string", KEY_META, "check/validation", "(640|[1-9]|[1-9][0-9]|[1-5][0-9][0-9]|6[0-3][0-9])x(480|[1-9]|[1-9][0-9]|[1-3][0-9][0-9]|4[0-7][0-9])", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. legal: 1x1 - 640x480", KEY_META, "default", "128x64", KEY_META, "description", "Width and height of the display in pixel. The supported sizes may depend on the ConnectionType", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#/useft2", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If LCDproc has been compiled with FreeType 2 support this option can be used to turn if off intentionally.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/glcd/#/x11_backlightcolor", KEY_VALUE, "0x80FF80", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x[0-9A-F]{6}", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Colors are in RRGGBB format prefixed with \"0x\"", KEY_META, "default", "0x80FF80", KEY_META, "description", "The color of the backlight as full brightness.", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#/x11_border", KEY_VALUE, "20", KEY_META, "check/type", "unsigned_long", KEY_META, "check/validation", "^([1-9]\\d*|0)$", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Must be a positive number", KEY_META, "default", "20", KEY_META, "description", "Adds a border (empty space) around the LCD portion of X11 window.", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("/glcd/#/x11_inverted", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Inverts the pixels.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/glcd/#/x11_pixelcolor", KEY_VALUE, "0x000000", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x[0-9A-F]{6}", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Colors are in RRGGBB format prefixed with \"0x\"", KEY_META, "default", "0x000000", KEY_META, "description", "The color of each dot at full contrast.", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcd/#/x11_pixelsize", KEY_VALUE, "3+1", KEY_META, "check/type", "string", KEY_META, "check/validation", "\\d+\\+\\d+", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "[number]+[number] has to be provided, eg: 3+1 or 5+2", KEY_META, "default", "3+1", KEY_META, "description", "Each LCD dot is drawn in the X window as a filled rectangle of this size\n    plus a gap between each filled rectangle. A PixelSize of 3+1\n    would draw a 3x3 filled rectangle with a gap of 1 pixel to the right and\n    bottom, effectively using a 4x4 area of the window. Default is 3+1.", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcdlib/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a glcdlib driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "GlcdlibDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/glcdlib/#/CharEncoding", KEY_VALUE, "iso8859-2", KEY_META, "check/type", "string", KEY_META, "default", "iso8859-2", KEY_META, "description", "character encoding to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcdlib/#/backlight", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/glcdlib/#/brightness", KEY_VALUE, "50", KEY_META, "check/range", "0-100", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "50", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/glcdlib/#/contrast", KEY_VALUE, "50", KEY_META, "check/range", "0-100", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "50", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/glcdlib/#/driver", KEY_VALUE, "image", KEY_META, "check/type", "string", KEY_META, "default", "image", KEY_META, "description", "\nSpecify which graphical display supported by graphlcd-base to use.\n    Legal values for <replaceable>GRAPHLCD-DRIVER</replaceable> are\n    specified in graphlcd\'s configuration file <filename>/etc/graphlcd.conf</filename>.\n    For graphlcd 0.13 they comprise avrctl, framebuffer, gu140x32f, gu256x64-372, \n    gu256x64C-3xx0, hd61830, image, ks0108, noritake800, sed1330, sed1520, serdisp, simlcd, t6963c\n", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcdlib/#/file", KEY_VALUE, "glcdlib", KEY_META, "default", "glcdlib", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcdlib/#/fontfile", KEY_VALUE, "/usr/share/fonts/corefonts/courbd.ttf", KEY_META, "check/type", "string", KEY_META, "default", "/usr/share/fonts/corefonts/courbd.ttf", KEY_META, "description", "path to font file to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcdlib/#/invert", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/glcdlib/#/minfontfacesize", KEY_VALUE, "7x12", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "7x12", KEY_META, "description", "path to font file to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcdlib/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/glcdlib/#/pixelshiftx", KEY_VALUE, "0", KEY_META, "check/type", "short", KEY_META, "check/validation", "([0-9]+)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Must be a positive number", KEY_META, "default", "0", KEY_META, "type", "short", KEY_END),
	keyNew ("/glcdlib/#/pixelshifty", KEY_VALUE, "2", KEY_META, "check/type", "short", KEY_META, "check/validation", "([0-9]+)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Must be a positive number", KEY_META, "default", "2", KEY_META, "type", "short", KEY_END),
	keyNew ("/glcdlib/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/glcdlib/#/showbigborder", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/glcdlib/#/showdebugframe", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/glcdlib/#/showthinborder", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/glcdlib/#/textresolution", KEY_VALUE, "16x4", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x4", KEY_META, "description", "Text resolution in fixed width characters.\n(if it won\'t fit according to available physical pixel resolution\nand the minimum available font face size in pixels, then\n\'DebugBorder\' will automatically be turned on)", KEY_META, "type", "string", KEY_END),
	keyNew ("/glcdlib/#/upsidedown", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/glcdlib/#/useft2", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "no=use graphlcd bitmap fonts (they have only one size / font file)\nyes=use fonts supported by FreeType2 (needs Freetype2 support in\nlibglcdprocdriver and its dependants)", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/glk/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a glk driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "GlkDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/glk/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/glk/#/contrast", KEY_VALUE, "500", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "500", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/glk/#/device", KEY_VALUE, "/dev/lcd", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "select the serial device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/glk/#/file", KEY_VALUE, "glk", KEY_META, "default", "glk", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/glk/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/glk/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/glk/#/speed", KEY_VALUE, "19200", KEY_META, "check/range", "9600, 19200, 38400, 57600, 115200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "19200", KEY_META, "description", "set the serial port speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/hd44780/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a hd44780 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Hd44780DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/hd44780/#/backlight", KEY_VALUE, "none", KEY_META, "check/enum", "#8", KEY_META, "check/enum/#0", "none", KEY_META, "check/enum/#1", "external", KEY_META, "check/enum/#2", "internal", KEY_META, "check/enum/#3", "internalCmds", KEY_META, "check/type", "enum", KEY_META, "default", "none", KEY_META, "description", "Specify if you have a switchable backlight and if yes, can select method for turning it on/off:\n#\n- none - no switchable backlight is available.\n- external - use external pin or any other method defined with ConnectionType backlight\n         handling.\n- internal - means that backlight is handled using internal commands according\n         to selected display model (with Model option). Depending on model,\n         Brightness and OffBrightness options can be taken into account.\n- internalCmds - means that commands for turning on and off backlight are given\n         with extra options BacklightOnCmd and BacklightOffCmd, which would be treated\n         as catch up (last resort) for other types of displays which have similar features.\n#\nYou can provide multiple occurences of this option to use more than one method.\nDefault is model specific: Winstar OLED and PT6314 VFD enables internal backlight mode,\nfor others it is set to none.", KEY_META, "gen/enum/type", "HD44780Backlight", KEY_META, "type", "enum", KEY_END),
	keyNew ("/hd44780/#/backlightcmdoff", KEY_VALUE, "0x1234", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x[0-9A-F]{4}", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "4 bytes can be encoded, as integer number in big-endian order (0x0000-0xFFFF)", KEY_META, "default", "0x1234", KEY_META, "description", "Commands for disabling internal backlight for use with Backlight=internalCmds.\nUp to 4 bytes can be encoded, as integer number in big-endian order.", KEY_META, "type", "string", KEY_END),
	keyNew ("/hd44780/#/backlightcmdon", KEY_VALUE, "0x1223", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x[0-9A-F]{4}", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "4 bytes can be encoded, as integer number in big-endian order (0x0000-0xFFFF)", KEY_META, "default", "0x1223", KEY_META, "description", "Commands for enabling internal backlight for use with Backlight=internalCmds.\nUp to 4 bytes can be encoded, as integer number in big-endian order.\n#\nNOTE: this is advanced option, if command contains bits other than only brighness handling,\nthey must be set accordingly to not disrupt display state. If for example \'FUNCTION SET\' command\nis used for this purpose, bits of interface length (4-bit / 8-bit) must be set according to\nselected ConnectionType.", KEY_META, "type", "string", KEY_END),
	keyNew ("/hd44780/#/brightness", KEY_VALUE, "800", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "800", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/hd44780/#/charmap", KEY_VALUE, "hd44780_default", KEY_META, "check/enum", "#7", KEY_META, "check/enum/#0", "hd44780_default", KEY_META, "check/enum/#1", "hd44780_euro", KEY_META, "check/enum/#2", "ea_ks0073", KEY_META, "check/enum/#3", "sed1278f_0b", KEY_META, "check/enum/#4", "hd44780_koi8_r", KEY_META, "check/enum/#5", "hd44780_cp1251", KEY_META, "check/enum/#6", "hd44780_8859_5", KEY_META, "check/enum/#7", "upd16314", KEY_META, "check/type", "enum", KEY_META, "default", "hd44780_default", KEY_META, "description", "Character map to to map ISO-8859-1 to the LCD\'s character set. (hd44780_koi8_r, hd44780_cp1251, hd44780_8859_5, upd16314 and weh001602a_1\nare possible if compiled with additional charmaps)", KEY_META, "gen/enum/type", "HD44780Charmap", KEY_META, "type", "enum", KEY_END),
	keyNew ("/hd44780/#/connectiontype", KEY_VALUE, "4bit", KEY_META, "check/enum", "#_27", KEY_META, "check/enum/#0", "4bit", KEY_META, "check/enum/#1", "8bit", KEY_META, "check/enum/#2", "winamp", KEY_META, "check/enum/#3", "lcm162", KEY_META, "check/enum/#4", "serialLpt", KEY_META, "check/enum/#5", "picanlcd", KEY_META, "check/enum/#6", "lcdserializer", KEY_META, "check/enum/#7", "los-panel", KEY_META, "check/enum/#8", "vdr-lcd", KEY_META, "check/enum/#9", "vdr-wakeup", KEY_META, "check/enum/#_10", "ezio", KEY_META, "check/enum/#_11", "pertelian", KEY_META, "check/enum/#_12", "lis2", KEY_META, "check/enum/#_13", "mplay", KEY_META, "check/enum/#_14", "usblcd", KEY_META, "check/enum/#_15", "bwctusb", KEY_META, "check/enum/#_16", "lcd2usb", KEY_META, "check/enum/#_17", "usbtiny", KEY_META, "check/enum/#_18", "uss720", KEY_META, "check/enum/#_19", "USB-4-all", KEY_META, "check/enum/#_20", "ftdi", KEY_META, "check/enum/#_21", "i2c", KEY_META, "check/enum/#_22", "piplate", KEY_META, "check/enum/#_23", "spi", KEY_META, "check/enum/#_24", "pifacecad", KEY_META, "check/enum/#_25", "ethlcd", KEY_META, "check/enum/#_26", "raspberrypi", KEY_META, "check/enum/#_27", "gpio", KEY_META, "check/type", "enum", KEY_META, "default", "4bit", KEY_META, "description", "Select what type of connection. See documentation for available types:\n  https://github.com/lcdproc/lcdproc/blob/master/docs/lcdproc-user/drivers/hd44780.docbook", KEY_META, "gen/enum/type", "HD44780ConnectionType", KEY_META, "type", "enum", KEY_END),
	keyNew ("/hd44780/#/contrast", KEY_VALUE, "800", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "800", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/hd44780/#/delaybus", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "You can reduce the inserted delays by setting this to false.\nOn fast PCs it is possible your LCD does not respond correctly.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/hd44780/#/delaymult", KEY_VALUE, "1", KEY_META, "check/range", "1, 2, 4, 8, 16", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1", KEY_META, "description", "If your display is slow and cannot keep up with the flow of data from\nLCDd, garbage can appear on the LCDd. Set this delay factor to 2 or 4\nto increase the delays.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/hd44780/#/device", KEY_VALUE, "/dev/lcd", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Device of the serial, I2C, or SPI interface", KEY_META, "type", "string", KEY_END),
	keyNew ("/hd44780/#/extendedmode", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "If you have an HD66712, a KS0073 or another controller with \'extended mode\',\nset this flag to get into 4-line mode. On displays with just two lines, do\nnot set this flag.\nAs an additional restriction, controllers with and without extended mode\nAND 4 lines cannot be mixed for those connection types that support more\nthan one display!\nNOTE: This option is deprecated in favour of choosing Model=extended option.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/hd44780/#/file", KEY_VALUE, "hd44780", KEY_META, "default", "hd44780", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/hd44780/#/fontbank", KEY_VALUE, "0", KEY_META, "check/enum", "#3", KEY_META, "check/enum/#0", "0", KEY_META, "check/enum/#1", "1", KEY_META, "check/enum/#2", "2", KEY_META, "check/enum/#3", "3", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Font bank to be used for some displays such as the WINSTAR WEH001602A\n0: English/Japanese (default)\n1: Western Europe I\n2: English/Rusian\n3: Western Europe II", KEY_META, "gen/enum/type", "HD44780Fontbank", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/hd44780/#/keepalivedisplay", KEY_VALUE, "0", KEY_META, "check/range", "0-10", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Some displays (e.g. vdr-wakeup) need a message from the driver to that it\nis still alive. When set to a value bigger then null the character in the\nupper left corner is updated every <KeepAliveDisplay> seconds.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/hd44780/#/keymatrix_4_1", KEY_VALUE, "Enter", KEY_META, "check/type", "string", KEY_META, "default", "Enter", KEY_META, "description", "If you have a keypad you can assign keystrings to the keys. See\ndocumentation for used terms and how to wire it. For example to give directly connected\nkey 4 the string \'Enter\', use:\n   KeyDirect_4=Enter For matrix keys use the\nX and Y coordinates of the key: \n  KeyMatrix_1_3=Enter", KEY_META, "type", "string", KEY_END),
	keyNew ("/hd44780/#/keymatrix_4_2", KEY_VALUE, "Up", KEY_META, "check/type", "string", KEY_META, "default", "Up", KEY_META, "description", "If you have a keypad you can assign keystrings to the keys. See\ndocumentation for used terms and how to wire it. For example to give directly connected\nkey 4 the string \'Enter\', use:\n   KeyDirect_4=Enter For matrix keys use the\nX and Y coordinates of the key: \n  KeyMatrix_1_3=Enter", KEY_META, "type", "string", KEY_END),
	keyNew ("/hd44780/#/keymatrix_4_3", KEY_VALUE, "Down", KEY_META, "check/type", "string", KEY_META, "default", "Down", KEY_META, "description", "If you have a keypad you can assign keystrings to the keys. See\ndocumentation for used terms and how to wire it. For example to give directly connected\nkey 4 the string \'Enter\', use:\n   KeyDirect_4=Enter For matrix keys use the\nX and Y coordinates of the key: \n  KeyMatrix_1_3=Enter", KEY_META, "type", "string", KEY_END),
	keyNew ("/hd44780/#/keymatrix_4_4", KEY_VALUE, "Escape", KEY_META, "check/type", "string", KEY_META, "default", "Escape", KEY_META, "description", "If you have a keypad you can assign keystrings to the keys. See\ndocumentation for used terms and how to wire it. For example to give directly connected\nkey 4 the string \'Enter\', use:\n   KeyDirect_4=Enter For matrix keys use the\nX and Y coordinates of the key: \n  KeyMatrix_1_3=Enter", KEY_META, "type", "string", KEY_END),
	keyNew ("/hd44780/#/keypad", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "If you have a keypad connected.\nYou may also need to configure the keypad layout further on in this file.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/hd44780/#/lastline", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Specifies if the last line is pixel addressable (yes) or it controls an\nunderline effect (no).", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/hd44780/#/lineaddress", KEY_VALUE, "0x20", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a line Address. eg. 0x20, 0x10", KEY_META, "default", "0x20", KEY_META, "description", "In extended mode, on some controllers like the ST7036 (in 3 line mode) the next line in DDRAM won`t start 0x20 higher.", KEY_META, "type", "string", KEY_END),
	keyNew ("/hd44780/#/model", KEY_VALUE, "standard", KEY_META, "check/enum", "#3", KEY_META, "check/enum/#0", "standard", KEY_META, "check/enum/#1", "extended", KEY_META, "check/enum/#2", "winstar_oled", KEY_META, "check/enum/#3", "pt6314_vfd", KEY_META, "check/type", "enum", KEY_META, "default", "standard", KEY_META, "description", "Select model if have non-standard one which require extra initialization or handling or\n  just want extra features it offers.\n  Available: standard (default), extended, winstar_oled, pt6314_vfd\n  - standard is default, use for LCDs not mentioned below.\n  - extended, hd66712, ks0073: allows use 4-line \'extended\' mode,\n    same as deprecated now option ExtendedMode=yes\n  - winstar_oled, weh00xxyya: changes initialization for WINSTAR\'s WEH00xxyyA displays\n    and allows handling brightness\n  - pt6314_vfd: allows handling brightness on PTC\'s PT6314 VFDs\n  This option should be independent of connection type.", KEY_META, "type", "enum", KEY_END),
	keyNew ("/hd44780/#/offbrightness", KEY_VALUE, "300", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "300", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/hd44780/#/outputport", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "If you have the additional output port (\"bargraph\") and you want to be able to control it with the lcdproc OUTPUT command", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/hd44780/#/port", KEY_VALUE, "0x378", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x[0-9A-F]{3}", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Port must begin with \"0x\", followed by 3 Hexadezimal values, eg. 0x3BC", KEY_META, "default", "0x378", KEY_META, "description", "I/O address of the LPT port. Usual values are: 0x278, 0x378 and 0x3BC. For I2C connections this sets the slave address (usually 0x20).", KEY_META, "type", "string", KEY_END),
	keyNew ("/hd44780/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/hd44780/#/refreshdisplay", KEY_VALUE, "0", KEY_META, "check/range", "0-20", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "If you experience occasional garbage on your display you can use this\noption as workaround. If set to a value bigger than null it forces a\nfull screen refresh <RefreshDiplay> seconds.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/hd44780/#/size", KEY_VALUE, "20x4", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "Specifies the size of the LCD. In case of multiple combined displays, this should be the total size.", KEY_META, "type", "string", KEY_END),
	keyNew ("/hd44780/#/speed", KEY_VALUE, "0", KEY_META, "check/type", "unsigned_long", KEY_META, "default", "0", KEY_META, "description", "Bitrate of the serial port (0 for interface default)", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("/hd44780/#/vspan", KEY_META, "check/type", "string", KEY_META, "check/validation", "^([1-9][0-9]*(,[1-9][0-9]*)*)\?$", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Declarations must look like this: 2,2 or 2,2,1", KEY_META, "default", "", KEY_META, "description", "For multiple combined displays: how many lines does each display have.\nVspan=2,2 means both displays have 2 lines.", KEY_META, "type", "string", KEY_END),
	keyNew ("/icp_a106/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a icp_a106 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Icp_a106DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/icp_a106/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/icp_a106/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/icp_a106/#/device", KEY_VALUE, "/dev/lcd", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Device of the serial, I2C, or SPI interface", KEY_META, "type", "string", KEY_END),
	keyNew ("/icp_a106/#/file", KEY_VALUE, "icp_a106", KEY_META, "default", "icp_a106", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/icp_a106/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/icp_a106/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/icp_a106/#/size", KEY_VALUE, "20x2", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x2", KEY_META, "description", "Display dimensions", KEY_META, "type", "string", KEY_END),
	keyNew ("/imon/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a imon driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "ImonDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/imon/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/imon/#/charmap", KEY_VALUE, "none", KEY_META, "check/enum", "#5", KEY_META, "check/enum/#0", "hd44780_euro", KEY_META, "check/enum/#1", "upd16314", KEY_META, "check/enum/#2", "hd44780_koi8_r", KEY_META, "check/enum/#3", "hd44780_cp1251", KEY_META, "check/enum/#4", "hd44780_8859_5", KEY_META, "check/enum/#5", "none", KEY_META, "check/type", "enum", KEY_META, "default", "none", KEY_META, "description", "Character map to to map ISO-8859-1 to the displays character set. (upd16314, hd44780_koi8_r,\nhd44780_cp1251, hd44780_8859_5 are possible if compiled with additional\ncharmaps)", KEY_META, "gen/enum/type", "IMonCharmap", KEY_META, "type", "enum", KEY_END),
	keyNew ("/imon/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/imon/#/device", KEY_VALUE, "/dev/lcd0", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd0", KEY_META, "description", "select the device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/imon/#/file", KEY_VALUE, "imon", KEY_META, "default", "imon", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/imon/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/imon/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/imon/#/size", KEY_VALUE, "16x2", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "Display dimensions", KEY_META, "type", "string", KEY_END),
	keyNew ("/imonlcd/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a imonlcd driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "ImonlcdDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/imonlcd/#/backlight", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Set the backlight state", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/imonlcd/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/imonlcd/#/contrast", KEY_VALUE, "200", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "200", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/imonlcd/#/device", KEY_VALUE, "/dev/lcd0", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd0", KEY_META, "description", "select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/imonlcd/#/discmode", KEY_VALUE, "0", KEY_META, "check/enum", "#1", KEY_META, "check/enum/#0", "0", KEY_META, "check/enum/#1", "1", KEY_META, "check/type", "enum", KEY_META, "default", "0", KEY_META, "description", "Set the disc mode\n0 => spin the \'slim\' disc - two disc segments,\n1 => their complement spinning;", KEY_META, "gen/enum/type", "IMonLCDDiscMode", KEY_META, "type", "enum", KEY_END),
	keyNew ("/imonlcd/#/file", KEY_VALUE, "imonlcd", KEY_META, "default", "imonlcd", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/imonlcd/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/imonlcd/#/onexit", KEY_VALUE, "1", KEY_META, "check/range", "0-2", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1", KEY_META, "description", "Set the exit behavior\n0 means leave shutdown message,\n1 means show the big clock,\n2 means blank device", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/imonlcd/#/protocol", KEY_VALUE, "0", KEY_META, "check/enum", "#3", KEY_META, "check/enum/#0", "15c2:ffdc", KEY_META, "check/enum/#1", "15c2:0038", KEY_META, "check/enum/#2", "0", KEY_META, "check/enum/#3", "1", KEY_META, "check/type", "string", KEY_META, "default", "0", KEY_META, "description", "Specify which iMon protocol should be used\nChoose 0 for 15c2:ffdc device,\nChoose 1 for 15c2:0038 device", KEY_META, "gen/enum/#2/value", "0", KEY_META, "gen/enum/#3/value", "1", KEY_META, "gen/enum/type", "IMonLCDProtocol", KEY_META, "type", "string", KEY_END),
	keyNew ("/imonlcd/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/imonlcd/#/size", KEY_VALUE, "96x16", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "96x16", KEY_META, "description", "Specify the size of the display in pixels", KEY_META, "type", "string", KEY_END),
	keyNew ("/iowarrior/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a IOWarrior driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "IOWarriorDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/iowarrior/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/iowarrior/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/iowarrior/#/extendedmode", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "If you have an HD66712, a KS0073 or another \'almost HD44780-compatible\',\nset this flag to get into extended mode (4-line linear).", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/iowarrior/#/file", KEY_VALUE, "IOWarrior", KEY_META, "default", "IOWarrior", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/iowarrior/#/lastline", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Specifies if the last line is pixel addressable (yes) or it controls an\nunderline effect (no).", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/iowarrior/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/iowarrior/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/iowarrior/#/serialnumber", KEY_VALUE, "00000674", KEY_META, "check/type", "any", KEY_META, "default", "00000674", KEY_META, "description", "serial number. Must be exactly as listed by usbview (if not given, the 1st IOWarrior found gets used)", KEY_META, "type", "string", KEY_END),
	keyNew ("/iowarrior/#/size", KEY_VALUE, "20x4", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "Display dimensions", KEY_META, "type", "string", KEY_END),
	keyNew ("/irman/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a IrMan driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "IrManDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/irman/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/irman/#/config", KEY_VALUE, "/etc/irman.cfg", KEY_META, "check/type", "string", KEY_META, "default", "/etc/irman.cfg", KEY_META, "description", "Select the configuration file to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/irman/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/irman/#/device", KEY_VALUE, "/dev/irman", KEY_META, "check/type", "string", KEY_META, "default", "/dev/irman", KEY_META, "description", "select the device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/irman/#/file", KEY_VALUE, "IrMan", KEY_META, "default", "IrMan", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/irman/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/irman/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/irtrans/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a irtrans driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "IrtransDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/irtrans/#/backlight", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Does the device have a backlight\?", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/irtrans/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/irtrans/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/irtrans/#/file", KEY_VALUE, "irtrans", KEY_META, "default", "irtrans", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/irtrans/#/hostname", KEY_VALUE, "localhost", KEY_META, "check/type", "string", KEY_META, "default", "localhost", KEY_META, "description", "IRTrans device to connect to", KEY_META, "type", "string", KEY_END),
	keyNew ("/irtrans/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/irtrans/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/irtrans/#/size", KEY_VALUE, "16x2", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "Specify the size of the display in pixels", KEY_META, "type", "string", KEY_END),
	keyNew ("/joy/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a joy driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "JoyDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/joy/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/joy/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/joy/#/device", KEY_VALUE, "/dev/js0", KEY_META, "check/type", "string", KEY_META, "default", "/dev/js0", KEY_META, "description", "Select the input device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/joy/#/file", KEY_VALUE, "joy", KEY_META, "default", "joy", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/joy/#/map_axis1neg", KEY_VALUE, "Left", KEY_META, "check/type", "string", KEY_META, "default", "Left", KEY_META, "description", "set the axis map", KEY_META, "type", "string", KEY_END),
	keyNew ("/joy/#/map_axis1pos", KEY_VALUE, "Right", KEY_META, "check/type", "string", KEY_META, "default", "Right", KEY_META, "description", "set the axis map", KEY_META, "type", "string", KEY_END),
	keyNew ("/joy/#/map_axis2neg", KEY_VALUE, "Up", KEY_META, "check/type", "string", KEY_META, "default", "Up", KEY_META, "description", "set the axis map", KEY_META, "type", "string", KEY_END),
	keyNew ("/joy/#/map_axis2pos", KEY_VALUE, "Down", KEY_META, "check/type", "string", KEY_META, "default", "Down", KEY_META, "description", "set the axis map", KEY_META, "type", "string", KEY_END),
	keyNew ("/joy/#/map_button1", KEY_VALUE, "Enter", KEY_META, "check/type", "string", KEY_META, "default", "Enter", KEY_META, "description", "set the button map", KEY_META, "type", "string", KEY_END),
	keyNew ("/joy/#/map_button2", KEY_VALUE, "Escape", KEY_META, "check/type", "string", KEY_META, "default", "Escape", KEY_META, "description", "set the button map", KEY_META, "type", "string", KEY_END),
	keyNew ("/joy/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/joy/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/lb216/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a lb216 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Lb216DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/lb216/#/brightness", KEY_VALUE, "200", KEY_META, "check/range", "0-255", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "200", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/lb216/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/lb216/#/device", KEY_VALUE, "/dev/lcd", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Select the input device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/lb216/#/file", KEY_VALUE, "lb216", KEY_META, "default", "lb216", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/lb216/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/lb216/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/lb216/#/speed", KEY_VALUE, "9600", KEY_META, "check/range", "2400, 9600", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "9600", KEY_META, "description", "Set the communication speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/lcdm001/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a lcdm001 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Lcdm001DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/lcdm001/#/backkey", KEY_VALUE, "UpKey", KEY_META, "check/type", "string", KEY_META, "default", "UpKey", KEY_META, "description", "Normal Context: Back(Go to previous screen) \nMenu Context: Up/Left", KEY_META, "type", "string", KEY_END),
	keyNew ("/lcdm001/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/lcdm001/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/lcdm001/#/device", KEY_VALUE, "/dev/ttyS1", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyS1", KEY_META, "description", "select the device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/lcdm001/#/file", KEY_VALUE, "lcdm001", KEY_META, "default", "lcdm001", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/lcdm001/#/forwardkey", KEY_VALUE, "DownKey", KEY_META, "check/type", "string", KEY_META, "default", "DownKey", KEY_META, "description", "Normal Context: Forward(Go to next screen) \nMenu Context: Down/Right", KEY_META, "type", "string", KEY_END),
	keyNew ("/lcdm001/#/mainmenukey", KEY_VALUE, "RightKey", KEY_META, "check/type", "string", KEY_META, "default", "RightKey", KEY_META, "description", "Normal Context: Open main menu \nMenu Context: Exit/Cancel", KEY_META, "type", "string", KEY_END),
	keyNew ("/lcdm001/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/lcdm001/#/pausekey", KEY_VALUE, "LeftKey", KEY_META, "check/type", "string", KEY_META, "default", "LeftKey", KEY_META, "description", "Normal Context: Pause/Continue \nMenu Context: Enter/select", KEY_META, "type", "string", KEY_END),
	keyNew ("/lcdm001/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/lcterm/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a lcterm driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "LctermDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/lcterm/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/lcterm/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/lcterm/#/device", KEY_VALUE, "/dev/ttyS1", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyS1", KEY_META, "description", "select the device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/lcterm/#/file", KEY_VALUE, "lcterm", KEY_META, "default", "lcterm", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/lcterm/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/lcterm/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/lcterm/#/size", KEY_VALUE, "16x2", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "Specify the size of the display in pixels", KEY_META, "type", "string", KEY_END),
	keyNew ("/linux_input/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a linux_input driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Linux_inputDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/linux_input/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/linux_input/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/linux_input/#/device", KEY_VALUE, "/dev/input/event0", KEY_META, "check/type", "string", KEY_META, "default", "/dev/input/event0", KEY_META, "description", "select the device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/linux_input/#/file", KEY_VALUE, "linux_input", KEY_META, "default", "linux_input", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/linux_input/#/key/_", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "specify a non-default key map, eg.: \n/linux_input/key/#01 = 1\n/linux_input/key/#28 = Enter", KEY_META, "type", "string", KEY_END),
	keyNew ("/linux_input/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/linux_input/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/lirc/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a lirc driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "LircDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/lirc/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/lirc/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/lirc/#/file", KEY_VALUE, "lirc", KEY_META, "default", "lirc", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/lirc/#/lircrc", KEY_VALUE, "~/.lircrc", KEY_META, "check/type", "string", KEY_META, "default", "~/.lircrc", KEY_META, "description", "select the device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/lirc/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/lirc/#/prog", KEY_VALUE, "lcdd", KEY_META, "check/type", "string", KEY_META, "default", "lcdd", KEY_META, "description", "Must be the same as in your lircrc", KEY_META, "type", "string", KEY_END),
	keyNew ("/lirc/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/lis/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a lis driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "LisDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/lis/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial brightness\n0-250 = 25%, 251-500 = 50%, 501-750 = 75%, 751-1000 = 100%", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/lis/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/lis/#/file", KEY_VALUE, "lis", KEY_META, "default", "lis", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/lis/#/lastline", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Specifies if the last line is pixel addressable (yes) or it only controls an\nunderline effect (no)", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/lis/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/lis/#/productid", KEY_VALUE, "0x6001", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([0-9A-F]{4})", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size ProductID: 0x0000-0xFFFF is allowed", KEY_META, "default", "0x6001", KEY_META, "description", "USB Product ID. Change only if testing a compatible device.", KEY_META, "type", "string", KEY_END),
	keyNew ("/lis/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/lis/#/size", KEY_VALUE, "16x2", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "Columns by lines", KEY_META, "type", "string", KEY_END),
	keyNew ("/lis/#/vendorid", KEY_VALUE, "0x0403", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([0-9A-F]{4})", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size VendorID: 0x0000-0xFFFF is allowed", KEY_META, "default", "0x0403", KEY_META, "description", "USB Vendor ID. Change only if testing a compatible device.", KEY_META, "type", "string", KEY_END),
	keyNew ("/md8800/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a MD8800 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "MD8800DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/md8800/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/md8800/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/md8800/#/device", KEY_VALUE, "/dev/ttyS1", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyS1", KEY_META, "description", "select the device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/md8800/#/file", KEY_VALUE, "MD8800", KEY_META, "default", "MD8800", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/md8800/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Set the initial off-brightness\nThis value is used when the display is normally\nswitched off in case LCDd is inactive", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/md8800/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/md8800/#/size", KEY_VALUE, "16x2", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "display size", KEY_META, "type", "string", KEY_END),
	keyNew ("/mdm166a/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a mdm166a driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Mdm166aDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/mdm166a/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/mdm166a/#/clock", KEY_VALUE, "no", KEY_META, "check/enum", "#2", KEY_META, "check/enum/#0", "no", KEY_META, "check/enum/#1", "small", KEY_META, "check/enum/#2", "big", KEY_META, "check/type", "enum", KEY_META, "default", "no", KEY_META, "description", "Show self-running clock after LCDd shutdown", KEY_META, "type", "enum", KEY_END),
	keyNew ("/mdm166a/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/mdm166a/#/dimming", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Dim display, no dimming gives full brightness", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/mdm166a/#/file", KEY_VALUE, "mdm166a", KEY_META, "default", "mdm166a", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/mdm166a/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/mdm166a/#/offdimming", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Dim display in case LCDd is inactive", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/mdm166a/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/menu/downkey", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Up to six keys are supported. The MenuKey (to enter and exit the menu), the\nEnterKey (to select values) and at least one movement keys are required.", KEY_META, "type", "string", KEY_END),
	keyNew ("/menu/enterkey", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Up to six keys are supported. The MenuKey (to enter and exit the menu), the\nEnterKey (to select values) and at least one movement keys are required.", KEY_META, "type", "string", KEY_END),
	keyNew ("/menu/leftkey", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Up to six keys are supported. The MenuKey (to enter and exit the menu), the\nEnterKey (to select values) and at least one movement keys are required.", KEY_META, "type", "string", KEY_END),
	keyNew ("/menu/menukey", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Up to six keys are supported. The MenuKey (to enter and exit the menu), the\nEnterKey (to select values) and at least one movement keys are required.", KEY_META, "type", "string", KEY_END),
	keyNew ("/menu/permissivegoto", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "If true the server allows transitions between different client`s menus", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/menu/rightkey", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Up to six keys are supported. The MenuKey (to enter and exit the menu), the\nEnterKey (to select values) and at least one movement keys are required.", KEY_META, "type", "string", KEY_END),
	keyNew ("/menu/upkey", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Up to six keys are supported. The MenuKey (to enter and exit the menu), the\nEnterKey (to select values) and at least one movement keys are required.", KEY_META, "type", "string", KEY_END),
	keyNew ("/ms6931/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a ms6931 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Ms6931DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/ms6931/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/ms6931/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/ms6931/#/device", KEY_VALUE, "/dev/ttyS1", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyS1", KEY_META, "description", "select the device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/ms6931/#/file", KEY_VALUE, "ms6931", KEY_META, "default", "ms6931", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/ms6931/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/ms6931/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/ms6931/#/size", KEY_VALUE, "16x2", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "display size", KEY_META, "type", "string", KEY_END),
	keyNew ("/mtc_s16209x/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a mtc_s16209x driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Mtc_s16209xDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/mtc_s16209x/#/brightness", KEY_VALUE, "200", KEY_META, "check/range", "0-255", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "200", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/mtc_s16209x/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/mtc_s16209x/#/device", KEY_VALUE, "/dev/lcd", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/mtc_s16209x/#/file", KEY_VALUE, "mtc_s16209x", KEY_META, "default", "mtc_s16209x", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/mtc_s16209x/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/mtc_s16209x/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/mtxorb/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a MtxOrb driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "MtxOrbDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/mtxorb/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/mtxorb/#/contrast", KEY_VALUE, "480", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "480", KEY_META, "description", "Set the initial contrast\nNOTE: The driver will ignore this if the display\nis a vfd or vkd as they don\'t have this feature", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/mtxorb/#/device", KEY_VALUE, "/dev/ttyS0", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyS0", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/mtxorb/#/file", KEY_VALUE, "MtxOrb", KEY_META, "default", "MtxOrb", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/mtxorb/#/hasadjustablebacklight", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Some old displays do not have an adjustable backlight but only can\nswitch the backlight on/off. If you experience randomly appearing block\ncharacters, try setting this to false.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/mtxorb/#/keymap_a", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "By default no keys are mapped, meaning the keypad is not used at all.", KEY_META, "type", "string", KEY_END),
	keyNew ("/mtxorb/#/keymap_b", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "By default no keys are mapped, meaning the keypad is not used at all.", KEY_META, "type", "string", KEY_END),
	keyNew ("/mtxorb/#/keymap_c", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "By default no keys are mapped, meaning the keypad is not used at all.", KEY_META, "type", "string", KEY_END),
	keyNew ("/mtxorb/#/keymap_d", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "By default no keys are mapped, meaning the keypad is not used at all.", KEY_META, "type", "string", KEY_END),
	keyNew ("/mtxorb/#/keymap_e", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "By default no keys are mapped, meaning the keypad is not used at all.", KEY_META, "type", "string", KEY_END),
	keyNew ("/mtxorb/#/keymap_f", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "By default no keys are mapped, meaning the keypad is not used at all.", KEY_META, "type", "string", KEY_END),
	keyNew ("/mtxorb/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Set the initial off-brightness\nThis value is used when the display is normally\nswitched off in case LCDd is inactive", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/mtxorb/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/mtxorb/#/size", KEY_VALUE, "20x4", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "display size", KEY_META, "type", "string", KEY_END),
	keyNew ("/mtxorb/#/speed", KEY_VALUE, "19200", KEY_META, "check/range", "1200, 2400, 9600, 19200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "19200", KEY_META, "description", "Set the communication", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/mtxorb/#/type", KEY_VALUE, "lcd", KEY_META, "check/enum", "#3", KEY_META, "check/enum/#0", "lcd lkd, vfd, vkd", KEY_META, "check/enum/#1", "lkd", KEY_META, "check/enum/#2", "vfd", KEY_META, "check/enum/#3", "vkd", KEY_META, "check/type", "enum", KEY_META, "default", "lcd", KEY_META, "description", "Set the display type", KEY_META, "type", "enum", KEY_END),
	keyNew ("/mx5000/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a mx5000 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Mx5000DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/mx5000/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/mx5000/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/mx5000/#/device", KEY_VALUE, "/dev/hiddev0", KEY_META, "check/type", "string", KEY_META, "default", "/dev/hiddev0", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/mx5000/#/file", KEY_VALUE, "mx5000", KEY_META, "default", "mx5000", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/mx5000/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/mx5000/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/mx5000/#/waitafterrefresh", KEY_VALUE, "1000", KEY_META, "check/type", "unsigned_long", KEY_META, "check/validation", "[1-9]\\d*", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "WaitAfterRefresh must be a positive number", KEY_META, "default", "1000", KEY_META, "description", "Time to wait in ms after the refresh screen has been sent", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("/noritakevfd/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a NoritakeVFD driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "NoritakeVFDDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/noritakevfd/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/noritakevfd/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/noritakevfd/#/device", KEY_VALUE, "/dev/lcd", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "device where the VFD is. Usual values are /dev/ttyS0 and /dev/ttyS1", KEY_META, "type", "string", KEY_END),
	keyNew ("/noritakevfd/#/file", KEY_VALUE, "NoritakeVFD", KEY_META, "default", "NoritakeVFD", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/noritakevfd/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Set the initial off-brightness\nThis value is used when the display is normally\nswitched off in case LCDd is inactive", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/noritakevfd/#/parity", KEY_VALUE, "0", KEY_META, "check/range", "0-2", KEY_META, "check/type", "unsigned_long", KEY_META, "default", "0", KEY_META, "description", "Set serial data parity\nMeaning: 0(=none), 1(=odd), 2(=even)", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("/noritakevfd/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/noritakevfd/#/size", KEY_VALUE, "20x4", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "display size", KEY_META, "type", "string", KEY_END),
	keyNew ("/noritakevfd/#/speed", KEY_VALUE, "19200", KEY_META, "check/range", "1200, 2400, 9600, 19200, 115200", KEY_META, "check/type", "unsigned_long", KEY_META, "default", "19200", KEY_META, "description", "set the serial port speed", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("/olimex_mod_lcd1x9/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a Olimex_MOD_LCD1x9 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Olimex_MOD_LCD1x9DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/olimex_mod_lcd1x9/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/olimex_mod_lcd1x9/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/olimex_mod_lcd1x9/#/device", KEY_VALUE, "/dev/i2c-0", KEY_META, "check/type", "string", KEY_META, "default", "/dev/i2c-0", KEY_META, "description", "device file of the i2c controller", KEY_META, "type", "string", KEY_END),
	keyNew ("/olimex_mod_lcd1x9/#/file", KEY_VALUE, "Olimex_MOD_LCD1x9", KEY_META, "default", "Olimex_MOD_LCD1x9", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/olimex_mod_lcd1x9/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/olimex_mod_lcd1x9/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/picolcd/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a picolcd driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "PicolcdDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/picolcd/#/backlight", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Sets the initial state of the backlight upon start-up.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/picolcd/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial brightness [default: 1000; legal: 0 - 1000]. Works only\nwith the 20x4 device", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/picolcd/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/picolcd/#/file", KEY_VALUE, "picolcd", KEY_META, "default", "picolcd", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/picolcd/#/key0light", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If Keylights is on, the you can unlight specific keys below:\nKey0 is the directional pad.  Key1 - Key5 correspond to the F1 - F5 keys.\nThere is no LED for the +/- keys.  This is a handy way to indicate to users\nwhich keys are disabled.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/picolcd/#/key1light", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If Keylights is on, the you can unlight specific keys below:\nKey0 is the directional pad.  Key1 - Key5 correspond to the F1 - F5 keys.\nThere is no LED for the +/- keys.  This is a handy way to indicate to users\nwhich keys are disabled.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/picolcd/#/key2light", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If Keylights is on, the you can unlight specific keys below:\nKey0 is the directional pad.  Key1 - Key5 correspond to the F1 - F5 keys.\nThere is no LED for the +/- keys.  This is a handy way to indicate to users\nwhich keys are disabled.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/picolcd/#/key3light", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If Keylights is on, the you can unlight specific keys below:\nKey0 is the directional pad.  Key1 - Key5 correspond to the F1 - F5 keys.\nThere is no LED for the +/- keys.  This is a handy way to indicate to users\nwhich keys are disabled.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/picolcd/#/key4light", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If Keylights is on, the you can unlight specific keys below:\nKey0 is the directional pad.  Key1 - Key5 correspond to the F1 - F5 keys.\nThere is no LED for the +/- keys.  This is a handy way to indicate to users\nwhich keys are disabled.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/picolcd/#/key5light", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If Keylights is on, the you can unlight specific keys below:\nKey0 is the directional pad.  Key1 - Key5 correspond to the F1 - F5 keys.\nThere is no LED for the +/- keys.  This is a handy way to indicate to users\nwhich keys are disabled.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/picolcd/#/keylights", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Light the keys\?", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/picolcd/#/keyrepeatdelay", KEY_VALUE, "300", KEY_META, "check/range", "0-3000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "300", KEY_META, "description", "Key auto repeat is only available if the picoLCD driver is built with\nlibusb-1.0. Use KeyRepeatDelay and KeyRepeatInterval to configure key auto\nrepeat.\n#\nKey auto repeat delay (time in ms from first key report to first repeat). Use\nzero to disable auto repeat.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/picolcd/#/keyrepeatinterval", KEY_VALUE, "200", KEY_META, "check/range", "0-3000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "200", KEY_META, "description", "Key auto repeat interval (time in ms between repeat reports). Only used if\nKeyRepeatDelay is not zero.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/picolcd/#/keytimeout", KEY_VALUE, "500", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "500", KEY_META, "description", "KeyTimeout is only used if the picoLCD driver is built with libusb-0.1. When\nbuilt with libusb-1.0 key and IR data is input asynchronously so there is no\nneed to wait for the USB data.\nKeyTimeout is the time in ms that LCDd spends waiting for a key press before\ncycling through other duties.  Higher values make LCDd use less CPU time and\nmake key presses more detectable.  Lower values make LCDd more responsive\nbut a little prone to missing key presses.  500 (.5 second) is the default\nand a balanced value.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/picolcd/#/linklights", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Link the key lights to the backlight\?", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/picolcd/#/lircflushthreshold", KEY_VALUE, "1000", KEY_META, "check/type", "unsigned_long", KEY_META, "check/validation", "[1-9]\\d{3,}", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "LircFlushThreshold must be a positive number >1000", KEY_META, "default", "1000", KEY_META, "description", "Threshold in microseconds of the gap that triggers flushing the IR data\nto lirc [default: 8000; legal: 1000 - ]\nIf LircTime_us is on values greater than 32.767ms will disable the flush\nIf LircTime_us is off values greater than 1.999938s will disable the flush", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("/picolcd/#/lirchost", KEY_VALUE, "127.0.0.1", KEY_META, "check/type", "string", KEY_META, "default", "127.0.0.1", KEY_META, "description", "Host name or IP address of the LIRC instance that is to receive IR codes If not set, or set to an empty value, IR support is disabled.", KEY_META, "type", "string", KEY_END),
	keyNew ("/picolcd/#/lircport", KEY_VALUE, "8765", KEY_META, "default", "8765", KEY_META, "description", "UDP port on which LIRC is listening", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/picolcd/#/lirctime_us", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "UDP data time unit for LIRC\nOn:  times sent in microseconds (requires LIRC UDP driver that accepts this).\nOff: times sent in \'jiffies\' (1/16384s) (supported by standard LIRC UDP driver).", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/picolcd/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Set the brightness while the backlight is \'off\'.\nWorks only with the 20x4 device.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/picolcd/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/pyramid/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a pyramid driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "PyramidDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/pyramid/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/pyramid/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/pyramid/#/device", KEY_VALUE, "/dev/lcd", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "device to connect to", KEY_META, "type", "string", KEY_END),
	keyNew ("/pyramid/#/file", KEY_VALUE, "pyramid", KEY_META, "default", "pyramid", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/pyramid/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/pyramid/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/rawserial/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a rawserial driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "RawserialDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/rawserial/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/rawserial/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/rawserial/#/device", KEY_VALUE, "/dev/cuaU0", KEY_META, "check/type", "string", KEY_META, "default", "/dev/cuaU0", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/rawserial/#/file", KEY_VALUE, "rawserial", KEY_META, "default", "rawserial", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/rawserial/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/rawserial/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/rawserial/#/size", KEY_VALUE, "40x4", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "40x4", KEY_META, "description", "Specifies the size of the LCD. If this driver is loaded as a secondary driver\nit always adopts to the size of the primary driver. If loaded as the only\n(or primary) driver, the size can be set.", KEY_META, "type", "string", KEY_END),
	keyNew ("/rawserial/#/speed", KEY_VALUE, "9600", KEY_META, "check/type", "unsigned_long", KEY_META, "default", "9600", KEY_META, "description", "Serial port baudrate [default: 9600]", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("/rawserial/#/updaterate", KEY_VALUE, "1", KEY_META, "check/type", "float", KEY_META, "default", "1", KEY_META, "description", "How often to dump the LCD contents out the port, in Hertz (times per second)\n1 = once per second, 4 is 4 times per second, 0.1 is once every 10 seconds.\n[default: 1; legal: 0.0005 - 10]", KEY_META, "type", "float", KEY_END),
	keyNew ("/sed1330/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a sed1330 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Sed1330DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/sed1330/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/sed1330/#/cellsize", KEY_VALUE, "6x10", KEY_META, "check/type", "string", KEY_META, "check/validation", "[6-8]x([1-9][0-6]|[1-9])", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Width x Height of a character cell in pixels [legal: 6x7 - 8x16]", KEY_META, "default", "6x10", KEY_META, "description", "Width x Height of a character cell in pixels", KEY_META, "type", "string", KEY_END),
	keyNew ("/sed1330/#/connectiontype", KEY_VALUE, "classic", KEY_META, "check/enum", "#1", KEY_META, "check/enum/#0", "classic", KEY_META, "check/enum/#1", "bitshaker", KEY_META, "check/type", "enum", KEY_META, "default", "classic", KEY_META, "description", "Select what type of connection", KEY_META, "type", "enum", KEY_END),
	keyNew ("/sed1330/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/sed1330/#/file", KEY_VALUE, "sed1330", KEY_META, "default", "sed1330", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/sed1330/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/sed1330/#/port", KEY_VALUE, "0x378", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([0-9A-F]{3})", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid port declaration. It should start with 0x, followed by 3 HEX values. eg. 0x3BC", KEY_META, "default", "0x378", KEY_META, "description", "Port where the LPT is. Common values are 0x278, 0x378 and 0x3BC", KEY_META, "type", "string", KEY_END),
	keyNew ("/sed1330/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/sed1330/#/type", KEY_VALUE, "G321D", KEY_META, "check/enum", "#5", KEY_META, "check/enum/#0", "G321D", KEY_META, "check/enum/#1", "G121C", KEY_META, "check/enum/#2", "G242C", KEY_META, "check/enum/#3", "G191D", KEY_META, "check/enum/#4", "G2446", KEY_META, "check/enum/#5", "SP14Q002", KEY_META, "check/type", "enum", KEY_META, "default", "G321D", KEY_META, "description", "Type of LCD module.\nNote: Currently only tested with G321D & SP14Q002.", KEY_META, "type", "enum", KEY_END),
	keyNew ("/sed1520/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a sed1520 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Sed1520DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/sed1520/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/sed1520/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/sed1520/#/delaymult", KEY_VALUE, "1", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1", KEY_META, "description", "On fast machines it may be necessary to slow down transfer to the display.\nIf this value is set to zero, delay is disabled. Any value greater than\nzero slows down each write by one microsecond.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/sed1520/#/file", KEY_VALUE, "sed1520", KEY_META, "default", "sed1520", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/sed1520/#/haveinverter", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "The original wiring used an inverter to drive the control lines. If you do\nnot use an inverter set haveInverter to no.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/sed1520/#/interfacetype", KEY_VALUE, "80", KEY_META, "check/range", "68, 80", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "80", KEY_META, "description", "Select the interface type (wiring) for the display. Supported values are\n68 for 68-style connection (RESET level high) and 80 for 80-style connection\n(RESET level low)", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/sed1520/#/invertedmapping", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "On some displays column data in memory is mapped to segment lines from right\nto left. This is called inverted mapping (not to be confused with\n\'haveInverter\' from above).", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/sed1520/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/sed1520/#/port", KEY_VALUE, "0x378", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([0-9A-F]{3})", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid port declaration. It should start with 0x, followed by 3 HEX values. eg. 0x3BC", KEY_META, "default", "0x378", KEY_META, "description", "Port where the LPT is. Usual values are 0x278, 0x378 and 0x3BC", KEY_META, "type", "string", KEY_END),
	keyNew ("/sed1520/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/sed1520/#/usehardreset", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "At least one display is reported (Everbouquet MG1203D) that requires sending\nthree times 0xFF before a reset during initialization.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/serialpos/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a serialPOS driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "SerialPOSDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/serialpos/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/serialpos/#/cellsize", KEY_VALUE, "5x8", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "5x8", KEY_META, "description", "Specifies the cell size of each character cell on the display in characters.", KEY_META, "type", "string", KEY_END),
	keyNew ("/serialpos/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/serialpos/#/custom_chars", KEY_VALUE, "0", KEY_META, "check/type", "unsigned_long", KEY_META, "check/validation", "^([1-9]\\d*|0)$", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Must be any positive number including 0", KEY_META, "default", "0", KEY_META, "description", "Specifies the number of custom characters supported by the display.\nCustom characters are only used for the rendering of horizontal bars\nand vertical bars. For displays whose cell character cell widths are\nlower than the number of custom characters supported,\nthen custom characters will be used to render the horizontal bars.\nFor more information look at: https://github.com/lcdproc/lcdproc/blob/master/docs/lcdproc-user/drivers/serialPOS.docbook", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("/serialpos/#/device", KEY_VALUE, "/dev/ttyS0", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyS0", KEY_META, "description", "Device to use in serial mode", KEY_META, "type", "string", KEY_END),
	keyNew ("/serialpos/#/file", KEY_VALUE, "serialPOS", KEY_META, "default", "serialPOS", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/serialpos/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/serialpos/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/serialpos/#/size", KEY_VALUE, "16x2", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "Specifies the size of the display in characters.", KEY_META, "type", "string", KEY_END),
	keyNew ("/serialpos/#/speed", KEY_VALUE, "9600", KEY_META, "check/range", "1200, 2400, 4800, 9600, 19200, 115200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "9600", KEY_META, "description", "communication baud rate with the display", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/serialpos/#/type", KEY_VALUE, "AEDEX", KEY_META, "check/enum", "#5", KEY_META, "check/enum/#0", "AEDEX", KEY_META, "check/enum/#1", "CD5220", KEY_META, "check/enum/#2", "Epson", KEY_META, "check/enum/#3", "Emax", KEY_META, "check/enum/#4", "LogicControls", KEY_META, "check/enum/#5", "Ultimate", KEY_META, "check/type", "enum", KEY_META, "default", "AEDEX", KEY_META, "description", "Set the communication protocol to use with the POS display.", KEY_META, "type", "enum", KEY_END),
	keyNew ("/serialvfd/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a serialVFD driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "SerialVFDDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/serialvfd/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial brightness. (4 steps 0-250, 251-500, 501-750, 751-1000)", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/serialvfd/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/serialvfd/#/custom-characters", KEY_VALUE, "0", KEY_META, "check/type", "unsigned_long", KEY_META, "check/validation", "^([1-9]\\d*|0)$", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Must be any positive number including 0", KEY_META, "default", "0", KEY_META, "description", "Number of Custom-Characters. default is display type dependent", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("/serialvfd/#/device", KEY_VALUE, "/dev/ttyS1", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyS1", KEY_META, "description", "Device to use in serial mode. Usual values are /dev/ttyS0 and /dev/ttyS1", KEY_META, "type", "string", KEY_END),
	keyNew ("/serialvfd/#/file", KEY_VALUE, "serialVFD", KEY_META, "default", "serialVFD", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/serialvfd/#/iso_8859_1", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "enable ISO 8859 1 compatibility", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/serialvfd/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Set the initial off-brightness\nThis value is used when the display is normally\nswitched off in case LCDd is inactive\n(4 steps 0-250, 251-500, 501-750, 751-1000)", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/serialvfd/#/port", KEY_VALUE, "0x378", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([0-9A-F]{3})", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid port declaration. It should start with 0x, followed by 3 HEX values. eg. 0x3BC", KEY_META, "default", "0x378", KEY_META, "description", "Portaddress where the LPT is. Used in parallel mode only. Usual values are 0x278, 0x378 and 0x3BC.", KEY_META, "type", "string", KEY_END),
	keyNew ("/serialvfd/#/portwait", KEY_VALUE, "2", KEY_META, "check/range", "0-255", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "2", KEY_META, "description", "Set parallel port timing delay (us). Used in parallel mode only.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/serialvfd/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/serialvfd/#/size", KEY_VALUE, "20x2", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x2", KEY_META, "description", "Specifies the size of the VFD.", KEY_META, "type", "string", KEY_END),
	keyNew ("/serialvfd/#/speed", KEY_VALUE, "9600", KEY_META, "check/range", "1200, 2400, 9600, 19200, 115200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "9600", KEY_META, "description", "set the serial port speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/serialvfd/#/type", KEY_VALUE, "0", KEY_META, "check/enum", "#_17", KEY_META, "check/enum/#0", "nec_fipc8367", KEY_META, "check/enum/#1", "kd_rev_2_1", KEY_META, "check/enum/#2", "noritake_vfd", KEY_META, "check/enum/#3", "futaba_vfd", KEY_META, "check/enum/#4", "iee_s03601-95b", KEY_META, "check/enum/#5", "iee_s03601-96-080", KEY_META, "check/enum/#6", "futaba_na202sd08fa", KEY_META, "check/enum/#7", "samsung_20s207da4/20s207da6", KEY_META, "check/enum/#8", "nixdorf_ba6x/vt100", KEY_META, "check/enum/#_10", "1", KEY_META, "check/enum/#_11", "2", KEY_META, "check/enum/#_12", "3", KEY_META, "check/enum/#_13", "4", KEY_META, "check/enum/#_14", "5", KEY_META, "check/enum/#_15", "6", KEY_META, "check/enum/#_16", "7", KEY_META, "check/enum/#_17", "8", KEY_META, "check/enum/#_9", "0", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Specifies the displaytype.[default: 0]\n0 NEC (FIPC8367 based) VFDs.\n1 KD Rev 2.1.\n2 Noritake VFDs (*).\n3 Futaba VFDs\n4 IEE S03601-95B\n5 IEE S03601-96-080 (*)\n6 Futaba NA202SD08FA (allmost IEE compatible)\n7 Samsung 20S207DA4 and 20S207DA6\n8 Nixdorf BA6x / VT100\n(* most should work, not tested yet.)", KEY_META, "gen/enum/type", "SerialVFDType", KEY_META, "gen/enum/value", "8", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/serialvfd/#/use_parallel", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "\'no\' if display connected serial, \'yes\' if connected parallel.\nI.e. serial by default", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/server/autorotate", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If set to no, LCDd will start with screen rotation disabled. This has the same effect as if the ToggleRotateKey had been pressed. Rotation will start if the ToggleRotateKey is pressed. Note that this setting does not turn off priority sorting of screens", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/server/backlight", KEY_VALUE, "open", KEY_META, "check/enum", "#2", KEY_META, "check/enum/#0", "off", KEY_META, "check/enum/#1", "on", KEY_META, "check/enum/#2", "open", KEY_META, "default", "open", KEY_META, "description", "Set master backlight setting. If set to \'open\' a client may control the backlight for its own screens (only)", KEY_META, "type", "enum", KEY_END),
	keyNew ("/server/bind", KEY_VALUE, "127.0.0.1", KEY_META, "check/type", "string", KEY_META, "default", "127.0.0.1", KEY_META, "description", "Tells the driver to bind to the given interface", KEY_META, "opt", "a", KEY_META, "opt/long", "address", KEY_META, "type", "string", KEY_END),
	keyNew ("/server/driverpath", KEY_VALUE, "server/drivers/", KEY_META, "check/type", "string", KEY_META, "check/validation", ".*[\\/]$", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "The path has to contain a backslack (/) at the end", KEY_META, "default", "server/drivers/", KEY_META, "description", "Select the LCD size", KEY_META, "type", "string", KEY_END),
	keyNew ("/server/drivers/#", KEY_META, "check/reference/restrict", "#_52", KEY_META, "check/reference/restrict/#0", "@/bayrad/#", KEY_META, "check/reference/restrict/#1", "@/cfontz/#", KEY_META, "check/reference/restrict/#2", "@/cfontzpacket/#", KEY_META, "check/reference/restrict/#3", "@/curses/#", KEY_META, "check/reference/restrict/#4", "@/cwlnx/#", KEY_META, "check/reference/restrict/#5", "@/ea65/#", KEY_META, "check/reference/restrict/#6", "@/eyeboxone/#", KEY_META, "check/reference/restrict/#7", "@/futaba/#", KEY_META, "check/reference/restrict/#8", "@/g15/#", KEY_META, "check/reference/restrict/#9", "@/glcd/#", KEY_META, "check/reference/restrict/#_10", "@/glcdlib/#", KEY_META, "check/reference/restrict/#_11", "@/glk/#", KEY_META, "check/reference/restrict/#_12", "@/hd44780/#", KEY_META, "check/reference/restrict/#_13", "@/icp_a106/#", KEY_META, "check/reference/restrict/#_14", "@/imon/#", KEY_META, "check/reference/restrict/#_15", "@/imonlcd/#", KEY_META, "check/reference/restrict/#_16", "@/iowarrior/#", KEY_META, "check/reference/restrict/#_17", "@/irman/#", KEY_META, "check/reference/restrict/#_18", "@/joy/#", KEY_META, "check/reference/restrict/#_19", "@/lb216/#", KEY_META, "check/reference/restrict/#_20", "@/lcdm001/#", KEY_META, "check/reference/restrict/#_21", "@/lcterm/#", KEY_META, "check/reference/restrict/#_22", "@/linux_input/#", KEY_META, "check/reference/restrict/#_23", "@/lirc/#", KEY_META, "check/reference/restrict/#_24", "@/lis/#", KEY_META, "check/reference/restrict/#_25", "@/md8800/#", KEY_META, "check/reference/restrict/#_26", "@/mdm166a/#", KEY_META, "check/reference/restrict/#_27", "@/ms6931/#", KEY_META, "check/reference/restrict/#_28", "@/mtc_s16209x/#", KEY_META, "check/reference/restrict/#_29", "@/mtxorb/#", KEY_META, "check/reference/restrict/#_30", "@/mx5000/#", KEY_META, "check/reference/restrict/#_31", "@/noritakevfd/#", KEY_META, "check/reference/restrict/#_32", "@/olimex_mod_lcd1x9/#", KEY_META, "check/reference/restrict/#_33", "@/picolcd/#", KEY_META, "check/reference/restrict/#_34", "@/pyramid/#", KEY_META, "check/reference/restrict/#_35", "@/rawserial/#", KEY_META, "check/reference/restrict/#_36", "@/sdeclcd/#", KEY_META, "check/reference/restrict/#_37", "@/sed1330/#", KEY_META, "check/reference/restrict/#_38", "@/sed1520/#", KEY_META, "check/reference/restrict/#_39", "@/serialpos/#", KEY_META, "check/reference/restrict/#_40", "@/serialvfd/#", KEY_META, "check/reference/restrict/#_41", "@/shuttlevfd/#", KEY_META, "check/reference/restrict/#_42", "@/sli/#", KEY_META, "check/reference/restrict/#_43", "@/stv5730/#", KEY_META, "check/reference/restrict/#_44", "@/svga/#", KEY_META, "check/reference/restrict/#_45", "@/t6963/#", KEY_META, "check/reference/restrict/#_46", "@/text/#", KEY_META, "check/reference/restrict/#_47", "@/tyan/#", KEY_META, "check/reference/restrict/#_48", "@/ula200/#", KEY_META, "check/reference/restrict/#_49", "@/vlsys_m428/#", KEY_META, "check/reference/restrict/#_50", "@/xosd/#", KEY_META, "check/reference/restrict/#_51", "@/yard2lcd/#", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "Tells the server to load a driver.\nThe given value is a reference the configuration of the driver, e.g. @/curses/#0", KEY_META, "type", "string", KEY_END),
	keyNew ("/server/foreground", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "The server will stay in the foreground if set to yes", KEY_META, "opt", "f", KEY_META, "opt/arg", "none", KEY_META, "opt/long", "foreground", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/server/frameinterval", KEY_VALUE, "125000", KEY_META, "check/type", "unsigned_long", KEY_META, "check/validation", "[1-9]\\d*", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "FrameInterval must be a positive number", KEY_META, "default", "125000", KEY_META, "description", "Sets the interval in microseconds for updating the display [default: 125000 meaning 8Hz]", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("/server/goodbye/#", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "GoodBye message: each entry represents a display line", KEY_META, "type", "string", KEY_END),
	keyNew ("/server/heartbeat", KEY_VALUE, "open", KEY_META, "check/enum", "#2", KEY_META, "check/enum/#0", "off", KEY_META, "check/enum/#1", "on", KEY_META, "check/enum/#2", "open", KEY_META, "default", "open", KEY_META, "description", "Set master heartbeat setting. If set to \'open\' a client may control the", KEY_META, "type", "enum", KEY_END),
	keyNew ("/server/hello/#", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "Hello message: each entry represents a display line", KEY_META, "type", "string", KEY_END),
	keyNew ("/server/nextscreenkey", KEY_VALUE, "Right", KEY_META, "check/type", "string", KEY_META, "default", "Right", KEY_META, "description", "The \'...Key=\' lines define what the server does with keypresses that\ndon\'t go to any client. The ToggleRotateKey stops rotation of screens, while\nthe PrevScreenKey and NextScreenKey go back / forward one screen (even if\nrotation is disabled.\nAssign the key string returned by the driver to the ...Key setting.", KEY_META, "type", "string", KEY_END),
	keyNew ("/server/port", KEY_VALUE, "13666", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "13666", KEY_META, "description", "Listen on this specified port. [default: 13666]", KEY_META, "opt", "p", KEY_META, "opt/long", "port", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/server/prevscreenkey", KEY_VALUE, "Left", KEY_META, "check/type", "string", KEY_META, "default", "Left", KEY_META, "description", "The \'...Key=\' lines define what the server does with keypresses that\ndon\'t go to any client. The ToggleRotateKey stops rotation of screens, while\nthe PrevScreenKey and NextScreenKey go back / forward one screen (even if\nrotation is disabled.\nAssign the key string returned by the driver to the ...Key setting.", KEY_META, "type", "string", KEY_END),
	keyNew ("/server/reportlevel", KEY_VALUE, "2", KEY_META, "check/range", "0-5", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "2", KEY_META, "description", "TSets the reporting level, defaults to warnings and errors only.", KEY_META, "opt", "r", KEY_META, "opt/long", "report-level", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/server/reporttosyslog", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Should we report to syslog instead of stderr\?", KEY_META, "opt", "s", KEY_META, "opt/arg", "none", KEY_META, "opt/long", "report-to-syslog", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/server/scrolldownkey", KEY_VALUE, "Down", KEY_META, "check/type", "string", KEY_META, "default", "Down", KEY_META, "description", "The \'...Key=\' lines define what the server does with keypresses that\ndon\'t go to any client. The ToggleRotateKey stops rotation of screens, while\nthe PrevScreenKey and NextScreenKey go back / forward one screen (even if\nrotation is disabled.\nAssign the key string returned by the driver to the ...Key setting.", KEY_META, "type", "string", KEY_END),
	keyNew ("/server/scrollupkey", KEY_VALUE, "Up", KEY_META, "check/type", "string", KEY_META, "default", "Up", KEY_META, "description", "The \'...Key=\' lines define what the server does with keypresses that\ndon\'t go to any client. The ToggleRotateKey stops rotation of screens, while\nthe PrevScreenKey and NextScreenKey go back / forward one screen (even if\nrotation is disabled.\nAssign the key string returned by the driver to the ...Key setting.", KEY_META, "type", "string", KEY_END),
	keyNew ("/server/serverscreen", KEY_VALUE, "on", KEY_META, "check/enum", "#2", KEY_META, "check/enum/#0", "off", KEY_META, "check/enum/#1", "on", KEY_META, "check/enum/#2", "blank", KEY_META, "default", "on", KEY_META, "description", "If yes, the the serverscreen will be rotated as a usual info screen. If no, it will be a background screen, only visible when no other screens are active. The special value \'blank\' is similar to no, but only a blank screen is displayed.", KEY_META, "opt", "i", KEY_META, "opt/long", "rotate-server-screen", KEY_META, "type", "enum", KEY_END),
	keyNew ("/server/titlespeed", KEY_VALUE, "10", KEY_META, "check/range", "0-10", KEY_META, "default", "10", KEY_META, "description", "title scrolling speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/server/togglerotatekey", KEY_VALUE, "Enter", KEY_META, "check/type", "string", KEY_META, "default", "Enter", KEY_META, "description", "The \'...Key=\' lines define what the server does with keypresses that\ndon\'t go to any client. The ToggleRotateKey stops rotation of screens, while\nthe PrevScreenKey and NextScreenKey go back / forward one screen (even if\nrotation is disabled.\nAssign the key string returned by the driver to the ...Key setting.", KEY_META, "type", "string", KEY_END),
	keyNew ("/server/user", KEY_VALUE, "nobody", KEY_META, "check/type", "string", KEY_META, "default", "nobody", KEY_META, "description", "set to run as.  LCDd will drop its root privileges and run as this user instead. [default: nobody]", KEY_META, "opt", "u", KEY_META, "opt/long", "user", KEY_META, "type", "string", KEY_END),
	keyNew ("/server/waittime", KEY_VALUE, "4", KEY_META, "check/type", "float", KEY_META, "default", "4", KEY_META, "description", "Sets the default time in seconds to displays a screen", KEY_META, "opt", "w", KEY_META, "opt/long", "wait-time", KEY_META, "type", "float", KEY_END),
	keyNew ("/sli/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a sli driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "SliDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/sli/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/sli/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/sli/#/device", KEY_VALUE, "/dev/lcd", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/sli/#/file", KEY_VALUE, "sli", KEY_META, "default", "sli", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/sli/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/sli/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/sli/#/speed", KEY_VALUE, "19200", KEY_META, "check/range", "1200, 2400, 9600, 19200, 38400, 57600, 115200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "19200", KEY_META, "description", "Set the communication speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/stv5730/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a stv5730 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Stv5730DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/stv5730/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/stv5730/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/stv5730/#/file", KEY_VALUE, "stv5730", KEY_META, "default", "stv5730", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/stv5730/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/stv5730/#/port", KEY_VALUE, "0x378", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([0-9A-F]{3})", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid port declaration. It should start with 0x, followed by 3 HEX values. eg. 0x3BC", KEY_META, "default", "0x378", KEY_META, "description", "Port the device is connected to", KEY_META, "type", "string", KEY_END),
	keyNew ("/stv5730/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/sureelec/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a SureElec driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "SureElecDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/sureelec/#/brightness", KEY_VALUE, "480", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "480", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/sureelec/#/contrast", KEY_VALUE, "480", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "480", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/sureelec/#/device", KEY_VALUE, "/dev/ttyUSB0", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyUSB0", KEY_META, "description", "Port the device is connected to  (by default first USB serial port)", KEY_META, "type", "string", KEY_END),
	keyNew ("/sureelec/#/edition", KEY_VALUE, "2", KEY_META, "check/range", "1-3", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "2", KEY_META, "description", "Edition level of the device (can be 1, 2 or 3)", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/sureelec/#/file", KEY_VALUE, "SureElec", KEY_META, "default", "SureElec", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/sureelec/#/offbrightness", KEY_VALUE, "480", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "480", KEY_META, "description", "Set the initial off-brightness\nThis value is used when the display is normally\nswitched off in case LCDd is inactive", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/sureelec/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/sureelec/#/size", KEY_VALUE, "16x2", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "set display size\nNote: The size can be obtained directly from device for edition 2 & 3.", KEY_META, "type", "string", KEY_END),
	keyNew ("/svga/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a svga driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "SvgaDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/svga/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "1-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/svga/#/contrast", KEY_VALUE, "500", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "500", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/svga/#/file", KEY_VALUE, "svga", KEY_META, "default", "svga", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/svga/#/mode", KEY_VALUE, "G320x240x256", KEY_META, "check/type", "string", KEY_META, "default", "G320x240x256", KEY_META, "description", "svgalib mode to use [default: G320x240x256 ]\nlegal values are supported svgalib modes. See man7 pages for allowed values", KEY_META, "type", "string", KEY_END),
	keyNew ("/svga/#/offbrightness", KEY_VALUE, "500", KEY_META, "check/range", "1-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "500", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/svga/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/svga/#/size", KEY_VALUE, "20x4", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "set display size", KEY_META, "type", "string", KEY_END),
	keyNew ("/t6963/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a t6963 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "T6963DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/t6963/#/bidirectional", KEY_VALUE, "1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Use LPT port in bi-directional mode. This should work on most LPT port and\nis required for proper timing!", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/t6963/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/t6963/#/cleargraphic", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Clear graphic memory on start-up.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/t6963/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/t6963/#/delaybus", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Insert additional delays into reads / writes.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/t6963/#/file", KEY_VALUE, "t6963", KEY_META, "default", "t6963", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/t6963/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/t6963/#/port", KEY_VALUE, "0x378", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([2-3][0-9A-F]{2}|400)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid port declaration. legal: 0x200 - 0x400", KEY_META, "default", "0x378", KEY_META, "description", "Parallel port to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/t6963/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/t6963/#/size", KEY_VALUE, "128x64", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "128x64", KEY_META, "description", "set display size in pixels", KEY_META, "type", "string", KEY_END),
	keyNew ("/text/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a text driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "TextDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/text/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/text/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/text/#/file", KEY_VALUE, "text", KEY_META, "default", "text", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/text/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/text/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/text/#/size", KEY_VALUE, "20x4", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "set display size", KEY_META, "type", "string", KEY_END),
	keyNew ("/tyan/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a tyan driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "TyanDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/tyan/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/tyan/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/tyan/#/device", KEY_VALUE, "/dev/lcd", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/tyan/#/file", KEY_VALUE, "tyan", KEY_META, "default", "tyan", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/tyan/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/tyan/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/tyan/#/size", KEY_VALUE, "16x2", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "set display size", KEY_META, "type", "string", KEY_END),
	keyNew ("/tyan/#/speed", KEY_VALUE, "9600", KEY_META, "check/range", "4800, 9600", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "9600", KEY_META, "description", "Set the communication speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/ula200/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a ula200 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Ula200DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/ula200/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/ula200/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/ula200/#/file", KEY_VALUE, "ula200", KEY_META, "default", "ula200", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/ula200/#/keymap_a", KEY_VALUE, "Up", KEY_META, "check/type", "string", KEY_META, "default", "Up", KEY_META, "description", "If you have a non standard keypad you can associate any keystrings to keys.\nThere are 6 input key in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n#\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("/ula200/#/keymap_b", KEY_VALUE, "Down", KEY_META, "check/type", "string", KEY_META, "default", "Down", KEY_META, "description", "If you have a non standard keypad you can associate any keystrings to keys.\nThere are 6 input key in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n#\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("/ula200/#/keymap_c", KEY_VALUE, "Left", KEY_META, "check/type", "string", KEY_META, "default", "Left", KEY_META, "description", "If you have a non standard keypad you can associate any keystrings to keys.\nThere are 6 input key in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n#\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("/ula200/#/keymap_d", KEY_VALUE, "Right", KEY_META, "check/type", "string", KEY_META, "default", "Right", KEY_META, "description", "If you have a non standard keypad you can associate any keystrings to keys.\nThere are 6 input key in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n#\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("/ula200/#/keymap_e", KEY_VALUE, "Enter", KEY_META, "check/type", "string", KEY_META, "default", "Enter", KEY_META, "description", "If you have a non standard keypad you can associate any keystrings to keys.\nThere are 6 input key in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n#\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("/ula200/#/keymap_f", KEY_VALUE, "Escape", KEY_META, "check/type", "string", KEY_META, "default", "Escape", KEY_META, "description", "If you have a non standard keypad you can associate any keystrings to keys.\nThere are 6 input key in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n#\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("/ula200/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/ula200/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/ula200/#/size", KEY_VALUE, "20x4", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "Select the LCD size", KEY_META, "type", "string", KEY_END),
	keyNew ("/vlsys_m428/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a vlsys_m428 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Vlsys_m428DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/vlsys_m428/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/vlsys_m428/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/vlsys_m428/#/device", KEY_VALUE, "/dev/ttyUSB0", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyUSB0", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("/vlsys_m428/#/file", KEY_VALUE, "vlsys_m428", KEY_META, "default", "vlsys_m428", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/vlsys_m428/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/vlsys_m428/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/xosd/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a xosd driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "XosdDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/xosd/#/Font", KEY_VALUE, "-*-terminus-*-r-*-*-*-320-*-*-*-*-*", KEY_META, "check/type", "string", KEY_META, "default", "-*-terminus-*-r-*-*-*-320-*-*-*-*-*", KEY_META, "description", "X font to use, in XLFD format, as given by \'xfontsel\'", KEY_META, "type", "string", KEY_END),
	keyNew ("/xosd/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/xosd/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/xosd/#/file", KEY_VALUE, "xosd", KEY_META, "default", "xosd", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/xosd/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/xosd/#/offset", KEY_VALUE, "0x0", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid offset declaration. Examples: 200x200", KEY_META, "default", "0x0", KEY_META, "description", "Offset in pixels from the top-left corner of the monitor [default: 0x0]", KEY_META, "type", "string", KEY_END),
	keyNew ("/xosd/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/xosd/#/size", KEY_VALUE, "20x4", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "set display size", KEY_META, "type", "string", KEY_END),
	keyNew ("/yard2lcd/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a yard2LCD driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Yard2LCDDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("/yard2lcd/#/brightness", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/yard2lcd/#/contrast", KEY_VALUE, "1000", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/yard2lcd/#/file", KEY_VALUE, "yard2LCD", KEY_META, "default", "yard2LCD", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("/yard2lcd/#/offbrightness", KEY_VALUE, "0", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("/yard2lcd/#/reboot", KEY_VALUE, "0", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("/yard2lcd/#/size", KEY_VALUE, "20x4", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "set display size", KEY_META, "type", "string", KEY_END),
	KS_END);
;
	Elektra * e = elektraOpen ("/sw/lcdproc/lcdd/#0/current", defaults, error);

	if (e == NULL)
	{
		return -1;
	}

	KeySet * contract = ksNew (1,
	keyNew ("system/elektra/ensure/plugins/global/gopts", KEY_VALUE, "mounted", KEY_END),
	KS_END);
;

	ElektraError * err = NULL;
	elektraEnsure (e, contract, &err);

	if (err != NULL)
	{
		*error = err;
		return -1;
	}

	helpKey = elektraHelpKey (e);
	if (helpKey != NULL)
	{
		elektraClose (e);
		return 2;
	}

	KeySet * defaultContext = ksNew (0,
	KS_END);
;
	ksAppend (elektraContext (e), defaultContext);
	ksDel (defaultContext);

	*elektra = e;
	return 0;
}

/**
 * Checks whether specload mode was invoked and if so, sends the specification over stdout
 * in the format expected by specload.
 *
 * You MUST not output anything to stdout before invoking this function. Ideally invoking this
 * is the first thing you do in your main()-function.
 *
 * This function will ONLY RETURN, if specload mode was NOT invoked. Otherwise it will call `exit()`.
 *
 * @param argc pass the value of argc from main
 * @param argv pass the value of argv from main
 */
void doSpecloadCheck (int argc, const char ** argv)
{
	if (argc != 2 || strcmp (argv[1], "--elektra-spec") != 0)
	{
		return;
	}

	KeySet * spec = ksNew (648,
	keyNew ("spec/sw/lcdproc/lcdd/#0/current", KEY_META, "infos/plugins", "ini validation type", KEY_META, "mountpoint", "LCDd.conf", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/bayrad", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/bayrad/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a bayrad driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "BayradDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/bayrad/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/bayrad/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/bayrad/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/bayrad/#/file", KEY_META, "default", "bayrad", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/bayrad/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/bayrad/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/bayrad/#/speed", KEY_META, "check/range", "1200, 2400, 9600, 19200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "9600", KEY_META, "description", "Set the communication speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontz", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontz/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a CFontz driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "CFontzDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontz/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial brightness", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontz/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "350", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontz/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontz/#/file", KEY_META, "default", "CFontz", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontz/#/newfirmware", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Set the firmware version (New means >= 2.0)", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontz/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Set the initial off-brightness. This value is used when the display is normally switched off in case LCDd is inactive", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontz/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontz/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "Select the LCD size", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontz/#/speed", KEY_META, "check/range", "1200, 2400, 9600, 19200, 115200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "9600", KEY_META, "description", "Set the communication speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontzpacket", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontzpacket/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a CFontzPacket driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "CFontzPacketDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontzpacket/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial brightness", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontzpacket/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "350", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontzpacket/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontzpacket/#/file", KEY_META, "default", "CFontzPacket", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontzpacket/#/model", KEY_META, "check/enum", "#3", KEY_META, "check/enum/#0", "533", KEY_META, "check/enum/#1", "631", KEY_META, "check/enum/#2", "633", KEY_META, "check/enum/#3", "635", KEY_META, "check/type", "enum", KEY_META, "default", "633", KEY_META, "description", "Select the LCD model", KEY_META, "gen/enum/type", "CFontzPacketModel", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontzpacket/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Set the initial off-brightness. This value is used when the display is normally switched off in case LCDd is inactive", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontzpacket/#/oldfirmware", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Very old 633 firmware versions do not support partial screen updates using \'Send Data to LCD\' command (31). For those devices it may be necessary to enable this flag", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontzpacket/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontzpacket/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "Override the LCD size known for the selected model. Usually setting this value should not be necessary.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontzpacket/#/speed", KEY_META, "check/range", "19200, 115200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "115200", KEY_META, "description", "Override the default communication speed known for the selected model. Default value depends on model.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cfontzpacket/#/usb", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Enable the USB flag if the device is connected to an USB port. For serial ports leave it disabled.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/curses", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/curses/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a curses driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "CursesDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/curses/#/background", KEY_META, "check/enum", "#7", KEY_META, "check/enum/#0", "red", KEY_META, "check/enum/#1", "black", KEY_META, "check/enum/#2", "green", KEY_META, "check/enum/#3", "yellow", KEY_META, "check/enum/#4", "blue", KEY_META, "check/enum/#5", "magenta", KEY_META, "check/enum/#6", "cyan", KEY_META, "check/enum/#7", "white", KEY_META, "check/type", "enum", KEY_META, "default", "cyan", KEY_META, "description", "background color when \"backlight\" is off", KEY_META, "gen/enum/type", "CursesColor", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/curses/#/backlight", KEY_META, "check/enum", "#7", KEY_META, "check/enum/#0", "red", KEY_META, "check/enum/#1", "black", KEY_META, "check/enum/#2", "green", KEY_META, "check/enum/#3", "yellow", KEY_META, "check/enum/#4", "blue", KEY_META, "check/enum/#5", "magenta", KEY_META, "check/enum/#6", "cyan", KEY_META, "check/enum/#7", "white", KEY_META, "check/type", "enum", KEY_META, "default", "red", KEY_META, "description", "background color when \"backlight\" is on", KEY_META, "gen/enum/type", "CursesColor", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/curses/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/curses/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/curses/#/drawborder", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "draw Border", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/curses/#/file", KEY_META, "default", "curses", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/curses/#/foreground", KEY_META, "check/enum", "#7", KEY_META, "check/enum/#0", "red", KEY_META, "check/enum/#1", "black", KEY_META, "check/enum/#2", "green", KEY_META, "check/enum/#3", "yellow", KEY_META, "check/enum/#4", "blue", KEY_META, "check/enum/#5", "magenta", KEY_META, "check/enum/#6", "cyan", KEY_META, "check/enum/#7", "white", KEY_META, "check/type", "enum", KEY_META, "default", "blue", KEY_META, "description", "Color settings", KEY_META, "gen/enum/type", "CursesColor", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/curses/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/curses/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/curses/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "display size", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/curses/#/topleftx", KEY_META, "check/type", "unsigned_short", KEY_META, "check/validation", "([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Must be a positive number", KEY_META, "default", "7", KEY_META, "description", "What position (X,Y) to start the left top corner at...", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/curses/#/toplefty", KEY_META, "check/type", "unsigned_short", KEY_META, "check/validation", "([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Must be a positive number", KEY_META, "default", "7", KEY_META, "description", "What position (X,Y) to start the left top corner at...", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/curses/#/useacs", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "use ASC symbols for icons & bars", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a CwLnx driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "CwLnxDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#/file", KEY_META, "default", "CwLnx", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#/keymap_a", KEY_META, "check/type", "string", KEY_META, "default", "Up", KEY_META, "description", "If you have a non-standard keypad you can associate any keystrings to keys.\nThere are 6 input keys in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n\nThe built-in default mapping hardcoded in the driver.\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#/keymap_b", KEY_META, "check/type", "string", KEY_META, "default", "Down", KEY_META, "description", "If you have a non-standard keypad you can associate any keystrings to keys.\nThere are 6 input keys in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n\nThe built-in default mapping hardcoded in the driver.\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#/keymap_c", KEY_META, "check/type", "string", KEY_META, "default", "Left", KEY_META, "description", "If you have a non-standard keypad you can associate any keystrings to keys.\nThere are 6 input keys in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n\nThe built-in default mapping hardcoded in the driver.\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#/keymap_d", KEY_META, "check/type", "string", KEY_META, "default", "Right", KEY_META, "description", "If you have a non-standard keypad you can associate any keystrings to keys.\nThere are 6 input keys in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n\nThe built-in default mapping hardcoded in the driver.\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#/keymap_e", KEY_META, "check/type", "string", KEY_META, "default", "Enter", KEY_META, "description", "If you have a non-standard keypad you can associate any keystrings to keys.\nThere are 6 input keys in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n\nThe built-in default mapping hardcoded in the driver.\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#/keymap_f", KEY_META, "check/type", "string", KEY_META, "default", "Escape", KEY_META, "description", "If you have a non-standard keypad you can associate any keystrings to keys.\nThere are 6 input keys in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n\nThe built-in default mapping hardcoded in the driver.\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#/keypad", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If you have a keypad connected. Keypad layout is currently not configureable from the config file.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#/keypad_test_mode", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "permits one to test keypad assignment", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#/model", KEY_META, "check/enum", "#2", KEY_META, "check/enum/#0", "12232", KEY_META, "check/enum/#1", "12832", KEY_META, "check/enum/#2", "1602", KEY_META, "check/type", "enum", KEY_META, "default", "12232", KEY_META, "description", "Select the LCD model", KEY_META, "gen/enum/type", "CwLnxModel", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "Select the LCD size. Default depends on model: \n12232: 20x4 \n12832: 21x4 \n1602: 16x2", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/cwlnx/#/speed", KEY_META, "check/range", "9600, 19200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "19200", KEY_META, "description", "Set the communication speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ea65", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ea65/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a ea65 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Ea65DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ea65/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "500", KEY_META, "description", "As the VFD is self luminescent we don\'t have a backlight\nBut we can use the backlight functions to control the front LEDs\nBrightness 0 to 299 -> LEDs off\nBrightness 300 to 699 -> LEDs half bright\nBrightness 700 to 1000 -> LEDs full bright", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ea65/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ea65/#/file", KEY_META, "default", "ea65", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ea65/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "OffBrightness is the the value used for the \'backlight off\' state", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ea65/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a EyeboxOne driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "EyeboxOneDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone/#/backlight", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Switch on the backlight\?", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone/#/cursor", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Switch on the cursor\?", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyS1", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone/#/downkey", KEY_META, "check/type", "string", KEY_META, "default", "B", KEY_META, "description", "Enter Key is a \r character, so it\'s hardcoded in the driver", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone/#/escapekey", KEY_META, "check/type", "string", KEY_META, "default", "P", KEY_META, "description", "Enter Key is a \r character, so it\'s hardcoded in the driver", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone/#/file", KEY_META, "default", "EyeboxOne", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone/#/keypad_test_mode", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "You can find out which key of your display sends which\ncharacter by setting keypad_test_mode to yes and running\nLCDd. LCDd will output all characters it receives.\nAfterwards you can modify the settings above and set\nkeypad_set_mode to no again.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone/#/leftkey", KEY_META, "check/type", "string", KEY_META, "default", "D", KEY_META, "description", "Enter Key is a \r character, so it\'s hardcoded in the driver", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone/#/rightkey", KEY_META, "check/type", "string", KEY_META, "default", "C", KEY_META, "description", "Enter Key is a \r character, so it\'s hardcoded in the driver", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "Set the display size", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone/#/speed", KEY_META, "check/range", "1200, 2400, 9600, 19200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "19200", KEY_META, "description", "Set the communication speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/eyeboxone/#/upkey", KEY_META, "check/type", "string", KEY_META, "default", "A", KEY_META, "description", "Enter Key is a \r character, so it\'s hardcoded in the driver", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/futaba", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/futaba/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a futaba driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "FutabaDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/futaba/#/file", KEY_META, "default", "futaba", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/g15", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/g15/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a g15 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "G15DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/g15/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/g15/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/g15/#/file", KEY_META, "default", "g15", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/g15/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/g15/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/g15/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x5", KEY_META, "description", "Display size (currently unused)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a glcd driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "GlcdDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/bidirectional", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Use LPT port in bi-directional mode. This should work on most LPT port and is required for proper timing!", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "800", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/cellsize", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "6x8", KEY_META, "description", "Width and height of a character cell in pixels. This value is only used the driver has been compiled with FreeType and it is enabled. Otherwise the default 6x8 cell is used.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/connectiontype", KEY_META, "check/enum", "#6", KEY_META, "check/enum/#0", "t6963", KEY_META, "check/enum/#1", "png", KEY_META, "check/enum/#2", "serdisplib", KEY_META, "check/enum/#3", "glcd2usb", KEY_META, "check/enum/#4", "x11", KEY_META, "check/enum/#5", "picolcdgfx", KEY_META, "check/enum/#6", "xyz", KEY_META, "check/type", "enum", KEY_META, "default", "t6963", KEY_META, "description", "Select what type of connection. See documentation for types.", KEY_META, "gen/enum/type", "GlcdConnectionType", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "600", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/delaybus", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Insert additional delays into reads / writes.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/file", KEY_META, "default", "glcd", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/fonthasicons", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Some fonts miss the Unicode characters used to represent icons. In this case the built-in 5x8 font can used if this option is turned off.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/keymap_a", KEY_META, "check/type", "string", KEY_META, "default", "Up", KEY_META, "description", "Assign key strings to keys. There may be up to 16 keys numbered \'A\' to \'Z\'.\nBy default keys \'A\' to \'F\' are assigned Up, Down, Left, Right, Enter, Escape.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/keymap_b", KEY_META, "check/type", "string", KEY_META, "default", "Down", KEY_META, "description", "Assign key strings to keys. There may be up to 16 keys numbered \'A\' to \'Z\'.\nBy default keys \'A\' to \'F\' are assigned Up, Down, Left, Right, Enter, Escape.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/keymap_c", KEY_META, "check/type", "string", KEY_META, "default", "Left", KEY_META, "description", "Assign key strings to keys. There may be up to 16 keys numbered \'A\' to \'Z\'.\nBy default keys \'A\' to \'F\' are assigned Up, Down, Left, Right, Enter, Escape.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/keymap_d", KEY_META, "check/type", "string", KEY_META, "default", "Right", KEY_META, "description", "Assign key strings to keys. There may be up to 16 keys numbered \'A\' to \'Z\'.\nBy default keys \'A\' to \'F\' are assigned Up, Down, Left, Right, Enter, Escape.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/keymap_e", KEY_META, "check/type", "string", KEY_META, "default", "Enter", KEY_META, "description", "Assign key strings to keys. There may be up to 16 keys numbered \'A\' to \'Z\'.\nBy default keys \'A\' to \'F\' are assigned Up, Down, Left, Right, Enter, Escape.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/keymap_f", KEY_META, "check/type", "string", KEY_META, "default", "Escape", KEY_META, "description", "Assign key strings to keys. There may be up to 16 keys numbered \'A\' to \'Z\'.\nBy default keys \'A\' to \'F\' are assigned Up, Down, Left, Right, Enter, Escape.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/keyrepeatdelay", KEY_META, "check/range", "0-3000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "500", KEY_META, "description", "Time (ms) from first key report to first repeat. Set to 0 to disable repeated key reports.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/keyrepeatinterval", KEY_META, "check/range", "0-3000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "500", KEY_META, "description", "Time (ms) between repeated key reports. Ignored if KeyRepeatDelay is disabled (set to zero).", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/normal_font", KEY_META, "check/type", "string", KEY_META, "default", "/usr/local/lib/X11/fonts/TTF/andalemo.ttf", KEY_META, "description", "Path to font file to use for FreeType rendering. This font must be monospace and should contain some special Unicode characters like arrows (Andale Mono is recommended and can be fetched at http://corefonts.sf.net).", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "100", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/picolcdgfx_inverted", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Inverts the pixels.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/picolcdgfx_keytimeout", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9][0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Number must be any positive integer >0", KEY_META, "default", "125", KEY_META, "description", "Time in ms for usb_read to wait on a key press.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/port", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([2-3][0-9A-F]{2}|400)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid port declaration. legal: 0x200 - 0x400", KEY_META, "default", "0x378", KEY_META, "description", "Parallel port to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/serdisp_device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ppi0", KEY_META, "description", "The display device to use, e.g. serraw:/dev/ttyS0, parport:/dev/parport0 or USB:07c0/1501", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/serdisp_name", KEY_META, "check/type", "string", KEY_META, "default", "t6963", KEY_META, "description", "Name of the underlying serdisplib driver, e.g. ctinclud. See", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/serdisp_options", KEY_META, "check/type", "string", KEY_META, "default", "INVERT=1", KEY_META, "description", "Options string to pass to serdisplib during initialization. Use\nthis to set any display related options (e.g. wiring). The display size is\nalways set based on the Size configured above! By default, no options are\nset.\nImportant: The value must be quoted as it contains equal signs!", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "(640|[1-9]|[1-9][0-9]|[1-5][0-9][0-9]|6[0-3][0-9])x(480|[1-9]|[1-9][0-9]|[1-3][0-9][0-9]|4[0-7][0-9])", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. legal: 1x1 - 640x480", KEY_META, "default", "128x64", KEY_META, "description", "Width and height of the display in pixel. The supported sizes may depend on the ConnectionType", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/useft2", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If LCDproc has been compiled with FreeType 2 support this option can be used to turn if off intentionally.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/x11_backlightcolor", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x[0-9A-F]{6}", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Colors are in RRGGBB format prefixed with \"0x\"", KEY_META, "default", "0x80FF80", KEY_META, "description", "The color of the backlight as full brightness.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/x11_border", KEY_META, "check/type", "unsigned_long", KEY_META, "check/validation", "^([1-9]\\d*|0)$", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Must be a positive number", KEY_META, "default", "20", KEY_META, "description", "Adds a border (empty space) around the LCD portion of X11 window.", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/x11_inverted", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Inverts the pixels.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/x11_pixelcolor", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x[0-9A-F]{6}", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Colors are in RRGGBB format prefixed with \"0x\"", KEY_META, "default", "0x000000", KEY_META, "description", "The color of each dot at full contrast.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcd/#/x11_pixelsize", KEY_META, "check/type", "string", KEY_META, "check/validation", "\\d+\\+\\d+", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "[number]+[number] has to be provided, eg: 3+1 or 5+2", KEY_META, "default", "3+1", KEY_META, "description", "Each LCD dot is drawn in the X window as a filled rectangle of this size\n    plus a gap between each filled rectangle. A PixelSize of 3+1\n    would draw a 3x3 filled rectangle with a gap of 1 pixel to the right and\n    bottom, effectively using a 4x4 area of the window. Default is 3+1.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a glcdlib driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "GlcdlibDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/CharEncoding", KEY_META, "check/type", "string", KEY_META, "default", "iso8859-2", KEY_META, "description", "character encoding to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/backlight", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/brightness", KEY_META, "check/range", "0-100", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "50", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/contrast", KEY_META, "check/range", "0-100", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "50", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/driver", KEY_META, "check/type", "string", KEY_META, "default", "image", KEY_META, "description", "\nSpecify which graphical display supported by graphlcd-base to use.\n    Legal values for <replaceable>GRAPHLCD-DRIVER</replaceable> are\n    specified in graphlcd\'s configuration file <filename>/etc/graphlcd.conf</filename>.\n    For graphlcd 0.13 they comprise avrctl, framebuffer, gu140x32f, gu256x64-372, \n    gu256x64C-3xx0, hd61830, image, ks0108, noritake800, sed1330, sed1520, serdisp, simlcd, t6963c\n", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/file", KEY_META, "default", "glcdlib", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/fontfile", KEY_META, "check/type", "string", KEY_META, "default", "/usr/share/fonts/corefonts/courbd.ttf", KEY_META, "description", "path to font file to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/invert", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/minfontfacesize", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "7x12", KEY_META, "description", "path to font file to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/pixelshiftx", KEY_META, "check/type", "short", KEY_META, "check/validation", "([0-9]+)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Must be a positive number", KEY_META, "default", "0", KEY_META, "type", "short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/pixelshifty", KEY_META, "check/type", "short", KEY_META, "check/validation", "([0-9]+)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Must be a positive number", KEY_META, "default", "2", KEY_META, "type", "short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/showbigborder", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/showdebugframe", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/showthinborder", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/textresolution", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x4", KEY_META, "description", "Text resolution in fixed width characters.\n(if it won\'t fit according to available physical pixel resolution\nand the minimum available font face size in pixels, then\n\'DebugBorder\' will automatically be turned on)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/upsidedown", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glcdlib/#/useft2", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "no=use graphlcd bitmap fonts (they have only one size / font file)\nyes=use fonts supported by FreeType2 (needs Freetype2 support in\nlibglcdprocdriver and its dependants)", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glk", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glk/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a glk driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "GlkDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glk/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glk/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "500", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glk/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "select the serial device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glk/#/file", KEY_META, "default", "glk", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glk/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glk/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/glk/#/speed", KEY_META, "check/range", "9600, 19200, 38400, 57600, 115200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "19200", KEY_META, "description", "set the serial port speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a hd44780 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Hd44780DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/backlight", KEY_META, "check/enum", "#8", KEY_META, "check/enum/#0", "none", KEY_META, "check/enum/#1", "external", KEY_META, "check/enum/#2", "internal", KEY_META, "check/enum/#3", "internalCmds", KEY_META, "check/type", "enum", KEY_META, "default", "none", KEY_META, "description", "Specify if you have a switchable backlight and if yes, can select method for turning it on/off:\n#\n- none - no switchable backlight is available.\n- external - use external pin or any other method defined with ConnectionType backlight\n         handling.\n- internal - means that backlight is handled using internal commands according\n         to selected display model (with Model option). Depending on model,\n         Brightness and OffBrightness options can be taken into account.\n- internalCmds - means that commands for turning on and off backlight are given\n         with extra options BacklightOnCmd and BacklightOffCmd, which would be treated\n         as catch up (last resort) for other types of displays which have similar features.\n#\nYou can provide multiple occurences of this option to use more than one method.\nDefault is model specific: Winstar OLED and PT6314 VFD enables internal backlight mode,\nfor others it is set to none.", KEY_META, "gen/enum/type", "HD44780Backlight", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/backlightcmdoff", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x[0-9A-F]{4}", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "4 bytes can be encoded, as integer number in big-endian order (0x0000-0xFFFF)", KEY_META, "default", "0x1234", KEY_META, "description", "Commands for disabling internal backlight for use with Backlight=internalCmds.\nUp to 4 bytes can be encoded, as integer number in big-endian order.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/backlightcmdon", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x[0-9A-F]{4}", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "4 bytes can be encoded, as integer number in big-endian order (0x0000-0xFFFF)", KEY_META, "default", "0x1223", KEY_META, "description", "Commands for enabling internal backlight for use with Backlight=internalCmds.\nUp to 4 bytes can be encoded, as integer number in big-endian order.\n#\nNOTE: this is advanced option, if command contains bits other than only brighness handling,\nthey must be set accordingly to not disrupt display state. If for example \'FUNCTION SET\' command\nis used for this purpose, bits of interface length (4-bit / 8-bit) must be set according to\nselected ConnectionType.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "800", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/charmap", KEY_META, "check/enum", "#7", KEY_META, "check/enum/#0", "hd44780_default", KEY_META, "check/enum/#1", "hd44780_euro", KEY_META, "check/enum/#2", "ea_ks0073", KEY_META, "check/enum/#3", "sed1278f_0b", KEY_META, "check/enum/#4", "hd44780_koi8_r", KEY_META, "check/enum/#5", "hd44780_cp1251", KEY_META, "check/enum/#6", "hd44780_8859_5", KEY_META, "check/enum/#7", "upd16314", KEY_META, "check/type", "enum", KEY_META, "default", "hd44780_default", KEY_META, "description", "Character map to to map ISO-8859-1 to the LCD\'s character set. (hd44780_koi8_r, hd44780_cp1251, hd44780_8859_5, upd16314 and weh001602a_1\nare possible if compiled with additional charmaps)", KEY_META, "gen/enum/type", "HD44780Charmap", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/connectiontype", KEY_META, "check/enum", "#_27", KEY_META, "check/enum/#0", "4bit", KEY_META, "check/enum/#1", "8bit", KEY_META, "check/enum/#2", "winamp", KEY_META, "check/enum/#3", "lcm162", KEY_META, "check/enum/#4", "serialLpt", KEY_META, "check/enum/#5", "picanlcd", KEY_META, "check/enum/#6", "lcdserializer", KEY_META, "check/enum/#7", "los-panel", KEY_META, "check/enum/#8", "vdr-lcd", KEY_META, "check/enum/#9", "vdr-wakeup", KEY_META, "check/enum/#_10", "ezio", KEY_META, "check/enum/#_11", "pertelian", KEY_META, "check/enum/#_12", "lis2", KEY_META, "check/enum/#_13", "mplay", KEY_META, "check/enum/#_14", "usblcd", KEY_META, "check/enum/#_15", "bwctusb", KEY_META, "check/enum/#_16", "lcd2usb", KEY_META, "check/enum/#_17", "usbtiny", KEY_META, "check/enum/#_18", "uss720", KEY_META, "check/enum/#_19", "USB-4-all", KEY_META, "check/enum/#_20", "ftdi", KEY_META, "check/enum/#_21", "i2c", KEY_META, "check/enum/#_22", "piplate", KEY_META, "check/enum/#_23", "spi", KEY_META, "check/enum/#_24", "pifacecad", KEY_META, "check/enum/#_25", "ethlcd", KEY_META, "check/enum/#_26", "raspberrypi", KEY_META, "check/enum/#_27", "gpio", KEY_META, "check/type", "enum", KEY_META, "default", "4bit", KEY_META, "description", "Select what type of connection. See documentation for available types:\n  https://github.com/lcdproc/lcdproc/blob/master/docs/lcdproc-user/drivers/hd44780.docbook", KEY_META, "gen/enum/type", "HD44780ConnectionType", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "800", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/delaybus", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "You can reduce the inserted delays by setting this to false.\nOn fast PCs it is possible your LCD does not respond correctly.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/delaymult", KEY_META, "check/range", "1, 2, 4, 8, 16", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1", KEY_META, "description", "If your display is slow and cannot keep up with the flow of data from\nLCDd, garbage can appear on the LCDd. Set this delay factor to 2 or 4\nto increase the delays.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Device of the serial, I2C, or SPI interface", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/extendedmode", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "If you have an HD66712, a KS0073 or another controller with \'extended mode\',\nset this flag to get into 4-line mode. On displays with just two lines, do\nnot set this flag.\nAs an additional restriction, controllers with and without extended mode\nAND 4 lines cannot be mixed for those connection types that support more\nthan one display!\nNOTE: This option is deprecated in favour of choosing Model=extended option.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/file", KEY_META, "default", "hd44780", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/fontbank", KEY_META, "check/enum", "#3", KEY_META, "check/enum/#0", "0", KEY_META, "check/enum/#1", "1", KEY_META, "check/enum/#2", "2", KEY_META, "check/enum/#3", "3", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Font bank to be used for some displays such as the WINSTAR WEH001602A\n0: English/Japanese (default)\n1: Western Europe I\n2: English/Rusian\n3: Western Europe II", KEY_META, "gen/enum/type", "HD44780Fontbank", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/keepalivedisplay", KEY_META, "check/range", "0-10", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Some displays (e.g. vdr-wakeup) need a message from the driver to that it\nis still alive. When set to a value bigger then null the character in the\nupper left corner is updated every <KeepAliveDisplay> seconds.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/keymatrix_4_1", KEY_META, "check/type", "string", KEY_META, "default", "Enter", KEY_META, "description", "If you have a keypad you can assign keystrings to the keys. See\ndocumentation for used terms and how to wire it. For example to give directly connected\nkey 4 the string \'Enter\', use:\n   KeyDirect_4=Enter For matrix keys use the\nX and Y coordinates of the key: \n  KeyMatrix_1_3=Enter", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/keymatrix_4_2", KEY_META, "check/type", "string", KEY_META, "default", "Up", KEY_META, "description", "If you have a keypad you can assign keystrings to the keys. See\ndocumentation for used terms and how to wire it. For example to give directly connected\nkey 4 the string \'Enter\', use:\n   KeyDirect_4=Enter For matrix keys use the\nX and Y coordinates of the key: \n  KeyMatrix_1_3=Enter", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/keymatrix_4_3", KEY_META, "check/type", "string", KEY_META, "default", "Down", KEY_META, "description", "If you have a keypad you can assign keystrings to the keys. See\ndocumentation for used terms and how to wire it. For example to give directly connected\nkey 4 the string \'Enter\', use:\n   KeyDirect_4=Enter For matrix keys use the\nX and Y coordinates of the key: \n  KeyMatrix_1_3=Enter", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/keymatrix_4_4", KEY_META, "check/type", "string", KEY_META, "default", "Escape", KEY_META, "description", "If you have a keypad you can assign keystrings to the keys. See\ndocumentation for used terms and how to wire it. For example to give directly connected\nkey 4 the string \'Enter\', use:\n   KeyDirect_4=Enter For matrix keys use the\nX and Y coordinates of the key: \n  KeyMatrix_1_3=Enter", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/keypad", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "If you have a keypad connected.\nYou may also need to configure the keypad layout further on in this file.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/lastline", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Specifies if the last line is pixel addressable (yes) or it controls an\nunderline effect (no).", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/lineaddress", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a line Address. eg. 0x20, 0x10", KEY_META, "default", "0x20", KEY_META, "description", "In extended mode, on some controllers like the ST7036 (in 3 line mode) the next line in DDRAM won`t start 0x20 higher.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/model", KEY_META, "check/enum", "#3", KEY_META, "check/enum/#0", "standard", KEY_META, "check/enum/#1", "extended", KEY_META, "check/enum/#2", "winstar_oled", KEY_META, "check/enum/#3", "pt6314_vfd", KEY_META, "check/type", "enum", KEY_META, "default", "standard", KEY_META, "description", "Select model if have non-standard one which require extra initialization or handling or\n  just want extra features it offers.\n  Available: standard (default), extended, winstar_oled, pt6314_vfd\n  - standard is default, use for LCDs not mentioned below.\n  - extended, hd66712, ks0073: allows use 4-line \'extended\' mode,\n    same as deprecated now option ExtendedMode=yes\n  - winstar_oled, weh00xxyya: changes initialization for WINSTAR\'s WEH00xxyyA displays\n    and allows handling brightness\n  - pt6314_vfd: allows handling brightness on PTC\'s PT6314 VFDs\n  This option should be independent of connection type.", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "300", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/outputport", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "If you have the additional output port (\"bargraph\") and you want to be able to control it with the lcdproc OUTPUT command", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/port", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x[0-9A-F]{3}", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Port must begin with \"0x\", followed by 3 Hexadezimal values, eg. 0x3BC", KEY_META, "default", "0x378", KEY_META, "description", "I/O address of the LPT port. Usual values are: 0x278, 0x378 and 0x3BC. For I2C connections this sets the slave address (usually 0x20).", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/refreshdisplay", KEY_META, "check/range", "0-20", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "If you experience occasional garbage on your display you can use this\noption as workaround. If set to a value bigger than null it forces a\nfull screen refresh <RefreshDiplay> seconds.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "Specifies the size of the LCD. In case of multiple combined displays, this should be the total size.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/speed", KEY_META, "check/type", "unsigned_long", KEY_META, "default", "0", KEY_META, "description", "Bitrate of the serial port (0 for interface default)", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/hd44780/#/vspan", KEY_META, "check/type", "string", KEY_META, "check/validation", "^([1-9][0-9]*(,[1-9][0-9]*)*)\?$", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Declarations must look like this: 2,2 or 2,2,1", KEY_META, "default", "", KEY_META, "description", "For multiple combined displays: how many lines does each display have.\nVspan=2,2 means both displays have 2 lines.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/icp_a106", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/icp_a106/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a icp_a106 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Icp_a106DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/icp_a106/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/icp_a106/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/icp_a106/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Device of the serial, I2C, or SPI interface", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/icp_a106/#/file", KEY_META, "default", "icp_a106", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/icp_a106/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/icp_a106/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/icp_a106/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x2", KEY_META, "description", "Display dimensions", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imon", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imon/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a imon driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "ImonDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imon/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imon/#/charmap", KEY_META, "check/enum", "#5", KEY_META, "check/enum/#0", "hd44780_euro", KEY_META, "check/enum/#1", "upd16314", KEY_META, "check/enum/#2", "hd44780_koi8_r", KEY_META, "check/enum/#3", "hd44780_cp1251", KEY_META, "check/enum/#4", "hd44780_8859_5", KEY_META, "check/enum/#5", "none", KEY_META, "check/type", "enum", KEY_META, "default", "none", KEY_META, "description", "Character map to to map ISO-8859-1 to the displays character set. (upd16314, hd44780_koi8_r,\nhd44780_cp1251, hd44780_8859_5 are possible if compiled with additional\ncharmaps)", KEY_META, "gen/enum/type", "IMonCharmap", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imon/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imon/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd0", KEY_META, "description", "select the device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imon/#/file", KEY_META, "default", "imon", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imon/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imon/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imon/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "Display dimensions", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imonlcd", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imonlcd/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a imonlcd driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "ImonlcdDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imonlcd/#/backlight", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Set the backlight state", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imonlcd/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imonlcd/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "200", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imonlcd/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd0", KEY_META, "description", "select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imonlcd/#/discmode", KEY_META, "check/enum", "#1", KEY_META, "check/enum/#0", "0", KEY_META, "check/enum/#1", "1", KEY_META, "check/type", "enum", KEY_META, "default", "0", KEY_META, "description", "Set the disc mode\n0 => spin the \'slim\' disc - two disc segments,\n1 => their complement spinning;", KEY_META, "gen/enum/type", "IMonLCDDiscMode", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imonlcd/#/file", KEY_META, "default", "imonlcd", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imonlcd/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imonlcd/#/onexit", KEY_META, "check/range", "0-2", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1", KEY_META, "description", "Set the exit behavior\n0 means leave shutdown message,\n1 means show the big clock,\n2 means blank device", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imonlcd/#/protocol", KEY_META, "check/enum", "#3", KEY_META, "check/enum/#0", "15c2:ffdc", KEY_META, "check/enum/#1", "15c2:0038", KEY_META, "check/enum/#2", "0", KEY_META, "check/enum/#3", "1", KEY_META, "check/type", "string", KEY_META, "default", "0", KEY_META, "description", "Specify which iMon protocol should be used\nChoose 0 for 15c2:ffdc device,\nChoose 1 for 15c2:0038 device", KEY_META, "gen/enum/#2/value", "0", KEY_META, "gen/enum/#3/value", "1", KEY_META, "gen/enum/type", "IMonLCDProtocol", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imonlcd/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/imonlcd/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "96x16", KEY_META, "description", "Specify the size of the display in pixels", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/iowarrior", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/iowarrior/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a IOWarrior driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "IOWarriorDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/iowarrior/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/iowarrior/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/iowarrior/#/extendedmode", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "If you have an HD66712, a KS0073 or another \'almost HD44780-compatible\',\nset this flag to get into extended mode (4-line linear).", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/iowarrior/#/file", KEY_META, "default", "IOWarrior", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/iowarrior/#/lastline", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Specifies if the last line is pixel addressable (yes) or it controls an\nunderline effect (no).", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/iowarrior/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/iowarrior/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/iowarrior/#/serialnumber", KEY_META, "check/type", "any", KEY_META, "default", "00000674", KEY_META, "description", "serial number. Must be exactly as listed by usbview (if not given, the 1st IOWarrior found gets used)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/iowarrior/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "Display dimensions", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irman", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irman/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a IrMan driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "IrManDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irman/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irman/#/config", KEY_META, "check/type", "string", KEY_META, "default", "/etc/irman.cfg", KEY_META, "description", "Select the configuration file to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irman/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irman/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/irman", KEY_META, "description", "select the device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irman/#/file", KEY_META, "default", "IrMan", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irman/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irman/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irtrans", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irtrans/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a irtrans driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "IrtransDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irtrans/#/backlight", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Does the device have a backlight\?", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irtrans/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irtrans/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irtrans/#/file", KEY_META, "default", "irtrans", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irtrans/#/hostname", KEY_META, "check/type", "string", KEY_META, "default", "localhost", KEY_META, "description", "IRTrans device to connect to", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irtrans/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irtrans/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/irtrans/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "Specify the size of the display in pixels", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/joy", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/joy/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a joy driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "JoyDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/joy/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/joy/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/joy/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/js0", KEY_META, "description", "Select the input device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/joy/#/file", KEY_META, "default", "joy", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/joy/#/map_axis1neg", KEY_META, "check/type", "string", KEY_META, "default", "Left", KEY_META, "description", "set the axis map", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/joy/#/map_axis1pos", KEY_META, "check/type", "string", KEY_META, "default", "Right", KEY_META, "description", "set the axis map", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/joy/#/map_axis2neg", KEY_META, "check/type", "string", KEY_META, "default", "Up", KEY_META, "description", "set the axis map", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/joy/#/map_axis2pos", KEY_META, "check/type", "string", KEY_META, "default", "Down", KEY_META, "description", "set the axis map", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/joy/#/map_button1", KEY_META, "check/type", "string", KEY_META, "default", "Enter", KEY_META, "description", "set the button map", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/joy/#/map_button2", KEY_META, "check/type", "string", KEY_META, "default", "Escape", KEY_META, "description", "set the button map", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/joy/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/joy/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lb216", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lb216/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a lb216 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Lb216DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lb216/#/brightness", KEY_META, "check/range", "0-255", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "200", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lb216/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lb216/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Select the input device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lb216/#/file", KEY_META, "default", "lb216", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lb216/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lb216/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lb216/#/speed", KEY_META, "check/range", "2400, 9600", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "9600", KEY_META, "description", "Set the communication speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcdm001", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcdm001/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a lcdm001 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Lcdm001DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcdm001/#/backkey", KEY_META, "check/type", "string", KEY_META, "default", "UpKey", KEY_META, "description", "Normal Context: Back(Go to previous screen) \nMenu Context: Up/Left", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcdm001/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcdm001/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcdm001/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyS1", KEY_META, "description", "select the device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcdm001/#/file", KEY_META, "default", "lcdm001", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcdm001/#/forwardkey", KEY_META, "check/type", "string", KEY_META, "default", "DownKey", KEY_META, "description", "Normal Context: Forward(Go to next screen) \nMenu Context: Down/Right", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcdm001/#/mainmenukey", KEY_META, "check/type", "string", KEY_META, "default", "RightKey", KEY_META, "description", "Normal Context: Open main menu \nMenu Context: Exit/Cancel", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcdm001/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcdm001/#/pausekey", KEY_META, "check/type", "string", KEY_META, "default", "LeftKey", KEY_META, "description", "Normal Context: Pause/Continue \nMenu Context: Enter/select", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcdm001/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcterm", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcterm/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a lcterm driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "LctermDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcterm/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcterm/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcterm/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyS1", KEY_META, "description", "select the device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcterm/#/file", KEY_META, "default", "lcterm", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcterm/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcterm/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lcterm/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "Specify the size of the display in pixels", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/linux_input", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/linux_input/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a linux_input driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Linux_inputDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/linux_input/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/linux_input/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/linux_input/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/input/event0", KEY_META, "description", "select the device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/linux_input/#/file", KEY_META, "default", "linux_input", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/linux_input/#/key/_", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "specify a non-default key map, eg.: \n/linux_input/key/#01 = 1\n/linux_input/key/#28 = Enter", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/linux_input/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/linux_input/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lirc", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lirc/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a lirc driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "LircDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lirc/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lirc/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lirc/#/file", KEY_META, "default", "lirc", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lirc/#/lircrc", KEY_META, "check/type", "string", KEY_META, "default", "~/.lircrc", KEY_META, "description", "select the device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lirc/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lirc/#/prog", KEY_META, "check/type", "string", KEY_META, "default", "lcdd", KEY_META, "description", "Must be the same as in your lircrc", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lirc/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lis", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lis/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a lis driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "LisDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lis/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial brightness\n0-250 = 25%, 251-500 = 50%, 501-750 = 75%, 751-1000 = 100%", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lis/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lis/#/file", KEY_META, "default", "lis", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lis/#/lastline", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Specifies if the last line is pixel addressable (yes) or it only controls an\nunderline effect (no)", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lis/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lis/#/productid", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([0-9A-F]{4})", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size ProductID: 0x0000-0xFFFF is allowed", KEY_META, "default", "0x6001", KEY_META, "description", "USB Product ID. Change only if testing a compatible device.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lis/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lis/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "Columns by lines", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/lis/#/vendorid", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([0-9A-F]{4})", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size VendorID: 0x0000-0xFFFF is allowed", KEY_META, "default", "0x0403", KEY_META, "description", "USB Vendor ID. Change only if testing a compatible device.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/md8800", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/md8800/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a MD8800 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "MD8800DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/md8800/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/md8800/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/md8800/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyS1", KEY_META, "description", "select the device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/md8800/#/file", KEY_META, "default", "MD8800", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/md8800/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Set the initial off-brightness\nThis value is used when the display is normally\nswitched off in case LCDd is inactive", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/md8800/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/md8800/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "display size", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mdm166a", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mdm166a/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a mdm166a driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Mdm166aDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mdm166a/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mdm166a/#/clock", KEY_META, "check/enum", "#2", KEY_META, "check/enum/#0", "no", KEY_META, "check/enum/#1", "small", KEY_META, "check/enum/#2", "big", KEY_META, "check/type", "enum", KEY_META, "default", "no", KEY_META, "description", "Show self-running clock after LCDd shutdown", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mdm166a/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mdm166a/#/dimming", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Dim display, no dimming gives full brightness", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mdm166a/#/file", KEY_META, "default", "mdm166a", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mdm166a/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mdm166a/#/offdimming", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Dim display in case LCDd is inactive", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mdm166a/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/menu/downkey", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Up to six keys are supported. The MenuKey (to enter and exit the menu), the\nEnterKey (to select values) and at least one movement keys are required.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/menu/enterkey", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Up to six keys are supported. The MenuKey (to enter and exit the menu), the\nEnterKey (to select values) and at least one movement keys are required.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/menu/leftkey", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Up to six keys are supported. The MenuKey (to enter and exit the menu), the\nEnterKey (to select values) and at least one movement keys are required.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/menu/menukey", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Up to six keys are supported. The MenuKey (to enter and exit the menu), the\nEnterKey (to select values) and at least one movement keys are required.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/menu/permissivegoto", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "If true the server allows transitions between different client`s menus", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/menu/rightkey", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Up to six keys are supported. The MenuKey (to enter and exit the menu), the\nEnterKey (to select values) and at least one movement keys are required.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/menu/upkey", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Up to six keys are supported. The MenuKey (to enter and exit the menu), the\nEnterKey (to select values) and at least one movement keys are required.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ms6931", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ms6931/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a ms6931 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Ms6931DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ms6931/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ms6931/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ms6931/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyS1", KEY_META, "description", "select the device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ms6931/#/file", KEY_META, "default", "ms6931", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ms6931/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ms6931/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ms6931/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "display size", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtc_s16209x", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtc_s16209x/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a mtc_s16209x driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Mtc_s16209xDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtc_s16209x/#/brightness", KEY_META, "check/range", "0-255", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "200", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtc_s16209x/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtc_s16209x/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtc_s16209x/#/file", KEY_META, "default", "mtc_s16209x", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtc_s16209x/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtc_s16209x/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a MtxOrb driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "MtxOrbDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "480", KEY_META, "description", "Set the initial contrast\nNOTE: The driver will ignore this if the display\nis a vfd or vkd as they don\'t have this feature", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyS0", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb/#/file", KEY_META, "default", "MtxOrb", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb/#/hasadjustablebacklight", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Some old displays do not have an adjustable backlight but only can\nswitch the backlight on/off. If you experience randomly appearing block\ncharacters, try setting this to false.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb/#/keymap_a", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "By default no keys are mapped, meaning the keypad is not used at all.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb/#/keymap_b", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "By default no keys are mapped, meaning the keypad is not used at all.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb/#/keymap_c", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "By default no keys are mapped, meaning the keypad is not used at all.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb/#/keymap_d", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "By default no keys are mapped, meaning the keypad is not used at all.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb/#/keymap_e", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "By default no keys are mapped, meaning the keypad is not used at all.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb/#/keymap_f", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "By default no keys are mapped, meaning the keypad is not used at all.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Set the initial off-brightness\nThis value is used when the display is normally\nswitched off in case LCDd is inactive", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "display size", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb/#/speed", KEY_META, "check/range", "1200, 2400, 9600, 19200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "19200", KEY_META, "description", "Set the communication", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mtxorb/#/type", KEY_META, "check/enum", "#3", KEY_META, "check/enum/#0", "lcd lkd, vfd, vkd", KEY_META, "check/enum/#1", "lkd", KEY_META, "check/enum/#2", "vfd", KEY_META, "check/enum/#3", "vkd", KEY_META, "check/type", "enum", KEY_META, "default", "lcd", KEY_META, "description", "Set the display type", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mx5000", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mx5000/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a mx5000 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Mx5000DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mx5000/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mx5000/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mx5000/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/hiddev0", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mx5000/#/file", KEY_META, "default", "mx5000", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mx5000/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mx5000/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/mx5000/#/waitafterrefresh", KEY_META, "check/type", "unsigned_long", KEY_META, "check/validation", "[1-9]\\d*", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "WaitAfterRefresh must be a positive number", KEY_META, "default", "1000", KEY_META, "description", "Time to wait in ms after the refresh screen has been sent", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/noritakevfd", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/noritakevfd/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a NoritakeVFD driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "NoritakeVFDDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/noritakevfd/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/noritakevfd/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/noritakevfd/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "device where the VFD is. Usual values are /dev/ttyS0 and /dev/ttyS1", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/noritakevfd/#/file", KEY_META, "default", "NoritakeVFD", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/noritakevfd/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Set the initial off-brightness\nThis value is used when the display is normally\nswitched off in case LCDd is inactive", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/noritakevfd/#/parity", KEY_META, "check/range", "0-2", KEY_META, "check/type", "unsigned_long", KEY_META, "default", "0", KEY_META, "description", "Set serial data parity\nMeaning: 0(=none), 1(=odd), 2(=even)", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/noritakevfd/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/noritakevfd/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "display size", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/noritakevfd/#/speed", KEY_META, "check/range", "1200, 2400, 9600, 19200, 115200", KEY_META, "check/type", "unsigned_long", KEY_META, "default", "19200", KEY_META, "description", "set the serial port speed", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/olimex_mod_lcd1x9", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/olimex_mod_lcd1x9/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a Olimex_MOD_LCD1x9 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Olimex_MOD_LCD1x9DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/olimex_mod_lcd1x9/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/olimex_mod_lcd1x9/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/olimex_mod_lcd1x9/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/i2c-0", KEY_META, "description", "device file of the i2c controller", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/olimex_mod_lcd1x9/#/file", KEY_META, "default", "Olimex_MOD_LCD1x9", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/olimex_mod_lcd1x9/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/olimex_mod_lcd1x9/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a picolcd driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "PicolcdDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/backlight", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Sets the initial state of the backlight upon start-up.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial brightness [default: 1000; legal: 0 - 1000]. Works only\nwith the 20x4 device", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/file", KEY_META, "default", "picolcd", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/key0light", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If Keylights is on, the you can unlight specific keys below:\nKey0 is the directional pad.  Key1 - Key5 correspond to the F1 - F5 keys.\nThere is no LED for the +/- keys.  This is a handy way to indicate to users\nwhich keys are disabled.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/key1light", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If Keylights is on, the you can unlight specific keys below:\nKey0 is the directional pad.  Key1 - Key5 correspond to the F1 - F5 keys.\nThere is no LED for the +/- keys.  This is a handy way to indicate to users\nwhich keys are disabled.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/key2light", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If Keylights is on, the you can unlight specific keys below:\nKey0 is the directional pad.  Key1 - Key5 correspond to the F1 - F5 keys.\nThere is no LED for the +/- keys.  This is a handy way to indicate to users\nwhich keys are disabled.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/key3light", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If Keylights is on, the you can unlight specific keys below:\nKey0 is the directional pad.  Key1 - Key5 correspond to the F1 - F5 keys.\nThere is no LED for the +/- keys.  This is a handy way to indicate to users\nwhich keys are disabled.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/key4light", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If Keylights is on, the you can unlight specific keys below:\nKey0 is the directional pad.  Key1 - Key5 correspond to the F1 - F5 keys.\nThere is no LED for the +/- keys.  This is a handy way to indicate to users\nwhich keys are disabled.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/key5light", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If Keylights is on, the you can unlight specific keys below:\nKey0 is the directional pad.  Key1 - Key5 correspond to the F1 - F5 keys.\nThere is no LED for the +/- keys.  This is a handy way to indicate to users\nwhich keys are disabled.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/keylights", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Light the keys\?", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/keyrepeatdelay", KEY_META, "check/range", "0-3000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "300", KEY_META, "description", "Key auto repeat is only available if the picoLCD driver is built with\nlibusb-1.0. Use KeyRepeatDelay and KeyRepeatInterval to configure key auto\nrepeat.\n#\nKey auto repeat delay (time in ms from first key report to first repeat). Use\nzero to disable auto repeat.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/keyrepeatinterval", KEY_META, "check/range", "0-3000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "200", KEY_META, "description", "Key auto repeat interval (time in ms between repeat reports). Only used if\nKeyRepeatDelay is not zero.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/keytimeout", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "500", KEY_META, "description", "KeyTimeout is only used if the picoLCD driver is built with libusb-0.1. When\nbuilt with libusb-1.0 key and IR data is input asynchronously so there is no\nneed to wait for the USB data.\nKeyTimeout is the time in ms that LCDd spends waiting for a key press before\ncycling through other duties.  Higher values make LCDd use less CPU time and\nmake key presses more detectable.  Lower values make LCDd more responsive\nbut a little prone to missing key presses.  500 (.5 second) is the default\nand a balanced value.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/linklights", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Link the key lights to the backlight\?", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/lircflushthreshold", KEY_META, "check/type", "unsigned_long", KEY_META, "check/validation", "[1-9]\\d{3,}", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "LircFlushThreshold must be a positive number >1000", KEY_META, "default", "1000", KEY_META, "description", "Threshold in microseconds of the gap that triggers flushing the IR data\nto lirc [default: 8000; legal: 1000 - ]\nIf LircTime_us is on values greater than 32.767ms will disable the flush\nIf LircTime_us is off values greater than 1.999938s will disable the flush", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/lirchost", KEY_META, "check/type", "string", KEY_META, "default", "127.0.0.1", KEY_META, "description", "Host name or IP address of the LIRC instance that is to receive IR codes If not set, or set to an empty value, IR support is disabled.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/lircport", KEY_META, "default", "8765", KEY_META, "description", "UDP port on which LIRC is listening", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/lirctime_us", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "UDP data time unit for LIRC\nOn:  times sent in microseconds (requires LIRC UDP driver that accepts this).\nOff: times sent in \'jiffies\' (1/16384s) (supported by standard LIRC UDP driver).", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Set the brightness while the backlight is \'off\'.\nWorks only with the 20x4 device.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/picolcd/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/pyramid", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/pyramid/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a pyramid driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "PyramidDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/pyramid/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/pyramid/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/pyramid/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "device to connect to", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/pyramid/#/file", KEY_META, "default", "pyramid", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/pyramid/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/pyramid/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/rawserial", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/rawserial/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a rawserial driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "RawserialDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/rawserial/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/rawserial/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/rawserial/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/cuaU0", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/rawserial/#/file", KEY_META, "default", "rawserial", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/rawserial/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/rawserial/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/rawserial/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "40x4", KEY_META, "description", "Specifies the size of the LCD. If this driver is loaded as a secondary driver\nit always adopts to the size of the primary driver. If loaded as the only\n(or primary) driver, the size can be set.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/rawserial/#/speed", KEY_META, "check/type", "unsigned_long", KEY_META, "default", "9600", KEY_META, "description", "Serial port baudrate [default: 9600]", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/rawserial/#/updaterate", KEY_META, "check/type", "float", KEY_META, "default", "1", KEY_META, "description", "How often to dump the LCD contents out the port, in Hertz (times per second)\n1 = once per second, 4 is 4 times per second, 0.1 is once every 10 seconds.\n[default: 1; legal: 0.0005 - 10]", KEY_META, "type", "float", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1330", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1330/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a sed1330 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Sed1330DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1330/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1330/#/cellsize", KEY_META, "check/type", "string", KEY_META, "check/validation", "[6-8]x([1-9][0-6]|[1-9])", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Width x Height of a character cell in pixels [legal: 6x7 - 8x16]", KEY_META, "default", "6x10", KEY_META, "description", "Width x Height of a character cell in pixels", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1330/#/connectiontype", KEY_META, "check/enum", "#1", KEY_META, "check/enum/#0", "classic", KEY_META, "check/enum/#1", "bitshaker", KEY_META, "check/type", "enum", KEY_META, "default", "classic", KEY_META, "description", "Select what type of connection", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1330/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1330/#/file", KEY_META, "default", "sed1330", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1330/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1330/#/port", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([0-9A-F]{3})", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid port declaration. It should start with 0x, followed by 3 HEX values. eg. 0x3BC", KEY_META, "default", "0x378", KEY_META, "description", "Port where the LPT is. Common values are 0x278, 0x378 and 0x3BC", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1330/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1330/#/type", KEY_META, "check/enum", "#5", KEY_META, "check/enum/#0", "G321D", KEY_META, "check/enum/#1", "G121C", KEY_META, "check/enum/#2", "G242C", KEY_META, "check/enum/#3", "G191D", KEY_META, "check/enum/#4", "G2446", KEY_META, "check/enum/#5", "SP14Q002", KEY_META, "check/type", "enum", KEY_META, "default", "G321D", KEY_META, "description", "Type of LCD module.\nNote: Currently only tested with G321D & SP14Q002.", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1520", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1520/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a sed1520 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Sed1520DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1520/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1520/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1520/#/delaymult", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1", KEY_META, "description", "On fast machines it may be necessary to slow down transfer to the display.\nIf this value is set to zero, delay is disabled. Any value greater than\nzero slows down each write by one microsecond.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1520/#/file", KEY_META, "default", "sed1520", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1520/#/haveinverter", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "The original wiring used an inverter to drive the control lines. If you do\nnot use an inverter set haveInverter to no.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1520/#/interfacetype", KEY_META, "check/range", "68, 80", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "80", KEY_META, "description", "Select the interface type (wiring) for the display. Supported values are\n68 for 68-style connection (RESET level high) and 80 for 80-style connection\n(RESET level low)", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1520/#/invertedmapping", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "On some displays column data in memory is mapped to segment lines from right\nto left. This is called inverted mapping (not to be confused with\n\'haveInverter\' from above).", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1520/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1520/#/port", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([0-9A-F]{3})", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid port declaration. It should start with 0x, followed by 3 HEX values. eg. 0x3BC", KEY_META, "default", "0x378", KEY_META, "description", "Port where the LPT is. Usual values are 0x278, 0x378 and 0x3BC", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1520/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sed1520/#/usehardreset", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "At least one display is reported (Everbouquet MG1203D) that requires sending\nthree times 0xFF before a reset during initialization.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialpos", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialpos/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a serialPOS driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "SerialPOSDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialpos/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialpos/#/cellsize", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "5x8", KEY_META, "description", "Specifies the cell size of each character cell on the display in characters.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialpos/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialpos/#/custom_chars", KEY_META, "check/type", "unsigned_long", KEY_META, "check/validation", "^([1-9]\\d*|0)$", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Must be any positive number including 0", KEY_META, "default", "0", KEY_META, "description", "Specifies the number of custom characters supported by the display.\nCustom characters are only used for the rendering of horizontal bars\nand vertical bars. For displays whose cell character cell widths are\nlower than the number of custom characters supported,\nthen custom characters will be used to render the horizontal bars.\nFor more information look at: https://github.com/lcdproc/lcdproc/blob/master/docs/lcdproc-user/drivers/serialPOS.docbook", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialpos/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyS0", KEY_META, "description", "Device to use in serial mode", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialpos/#/file", KEY_META, "default", "serialPOS", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialpos/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialpos/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialpos/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "Specifies the size of the display in characters.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialpos/#/speed", KEY_META, "check/range", "1200, 2400, 4800, 9600, 19200, 115200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "9600", KEY_META, "description", "communication baud rate with the display", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialpos/#/type", KEY_META, "check/enum", "#5", KEY_META, "check/enum/#0", "AEDEX", KEY_META, "check/enum/#1", "CD5220", KEY_META, "check/enum/#2", "Epson", KEY_META, "check/enum/#3", "Emax", KEY_META, "check/enum/#4", "LogicControls", KEY_META, "check/enum/#5", "Ultimate", KEY_META, "check/type", "enum", KEY_META, "default", "AEDEX", KEY_META, "description", "Set the communication protocol to use with the POS display.", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialvfd", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialvfd/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a serialVFD driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "SerialVFDDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialvfd/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial brightness. (4 steps 0-250, 251-500, 501-750, 751-1000)", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialvfd/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialvfd/#/custom-characters", KEY_META, "check/type", "unsigned_long", KEY_META, "check/validation", "^([1-9]\\d*|0)$", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Must be any positive number including 0", KEY_META, "default", "0", KEY_META, "description", "Number of Custom-Characters. default is display type dependent", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialvfd/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyS1", KEY_META, "description", "Device to use in serial mode. Usual values are /dev/ttyS0 and /dev/ttyS1", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialvfd/#/file", KEY_META, "default", "serialVFD", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialvfd/#/iso_8859_1", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "enable ISO 8859 1 compatibility", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialvfd/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Set the initial off-brightness\nThis value is used when the display is normally\nswitched off in case LCDd is inactive\n(4 steps 0-250, 251-500, 501-750, 751-1000)", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialvfd/#/port", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([0-9A-F]{3})", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid port declaration. It should start with 0x, followed by 3 HEX values. eg. 0x3BC", KEY_META, "default", "0x378", KEY_META, "description", "Portaddress where the LPT is. Used in parallel mode only. Usual values are 0x278, 0x378 and 0x3BC.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialvfd/#/portwait", KEY_META, "check/range", "0-255", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "2", KEY_META, "description", "Set parallel port timing delay (us). Used in parallel mode only.", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialvfd/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialvfd/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x2", KEY_META, "description", "Specifies the size of the VFD.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialvfd/#/speed", KEY_META, "check/range", "1200, 2400, 9600, 19200, 115200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "9600", KEY_META, "description", "set the serial port speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialvfd/#/type", KEY_META, "check/enum", "#_17", KEY_META, "check/enum/#0", "nec_fipc8367", KEY_META, "check/enum/#1", "kd_rev_2_1", KEY_META, "check/enum/#2", "noritake_vfd", KEY_META, "check/enum/#3", "futaba_vfd", KEY_META, "check/enum/#4", "iee_s03601-95b", KEY_META, "check/enum/#5", "iee_s03601-96-080", KEY_META, "check/enum/#6", "futaba_na202sd08fa", KEY_META, "check/enum/#7", "samsung_20s207da4/20s207da6", KEY_META, "check/enum/#8", "nixdorf_ba6x/vt100", KEY_META, "check/enum/#_10", "1", KEY_META, "check/enum/#_11", "2", KEY_META, "check/enum/#_12", "3", KEY_META, "check/enum/#_13", "4", KEY_META, "check/enum/#_14", "5", KEY_META, "check/enum/#_15", "6", KEY_META, "check/enum/#_16", "7", KEY_META, "check/enum/#_17", "8", KEY_META, "check/enum/#_9", "0", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "description", "Specifies the displaytype.[default: 0]\n0 NEC (FIPC8367 based) VFDs.\n1 KD Rev 2.1.\n2 Noritake VFDs (*).\n3 Futaba VFDs\n4 IEE S03601-95B\n5 IEE S03601-96-080 (*)\n6 Futaba NA202SD08FA (allmost IEE compatible)\n7 Samsung 20S207DA4 and 20S207DA6\n8 Nixdorf BA6x / VT100\n(* most should work, not tested yet.)", KEY_META, "gen/enum/type", "SerialVFDType", KEY_META, "gen/enum/value", "8", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/serialvfd/#/use_parallel", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "\'no\' if display connected serial, \'yes\' if connected parallel.\nI.e. serial by default", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/autorotate", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "If set to no, LCDd will start with screen rotation disabled. This has the same effect as if the ToggleRotateKey had been pressed. Rotation will start if the ToggleRotateKey is pressed. Note that this setting does not turn off priority sorting of screens", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/backlight", KEY_META, "check/enum", "#2", KEY_META, "check/enum/#0", "off", KEY_META, "check/enum/#1", "on", KEY_META, "check/enum/#2", "open", KEY_META, "default", "open", KEY_META, "description", "Set master backlight setting. If set to \'open\' a client may control the backlight for its own screens (only)", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/bind", KEY_META, "check/type", "string", KEY_META, "default", "127.0.0.1", KEY_META, "description", "Tells the driver to bind to the given interface", KEY_META, "opt", "a", KEY_META, "opt/long", "address", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/driverpath", KEY_META, "check/type", "string", KEY_META, "check/validation", ".*[\\/]$", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "The path has to contain a backslack (/) at the end", KEY_META, "default", "server/drivers/", KEY_META, "description", "Select the LCD size", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/drivers", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/drivers/#", KEY_META, "check/reference/restrict", "#_52", KEY_META, "check/reference/restrict/#0", "@/bayrad/#", KEY_META, "check/reference/restrict/#1", "@/cfontz/#", KEY_META, "check/reference/restrict/#2", "@/cfontzpacket/#", KEY_META, "check/reference/restrict/#3", "@/curses/#", KEY_META, "check/reference/restrict/#4", "@/cwlnx/#", KEY_META, "check/reference/restrict/#5", "@/ea65/#", KEY_META, "check/reference/restrict/#6", "@/eyeboxone/#", KEY_META, "check/reference/restrict/#7", "@/futaba/#", KEY_META, "check/reference/restrict/#8", "@/g15/#", KEY_META, "check/reference/restrict/#9", "@/glcd/#", KEY_META, "check/reference/restrict/#_10", "@/glcdlib/#", KEY_META, "check/reference/restrict/#_11", "@/glk/#", KEY_META, "check/reference/restrict/#_12", "@/hd44780/#", KEY_META, "check/reference/restrict/#_13", "@/icp_a106/#", KEY_META, "check/reference/restrict/#_14", "@/imon/#", KEY_META, "check/reference/restrict/#_15", "@/imonlcd/#", KEY_META, "check/reference/restrict/#_16", "@/iowarrior/#", KEY_META, "check/reference/restrict/#_17", "@/irman/#", KEY_META, "check/reference/restrict/#_18", "@/joy/#", KEY_META, "check/reference/restrict/#_19", "@/lb216/#", KEY_META, "check/reference/restrict/#_20", "@/lcdm001/#", KEY_META, "check/reference/restrict/#_21", "@/lcterm/#", KEY_META, "check/reference/restrict/#_22", "@/linux_input/#", KEY_META, "check/reference/restrict/#_23", "@/lirc/#", KEY_META, "check/reference/restrict/#_24", "@/lis/#", KEY_META, "check/reference/restrict/#_25", "@/md8800/#", KEY_META, "check/reference/restrict/#_26", "@/mdm166a/#", KEY_META, "check/reference/restrict/#_27", "@/ms6931/#", KEY_META, "check/reference/restrict/#_28", "@/mtc_s16209x/#", KEY_META, "check/reference/restrict/#_29", "@/mtxorb/#", KEY_META, "check/reference/restrict/#_30", "@/mx5000/#", KEY_META, "check/reference/restrict/#_31", "@/noritakevfd/#", KEY_META, "check/reference/restrict/#_32", "@/olimex_mod_lcd1x9/#", KEY_META, "check/reference/restrict/#_33", "@/picolcd/#", KEY_META, "check/reference/restrict/#_34", "@/pyramid/#", KEY_META, "check/reference/restrict/#_35", "@/rawserial/#", KEY_META, "check/reference/restrict/#_36", "@/sdeclcd/#", KEY_META, "check/reference/restrict/#_37", "@/sed1330/#", KEY_META, "check/reference/restrict/#_38", "@/sed1520/#", KEY_META, "check/reference/restrict/#_39", "@/serialpos/#", KEY_META, "check/reference/restrict/#_40", "@/serialvfd/#", KEY_META, "check/reference/restrict/#_41", "@/shuttlevfd/#", KEY_META, "check/reference/restrict/#_42", "@/sli/#", KEY_META, "check/reference/restrict/#_43", "@/stv5730/#", KEY_META, "check/reference/restrict/#_44", "@/svga/#", KEY_META, "check/reference/restrict/#_45", "@/t6963/#", KEY_META, "check/reference/restrict/#_46", "@/text/#", KEY_META, "check/reference/restrict/#_47", "@/tyan/#", KEY_META, "check/reference/restrict/#_48", "@/ula200/#", KEY_META, "check/reference/restrict/#_49", "@/vlsys_m428/#", KEY_META, "check/reference/restrict/#_50", "@/xosd/#", KEY_META, "check/reference/restrict/#_51", "@/yard2lcd/#", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "Tells the server to load a driver.\nThe given value is a reference the configuration of the driver, e.g. @/curses/#0", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/foreground", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "The server will stay in the foreground if set to yes", KEY_META, "opt", "f", KEY_META, "opt/arg", "none", KEY_META, "opt/long", "foreground", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/frameinterval", KEY_META, "check/type", "unsigned_long", KEY_META, "check/validation", "[1-9]\\d*", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "FrameInterval must be a positive number", KEY_META, "default", "125000", KEY_META, "description", "Sets the interval in microseconds for updating the display [default: 125000 meaning 8Hz]", KEY_META, "type", "unsigned_long", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/goodbye/#", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "GoodBye message: each entry represents a display line", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/heartbeat", KEY_META, "check/enum", "#2", KEY_META, "check/enum/#0", "off", KEY_META, "check/enum/#1", "on", KEY_META, "check/enum/#2", "open", KEY_META, "default", "open", KEY_META, "description", "Set master heartbeat setting. If set to \'open\' a client may control the", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/hello/#", KEY_META, "check/type", "string", KEY_META, "default", "", KEY_META, "description", "Hello message: each entry represents a display line", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/nextscreenkey", KEY_META, "check/type", "string", KEY_META, "default", "Right", KEY_META, "description", "The \'...Key=\' lines define what the server does with keypresses that\ndon\'t go to any client. The ToggleRotateKey stops rotation of screens, while\nthe PrevScreenKey and NextScreenKey go back / forward one screen (even if\nrotation is disabled.\nAssign the key string returned by the driver to the ...Key setting.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/port", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "13666", KEY_META, "description", "Listen on this specified port. [default: 13666]", KEY_META, "opt", "p", KEY_META, "opt/long", "port", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/prevscreenkey", KEY_META, "check/type", "string", KEY_META, "default", "Left", KEY_META, "description", "The \'...Key=\' lines define what the server does with keypresses that\ndon\'t go to any client. The ToggleRotateKey stops rotation of screens, while\nthe PrevScreenKey and NextScreenKey go back / forward one screen (even if\nrotation is disabled.\nAssign the key string returned by the driver to the ...Key setting.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/reportlevel", KEY_META, "check/range", "0-5", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "2", KEY_META, "description", "TSets the reporting level, defaults to warnings and errors only.", KEY_META, "opt", "r", KEY_META, "opt/long", "report-level", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/reporttosyslog", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Should we report to syslog instead of stderr\?", KEY_META, "opt", "s", KEY_META, "opt/arg", "none", KEY_META, "opt/long", "report-to-syslog", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/scrolldownkey", KEY_META, "check/type", "string", KEY_META, "default", "Down", KEY_META, "description", "The \'...Key=\' lines define what the server does with keypresses that\ndon\'t go to any client. The ToggleRotateKey stops rotation of screens, while\nthe PrevScreenKey and NextScreenKey go back / forward one screen (even if\nrotation is disabled.\nAssign the key string returned by the driver to the ...Key setting.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/scrollupkey", KEY_META, "check/type", "string", KEY_META, "default", "Up", KEY_META, "description", "The \'...Key=\' lines define what the server does with keypresses that\ndon\'t go to any client. The ToggleRotateKey stops rotation of screens, while\nthe PrevScreenKey and NextScreenKey go back / forward one screen (even if\nrotation is disabled.\nAssign the key string returned by the driver to the ...Key setting.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/serverscreen", KEY_META, "check/enum", "#2", KEY_META, "check/enum/#0", "off", KEY_META, "check/enum/#1", "on", KEY_META, "check/enum/#2", "blank", KEY_META, "default", "on", KEY_META, "description", "If yes, the the serverscreen will be rotated as a usual info screen. If no, it will be a background screen, only visible when no other screens are active. The special value \'blank\' is similar to no, but only a blank screen is displayed.", KEY_META, "opt", "i", KEY_META, "opt/long", "rotate-server-screen", KEY_META, "type", "enum", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/titlespeed", KEY_META, "check/range", "0-10", KEY_META, "default", "10", KEY_META, "description", "title scrolling speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/togglerotatekey", KEY_META, "check/type", "string", KEY_META, "default", "Enter", KEY_META, "description", "The \'...Key=\' lines define what the server does with keypresses that\ndon\'t go to any client. The ToggleRotateKey stops rotation of screens, while\nthe PrevScreenKey and NextScreenKey go back / forward one screen (even if\nrotation is disabled.\nAssign the key string returned by the driver to the ...Key setting.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/user", KEY_META, "check/type", "string", KEY_META, "default", "nobody", KEY_META, "description", "set to run as.  LCDd will drop its root privileges and run as this user instead. [default: nobody]", KEY_META, "opt", "u", KEY_META, "opt/long", "user", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/server/waittime", KEY_META, "check/type", "float", KEY_META, "default", "4", KEY_META, "description", "Sets the default time in seconds to displays a screen", KEY_META, "opt", "w", KEY_META, "opt/long", "wait-time", KEY_META, "type", "float", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sli", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sli/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a sli driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "SliDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sli/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sli/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sli/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sli/#/file", KEY_META, "default", "sli", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sli/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sli/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sli/#/speed", KEY_META, "check/range", "1200, 2400, 9600, 19200, 38400, 57600, 115200", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "19200", KEY_META, "description", "Set the communication speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/stv5730", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/stv5730/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a stv5730 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Stv5730DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/stv5730/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/stv5730/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/stv5730/#/file", KEY_META, "default", "stv5730", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/stv5730/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/stv5730/#/port", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([0-9A-F]{3})", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid port declaration. It should start with 0x, followed by 3 HEX values. eg. 0x3BC", KEY_META, "default", "0x378", KEY_META, "description", "Port the device is connected to", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/stv5730/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sureelec", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sureelec/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a SureElec driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "SureElecDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sureelec/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "480", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sureelec/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "480", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sureelec/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyUSB0", KEY_META, "description", "Port the device is connected to  (by default first USB serial port)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sureelec/#/edition", KEY_META, "check/range", "1-3", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "2", KEY_META, "description", "Edition level of the device (can be 1, 2 or 3)", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sureelec/#/file", KEY_META, "default", "SureElec", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sureelec/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "480", KEY_META, "description", "Set the initial off-brightness\nThis value is used when the display is normally\nswitched off in case LCDd is inactive", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sureelec/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/sureelec/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "set display size\nNote: The size can be obtained directly from device for edition 2 & 3.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/svga", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/svga/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a svga driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "SvgaDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/svga/#/brightness", KEY_META, "check/range", "1-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/svga/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "500", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/svga/#/file", KEY_META, "default", "svga", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/svga/#/mode", KEY_META, "check/type", "string", KEY_META, "default", "G320x240x256", KEY_META, "description", "svgalib mode to use [default: G320x240x256 ]\nlegal values are supported svgalib modes. See man7 pages for allowed values", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/svga/#/offbrightness", KEY_META, "check/range", "1-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "500", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/svga/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/svga/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "set display size", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/t6963", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/t6963/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a t6963 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "T6963DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/t6963/#/bidirectional", KEY_META, "check/type", "boolean", KEY_META, "default", "1", KEY_META, "description", "Use LPT port in bi-directional mode. This should work on most LPT port and\nis required for proper timing!", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/t6963/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/t6963/#/cleargraphic", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Clear graphic memory on start-up.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/t6963/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/t6963/#/delaybus", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Insert additional delays into reads / writes.", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/t6963/#/file", KEY_META, "default", "t6963", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/t6963/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/t6963/#/port", KEY_META, "check/type", "string", KEY_META, "check/validation", "0x([2-3][0-9A-F]{2}|400)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid port declaration. legal: 0x200 - 0x400", KEY_META, "default", "0x378", KEY_META, "description", "Parallel port to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/t6963/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/t6963/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "128x64", KEY_META, "description", "set display size in pixels", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/text", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/text/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a text driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "TextDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/text/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/text/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/text/#/file", KEY_META, "default", "text", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/text/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/text/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/text/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "set display size", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/tyan", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/tyan/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a tyan driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "TyanDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/tyan/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/tyan/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/tyan/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/lcd", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/tyan/#/file", KEY_META, "default", "tyan", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/tyan/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/tyan/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/tyan/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "16x2", KEY_META, "description", "set display size", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/tyan/#/speed", KEY_META, "check/range", "4800, 9600", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "9600", KEY_META, "description", "Set the communication speed", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ula200", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ula200/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a ula200 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Ula200DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ula200/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ula200/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ula200/#/file", KEY_META, "default", "ula200", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ula200/#/keymap_a", KEY_META, "check/type", "string", KEY_META, "default", "Up", KEY_META, "description", "If you have a non standard keypad you can associate any keystrings to keys.\nThere are 6 input key in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n#\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ula200/#/keymap_b", KEY_META, "check/type", "string", KEY_META, "default", "Down", KEY_META, "description", "If you have a non standard keypad you can associate any keystrings to keys.\nThere are 6 input key in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n#\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ula200/#/keymap_c", KEY_META, "check/type", "string", KEY_META, "default", "Left", KEY_META, "description", "If you have a non standard keypad you can associate any keystrings to keys.\nThere are 6 input key in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n#\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ula200/#/keymap_d", KEY_META, "check/type", "string", KEY_META, "default", "Right", KEY_META, "description", "If you have a non standard keypad you can associate any keystrings to keys.\nThere are 6 input key in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n#\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ula200/#/keymap_e", KEY_META, "check/type", "string", KEY_META, "default", "Enter", KEY_META, "description", "If you have a non standard keypad you can associate any keystrings to keys.\nThere are 6 input key in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n#\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ula200/#/keymap_f", KEY_META, "check/type", "string", KEY_META, "default", "Escape", KEY_META, "description", "If you have a non standard keypad you can associate any keystrings to keys.\nThere are 6 input key in the CwLnx hardware that generate characters\nfrom \'A\' to \'F\'.\n#\nYou can leave those unchanged if you have a standard keypad.\nYou can change it if you want to report other keystrings or have a non\nstandard keypad.", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ula200/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ula200/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/ula200/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "Select the LCD size", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/vlsys_m428", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/vlsys_m428/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a vlsys_m428 driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Vlsys_m428DriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/vlsys_m428/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/vlsys_m428/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/vlsys_m428/#/device", KEY_META, "check/type", "string", KEY_META, "default", "/dev/ttyUSB0", KEY_META, "description", "Select the output device to use", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/vlsys_m428/#/file", KEY_META, "default", "vlsys_m428", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/vlsys_m428/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/vlsys_m428/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/xosd", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/xosd/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a xosd driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "XosdDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/xosd/#/Font", KEY_META, "check/type", "string", KEY_META, "default", "-*-terminus-*-r-*-*-*-320-*-*-*-*-*", KEY_META, "description", "X font to use, in XLFD format, as given by \'xfontsel\'", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/xosd/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/xosd/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/xosd/#/file", KEY_META, "default", "xosd", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/xosd/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/xosd/#/offset", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid offset declaration. Examples: 200x200", KEY_META, "default", "0x0", KEY_META, "description", "Offset in pixels from the top-left corner of the monitor [default: 0x0]", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/xosd/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/xosd/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "set display size", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/yard2lcd", KEY_META, "array", "#0", KEY_META, "default", "", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/yard2lcd/#", KEY_META, "check/type", "any", KEY_META, "default", "", KEY_META, "description", "Configuration for a yard2LCD driver.", KEY_META, "gen/struct/alloc", "0", KEY_META, "gen/struct/type", "Yard2LCDDriverConfig", KEY_META, "type", "struct", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/yard2lcd/#/brightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/yard2lcd/#/contrast", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "1000", KEY_META, "description", "Set the initial contrast", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/yard2lcd/#/file", KEY_META, "default", "yard2LCD", KEY_META, "description", "name of the shared library to load (without prefix and extension)", KEY_META, "type", "string", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/yard2lcd/#/offbrightness", KEY_META, "check/range", "0-1000", KEY_META, "check/type", "unsigned_short", KEY_META, "default", "0", KEY_META, "type", "unsigned_short", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/yard2lcd/#/reboot", KEY_META, "check/type", "boolean", KEY_META, "default", "0", KEY_META, "description", "Reinitialize the LCD`s BIOS normally you shouldn`t need this", KEY_META, "type", "boolean", KEY_END),
	keyNew ("spec/sw/lcdproc/lcdd/#0/current/yard2lcd/#/size", KEY_META, "check/type", "string", KEY_META, "check/validation", "([1-9]+[0-9]*)x([1-9]+[0-9]*)", KEY_META, "check/validation/match", "LINE", KEY_META, "check/validation/message", "Not a valid size declaration. Examples: 20x4, 19x3, 40x150", KEY_META, "default", "20x4", KEY_META, "description", "set display size", KEY_META, "type", "string", KEY_END),
	KS_END);
;

	Key * parentKey = keyNew ("spec/sw/lcdproc/lcdd/#0/current", KEY_END);

	KeySet * specloadConf = ksNew (1, keyNew ("system/sendspec", KEY_END), KS_END);
	ElektraInvokeHandle * specload = elektraInvokeOpen ("specload", specloadConf, parentKey);

	int result = elektraInvoke2Args (specload, "sendspec", spec, parentKey);

	elektraInvokeClose (specload, parentKey);
	keyDel (parentKey);
	ksDel (specloadConf);
	ksDel (spec);

	exit (result == ELEKTRA_PLUGIN_STATUS_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE);
}

/**
 * Extracts the help message from the @p errorKey used in elektraGetOpts().
 *
 * @param errorKey The same Key as passed to elektraGetOpts() as errorKey.
 * @param usage	   If this is not NULL, it will be used instead of the default usage line.
 * @param prefix   If this is not NULL, it will be inserted between the usage line and the options list.
 *
 * @return The full help message extracted from @p errorKey, or NULL if no help message was found.
 * The returned string has to be freed with elektraFree().
 */

/**
 * Outputs the help message to stdout
 *
 * @param usage	   If this is not NULL, it will be used instead of the default usage line.
 * @param prefix   If this is not NULL, it will be inserted between the usage line and the options list.
 */
void printHelpMessage (const char * usage, const char * prefix)
{
	if (helpKey == NULL)
	{
		return;
	}

	char * help = elektraGetOptsHelpMessage (helpKey, usage, prefix);
	printf ("%s", help);
	elektraFree (help);
	keyDel (helpKey);
	helpKey = NULL;
}

// clang-format off

// clang-format on

// -------------------------
// Enum conversion functions
// -------------------------

ELEKTRA_KEY_TO_SIGNATURE (CFontzPacketModel, EnumCFontzPacketModel)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	
	if (strcmp (string, "533") == 0)
	{
		*variable = C_FONTZ_PACKET_MODEL_533;
		return 1;
	}
	if (strcmp (string, "631") == 0)
	{
		*variable = C_FONTZ_PACKET_MODEL_631;
		return 1;
	}
	if (strcmp (string, "633") == 0)
	{
		*variable = C_FONTZ_PACKET_MODEL_633;
		return 1;
	}
	if (strcmp (string, "635") == 0)
	{
		*variable = C_FONTZ_PACKET_MODEL_635;
		return 1;
	}

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (CFontzPacketModel, EnumCFontzPacketModel)
{
	switch (value)
	{
	case C_FONTZ_PACKET_MODEL_533:
		return elektraStrDup ("533");
	case C_FONTZ_PACKET_MODEL_631:
		return elektraStrDup ("631");
	case C_FONTZ_PACKET_MODEL_633:
		return elektraStrDup ("633");
	case C_FONTZ_PACKET_MODEL_635:
		return elektraStrDup ("635");
	}

	// should be unreachable
	return "";
}
ELEKTRA_KEY_TO_SIGNATURE (CursesColor, EnumCursesColor)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	
	if (strcmp (string, "red") == 0)
	{
		*variable = CURSES_COLOR_RED;
		return 1;
	}
	if (strcmp (string, "black") == 0)
	{
		*variable = CURSES_COLOR_BLACK;
		return 1;
	}
	if (strcmp (string, "green") == 0)
	{
		*variable = CURSES_COLOR_GREEN;
		return 1;
	}
	if (strcmp (string, "yellow") == 0)
	{
		*variable = CURSES_COLOR_YELLOW;
		return 1;
	}
	if (strcmp (string, "blue") == 0)
	{
		*variable = CURSES_COLOR_BLUE;
		return 1;
	}
	if (strcmp (string, "magenta") == 0)
	{
		*variable = CURSES_COLOR_MAGENTA;
		return 1;
	}
	if (strcmp (string, "cyan") == 0)
	{
		*variable = CURSES_COLOR_CYAN;
		return 1;
	}
	if (strcmp (string, "white") == 0)
	{
		*variable = CURSES_COLOR_WHITE;
		return 1;
	}

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (CursesColor, EnumCursesColor)
{
	switch (value)
	{
	case CURSES_COLOR_RED:
		return elektraStrDup ("red");
	case CURSES_COLOR_BLACK:
		return elektraStrDup ("black");
	case CURSES_COLOR_GREEN:
		return elektraStrDup ("green");
	case CURSES_COLOR_YELLOW:
		return elektraStrDup ("yellow");
	case CURSES_COLOR_BLUE:
		return elektraStrDup ("blue");
	case CURSES_COLOR_MAGENTA:
		return elektraStrDup ("magenta");
	case CURSES_COLOR_CYAN:
		return elektraStrDup ("cyan");
	case CURSES_COLOR_WHITE:
		return elektraStrDup ("white");
	}

	// should be unreachable
	return "";
}
ELEKTRA_KEY_TO_SIGNATURE (CwLnxModel, EnumCwLnxModel)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	
	if (strcmp (string, "12232") == 0)
	{
		*variable = CW_LNX_MODEL_12232;
		return 1;
	}
	if (strcmp (string, "12832") == 0)
	{
		*variable = CW_LNX_MODEL_12832;
		return 1;
	}
	if (strcmp (string, "1602") == 0)
	{
		*variable = CW_LNX_MODEL_1602;
		return 1;
	}

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (CwLnxModel, EnumCwLnxModel)
{
	switch (value)
	{
	case CW_LNX_MODEL_12232:
		return elektraStrDup ("12232");
	case CW_LNX_MODEL_12832:
		return elektraStrDup ("12832");
	case CW_LNX_MODEL_1602:
		return elektraStrDup ("1602");
	}

	// should be unreachable
	return "";
}
ELEKTRA_KEY_TO_SIGNATURE (GlcdConnectionType, EnumGlcdConnectionType)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	
	if (strcmp (string, "t6963") == 0)
	{
		*variable = GLCD_CONNECTION_TYPE_T6963;
		return 1;
	}
	if (strcmp (string, "png") == 0)
	{
		*variable = GLCD_CONNECTION_TYPE_PNG;
		return 1;
	}
	if (strcmp (string, "serdisplib") == 0)
	{
		*variable = GLCD_CONNECTION_TYPE_SERDISPLIB;
		return 1;
	}
	if (strcmp (string, "glcd2usb") == 0)
	{
		*variable = GLCD_CONNECTION_TYPE_GLCD2USB;
		return 1;
	}
	if (strcmp (string, "x11") == 0)
	{
		*variable = GLCD_CONNECTION_TYPE_X11;
		return 1;
	}
	if (strcmp (string, "picolcdgfx") == 0)
	{
		*variable = GLCD_CONNECTION_TYPE_PICOLCDGFX;
		return 1;
	}
	if (strcmp (string, "xyz") == 0)
	{
		*variable = GLCD_CONNECTION_TYPE_XYZ;
		return 1;
	}

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (GlcdConnectionType, EnumGlcdConnectionType)
{
	switch (value)
	{
	case GLCD_CONNECTION_TYPE_T6963:
		return elektraStrDup ("t6963");
	case GLCD_CONNECTION_TYPE_PNG:
		return elektraStrDup ("png");
	case GLCD_CONNECTION_TYPE_SERDISPLIB:
		return elektraStrDup ("serdisplib");
	case GLCD_CONNECTION_TYPE_GLCD2USB:
		return elektraStrDup ("glcd2usb");
	case GLCD_CONNECTION_TYPE_X11:
		return elektraStrDup ("x11");
	case GLCD_CONNECTION_TYPE_PICOLCDGFX:
		return elektraStrDup ("picolcdgfx");
	case GLCD_CONNECTION_TYPE_XYZ:
		return elektraStrDup ("xyz");
	}

	// should be unreachable
	return "";
}
ELEKTRA_KEY_TO_SIGNATURE (HD44780Backlight, EnumHD44780Backlight)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	
	if (strcmp (string, "none") == 0)
	{
		*variable = H_D44780_BACKLIGHT_NONE;
		return 1;
	}
	if (strcmp (string, "external") == 0)
	{
		*variable = H_D44780_BACKLIGHT_EXTERNAL;
		return 1;
	}
	if (strcmp (string, "internal") == 0)
	{
		*variable = H_D44780_BACKLIGHT_INTERNAL;
		return 1;
	}
	if (strcmp (string, "internalCmds") == 0)
	{
		*variable = H_D44780_BACKLIGHT_INTERNAL_CMDS;
		return 1;
	}

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (HD44780Backlight, EnumHD44780Backlight)
{
	switch (value)
	{
	case H_D44780_BACKLIGHT_NONE:
		return elektraStrDup ("none");
	case H_D44780_BACKLIGHT_EXTERNAL:
		return elektraStrDup ("external");
	case H_D44780_BACKLIGHT_INTERNAL:
		return elektraStrDup ("internal");
	case H_D44780_BACKLIGHT_INTERNAL_CMDS:
		return elektraStrDup ("internalCmds");
	}

	// should be unreachable
	return "";
}
ELEKTRA_KEY_TO_SIGNATURE (HD44780Charmap, EnumHD44780Charmap)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	
	if (strcmp (string, "hd44780_default") == 0)
	{
		*variable = H_D44780_CHARMAP_HD44780_DEFAULT;
		return 1;
	}
	if (strcmp (string, "hd44780_euro") == 0)
	{
		*variable = H_D44780_CHARMAP_HD44780_EURO;
		return 1;
	}
	if (strcmp (string, "ea_ks0073") == 0)
	{
		*variable = H_D44780_CHARMAP_EA_KS0073;
		return 1;
	}
	if (strcmp (string, "sed1278f_0b") == 0)
	{
		*variable = H_D44780_CHARMAP_SED1278F_0B;
		return 1;
	}
	if (strcmp (string, "hd44780_koi8_r") == 0)
	{
		*variable = H_D44780_CHARMAP_HD44780_KOI8_R;
		return 1;
	}
	if (strcmp (string, "hd44780_cp1251") == 0)
	{
		*variable = H_D44780_CHARMAP_HD44780_CP1251;
		return 1;
	}
	if (strcmp (string, "hd44780_8859_5") == 0)
	{
		*variable = H_D44780_CHARMAP_HD44780_8859_5;
		return 1;
	}
	if (strcmp (string, "upd16314") == 0)
	{
		*variable = H_D44780_CHARMAP_UPD16314;
		return 1;
	}

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (HD44780Charmap, EnumHD44780Charmap)
{
	switch (value)
	{
	case H_D44780_CHARMAP_HD44780_DEFAULT:
		return elektraStrDup ("hd44780_default");
	case H_D44780_CHARMAP_HD44780_EURO:
		return elektraStrDup ("hd44780_euro");
	case H_D44780_CHARMAP_EA_KS0073:
		return elektraStrDup ("ea_ks0073");
	case H_D44780_CHARMAP_SED1278F_0B:
		return elektraStrDup ("sed1278f_0b");
	case H_D44780_CHARMAP_HD44780_KOI8_R:
		return elektraStrDup ("hd44780_koi8_r");
	case H_D44780_CHARMAP_HD44780_CP1251:
		return elektraStrDup ("hd44780_cp1251");
	case H_D44780_CHARMAP_HD44780_8859_5:
		return elektraStrDup ("hd44780_8859_5");
	case H_D44780_CHARMAP_UPD16314:
		return elektraStrDup ("upd16314");
	}

	// should be unreachable
	return "";
}
ELEKTRA_KEY_TO_SIGNATURE (HD44780ConnectionType, EnumHD44780ConnectionType)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	
	if (strcmp (string, "4bit") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_4BIT;
		return 1;
	}
	if (strcmp (string, "8bit") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_8BIT;
		return 1;
	}
	if (strcmp (string, "winamp") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_WINAMP;
		return 1;
	}
	if (strcmp (string, "lcm162") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_LCM162;
		return 1;
	}
	if (strcmp (string, "serialLpt") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_SERIAL_LPT;
		return 1;
	}
	if (strcmp (string, "picanlcd") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_PICANLCD;
		return 1;
	}
	if (strcmp (string, "lcdserializer") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_LCDSERIALIZER;
		return 1;
	}
	if (strcmp (string, "los-panel") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_LOS_PANEL;
		return 1;
	}
	if (strcmp (string, "vdr-lcd") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_VDR_LCD;
		return 1;
	}
	if (strcmp (string, "vdr-wakeup") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_VDR_WAKEUP;
		return 1;
	}
	if (strcmp (string, "ezio") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_EZIO;
		return 1;
	}
	if (strcmp (string, "pertelian") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_PERTELIAN;
		return 1;
	}
	if (strcmp (string, "lis2") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_LIS2;
		return 1;
	}
	if (strcmp (string, "mplay") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_MPLAY;
		return 1;
	}
	if (strcmp (string, "usblcd") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_USBLCD;
		return 1;
	}
	if (strcmp (string, "bwctusb") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_BWCTUSB;
		return 1;
	}
	if (strcmp (string, "lcd2usb") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_LCD2USB;
		return 1;
	}
	if (strcmp (string, "usbtiny") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_USBTINY;
		return 1;
	}
	if (strcmp (string, "uss720") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_USS720;
		return 1;
	}
	if (strcmp (string, "USB-4-all") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_U_S_B_4_ALL;
		return 1;
	}
	if (strcmp (string, "ftdi") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_FTDI;
		return 1;
	}
	if (strcmp (string, "i2c") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_I2C;
		return 1;
	}
	if (strcmp (string, "piplate") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_PIPLATE;
		return 1;
	}
	if (strcmp (string, "spi") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_SPI;
		return 1;
	}
	if (strcmp (string, "pifacecad") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_PIFACECAD;
		return 1;
	}
	if (strcmp (string, "ethlcd") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_ETHLCD;
		return 1;
	}
	if (strcmp (string, "raspberrypi") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_RASPBERRYPI;
		return 1;
	}
	if (strcmp (string, "gpio") == 0)
	{
		*variable = H_D44780_CONNECTION_TYPE_GPIO;
		return 1;
	}

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (HD44780ConnectionType, EnumHD44780ConnectionType)
{
	switch (value)
	{
	case H_D44780_CONNECTION_TYPE_4BIT:
		return elektraStrDup ("4bit");
	case H_D44780_CONNECTION_TYPE_8BIT:
		return elektraStrDup ("8bit");
	case H_D44780_CONNECTION_TYPE_WINAMP:
		return elektraStrDup ("winamp");
	case H_D44780_CONNECTION_TYPE_LCM162:
		return elektraStrDup ("lcm162");
	case H_D44780_CONNECTION_TYPE_SERIAL_LPT:
		return elektraStrDup ("serialLpt");
	case H_D44780_CONNECTION_TYPE_PICANLCD:
		return elektraStrDup ("picanlcd");
	case H_D44780_CONNECTION_TYPE_LCDSERIALIZER:
		return elektraStrDup ("lcdserializer");
	case H_D44780_CONNECTION_TYPE_LOS_PANEL:
		return elektraStrDup ("los-panel");
	case H_D44780_CONNECTION_TYPE_VDR_LCD:
		return elektraStrDup ("vdr-lcd");
	case H_D44780_CONNECTION_TYPE_VDR_WAKEUP:
		return elektraStrDup ("vdr-wakeup");
	case H_D44780_CONNECTION_TYPE_EZIO:
		return elektraStrDup ("ezio");
	case H_D44780_CONNECTION_TYPE_PERTELIAN:
		return elektraStrDup ("pertelian");
	case H_D44780_CONNECTION_TYPE_LIS2:
		return elektraStrDup ("lis2");
	case H_D44780_CONNECTION_TYPE_MPLAY:
		return elektraStrDup ("mplay");
	case H_D44780_CONNECTION_TYPE_USBLCD:
		return elektraStrDup ("usblcd");
	case H_D44780_CONNECTION_TYPE_BWCTUSB:
		return elektraStrDup ("bwctusb");
	case H_D44780_CONNECTION_TYPE_LCD2USB:
		return elektraStrDup ("lcd2usb");
	case H_D44780_CONNECTION_TYPE_USBTINY:
		return elektraStrDup ("usbtiny");
	case H_D44780_CONNECTION_TYPE_USS720:
		return elektraStrDup ("uss720");
	case H_D44780_CONNECTION_TYPE_U_S_B_4_ALL:
		return elektraStrDup ("USB-4-all");
	case H_D44780_CONNECTION_TYPE_FTDI:
		return elektraStrDup ("ftdi");
	case H_D44780_CONNECTION_TYPE_I2C:
		return elektraStrDup ("i2c");
	case H_D44780_CONNECTION_TYPE_PIPLATE:
		return elektraStrDup ("piplate");
	case H_D44780_CONNECTION_TYPE_SPI:
		return elektraStrDup ("spi");
	case H_D44780_CONNECTION_TYPE_PIFACECAD:
		return elektraStrDup ("pifacecad");
	case H_D44780_CONNECTION_TYPE_ETHLCD:
		return elektraStrDup ("ethlcd");
	case H_D44780_CONNECTION_TYPE_RASPBERRYPI:
		return elektraStrDup ("raspberrypi");
	case H_D44780_CONNECTION_TYPE_GPIO:
		return elektraStrDup ("gpio");
	}

	// should be unreachable
	return "";
}
ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumHd44780Model, EnumHd44780Model)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	switch (string[0])
{
case 'e':
*variable = ELEKTRA_ENUM_HD44780_MODEL_EXTENDED;
return 1;
case 'p':
*variable = ELEKTRA_ENUM_HD44780_MODEL_PT6314_VFD;
return 1;
case 's':
*variable = ELEKTRA_ENUM_HD44780_MODEL_STANDARD;
return 1;
case 'w':
*variable = ELEKTRA_ENUM_HD44780_MODEL_WINSTAR_OLED;
return 1;
}

	

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumHd44780Model, EnumHd44780Model)
{
	switch (value)
	{
	case ELEKTRA_ENUM_HD44780_MODEL_STANDARD:
		return elektraStrDup ("standard");
	case ELEKTRA_ENUM_HD44780_MODEL_EXTENDED:
		return elektraStrDup ("extended");
	case ELEKTRA_ENUM_HD44780_MODEL_WINSTAR_OLED:
		return elektraStrDup ("winstar_oled");
	case ELEKTRA_ENUM_HD44780_MODEL_PT6314_VFD:
		return elektraStrDup ("pt6314_vfd");
	}

	// should be unreachable
	return "";
}
ELEKTRA_KEY_TO_SIGNATURE (IMonCharmap, EnumIMonCharmap)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	
	if (strcmp (string, "hd44780_euro") == 0)
	{
		*variable = I_MON_CHARMAP_HD44780_EURO;
		return 1;
	}
	if (strcmp (string, "upd16314") == 0)
	{
		*variable = I_MON_CHARMAP_UPD16314;
		return 1;
	}
	if (strcmp (string, "hd44780_koi8_r") == 0)
	{
		*variable = I_MON_CHARMAP_HD44780_KOI8_R;
		return 1;
	}
	if (strcmp (string, "hd44780_cp1251") == 0)
	{
		*variable = I_MON_CHARMAP_HD44780_CP1251;
		return 1;
	}
	if (strcmp (string, "hd44780_8859_5") == 0)
	{
		*variable = I_MON_CHARMAP_HD44780_8859_5;
		return 1;
	}
	if (strcmp (string, "none") == 0)
	{
		*variable = I_MON_CHARMAP_NONE;
		return 1;
	}

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (IMonCharmap, EnumIMonCharmap)
{
	switch (value)
	{
	case I_MON_CHARMAP_HD44780_EURO:
		return elektraStrDup ("hd44780_euro");
	case I_MON_CHARMAP_UPD16314:
		return elektraStrDup ("upd16314");
	case I_MON_CHARMAP_HD44780_KOI8_R:
		return elektraStrDup ("hd44780_koi8_r");
	case I_MON_CHARMAP_HD44780_CP1251:
		return elektraStrDup ("hd44780_cp1251");
	case I_MON_CHARMAP_HD44780_8859_5:
		return elektraStrDup ("hd44780_8859_5");
	case I_MON_CHARMAP_NONE:
		return elektraStrDup ("none");
	}

	// should be unreachable
	return "";
}
ELEKTRA_KEY_TO_SIGNATURE (IMonLCDDiscMode, EnumIMonLCDDiscMode)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	switch (string[0])
{
case '0':
*variable = I_MON_L_C_D_DISC_MODE_0;
return 1;
case '1':
*variable = I_MON_L_C_D_DISC_MODE_1;
return 1;
}

	

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (IMonLCDDiscMode, EnumIMonLCDDiscMode)
{
	switch (value)
	{
	case I_MON_L_C_D_DISC_MODE_0:
		return elektraStrDup ("0");
	case I_MON_L_C_D_DISC_MODE_1:
		return elektraStrDup ("1");
	}

	// should be unreachable
	return "";
}
ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumMdm166aClock, EnumMdm166aClock)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	switch (string[0])
{
case 'b':
*variable = ELEKTRA_ENUM_MDM166A_CLOCK_BIG;
return 1;
case 'n':
*variable = ELEKTRA_ENUM_MDM166A_CLOCK_NO;
return 1;
case 's':
*variable = ELEKTRA_ENUM_MDM166A_CLOCK_SMALL;
return 1;
}

	

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumMdm166aClock, EnumMdm166aClock)
{
	switch (value)
	{
	case ELEKTRA_ENUM_MDM166A_CLOCK_NO:
		return elektraStrDup ("no");
	case ELEKTRA_ENUM_MDM166A_CLOCK_SMALL:
		return elektraStrDup ("small");
	case ELEKTRA_ENUM_MDM166A_CLOCK_BIG:
		return elektraStrDup ("big");
	}

	// should be unreachable
	return "";
}
ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumMtxorbType, EnumMtxorbType)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	
	if (strcmp (string, "lcd lkd, vfd, vkd") == 0)
	{
		*variable = ELEKTRA_ENUM_MTXORB_TYPE_LCD_LKD__VFD__VKD;
		return 1;
	}
	if (strcmp (string, "lkd") == 0)
	{
		*variable = ELEKTRA_ENUM_MTXORB_TYPE_LKD;
		return 1;
	}
	if (strcmp (string, "vfd") == 0)
	{
		*variable = ELEKTRA_ENUM_MTXORB_TYPE_VFD;
		return 1;
	}
	if (strcmp (string, "vkd") == 0)
	{
		*variable = ELEKTRA_ENUM_MTXORB_TYPE_VKD;
		return 1;
	}

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumMtxorbType, EnumMtxorbType)
{
	switch (value)
	{
	case ELEKTRA_ENUM_MTXORB_TYPE_LCD_LKD__VFD__VKD:
		return elektraStrDup ("lcd lkd, vfd, vkd");
	case ELEKTRA_ENUM_MTXORB_TYPE_LKD:
		return elektraStrDup ("lkd");
	case ELEKTRA_ENUM_MTXORB_TYPE_VFD:
		return elektraStrDup ("vfd");
	case ELEKTRA_ENUM_MTXORB_TYPE_VKD:
		return elektraStrDup ("vkd");
	}

	// should be unreachable
	return "";
}
ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumSed1330Connectiontype, EnumSed1330Connectiontype)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	switch (string[0])
{
case 'b':
*variable = ELEKTRA_ENUM_SED1330_CONNECTIONTYPE_BITSHAKER;
return 1;
case 'c':
*variable = ELEKTRA_ENUM_SED1330_CONNECTIONTYPE_CLASSIC;
return 1;
}

	

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumSed1330Connectiontype, EnumSed1330Connectiontype)
{
	switch (value)
	{
	case ELEKTRA_ENUM_SED1330_CONNECTIONTYPE_CLASSIC:
		return elektraStrDup ("classic");
	case ELEKTRA_ENUM_SED1330_CONNECTIONTYPE_BITSHAKER:
		return elektraStrDup ("bitshaker");
	}

	// should be unreachable
	return "";
}
ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumSed1330Type, EnumSed1330Type)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	
	if (strcmp (string, "G321D") == 0)
	{
		*variable = ELEKTRA_ENUM_SED1330_TYPE_G321_D;
		return 1;
	}
	if (strcmp (string, "G121C") == 0)
	{
		*variable = ELEKTRA_ENUM_SED1330_TYPE_G121_C;
		return 1;
	}
	if (strcmp (string, "G242C") == 0)
	{
		*variable = ELEKTRA_ENUM_SED1330_TYPE_G242_C;
		return 1;
	}
	if (strcmp (string, "G191D") == 0)
	{
		*variable = ELEKTRA_ENUM_SED1330_TYPE_G191_D;
		return 1;
	}
	if (strcmp (string, "G2446") == 0)
	{
		*variable = ELEKTRA_ENUM_SED1330_TYPE_G2446;
		return 1;
	}
	if (strcmp (string, "SP14Q002") == 0)
	{
		*variable = ELEKTRA_ENUM_SED1330_TYPE_S_P14_Q002;
		return 1;
	}

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumSed1330Type, EnumSed1330Type)
{
	switch (value)
	{
	case ELEKTRA_ENUM_SED1330_TYPE_G321_D:
		return elektraStrDup ("G321D");
	case ELEKTRA_ENUM_SED1330_TYPE_G121_C:
		return elektraStrDup ("G121C");
	case ELEKTRA_ENUM_SED1330_TYPE_G242_C:
		return elektraStrDup ("G242C");
	case ELEKTRA_ENUM_SED1330_TYPE_G191_D:
		return elektraStrDup ("G191D");
	case ELEKTRA_ENUM_SED1330_TYPE_G2446:
		return elektraStrDup ("G2446");
	case ELEKTRA_ENUM_SED1330_TYPE_S_P14_Q002:
		return elektraStrDup ("SP14Q002");
	}

	// should be unreachable
	return "";
}
ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumSerialposType, EnumSerialposType)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	
	if (strcmp (string, "AEDEX") == 0)
	{
		*variable = ELEKTRA_ENUM_SERIALPOS_TYPE_A_E_D_E_X;
		return 1;
	}
	if (strcmp (string, "CD5220") == 0)
	{
		*variable = ELEKTRA_ENUM_SERIALPOS_TYPE_C_D5220;
		return 1;
	}
	if (strcmp (string, "Epson") == 0)
	{
		*variable = ELEKTRA_ENUM_SERIALPOS_TYPE_EPSON;
		return 1;
	}
	if (strcmp (string, "Emax") == 0)
	{
		*variable = ELEKTRA_ENUM_SERIALPOS_TYPE_EMAX;
		return 1;
	}
	if (strcmp (string, "LogicControls") == 0)
	{
		*variable = ELEKTRA_ENUM_SERIALPOS_TYPE_LOGIC_CONTROLS;
		return 1;
	}
	if (strcmp (string, "Ultimate") == 0)
	{
		*variable = ELEKTRA_ENUM_SERIALPOS_TYPE_ULTIMATE;
		return 1;
	}

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumSerialposType, EnumSerialposType)
{
	switch (value)
	{
	case ELEKTRA_ENUM_SERIALPOS_TYPE_A_E_D_E_X:
		return elektraStrDup ("AEDEX");
	case ELEKTRA_ENUM_SERIALPOS_TYPE_C_D5220:
		return elektraStrDup ("CD5220");
	case ELEKTRA_ENUM_SERIALPOS_TYPE_EPSON:
		return elektraStrDup ("Epson");
	case ELEKTRA_ENUM_SERIALPOS_TYPE_EMAX:
		return elektraStrDup ("Emax");
	case ELEKTRA_ENUM_SERIALPOS_TYPE_LOGIC_CONTROLS:
		return elektraStrDup ("LogicControls");
	case ELEKTRA_ENUM_SERIALPOS_TYPE_ULTIMATE:
		return elektraStrDup ("Ultimate");
	}

	// should be unreachable
	return "";
}
ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumServerBacklight, EnumServerBacklight)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	
	if (strcmp (string, "off") == 0)
	{
		*variable = ELEKTRA_ENUM_SERVER_BACKLIGHT_OFF;
		return 1;
	}
	if (strcmp (string, "on") == 0)
	{
		*variable = ELEKTRA_ENUM_SERVER_BACKLIGHT_ON;
		return 1;
	}
	if (strcmp (string, "open") == 0)
	{
		*variable = ELEKTRA_ENUM_SERVER_BACKLIGHT_OPEN;
		return 1;
	}

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumServerBacklight, EnumServerBacklight)
{
	switch (value)
	{
	case ELEKTRA_ENUM_SERVER_BACKLIGHT_OFF:
		return elektraStrDup ("off");
	case ELEKTRA_ENUM_SERVER_BACKLIGHT_ON:
		return elektraStrDup ("on");
	case ELEKTRA_ENUM_SERVER_BACKLIGHT_OPEN:
		return elektraStrDup ("open");
	}

	// should be unreachable
	return "";
}
ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumServerHeartbeat, EnumServerHeartbeat)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	
	if (strcmp (string, "off") == 0)
	{
		*variable = ELEKTRA_ENUM_SERVER_HEARTBEAT_OFF;
		return 1;
	}
	if (strcmp (string, "on") == 0)
	{
		*variable = ELEKTRA_ENUM_SERVER_HEARTBEAT_ON;
		return 1;
	}
	if (strcmp (string, "open") == 0)
	{
		*variable = ELEKTRA_ENUM_SERVER_HEARTBEAT_OPEN;
		return 1;
	}

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumServerHeartbeat, EnumServerHeartbeat)
{
	switch (value)
	{
	case ELEKTRA_ENUM_SERVER_HEARTBEAT_OFF:
		return elektraStrDup ("off");
	case ELEKTRA_ENUM_SERVER_HEARTBEAT_ON:
		return elektraStrDup ("on");
	case ELEKTRA_ENUM_SERVER_HEARTBEAT_OPEN:
		return elektraStrDup ("open");
	}

	// should be unreachable
	return "";
}
ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumServerServerscreen, EnumServerServerscreen)
{
	const char * string;
	if (!elektraKeyToString (key, &string) || strlen (string) == 0)
	{
		return 0;
	}

	
	if (strcmp (string, "off") == 0)
	{
		*variable = ELEKTRA_ENUM_SERVER_SERVERSCREEN_OFF;
		return 1;
	}
	if (strcmp (string, "on") == 0)
	{
		*variable = ELEKTRA_ENUM_SERVER_SERVERSCREEN_ON;
		return 1;
	}
	if (strcmp (string, "blank") == 0)
	{
		*variable = ELEKTRA_ENUM_SERVER_SERVERSCREEN_BLANK;
		return 1;
	}

	return 0;
}

ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumServerServerscreen, EnumServerServerscreen)
{
	switch (value)
	{
	case ELEKTRA_ENUM_SERVER_SERVERSCREEN_OFF:
		return elektraStrDup ("off");
	case ELEKTRA_ENUM_SERVER_SERVERSCREEN_ON:
		return elektraStrDup ("on");
	case ELEKTRA_ENUM_SERVER_SERVERSCREEN_BLANK:
		return elektraStrDup ("blank");
	}

	// should be unreachable
	return "";
}

// -------------------------
// Enum accessor functions
// -------------------------

ELEKTRA_GET_SIGNATURE (CFontzPacketModel, EnumCFontzPacketModel)
{
	CFontzPacketModel result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumCFontzPacketModel) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (CFontzPacketModel) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (CFontzPacketModel, EnumCFontzPacketModel)
{
	CFontzPacketModel result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumCFontzPacketModel) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (CFontzPacketModel) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (CFontzPacketModel, EnumCFontzPacketModel)
{
	char * string = ELEKTRA_TO_STRING (EnumCFontzPacketModel) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (CFontzPacketModel, EnumCFontzPacketModel)
{
	char * string = ELEKTRA_TO_STRING (EnumCFontzPacketModel) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}
ELEKTRA_GET_SIGNATURE (CursesColor, EnumCursesColor)
{
	CursesColor result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumCursesColor) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (CursesColor) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (CursesColor, EnumCursesColor)
{
	CursesColor result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumCursesColor) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (CursesColor) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (CursesColor, EnumCursesColor)
{
	char * string = ELEKTRA_TO_STRING (EnumCursesColor) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (CursesColor, EnumCursesColor)
{
	char * string = ELEKTRA_TO_STRING (EnumCursesColor) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}
ELEKTRA_GET_SIGNATURE (CwLnxModel, EnumCwLnxModel)
{
	CwLnxModel result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumCwLnxModel) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (CwLnxModel) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (CwLnxModel, EnumCwLnxModel)
{
	CwLnxModel result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumCwLnxModel) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (CwLnxModel) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (CwLnxModel, EnumCwLnxModel)
{
	char * string = ELEKTRA_TO_STRING (EnumCwLnxModel) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (CwLnxModel, EnumCwLnxModel)
{
	char * string = ELEKTRA_TO_STRING (EnumCwLnxModel) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}
ELEKTRA_GET_SIGNATURE (GlcdConnectionType, EnumGlcdConnectionType)
{
	GlcdConnectionType result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumGlcdConnectionType) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (GlcdConnectionType) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (GlcdConnectionType, EnumGlcdConnectionType)
{
	GlcdConnectionType result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumGlcdConnectionType) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (GlcdConnectionType) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (GlcdConnectionType, EnumGlcdConnectionType)
{
	char * string = ELEKTRA_TO_STRING (EnumGlcdConnectionType) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (GlcdConnectionType, EnumGlcdConnectionType)
{
	char * string = ELEKTRA_TO_STRING (EnumGlcdConnectionType) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}
ELEKTRA_GET_SIGNATURE (HD44780Backlight, EnumHD44780Backlight)
{
	HD44780Backlight result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumHD44780Backlight) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (HD44780Backlight) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (HD44780Backlight, EnumHD44780Backlight)
{
	HD44780Backlight result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumHD44780Backlight) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (HD44780Backlight) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (HD44780Backlight, EnumHD44780Backlight)
{
	char * string = ELEKTRA_TO_STRING (EnumHD44780Backlight) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (HD44780Backlight, EnumHD44780Backlight)
{
	char * string = ELEKTRA_TO_STRING (EnumHD44780Backlight) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}
ELEKTRA_GET_SIGNATURE (HD44780Charmap, EnumHD44780Charmap)
{
	HD44780Charmap result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumHD44780Charmap) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (HD44780Charmap) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (HD44780Charmap, EnumHD44780Charmap)
{
	HD44780Charmap result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumHD44780Charmap) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (HD44780Charmap) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (HD44780Charmap, EnumHD44780Charmap)
{
	char * string = ELEKTRA_TO_STRING (EnumHD44780Charmap) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (HD44780Charmap, EnumHD44780Charmap)
{
	char * string = ELEKTRA_TO_STRING (EnumHD44780Charmap) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}
ELEKTRA_GET_SIGNATURE (HD44780ConnectionType, EnumHD44780ConnectionType)
{
	HD44780ConnectionType result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumHD44780ConnectionType) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (HD44780ConnectionType) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (HD44780ConnectionType, EnumHD44780ConnectionType)
{
	HD44780ConnectionType result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumHD44780ConnectionType) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (HD44780ConnectionType) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (HD44780ConnectionType, EnumHD44780ConnectionType)
{
	char * string = ELEKTRA_TO_STRING (EnumHD44780ConnectionType) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (HD44780ConnectionType, EnumHD44780ConnectionType)
{
	char * string = ELEKTRA_TO_STRING (EnumHD44780ConnectionType) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}
ELEKTRA_GET_SIGNATURE (ElektraEnumHd44780Model, EnumHd44780Model)
{
	ElektraEnumHd44780Model result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumHd44780Model) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumHd44780Model) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumHd44780Model, EnumHd44780Model)
{
	ElektraEnumHd44780Model result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumHd44780Model) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumHd44780Model) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (ElektraEnumHd44780Model, EnumHd44780Model)
{
	char * string = ELEKTRA_TO_STRING (EnumHd44780Model) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumHd44780Model, EnumHd44780Model)
{
	char * string = ELEKTRA_TO_STRING (EnumHd44780Model) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}
ELEKTRA_GET_SIGNATURE (IMonCharmap, EnumIMonCharmap)
{
	IMonCharmap result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumIMonCharmap) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (IMonCharmap) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (IMonCharmap, EnumIMonCharmap)
{
	IMonCharmap result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumIMonCharmap) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (IMonCharmap) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (IMonCharmap, EnumIMonCharmap)
{
	char * string = ELEKTRA_TO_STRING (EnumIMonCharmap) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (IMonCharmap, EnumIMonCharmap)
{
	char * string = ELEKTRA_TO_STRING (EnumIMonCharmap) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}
ELEKTRA_GET_SIGNATURE (IMonLCDDiscMode, EnumIMonLCDDiscMode)
{
	IMonLCDDiscMode result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumIMonLCDDiscMode) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (IMonLCDDiscMode) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (IMonLCDDiscMode, EnumIMonLCDDiscMode)
{
	IMonLCDDiscMode result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumIMonLCDDiscMode) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (IMonLCDDiscMode) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (IMonLCDDiscMode, EnumIMonLCDDiscMode)
{
	char * string = ELEKTRA_TO_STRING (EnumIMonLCDDiscMode) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (IMonLCDDiscMode, EnumIMonLCDDiscMode)
{
	char * string = ELEKTRA_TO_STRING (EnumIMonLCDDiscMode) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}
ELEKTRA_GET_SIGNATURE (ElektraEnumMdm166aClock, EnumMdm166aClock)
{
	ElektraEnumMdm166aClock result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumMdm166aClock) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumMdm166aClock) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumMdm166aClock, EnumMdm166aClock)
{
	ElektraEnumMdm166aClock result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumMdm166aClock) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumMdm166aClock) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (ElektraEnumMdm166aClock, EnumMdm166aClock)
{
	char * string = ELEKTRA_TO_STRING (EnumMdm166aClock) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumMdm166aClock, EnumMdm166aClock)
{
	char * string = ELEKTRA_TO_STRING (EnumMdm166aClock) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}
ELEKTRA_GET_SIGNATURE (ElektraEnumMtxorbType, EnumMtxorbType)
{
	ElektraEnumMtxorbType result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumMtxorbType) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumMtxorbType) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumMtxorbType, EnumMtxorbType)
{
	ElektraEnumMtxorbType result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumMtxorbType) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumMtxorbType) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (ElektraEnumMtxorbType, EnumMtxorbType)
{
	char * string = ELEKTRA_TO_STRING (EnumMtxorbType) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumMtxorbType, EnumMtxorbType)
{
	char * string = ELEKTRA_TO_STRING (EnumMtxorbType) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}
ELEKTRA_GET_SIGNATURE (ElektraEnumSed1330Connectiontype, EnumSed1330Connectiontype)
{
	ElektraEnumSed1330Connectiontype result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumSed1330Connectiontype) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumSed1330Connectiontype) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumSed1330Connectiontype, EnumSed1330Connectiontype)
{
	ElektraEnumSed1330Connectiontype result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumSed1330Connectiontype) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumSed1330Connectiontype) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (ElektraEnumSed1330Connectiontype, EnumSed1330Connectiontype)
{
	char * string = ELEKTRA_TO_STRING (EnumSed1330Connectiontype) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumSed1330Connectiontype, EnumSed1330Connectiontype)
{
	char * string = ELEKTRA_TO_STRING (EnumSed1330Connectiontype) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}
ELEKTRA_GET_SIGNATURE (ElektraEnumSed1330Type, EnumSed1330Type)
{
	ElektraEnumSed1330Type result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumSed1330Type) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumSed1330Type) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumSed1330Type, EnumSed1330Type)
{
	ElektraEnumSed1330Type result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumSed1330Type) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumSed1330Type) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (ElektraEnumSed1330Type, EnumSed1330Type)
{
	char * string = ELEKTRA_TO_STRING (EnumSed1330Type) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumSed1330Type, EnumSed1330Type)
{
	char * string = ELEKTRA_TO_STRING (EnumSed1330Type) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}
ELEKTRA_GET_SIGNATURE (ElektraEnumSerialposType, EnumSerialposType)
{
	ElektraEnumSerialposType result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumSerialposType) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumSerialposType) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumSerialposType, EnumSerialposType)
{
	ElektraEnumSerialposType result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumSerialposType) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumSerialposType) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (ElektraEnumSerialposType, EnumSerialposType)
{
	char * string = ELEKTRA_TO_STRING (EnumSerialposType) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumSerialposType, EnumSerialposType)
{
	char * string = ELEKTRA_TO_STRING (EnumSerialposType) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}
ELEKTRA_GET_SIGNATURE (ElektraEnumServerBacklight, EnumServerBacklight)
{
	ElektraEnumServerBacklight result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumServerBacklight) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumServerBacklight) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumServerBacklight, EnumServerBacklight)
{
	ElektraEnumServerBacklight result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumServerBacklight) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumServerBacklight) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (ElektraEnumServerBacklight, EnumServerBacklight)
{
	char * string = ELEKTRA_TO_STRING (EnumServerBacklight) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumServerBacklight, EnumServerBacklight)
{
	char * string = ELEKTRA_TO_STRING (EnumServerBacklight) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}
ELEKTRA_GET_SIGNATURE (ElektraEnumServerHeartbeat, EnumServerHeartbeat)
{
	ElektraEnumServerHeartbeat result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumServerHeartbeat) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumServerHeartbeat) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumServerHeartbeat, EnumServerHeartbeat)
{
	ElektraEnumServerHeartbeat result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumServerHeartbeat) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumServerHeartbeat) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (ElektraEnumServerHeartbeat, EnumServerHeartbeat)
{
	char * string = ELEKTRA_TO_STRING (EnumServerHeartbeat) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumServerHeartbeat, EnumServerHeartbeat)
{
	char * string = ELEKTRA_TO_STRING (EnumServerHeartbeat) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}
ELEKTRA_GET_SIGNATURE (ElektraEnumServerServerscreen, EnumServerServerscreen)
{
	ElektraEnumServerServerscreen result;
	const Key * key = elektraFindKey (elektra, keyname, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumServerServerscreen) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumServerServerscreen) 0;
	}
	return result;
}

ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumServerServerscreen, EnumServerServerscreen)
{
	ElektraEnumServerServerscreen result;
	const Key * key = elektraFindArrayElementKey (elektra, keyname, index, KDB_TYPE_ENUM);
	if (!ELEKTRA_KEY_TO (EnumServerServerscreen) (key, &result))
	{
		elektraFatalError (elektra, elektraErrorConversionFromString (KDB_TYPE_ENUM, keyname, keyString (key)));
		return (ElektraEnumServerServerscreen) 0;
	}
	return result;
}

ELEKTRA_SET_SIGNATURE (ElektraEnumServerServerscreen, EnumServerServerscreen)
{
	char * string = ELEKTRA_TO_STRING (EnumServerServerscreen) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawString (elektra, keyname, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumServerServerscreen, EnumServerServerscreen)
{
	char * string = ELEKTRA_TO_STRING (EnumServerServerscreen) (value);
	if (string == 0)
	{
		*error = elektraErrorConversionToString (KDB_TYPE_ENUM, keyname);
		return;
	}
	elektraSetRawStringArrayElement (elektra, keyname, index, string, KDB_TYPE_ENUM, error);
	elektraFree (string);
}


// clang-format off

// clang-format on

// -------------------------
// Struct accessor functions
// -------------------------


ELEKTRA_GET_OUT_PTR_SIGNATURE (BayradDriverConfig, StructBayradDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (BayradDriverConfig, StructBayradDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const BayradDriverConfig *, StructBayradDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const BayradDriverConfig *, StructBayradDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (CFontzDriverConfig, StructCFontzDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "newfirmware", 14);
	
	
	result->newfirmware = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (CFontzDriverConfig, StructCFontzDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "newfirmware", 14);
	
	
	result->newfirmware = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const CFontzDriverConfig *, StructCFontzDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "newfirmware", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->newfirmware, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const CFontzDriverConfig *, StructCFontzDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "newfirmware", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->newfirmware, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (CFontzPacketDriverConfig, StructCFontzPacketDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "model", 14);
	
	
	result->model = ELEKTRA_GET (EnumCFontzPacketModel) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "oldfirmware", 14);
	
	
	result->oldfirmware = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "usb", 14);
	
	
	result->usb = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (CFontzPacketDriverConfig, StructCFontzPacketDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "model", 14);
	
	
	result->model = ELEKTRA_GET (EnumCFontzPacketModel) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "oldfirmware", 14);
	
	
	result->oldfirmware = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "usb", 14);
	
	
	result->usb = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const CFontzPacketDriverConfig *, StructCFontzPacketDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "model", 14);
	
	ELEKTRA_SET (EnumCFontzPacketModel) (elektra, field, value->model, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "oldfirmware", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->oldfirmware, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "usb", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->usb, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const CFontzPacketDriverConfig *, StructCFontzPacketDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "model", 14);
	
	ELEKTRA_SET (EnumCFontzPacketModel) (elektra, field, value->model, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "oldfirmware", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->oldfirmware, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "usb", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->usb, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (CursesDriverConfig, StructCursesDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "background", 14);
	
	
	result->background = ELEKTRA_GET (EnumCursesColor) (elektra, field);

	strncpy (&field[nameLen], "backlight", 14);
	
	
	result->backlight = ELEKTRA_GET (EnumCursesColor) (elektra, field);

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "drawborder", 14);
	
	
	result->drawborder = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "foreground", 14);
	
	
	result->foreground = ELEKTRA_GET (EnumCursesColor) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "topleftx", 14);
	
	
	result->topleftx = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "toplefty", 14);
	
	
	result->toplefty = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "useacs", 14);
	
	
	result->useacs = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (CursesDriverConfig, StructCursesDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "background", 14);
	
	
	result->background = ELEKTRA_GET (EnumCursesColor) (elektra, field);

	strncpy (&field[nameLen], "backlight", 14);
	
	
	result->backlight = ELEKTRA_GET (EnumCursesColor) (elektra, field);

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "drawborder", 14);
	
	
	result->drawborder = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "foreground", 14);
	
	
	result->foreground = ELEKTRA_GET (EnumCursesColor) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "topleftx", 14);
	
	
	result->topleftx = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "toplefty", 14);
	
	
	result->toplefty = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "useacs", 14);
	
	
	result->useacs = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const CursesDriverConfig *, StructCursesDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "background", 14);
	
	ELEKTRA_SET (EnumCursesColor) (elektra, field, value->background, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "backlight", 14);
	
	ELEKTRA_SET (EnumCursesColor) (elektra, field, value->backlight, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "drawborder", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->drawborder, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "foreground", 14);
	
	ELEKTRA_SET (EnumCursesColor) (elektra, field, value->foreground, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "topleftx", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->topleftx, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "toplefty", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->toplefty, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "useacs", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->useacs, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const CursesDriverConfig *, StructCursesDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "background", 14);
	
	ELEKTRA_SET (EnumCursesColor) (elektra, field, value->background, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "backlight", 14);
	
	ELEKTRA_SET (EnumCursesColor) (elektra, field, value->backlight, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "drawborder", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->drawborder, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "foreground", 14);
	
	ELEKTRA_SET (EnumCursesColor) (elektra, field, value->foreground, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "topleftx", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->topleftx, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "toplefty", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->toplefty, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "useacs", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->useacs, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (CwLnxDriverConfig, StructCwLnxDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 17 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 17);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 17);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 17);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 17);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_a", 17);
	
	
	result->keymapA = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_b", 17);
	
	
	result->keymapB = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_c", 17);
	
	
	result->keymapC = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_d", 17);
	
	
	result->keymapD = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_e", 17);
	
	
	result->keymapE = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_f", 17);
	
	
	result->keymapF = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keypad", 17);
	
	
	result->keypad = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "keypad_test_mode", 17);
	
	
	result->keypadTestMode = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "model", 17);
	
	
	result->model = ELEKTRA_GET (EnumCwLnxModel) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 17);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 17);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 17);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 17);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (CwLnxDriverConfig, StructCwLnxDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 17 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 17);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 17);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 17);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 17);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_a", 17);
	
	
	result->keymapA = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_b", 17);
	
	
	result->keymapB = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_c", 17);
	
	
	result->keymapC = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_d", 17);
	
	
	result->keymapD = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_e", 17);
	
	
	result->keymapE = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_f", 17);
	
	
	result->keymapF = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keypad", 17);
	
	
	result->keypad = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "keypad_test_mode", 17);
	
	
	result->keypadTestMode = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "model", 17);
	
	
	result->model = ELEKTRA_GET (EnumCwLnxModel) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 17);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 17);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 17);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 17);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const CwLnxDriverConfig *, StructCwLnxDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 17 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_a", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapA, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_b", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapB, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_c", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapC, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_d", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapD, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_e", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapE, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_f", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapF, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keypad", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->keypad, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keypad_test_mode", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->keypadTestMode, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "model", 17);
	
	ELEKTRA_SET (EnumCwLnxModel) (elektra, field, value->model, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const CwLnxDriverConfig *, StructCwLnxDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 17 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_a", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapA, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_b", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapB, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_c", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapC, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_d", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapD, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_e", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapE, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_f", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapF, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keypad", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->keypad, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keypad_test_mode", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->keypadTestMode, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "model", 17);
	
	ELEKTRA_SET (EnumCwLnxModel) (elektra, field, value->model, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (Ea65DriverConfig, StructEa65DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Ea65DriverConfig, StructEa65DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const Ea65DriverConfig *, StructEa65DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Ea65DriverConfig *, StructEa65DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (EyeboxOneDriverConfig, StructEyeboxOneDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 17 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 17);
	
	
	result->backlight = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "brightness", 17);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 17);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "cursor", 17);
	
	
	result->cursor = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "device", 17);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "downkey", 17);
	
	
	result->downkey = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "escapekey", 17);
	
	
	result->escapekey = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 17);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keypad_test_mode", 17);
	
	
	result->keypadTestMode = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "leftkey", 17);
	
	
	result->leftkey = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 17);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 17);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "rightkey", 17);
	
	
	result->rightkey = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "size", 17);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 17);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "upkey", 17);
	
	
	result->upkey = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (EyeboxOneDriverConfig, StructEyeboxOneDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 17 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 17);
	
	
	result->backlight = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "brightness", 17);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 17);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "cursor", 17);
	
	
	result->cursor = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "device", 17);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "downkey", 17);
	
	
	result->downkey = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "escapekey", 17);
	
	
	result->escapekey = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 17);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keypad_test_mode", 17);
	
	
	result->keypadTestMode = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "leftkey", 17);
	
	
	result->leftkey = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 17);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 17);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "rightkey", 17);
	
	
	result->rightkey = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "size", 17);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 17);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "upkey", 17);
	
	
	result->upkey = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const EyeboxOneDriverConfig *, StructEyeboxOneDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 17 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->backlight, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "cursor", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->cursor, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "downkey", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->downkey, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "escapekey", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->escapekey, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keypad_test_mode", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->keypadTestMode, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "leftkey", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->leftkey, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "rightkey", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->rightkey, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "upkey", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->upkey, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const EyeboxOneDriverConfig *, StructEyeboxOneDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 17 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->backlight, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "cursor", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->cursor, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "downkey", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->downkey, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "escapekey", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->escapekey, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keypad_test_mode", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->keypadTestMode, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "leftkey", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->leftkey, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "rightkey", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->rightkey, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "upkey", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->upkey, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (FutabaDriverConfig, StructFutabaDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 5 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "file", 5);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (FutabaDriverConfig, StructFutabaDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 5 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "file", 5);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const FutabaDriverConfig *, StructFutabaDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 5 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "file", 5);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const FutabaDriverConfig *, StructFutabaDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 5 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "file", 5);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (G15DriverConfig, StructG15DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (G15DriverConfig, StructG15DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const G15DriverConfig *, StructG15DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const G15DriverConfig *, StructG15DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (GlcdDriverConfig, StructGlcdDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 22 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "bidirectional", 22);
	
	
	result->bidirectional = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "brightness", 22);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "cellsize", 22);
	
	
	result->cellsize = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "connectiontype", 22);
	
	
	result->connectiontype = ELEKTRA_GET (EnumGlcdConnectionType) (elektra, field);

	strncpy (&field[nameLen], "contrast", 22);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "delaybus", 22);
	
	
	result->delaybus = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "file", 22);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "fonthasicons", 22);
	
	
	result->fonthasicons = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "keymap_a", 22);
	
	
	result->keymapA = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_b", 22);
	
	
	result->keymapB = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_c", 22);
	
	
	result->keymapC = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_d", 22);
	
	
	result->keymapD = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_e", 22);
	
	
	result->keymapE = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_f", 22);
	
	
	result->keymapF = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keyrepeatdelay", 22);
	
	
	result->keyrepeatdelay = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "keyrepeatinterval", 22);
	
	
	result->keyrepeatinterval = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "normal_font", 22);
	
	
	result->normalFont = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 22);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "picolcdgfx_inverted", 22);
	
	
	result->picolcdgfxInverted = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "picolcdgfx_keytimeout", 22);
	
	
	result->picolcdgfxKeytimeout = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "port", 22);
	
	
	result->port = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 22);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "serdisp_device", 22);
	
	
	result->serdispDevice = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "serdisp_name", 22);
	
	
	result->serdispName = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "serdisp_options", 22);
	
	
	result->serdispOptions = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "size", 22);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "useft2", 22);
	
	
	result->useft2 = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "x11_backlightcolor", 22);
	
	
	result->x11Backlightcolor = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "x11_border", 22);
	
	
	result->x11Border = ELEKTRA_GET (UnsignedLong) (elektra, field);

	strncpy (&field[nameLen], "x11_inverted", 22);
	
	
	result->x11Inverted = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "x11_pixelcolor", 22);
	
	
	result->x11Pixelcolor = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "x11_pixelsize", 22);
	
	
	result->x11Pixelsize = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (GlcdDriverConfig, StructGlcdDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 22 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "bidirectional", 22);
	
	
	result->bidirectional = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "brightness", 22);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "cellsize", 22);
	
	
	result->cellsize = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "connectiontype", 22);
	
	
	result->connectiontype = ELEKTRA_GET (EnumGlcdConnectionType) (elektra, field);

	strncpy (&field[nameLen], "contrast", 22);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "delaybus", 22);
	
	
	result->delaybus = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "file", 22);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "fonthasicons", 22);
	
	
	result->fonthasicons = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "keymap_a", 22);
	
	
	result->keymapA = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_b", 22);
	
	
	result->keymapB = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_c", 22);
	
	
	result->keymapC = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_d", 22);
	
	
	result->keymapD = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_e", 22);
	
	
	result->keymapE = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_f", 22);
	
	
	result->keymapF = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keyrepeatdelay", 22);
	
	
	result->keyrepeatdelay = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "keyrepeatinterval", 22);
	
	
	result->keyrepeatinterval = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "normal_font", 22);
	
	
	result->normalFont = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 22);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "picolcdgfx_inverted", 22);
	
	
	result->picolcdgfxInverted = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "picolcdgfx_keytimeout", 22);
	
	
	result->picolcdgfxKeytimeout = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "port", 22);
	
	
	result->port = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 22);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "serdisp_device", 22);
	
	
	result->serdispDevice = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "serdisp_name", 22);
	
	
	result->serdispName = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "serdisp_options", 22);
	
	
	result->serdispOptions = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "size", 22);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "useft2", 22);
	
	
	result->useft2 = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "x11_backlightcolor", 22);
	
	
	result->x11Backlightcolor = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "x11_border", 22);
	
	
	result->x11Border = ELEKTRA_GET (UnsignedLong) (elektra, field);

	strncpy (&field[nameLen], "x11_inverted", 22);
	
	
	result->x11Inverted = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "x11_pixelcolor", 22);
	
	
	result->x11Pixelcolor = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "x11_pixelsize", 22);
	
	
	result->x11Pixelsize = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const GlcdDriverConfig *, StructGlcdDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 22 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "bidirectional", 22);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->bidirectional, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 22);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "cellsize", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->cellsize, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "connectiontype", 22);
	
	ELEKTRA_SET (EnumGlcdConnectionType) (elektra, field, value->connectiontype, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 22);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "delaybus", 22);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->delaybus, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "fonthasicons", 22);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->fonthasicons, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_a", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapA, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_b", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapB, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_c", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapC, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_d", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapD, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_e", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapE, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_f", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapF, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keyrepeatdelay", 22);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->keyrepeatdelay, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keyrepeatinterval", 22);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->keyrepeatinterval, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "normal_font", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->normalFont, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 22);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "picolcdgfx_inverted", 22);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->picolcdgfxInverted, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "picolcdgfx_keytimeout", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->picolcdgfxKeytimeout, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "port", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->port, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 22);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "serdisp_device", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->serdispDevice, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "serdisp_name", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->serdispName, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "serdisp_options", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->serdispOptions, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "useft2", 22);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->useft2, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "x11_backlightcolor", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->x11Backlightcolor, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "x11_border", 22);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->x11Border, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "x11_inverted", 22);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->x11Inverted, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "x11_pixelcolor", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->x11Pixelcolor, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "x11_pixelsize", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->x11Pixelsize, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const GlcdDriverConfig *, StructGlcdDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 22 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "bidirectional", 22);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->bidirectional, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 22);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "cellsize", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->cellsize, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "connectiontype", 22);
	
	ELEKTRA_SET (EnumGlcdConnectionType) (elektra, field, value->connectiontype, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 22);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "delaybus", 22);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->delaybus, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "fonthasicons", 22);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->fonthasicons, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_a", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapA, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_b", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapB, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_c", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapC, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_d", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapD, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_e", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapE, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_f", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapF, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keyrepeatdelay", 22);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->keyrepeatdelay, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keyrepeatinterval", 22);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->keyrepeatinterval, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "normal_font", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->normalFont, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 22);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "picolcdgfx_inverted", 22);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->picolcdgfxInverted, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "picolcdgfx_keytimeout", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->picolcdgfxKeytimeout, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "port", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->port, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 22);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "serdisp_device", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->serdispDevice, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "serdisp_name", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->serdispName, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "serdisp_options", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->serdispOptions, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "useft2", 22);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->useft2, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "x11_backlightcolor", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->x11Backlightcolor, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "x11_border", 22);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->x11Border, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "x11_inverted", 22);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->x11Inverted, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "x11_pixelcolor", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->x11Pixelcolor, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "x11_pixelsize", 22);
	
	ELEKTRA_SET (String) (elektra, field, value->x11Pixelsize, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (GlcdlibDriverConfig, StructGlcdlibDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 16 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "CharEncoding", 16);
	
	
	result->CharEncoding = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "backlight", 16);
	
	
	result->backlight = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "brightness", 16);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 16);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "driver", 16);
	
	
	result->driver = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 16);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "fontfile", 16);
	
	
	result->fontfile = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "invert", 16);
	
	
	result->invert = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "minfontfacesize", 16);
	
	
	result->minfontfacesize = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 16);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "pixelshiftx", 16);
	
	
	result->pixelshiftx = ELEKTRA_GET (Short) (elektra, field);

	strncpy (&field[nameLen], "pixelshifty", 16);
	
	
	result->pixelshifty = ELEKTRA_GET (Short) (elektra, field);

	strncpy (&field[nameLen], "reboot", 16);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "showbigborder", 16);
	
	
	result->showbigborder = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "showdebugframe", 16);
	
	
	result->showdebugframe = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "showthinborder", 16);
	
	
	result->showthinborder = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "textresolution", 16);
	
	
	result->textresolution = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "upsidedown", 16);
	
	
	result->upsidedown = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "useft2", 16);
	
	
	result->useft2 = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (GlcdlibDriverConfig, StructGlcdlibDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 16 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "CharEncoding", 16);
	
	
	result->CharEncoding = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "backlight", 16);
	
	
	result->backlight = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "brightness", 16);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 16);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "driver", 16);
	
	
	result->driver = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 16);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "fontfile", 16);
	
	
	result->fontfile = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "invert", 16);
	
	
	result->invert = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "minfontfacesize", 16);
	
	
	result->minfontfacesize = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 16);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "pixelshiftx", 16);
	
	
	result->pixelshiftx = ELEKTRA_GET (Short) (elektra, field);

	strncpy (&field[nameLen], "pixelshifty", 16);
	
	
	result->pixelshifty = ELEKTRA_GET (Short) (elektra, field);

	strncpy (&field[nameLen], "reboot", 16);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "showbigborder", 16);
	
	
	result->showbigborder = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "showdebugframe", 16);
	
	
	result->showdebugframe = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "showthinborder", 16);
	
	
	result->showthinborder = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "textresolution", 16);
	
	
	result->textresolution = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "upsidedown", 16);
	
	
	result->upsidedown = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "useft2", 16);
	
	
	result->useft2 = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const GlcdlibDriverConfig *, StructGlcdlibDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 16 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "CharEncoding", 16);
	
	ELEKTRA_SET (String) (elektra, field, value->CharEncoding, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "backlight", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->backlight, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 16);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 16);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "driver", 16);
	
	ELEKTRA_SET (String) (elektra, field, value->driver, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 16);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "fontfile", 16);
	
	ELEKTRA_SET (String) (elektra, field, value->fontfile, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "invert", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->invert, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "minfontfacesize", 16);
	
	ELEKTRA_SET (String) (elektra, field, value->minfontfacesize, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 16);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "pixelshiftx", 16);
	
	ELEKTRA_SET (Short) (elektra, field, value->pixelshiftx, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "pixelshifty", 16);
	
	ELEKTRA_SET (Short) (elektra, field, value->pixelshifty, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "showbigborder", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->showbigborder, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "showdebugframe", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->showdebugframe, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "showthinborder", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->showthinborder, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "textresolution", 16);
	
	ELEKTRA_SET (String) (elektra, field, value->textresolution, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "upsidedown", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->upsidedown, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "useft2", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->useft2, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const GlcdlibDriverConfig *, StructGlcdlibDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 16 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "CharEncoding", 16);
	
	ELEKTRA_SET (String) (elektra, field, value->CharEncoding, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "backlight", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->backlight, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 16);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 16);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "driver", 16);
	
	ELEKTRA_SET (String) (elektra, field, value->driver, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 16);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "fontfile", 16);
	
	ELEKTRA_SET (String) (elektra, field, value->fontfile, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "invert", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->invert, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "minfontfacesize", 16);
	
	ELEKTRA_SET (String) (elektra, field, value->minfontfacesize, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 16);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "pixelshiftx", 16);
	
	ELEKTRA_SET (Short) (elektra, field, value->pixelshiftx, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "pixelshifty", 16);
	
	ELEKTRA_SET (Short) (elektra, field, value->pixelshifty, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "showbigborder", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->showbigborder, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "showdebugframe", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->showdebugframe, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "showthinborder", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->showthinborder, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "textresolution", 16);
	
	ELEKTRA_SET (String) (elektra, field, value->textresolution, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "upsidedown", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->upsidedown, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "useft2", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->useft2, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (GlkDriverConfig, StructGlkDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (GlkDriverConfig, StructGlkDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const GlkDriverConfig *, StructGlkDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const GlkDriverConfig *, StructGlkDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (Hd44780DriverConfig, StructHd44780DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 17 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 17);
	
	
	result->backlight = ELEKTRA_GET (EnumHD44780Backlight) (elektra, field);

	strncpy (&field[nameLen], "backlightcmdoff", 17);
	
	
	result->backlightcmdoff = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "backlightcmdon", 17);
	
	
	result->backlightcmdon = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "brightness", 17);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "charmap", 17);
	
	
	result->charmap = ELEKTRA_GET (EnumHD44780Charmap) (elektra, field);

	strncpy (&field[nameLen], "connectiontype", 17);
	
	
	result->connectiontype = ELEKTRA_GET (EnumHD44780ConnectionType) (elektra, field);

	strncpy (&field[nameLen], "contrast", 17);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "delaybus", 17);
	
	
	result->delaybus = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "delaymult", 17);
	
	
	result->delaymult = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 17);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "extendedmode", 17);
	
	
	result->extendedmode = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "file", 17);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "fontbank", 17);
	
	
	result->fontbank = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "keepalivedisplay", 17);
	
	
	result->keepalivedisplay = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "keymatrix_4_1", 17);
	
	
	result->keymatrix41 = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymatrix_4_2", 17);
	
	
	result->keymatrix42 = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymatrix_4_3", 17);
	
	
	result->keymatrix43 = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymatrix_4_4", 17);
	
	
	result->keymatrix44 = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keypad", 17);
	
	
	result->keypad = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "lastline", 17);
	
	
	result->lastline = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "lineaddress", 17);
	
	
	result->lineaddress = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "model", 17);
	
	
	result->model = ELEKTRA_GET (EnumHd44780Model) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 17);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "outputport", 17);
	
	
	result->outputport = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "port", 17);
	
	
	result->port = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 17);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "refreshdisplay", 17);
	
	
	result->refreshdisplay = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "size", 17);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 17);
	
	
	result->speed = ELEKTRA_GET (UnsignedLong) (elektra, field);

	strncpy (&field[nameLen], "vspan", 17);
	
	
	result->vspan = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Hd44780DriverConfig, StructHd44780DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 17 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 17);
	
	
	result->backlight = ELEKTRA_GET (EnumHD44780Backlight) (elektra, field);

	strncpy (&field[nameLen], "backlightcmdoff", 17);
	
	
	result->backlightcmdoff = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "backlightcmdon", 17);
	
	
	result->backlightcmdon = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "brightness", 17);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "charmap", 17);
	
	
	result->charmap = ELEKTRA_GET (EnumHD44780Charmap) (elektra, field);

	strncpy (&field[nameLen], "connectiontype", 17);
	
	
	result->connectiontype = ELEKTRA_GET (EnumHD44780ConnectionType) (elektra, field);

	strncpy (&field[nameLen], "contrast", 17);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "delaybus", 17);
	
	
	result->delaybus = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "delaymult", 17);
	
	
	result->delaymult = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 17);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "extendedmode", 17);
	
	
	result->extendedmode = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "file", 17);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "fontbank", 17);
	
	
	result->fontbank = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "keepalivedisplay", 17);
	
	
	result->keepalivedisplay = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "keymatrix_4_1", 17);
	
	
	result->keymatrix41 = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymatrix_4_2", 17);
	
	
	result->keymatrix42 = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymatrix_4_3", 17);
	
	
	result->keymatrix43 = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymatrix_4_4", 17);
	
	
	result->keymatrix44 = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keypad", 17);
	
	
	result->keypad = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "lastline", 17);
	
	
	result->lastline = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "lineaddress", 17);
	
	
	result->lineaddress = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "model", 17);
	
	
	result->model = ELEKTRA_GET (EnumHd44780Model) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 17);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "outputport", 17);
	
	
	result->outputport = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "port", 17);
	
	
	result->port = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 17);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "refreshdisplay", 17);
	
	
	result->refreshdisplay = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "size", 17);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 17);
	
	
	result->speed = ELEKTRA_GET (UnsignedLong) (elektra, field);

	strncpy (&field[nameLen], "vspan", 17);
	
	
	result->vspan = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const Hd44780DriverConfig *, StructHd44780DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 17 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 17);
	
	ELEKTRA_SET (EnumHD44780Backlight) (elektra, field, value->backlight, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "backlightcmdoff", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->backlightcmdoff, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "backlightcmdon", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->backlightcmdon, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "charmap", 17);
	
	ELEKTRA_SET (EnumHD44780Charmap) (elektra, field, value->charmap, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "connectiontype", 17);
	
	ELEKTRA_SET (EnumHD44780ConnectionType) (elektra, field, value->connectiontype, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "delaybus", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->delaybus, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "delaymult", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->delaymult, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "extendedmode", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->extendedmode, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "fontbank", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->fontbank, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keepalivedisplay", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->keepalivedisplay, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymatrix_4_1", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymatrix41, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymatrix_4_2", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymatrix42, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymatrix_4_3", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymatrix43, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymatrix_4_4", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymatrix44, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keypad", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->keypad, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lastline", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->lastline, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lineaddress", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->lineaddress, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "model", 17);
	
	ELEKTRA_SET (EnumHd44780Model) (elektra, field, value->model, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "outputport", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->outputport, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "port", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->port, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "refreshdisplay", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->refreshdisplay, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 17);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "vspan", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->vspan, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Hd44780DriverConfig *, StructHd44780DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 17 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 17);
	
	ELEKTRA_SET (EnumHD44780Backlight) (elektra, field, value->backlight, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "backlightcmdoff", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->backlightcmdoff, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "backlightcmdon", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->backlightcmdon, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "charmap", 17);
	
	ELEKTRA_SET (EnumHD44780Charmap) (elektra, field, value->charmap, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "connectiontype", 17);
	
	ELEKTRA_SET (EnumHD44780ConnectionType) (elektra, field, value->connectiontype, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "delaybus", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->delaybus, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "delaymult", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->delaymult, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "extendedmode", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->extendedmode, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "fontbank", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->fontbank, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keepalivedisplay", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->keepalivedisplay, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymatrix_4_1", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymatrix41, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymatrix_4_2", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymatrix42, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymatrix_4_3", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymatrix43, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymatrix_4_4", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->keymatrix44, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keypad", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->keypad, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lastline", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->lastline, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lineaddress", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->lineaddress, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "model", 17);
	
	ELEKTRA_SET (EnumHd44780Model) (elektra, field, value->model, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "outputport", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->outputport, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "port", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->port, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "refreshdisplay", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->refreshdisplay, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 17);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "vspan", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->vspan, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (Icp_a106DriverConfig, StructIcp_a106DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Icp_a106DriverConfig, StructIcp_a106DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const Icp_a106DriverConfig *, StructIcp_a106DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Icp_a106DriverConfig *, StructIcp_a106DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (ImonDriverConfig, StructImonDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "charmap", 14);
	
	
	result->charmap = ELEKTRA_GET (EnumIMonCharmap) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (ImonDriverConfig, StructImonDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "charmap", 14);
	
	
	result->charmap = ELEKTRA_GET (EnumIMonCharmap) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const ImonDriverConfig *, StructImonDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "charmap", 14);
	
	ELEKTRA_SET (EnumIMonCharmap) (elektra, field, value->charmap, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const ImonDriverConfig *, StructImonDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "charmap", 14);
	
	ELEKTRA_SET (EnumIMonCharmap) (elektra, field, value->charmap, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (ImonlcdDriverConfig, StructImonlcdDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 14);
	
	
	result->backlight = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "discmode", 14);
	
	
	result->discmode = ELEKTRA_GET (EnumIMonLCDDiscMode) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "onexit", 14);
	
	
	result->onexit = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "protocol", 14);
	
	
	result->protocol = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (ImonlcdDriverConfig, StructImonlcdDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 14);
	
	
	result->backlight = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "discmode", 14);
	
	
	result->discmode = ELEKTRA_GET (EnumIMonLCDDiscMode) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "onexit", 14);
	
	
	result->onexit = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "protocol", 14);
	
	
	result->protocol = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const ImonlcdDriverConfig *, StructImonlcdDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->backlight, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "discmode", 14);
	
	ELEKTRA_SET (EnumIMonLCDDiscMode) (elektra, field, value->discmode, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "onexit", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->onexit, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "protocol", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->protocol, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const ImonlcdDriverConfig *, StructImonlcdDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->backlight, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "discmode", 14);
	
	ELEKTRA_SET (EnumIMonLCDDiscMode) (elektra, field, value->discmode, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "onexit", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->onexit, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "protocol", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->protocol, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (IOWarriorDriverConfig, StructIOWarriorDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "extendedmode", 14);
	
	
	result->extendedmode = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "lastline", 14);
	
	
	result->lastline = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "serialnumber", 14);
	
	
	result->serialnumber = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (IOWarriorDriverConfig, StructIOWarriorDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "extendedmode", 14);
	
	
	result->extendedmode = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "lastline", 14);
	
	
	result->lastline = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "serialnumber", 14);
	
	
	result->serialnumber = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const IOWarriorDriverConfig *, StructIOWarriorDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "extendedmode", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->extendedmode, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lastline", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->lastline, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "serialnumber", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->serialnumber, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const IOWarriorDriverConfig *, StructIOWarriorDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "extendedmode", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->extendedmode, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lastline", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->lastline, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "serialnumber", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->serialnumber, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (IrManDriverConfig, StructIrManDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "config", 14);
	
	
	result->config = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (IrManDriverConfig, StructIrManDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "config", 14);
	
	
	result->config = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const IrManDriverConfig *, StructIrManDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "config", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->config, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const IrManDriverConfig *, StructIrManDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "config", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->config, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (IrtransDriverConfig, StructIrtransDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 14);
	
	
	result->backlight = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "hostname", 14);
	
	
	result->hostname = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (IrtransDriverConfig, StructIrtransDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 14);
	
	
	result->backlight = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "hostname", 14);
	
	
	result->hostname = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const IrtransDriverConfig *, StructIrtransDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->backlight, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "hostname", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->hostname, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const IrtransDriverConfig *, StructIrtransDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->backlight, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "hostname", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->hostname, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (JoyDriverConfig, StructJoyDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "map_axis1neg", 14);
	
	
	result->mapAxis1neg = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "map_axis1pos", 14);
	
	
	result->mapAxis1pos = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "map_axis2neg", 14);
	
	
	result->mapAxis2neg = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "map_axis2pos", 14);
	
	
	result->mapAxis2pos = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "map_button1", 14);
	
	
	result->mapButton1 = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "map_button2", 14);
	
	
	result->mapButton2 = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (JoyDriverConfig, StructJoyDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "map_axis1neg", 14);
	
	
	result->mapAxis1neg = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "map_axis1pos", 14);
	
	
	result->mapAxis1pos = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "map_axis2neg", 14);
	
	
	result->mapAxis2neg = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "map_axis2pos", 14);
	
	
	result->mapAxis2pos = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "map_button1", 14);
	
	
	result->mapButton1 = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "map_button2", 14);
	
	
	result->mapButton2 = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const JoyDriverConfig *, StructJoyDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "map_axis1neg", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->mapAxis1neg, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "map_axis1pos", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->mapAxis1pos, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "map_axis2neg", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->mapAxis2neg, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "map_axis2pos", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->mapAxis2pos, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "map_button1", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->mapButton1, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "map_button2", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->mapButton2, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const JoyDriverConfig *, StructJoyDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "map_axis1neg", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->mapAxis1neg, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "map_axis1pos", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->mapAxis1pos, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "map_axis2neg", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->mapAxis2neg, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "map_axis2pos", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->mapAxis2pos, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "map_button1", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->mapButton1, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "map_button2", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->mapButton2, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (Lb216DriverConfig, StructLb216DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Lb216DriverConfig, StructLb216DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const Lb216DriverConfig *, StructLb216DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Lb216DriverConfig *, StructLb216DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (Lcdm001DriverConfig, StructLcdm001DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backkey", 14);
	
	
	result->backkey = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "forwardkey", 14);
	
	
	result->forwardkey = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "mainmenukey", 14);
	
	
	result->mainmenukey = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "pausekey", 14);
	
	
	result->pausekey = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Lcdm001DriverConfig, StructLcdm001DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backkey", 14);
	
	
	result->backkey = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "forwardkey", 14);
	
	
	result->forwardkey = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "mainmenukey", 14);
	
	
	result->mainmenukey = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "pausekey", 14);
	
	
	result->pausekey = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const Lcdm001DriverConfig *, StructLcdm001DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backkey", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->backkey, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "forwardkey", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->forwardkey, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "mainmenukey", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->mainmenukey, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "pausekey", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->pausekey, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Lcdm001DriverConfig *, StructLcdm001DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backkey", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->backkey, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "forwardkey", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->forwardkey, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "mainmenukey", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->mainmenukey, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "pausekey", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->pausekey, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (LctermDriverConfig, StructLctermDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (LctermDriverConfig, StructLctermDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const LctermDriverConfig *, StructLctermDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const LctermDriverConfig *, StructLctermDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (Linux_inputDriverConfig, StructLinux_inputDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Linux_inputDriverConfig, StructLinux_inputDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const Linux_inputDriverConfig *, StructLinux_inputDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Linux_inputDriverConfig *, StructLinux_inputDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (LircDriverConfig, StructLircDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "lircrc", 14);
	
	
	result->lircrc = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "prog", 14);
	
	
	result->prog = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (LircDriverConfig, StructLircDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "lircrc", 14);
	
	
	result->lircrc = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "prog", 14);
	
	
	result->prog = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const LircDriverConfig *, StructLircDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lircrc", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->lircrc, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "prog", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->prog, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const LircDriverConfig *, StructLircDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lircrc", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->lircrc, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "prog", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->prog, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (LisDriverConfig, StructLisDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "lastline", 14);
	
	
	result->lastline = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "productid", 14);
	
	
	result->productid = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "vendorid", 14);
	
	
	result->vendorid = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (LisDriverConfig, StructLisDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "lastline", 14);
	
	
	result->lastline = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "productid", 14);
	
	
	result->productid = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "vendorid", 14);
	
	
	result->vendorid = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const LisDriverConfig *, StructLisDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lastline", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->lastline, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "productid", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->productid, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "vendorid", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->vendorid, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const LisDriverConfig *, StructLisDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lastline", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->lastline, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "productid", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->productid, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "vendorid", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->vendorid, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (MD8800DriverConfig, StructMD8800DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (MD8800DriverConfig, StructMD8800DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const MD8800DriverConfig *, StructMD8800DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const MD8800DriverConfig *, StructMD8800DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (Mdm166aDriverConfig, StructMdm166aDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "clock", 14);
	
	
	result->clock = ELEKTRA_GET (EnumMdm166aClock) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "dimming", 14);
	
	
	result->dimming = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "offdimming", 14);
	
	
	result->offdimming = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Mdm166aDriverConfig, StructMdm166aDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "clock", 14);
	
	
	result->clock = ELEKTRA_GET (EnumMdm166aClock) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "dimming", 14);
	
	
	result->dimming = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "offdimming", 14);
	
	
	result->offdimming = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const Mdm166aDriverConfig *, StructMdm166aDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "clock", 14);
	
	ELEKTRA_SET (EnumMdm166aClock) (elektra, field, value->clock, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "dimming", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->dimming, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offdimming", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->offdimming, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Mdm166aDriverConfig *, StructMdm166aDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "clock", 14);
	
	ELEKTRA_SET (EnumMdm166aClock) (elektra, field, value->clock, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "dimming", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->dimming, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offdimming", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->offdimming, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (Ms6931DriverConfig, StructMs6931DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Ms6931DriverConfig, StructMs6931DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const Ms6931DriverConfig *, StructMs6931DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Ms6931DriverConfig *, StructMs6931DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (Mtc_s16209xDriverConfig, StructMtc_s16209xDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Mtc_s16209xDriverConfig, StructMtc_s16209xDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const Mtc_s16209xDriverConfig *, StructMtc_s16209xDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Mtc_s16209xDriverConfig *, StructMtc_s16209xDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (MtxOrbDriverConfig, StructMtxOrbDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 23 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 23);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 23);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 23);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 23);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "hasadjustablebacklight", 23);
	
	
	result->hasadjustablebacklight = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "keymap_a", 23);
	
	
	result->keymapA = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_b", 23);
	
	
	result->keymapB = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_c", 23);
	
	
	result->keymapC = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_d", 23);
	
	
	result->keymapD = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_e", 23);
	
	
	result->keymapE = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_f", 23);
	
	
	result->keymapF = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 23);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 23);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 23);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 23);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "type", 23);
	
	
	result->type = ELEKTRA_GET (EnumMtxorbType) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (MtxOrbDriverConfig, StructMtxOrbDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 23 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 23);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 23);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 23);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 23);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "hasadjustablebacklight", 23);
	
	
	result->hasadjustablebacklight = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "keymap_a", 23);
	
	
	result->keymapA = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_b", 23);
	
	
	result->keymapB = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_c", 23);
	
	
	result->keymapC = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_d", 23);
	
	
	result->keymapD = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_e", 23);
	
	
	result->keymapE = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_f", 23);
	
	
	result->keymapF = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 23);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 23);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 23);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 23);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "type", 23);
	
	
	result->type = ELEKTRA_GET (EnumMtxorbType) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const MtxOrbDriverConfig *, StructMtxOrbDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 23 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 23);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 23);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "hasadjustablebacklight", 23);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->hasadjustablebacklight, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_a", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapA, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_b", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapB, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_c", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapC, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_d", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapD, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_e", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapE, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_f", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapF, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 23);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 23);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 23);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "type", 23);
	
	ELEKTRA_SET (EnumMtxorbType) (elektra, field, value->type, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const MtxOrbDriverConfig *, StructMtxOrbDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 23 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 23);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 23);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "hasadjustablebacklight", 23);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->hasadjustablebacklight, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_a", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapA, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_b", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapB, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_c", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapC, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_d", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapD, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_e", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapE, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_f", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapF, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 23);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 23);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 23);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 23);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "type", 23);
	
	ELEKTRA_SET (EnumMtxorbType) (elektra, field, value->type, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (Mx5000DriverConfig, StructMx5000DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 17 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 17);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 17);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 17);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 17);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 17);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 17);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "waitafterrefresh", 17);
	
	
	result->waitafterrefresh = ELEKTRA_GET (UnsignedLong) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Mx5000DriverConfig, StructMx5000DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 17 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 17);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 17);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 17);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 17);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 17);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 17);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "waitafterrefresh", 17);
	
	
	result->waitafterrefresh = ELEKTRA_GET (UnsignedLong) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const Mx5000DriverConfig *, StructMx5000DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 17 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "waitafterrefresh", 17);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->waitafterrefresh, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Mx5000DriverConfig *, StructMx5000DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 17 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 17);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 17);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 17);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "waitafterrefresh", 17);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->waitafterrefresh, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (NoritakeVFDDriverConfig, StructNoritakeVFDDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "parity", 14);
	
	
	result->parity = ELEKTRA_GET (UnsignedLong) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedLong) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (NoritakeVFDDriverConfig, StructNoritakeVFDDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "parity", 14);
	
	
	result->parity = ELEKTRA_GET (UnsignedLong) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedLong) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const NoritakeVFDDriverConfig *, StructNoritakeVFDDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "parity", 14);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->parity, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const NoritakeVFDDriverConfig *, StructNoritakeVFDDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "parity", 14);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->parity, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (Olimex_MOD_LCD1x9DriverConfig, StructOlimex_MOD_LCD1x9DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Olimex_MOD_LCD1x9DriverConfig, StructOlimex_MOD_LCD1x9DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const Olimex_MOD_LCD1x9DriverConfig *, StructOlimex_MOD_LCD1x9DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Olimex_MOD_LCD1x9DriverConfig *, StructOlimex_MOD_LCD1x9DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (PicolcdDriverConfig, StructPicolcdDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 19 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 19);
	
	
	result->backlight = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "brightness", 19);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 19);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 19);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "key0light", 19);
	
	
	result->key0light = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "key1light", 19);
	
	
	result->key1light = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "key2light", 19);
	
	
	result->key2light = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "key3light", 19);
	
	
	result->key3light = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "key4light", 19);
	
	
	result->key4light = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "key5light", 19);
	
	
	result->key5light = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "keylights", 19);
	
	
	result->keylights = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "keyrepeatdelay", 19);
	
	
	result->keyrepeatdelay = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "keyrepeatinterval", 19);
	
	
	result->keyrepeatinterval = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "keytimeout", 19);
	
	
	result->keytimeout = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "linklights", 19);
	
	
	result->linklights = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "lircflushthreshold", 19);
	
	
	result->lircflushthreshold = ELEKTRA_GET (UnsignedLong) (elektra, field);

	strncpy (&field[nameLen], "lirchost", 19);
	
	
	result->lirchost = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "lircport", 19);
	
	
	result->lircport = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "lirctime_us", 19);
	
	
	result->lirctimeUs = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 19);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 19);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (PicolcdDriverConfig, StructPicolcdDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 19 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 19);
	
	
	result->backlight = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "brightness", 19);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 19);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 19);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "key0light", 19);
	
	
	result->key0light = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "key1light", 19);
	
	
	result->key1light = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "key2light", 19);
	
	
	result->key2light = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "key3light", 19);
	
	
	result->key3light = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "key4light", 19);
	
	
	result->key4light = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "key5light", 19);
	
	
	result->key5light = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "keylights", 19);
	
	
	result->keylights = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "keyrepeatdelay", 19);
	
	
	result->keyrepeatdelay = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "keyrepeatinterval", 19);
	
	
	result->keyrepeatinterval = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "keytimeout", 19);
	
	
	result->keytimeout = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "linklights", 19);
	
	
	result->linklights = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "lircflushthreshold", 19);
	
	
	result->lircflushthreshold = ELEKTRA_GET (UnsignedLong) (elektra, field);

	strncpy (&field[nameLen], "lirchost", 19);
	
	
	result->lirchost = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "lircport", 19);
	
	
	result->lircport = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "lirctime_us", 19);
	
	
	result->lirctimeUs = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 19);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 19);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const PicolcdDriverConfig *, StructPicolcdDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 19 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->backlight, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 19);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 19);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 19);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "key0light", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->key0light, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "key1light", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->key1light, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "key2light", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->key2light, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "key3light", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->key3light, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "key4light", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->key4light, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "key5light", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->key5light, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keylights", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->keylights, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keyrepeatdelay", 19);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->keyrepeatdelay, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keyrepeatinterval", 19);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->keyrepeatinterval, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keytimeout", 19);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->keytimeout, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "linklights", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->linklights, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lircflushthreshold", 19);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->lircflushthreshold, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lirchost", 19);
	
	ELEKTRA_SET (String) (elektra, field, value->lirchost, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lircport", 19);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->lircport, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lirctime_us", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->lirctimeUs, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 19);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const PicolcdDriverConfig *, StructPicolcdDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 19 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "backlight", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->backlight, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 19);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 19);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 19);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "key0light", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->key0light, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "key1light", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->key1light, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "key2light", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->key2light, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "key3light", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->key3light, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "key4light", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->key4light, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "key5light", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->key5light, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keylights", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->keylights, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keyrepeatdelay", 19);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->keyrepeatdelay, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keyrepeatinterval", 19);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->keyrepeatinterval, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keytimeout", 19);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->keytimeout, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "linklights", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->linklights, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lircflushthreshold", 19);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->lircflushthreshold, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lirchost", 19);
	
	ELEKTRA_SET (String) (elektra, field, value->lirchost, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lircport", 19);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->lircport, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "lirctime_us", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->lirctimeUs, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 19);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 19);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (PyramidDriverConfig, StructPyramidDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (PyramidDriverConfig, StructPyramidDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const PyramidDriverConfig *, StructPyramidDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const PyramidDriverConfig *, StructPyramidDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (RawserialDriverConfig, StructRawserialDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedLong) (elektra, field);

	strncpy (&field[nameLen], "updaterate", 14);
	
	
	result->updaterate = ELEKTRA_GET (Float) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (RawserialDriverConfig, StructRawserialDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedLong) (elektra, field);

	strncpy (&field[nameLen], "updaterate", 14);
	
	
	result->updaterate = ELEKTRA_GET (Float) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const RawserialDriverConfig *, StructRawserialDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "updaterate", 14);
	
	ELEKTRA_SET (Float) (elektra, field, value->updaterate, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const RawserialDriverConfig *, StructRawserialDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "updaterate", 14);
	
	ELEKTRA_SET (Float) (elektra, field, value->updaterate, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (Sed1330DriverConfig, StructSed1330DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 15 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 15);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "cellsize", 15);
	
	
	result->cellsize = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "connectiontype", 15);
	
	
	result->connectiontype = ELEKTRA_GET (EnumSed1330Connectiontype) (elektra, field);

	strncpy (&field[nameLen], "contrast", 15);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 15);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 15);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "port", 15);
	
	
	result->port = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 15);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "type", 15);
	
	
	result->type = ELEKTRA_GET (EnumSed1330Type) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Sed1330DriverConfig, StructSed1330DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 15 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 15);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "cellsize", 15);
	
	
	result->cellsize = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "connectiontype", 15);
	
	
	result->connectiontype = ELEKTRA_GET (EnumSed1330Connectiontype) (elektra, field);

	strncpy (&field[nameLen], "contrast", 15);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 15);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 15);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "port", 15);
	
	
	result->port = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 15);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "type", 15);
	
	
	result->type = ELEKTRA_GET (EnumSed1330Type) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const Sed1330DriverConfig *, StructSed1330DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 15 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 15);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "cellsize", 15);
	
	ELEKTRA_SET (String) (elektra, field, value->cellsize, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "connectiontype", 15);
	
	ELEKTRA_SET (EnumSed1330Connectiontype) (elektra, field, value->connectiontype, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 15);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 15);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 15);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "port", 15);
	
	ELEKTRA_SET (String) (elektra, field, value->port, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 15);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "type", 15);
	
	ELEKTRA_SET (EnumSed1330Type) (elektra, field, value->type, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Sed1330DriverConfig *, StructSed1330DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 15 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 15);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "cellsize", 15);
	
	ELEKTRA_SET (String) (elektra, field, value->cellsize, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "connectiontype", 15);
	
	ELEKTRA_SET (EnumSed1330Connectiontype) (elektra, field, value->connectiontype, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 15);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 15);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 15);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "port", 15);
	
	ELEKTRA_SET (String) (elektra, field, value->port, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 15);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "type", 15);
	
	ELEKTRA_SET (EnumSed1330Type) (elektra, field, value->type, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (Sed1520DriverConfig, StructSed1520DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 16 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 16);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 16);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "delaymult", 16);
	
	
	result->delaymult = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 16);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "haveinverter", 16);
	
	
	result->haveinverter = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "interfacetype", 16);
	
	
	result->interfacetype = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "invertedmapping", 16);
	
	
	result->invertedmapping = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 16);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "port", 16);
	
	
	result->port = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 16);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "usehardreset", 16);
	
	
	result->usehardreset = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Sed1520DriverConfig, StructSed1520DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 16 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 16);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 16);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "delaymult", 16);
	
	
	result->delaymult = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 16);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "haveinverter", 16);
	
	
	result->haveinverter = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "interfacetype", 16);
	
	
	result->interfacetype = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "invertedmapping", 16);
	
	
	result->invertedmapping = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 16);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "port", 16);
	
	
	result->port = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 16);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "usehardreset", 16);
	
	
	result->usehardreset = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const Sed1520DriverConfig *, StructSed1520DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 16 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 16);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 16);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "delaymult", 16);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->delaymult, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 16);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "haveinverter", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->haveinverter, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "interfacetype", 16);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->interfacetype, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "invertedmapping", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->invertedmapping, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 16);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "port", 16);
	
	ELEKTRA_SET (String) (elektra, field, value->port, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "usehardreset", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->usehardreset, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Sed1520DriverConfig *, StructSed1520DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 16 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 16);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 16);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "delaymult", 16);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->delaymult, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 16);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "haveinverter", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->haveinverter, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "interfacetype", 16);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->interfacetype, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "invertedmapping", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->invertedmapping, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 16);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "port", 16);
	
	ELEKTRA_SET (String) (elektra, field, value->port, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "usehardreset", 16);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->usehardreset, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (SerialPOSDriverConfig, StructSerialPOSDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "cellsize", 14);
	
	
	result->cellsize = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "custom_chars", 14);
	
	
	result->customChars = ELEKTRA_GET (UnsignedLong) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "type", 14);
	
	
	result->type = ELEKTRA_GET (EnumSerialposType) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (SerialPOSDriverConfig, StructSerialPOSDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "cellsize", 14);
	
	
	result->cellsize = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "custom_chars", 14);
	
	
	result->customChars = ELEKTRA_GET (UnsignedLong) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "type", 14);
	
	
	result->type = ELEKTRA_GET (EnumSerialposType) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const SerialPOSDriverConfig *, StructSerialPOSDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "cellsize", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->cellsize, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "custom_chars", 14);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->customChars, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "type", 14);
	
	ELEKTRA_SET (EnumSerialposType) (elektra, field, value->type, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const SerialPOSDriverConfig *, StructSerialPOSDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "cellsize", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->cellsize, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "custom_chars", 14);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->customChars, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "type", 14);
	
	ELEKTRA_SET (EnumSerialposType) (elektra, field, value->type, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (SerialVFDDriverConfig, StructSerialVFDDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 18 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 18);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 18);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "custom-characters", 18);
	
	
	result->custom_characters = ELEKTRA_GET (UnsignedLong) (elektra, field);

	strncpy (&field[nameLen], "device", 18);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 18);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "iso_8859_1", 18);
	
	
	result->iso88591 = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 18);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "port", 18);
	
	
	result->port = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "portwait", 18);
	
	
	result->portwait = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 18);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 18);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 18);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "type", 18);
	
	
	result->type = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "use_parallel", 18);
	
	
	result->useParallel = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (SerialVFDDriverConfig, StructSerialVFDDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 18 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 18);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 18);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "custom-characters", 18);
	
	
	result->custom_characters = ELEKTRA_GET (UnsignedLong) (elektra, field);

	strncpy (&field[nameLen], "device", 18);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 18);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "iso_8859_1", 18);
	
	
	result->iso88591 = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 18);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "port", 18);
	
	
	result->port = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "portwait", 18);
	
	
	result->portwait = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 18);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 18);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 18);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "type", 18);
	
	
	result->type = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "use_parallel", 18);
	
	
	result->useParallel = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const SerialVFDDriverConfig *, StructSerialVFDDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 18 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 18);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 18);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "custom-characters", 18);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->custom_characters, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 18);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 18);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "iso_8859_1", 18);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->iso88591, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 18);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "port", 18);
	
	ELEKTRA_SET (String) (elektra, field, value->port, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "portwait", 18);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->portwait, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 18);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 18);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 18);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "type", 18);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->type, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "use_parallel", 18);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->useParallel, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const SerialVFDDriverConfig *, StructSerialVFDDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 18 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 18);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 18);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "custom-characters", 18);
	
	ELEKTRA_SET (UnsignedLong) (elektra, field, value->custom_characters, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 18);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 18);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "iso_8859_1", 18);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->iso88591, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 18);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "port", 18);
	
	ELEKTRA_SET (String) (elektra, field, value->port, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "portwait", 18);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->portwait, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 18);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 18);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 18);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "type", 18);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->type, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "use_parallel", 18);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->useParallel, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (SliDriverConfig, StructSliDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (SliDriverConfig, StructSliDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const SliDriverConfig *, StructSliDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const SliDriverConfig *, StructSliDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (Stv5730DriverConfig, StructStv5730DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "port", 14);
	
	
	result->port = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Stv5730DriverConfig, StructStv5730DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "port", 14);
	
	
	result->port = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const Stv5730DriverConfig *, StructStv5730DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "port", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->port, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Stv5730DriverConfig *, StructStv5730DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "port", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->port, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (SureElecDriverConfig, StructSureElecDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "edition", 14);
	
	
	result->edition = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (SureElecDriverConfig, StructSureElecDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "edition", 14);
	
	
	result->edition = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const SureElecDriverConfig *, StructSureElecDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "edition", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->edition, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const SureElecDriverConfig *, StructSureElecDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "edition", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->edition, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (SvgaDriverConfig, StructSvgaDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "mode", 14);
	
	
	result->mode = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (SvgaDriverConfig, StructSvgaDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "mode", 14);
	
	
	result->mode = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const SvgaDriverConfig *, StructSvgaDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "mode", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->mode, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const SvgaDriverConfig *, StructSvgaDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "mode", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->mode, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (T6963DriverConfig, StructT6963DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "bidirectional", 14);
	
	
	result->bidirectional = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "cleargraphic", 14);
	
	
	result->cleargraphic = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "delaybus", 14);
	
	
	result->delaybus = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "port", 14);
	
	
	result->port = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (T6963DriverConfig, StructT6963DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "bidirectional", 14);
	
	
	result->bidirectional = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "cleargraphic", 14);
	
	
	result->cleargraphic = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "delaybus", 14);
	
	
	result->delaybus = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "port", 14);
	
	
	result->port = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const T6963DriverConfig *, StructT6963DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "bidirectional", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->bidirectional, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "cleargraphic", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->cleargraphic, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "delaybus", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->delaybus, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "port", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->port, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const T6963DriverConfig *, StructT6963DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "bidirectional", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->bidirectional, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "cleargraphic", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->cleargraphic, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "delaybus", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->delaybus, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "port", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->port, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (TextDriverConfig, StructTextDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (TextDriverConfig, StructTextDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const TextDriverConfig *, StructTextDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const TextDriverConfig *, StructTextDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (TyanDriverConfig, StructTyanDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (TyanDriverConfig, StructTyanDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "speed", 14);
	
	
	result->speed = ELEKTRA_GET (UnsignedShort) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const TyanDriverConfig *, StructTyanDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const TyanDriverConfig *, StructTyanDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "speed", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->speed, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (Ula200DriverConfig, StructUla200DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_a", 14);
	
	
	result->keymapA = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_b", 14);
	
	
	result->keymapB = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_c", 14);
	
	
	result->keymapC = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_d", 14);
	
	
	result->keymapD = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_e", 14);
	
	
	result->keymapE = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_f", 14);
	
	
	result->keymapF = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Ula200DriverConfig, StructUla200DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_a", 14);
	
	
	result->keymapA = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_b", 14);
	
	
	result->keymapB = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_c", 14);
	
	
	result->keymapC = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_d", 14);
	
	
	result->keymapD = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_e", 14);
	
	
	result->keymapE = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "keymap_f", 14);
	
	
	result->keymapF = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const Ula200DriverConfig *, StructUla200DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_a", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapA, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_b", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapB, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_c", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapC, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_d", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapD, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_e", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapE, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_f", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapF, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Ula200DriverConfig *, StructUla200DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_a", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapA, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_b", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapB, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_c", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapC, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_d", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapD, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_e", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapE, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "keymap_f", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->keymapF, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (Vlsys_m428DriverConfig, StructVlsys_m428DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Vlsys_m428DriverConfig, StructVlsys_m428DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "device", 14);
	
	
	result->device = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const Vlsys_m428DriverConfig *, StructVlsys_m428DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Vlsys_m428DriverConfig *, StructVlsys_m428DriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "device", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->device, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (XosdDriverConfig, StructXosdDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "Font", 14);
	
	
	result->Font = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "offset", 14);
	
	
	result->offset = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (XosdDriverConfig, StructXosdDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "Font", 14);
	
	
	result->Font = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "offset", 14);
	
	
	result->offset = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const XosdDriverConfig *, StructXosdDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "Font", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->Font, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offset", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->offset, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const XosdDriverConfig *, StructXosdDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "Font", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->Font, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offset", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->offset, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_GET_OUT_PTR_SIGNATURE (Yard2LCDDriverConfig, StructYard2LCDDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Yard2LCDDriverConfig, StructYard2LCDDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	
	result->brightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "contrast", 14);
	
	
	result->contrast = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "file", 14);
	
	
	result->file = ELEKTRA_GET (String) (elektra, field);

	strncpy (&field[nameLen], "offbrightness", 14);
	
	
	result->offbrightness = ELEKTRA_GET (UnsignedShort) (elektra, field);

	strncpy (&field[nameLen], "reboot", 14);
	
	
	result->reboot = ELEKTRA_GET (Boolean) (elektra, field);

	strncpy (&field[nameLen], "size", 14);
	
	
	result->size = ELEKTRA_GET (String) (elektra, field);

	elektraFree (field);
}

ELEKTRA_SET_SIGNATURE (const Yard2LCDDriverConfig *, StructYard2LCDDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Yard2LCDDriverConfig *, StructYard2LCDDriverConfig)
{
	size_t nameLen = strlen (keyname);
	char * field = elektraCalloc ((nameLen + 1 + 14 +1 + ELEKTRA_MAX_ARRAY_SIZE) * sizeof (char));
	strcpy (field, keyname);
	field[nameLen] = '/';
	++nameLen;

	elektraWriteArrayNumber (&field[nameLen], index);
	nameLen = strlen (field);
	field[nameLen] = '/';
	++nameLen;

	strncpy (&field[nameLen], "brightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->brightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "contrast", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->contrast, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "file", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->file, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "offbrightness", 14);
	
	ELEKTRA_SET (UnsignedShort) (elektra, field, value->offbrightness, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "reboot", 14);
	
	ELEKTRA_SET (Boolean) (elektra, field, value->reboot, error);
	if (error != NULL)
	{
		return;
	}

	strncpy (&field[nameLen], "size", 14);
	
	ELEKTRA_SET (String) (elektra, field, value->size, error);
	if (error != NULL)
	{
		return;
	}

}

