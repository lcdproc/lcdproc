Recent changes:

Key:
     - Something removed
     + Something added
     * Something changed / fixed

v0.5dev (ongoing development)

v0.5.7
 * Fix using the left key to change the ring and checkbox menu items
 * sed1520: Add an option for inverted segment mapping (idea by R. Buchert)
 * Fix build if compiled with --enable-permissive-menu-goto (#3599609)
 * glcd driver: Add direct support for framebuffer with paged memory layout
 + hd44780: Added 'raspberrypi' connection type (P. Corner)
 * Build system: Rename configure.in to configure.ac
 + hd44780: Add support for FreeBSD's iic I2C framework
 + hd44780: New connection type 'piplate' (J. Brogdon)
 * lcdproc client: Add option to turn off title on OldTime screen
 + glcd driver: Connection type 'x11' that draws to a X window (S. Meharg)
 + glcd driver: 'picolcdgfx' for picoLCD 256x64 from Mini-Box.com (S. Meharg)
 * picolcd: Improvements to IR processing (M. Jones)
 + hd44780: Added 'spi' connection type (S. Dawson)
 * hd55780/serial: Change data escape handling (G. Smith)
 + hd44780: New connection type 'pifacecad' (T. Preston)
 + new driver: rawserial - dump framebuffer to serial port (C. Wolf)

v0.5.6
 - Remove deprecated CFontz633 driver. Use CFontzPacket with Model=633 instead!
 + new driver: glcd unified graphic display driver (M. Dolze)
 + new driver: vlsys_m428 driver for Moneual MonCaso 320 (W. Hauck)
 * hd44780/serial: Fix handling of keys for LoS-Panel (M. Kirchner)
 * hd44780/serial: Change backlight handling (see commit message)
 + glcd driver: Add FreeType 2 rendering support (B. Walle and M. Dolze)
 + glcd driver: New connection type 'png', writes framebuffer as PNG image
 + hd44780: Fix vBars and block icon not working correctly on the same screen
 * Build system: Link LCDd itself with pthread library if available
 * lcdproc client: Unbreak build on OpenBSD non-i386 (E. Barrett)
 * lcdproc client: Fix core dumps on FreeBSD if started as non-root
 + glcd driver: new connection type 'serdisplib'
 + glcd driver: new connection type 'glcd2usb'
 * lcdproc client: On FreeBSD show only unique processes in 'S' screen
 * picolcd: Use libusb-1.0 asynchronous transfers to fix missed keys (M. Jones)
 * sed1520: Make it work without using an external inverter
 * lis driver: Make the last pixel row work (add lastline option parsing)
 * hd44780/ftdi: Fix error in setting backlight and add backlight for 4bit mode
 + new driver: sdeclcd for Watchguard FireBox appliances (F. Mertz)
 + hd44780: Added 'usb4all' connection type (T. Mohaupt)
 * Fix RPM init script hanging on recent OS (#3488223)
 * lcdproc client: Fix build on OpenBSD >= 5.0
 * hd44780/serial: Add support for adjustable backlight
 * Build system: Fix build with automake >= 1.11.3 (#3494755)
 * Fix clang compiler warnings about obsolete GNU designated initializers
 * Fix crash when adding a menu item without text and options (S. Dawson)

v0.5.5
 + sed1330 driver: Add support for HG25504 (L. Lagendijk)
 * Build system: Fix xosd detection
 * Reset ccmode on screen clear for NoritakeVFD, tyan driver
 * All drivers: Refactor CGmode enum into lcd.h
 * hd44780: New (possible faster) screen update algorithm
 + Add an option to start LCDd with screen rotation disabled (M. Zanetti)
 * Rewrite of the t6963 driver fixing several problems (see commit note).
 * picolcd: Add OffBrightness (S. Crane), more icons and fix bignum
 * lcdproc: Fix a single dot appearing on line 5 of the D screen
 * lcdproc: Make the iface screen more readable on 2x16 displays
 * All drivers: sync functions with prototypes
 * Port remaining drivers to 0.5 API (glk, lcdm001, stv5730)
 * Pyramid: Only send LED state changes, doxygen-ize
 * sed1520: Make it work with 68-family style interface, convert to 0.5 API
 * hd44780/imon: Add charmap for NEC uPD16314 and make it configurable
 * Pyramid: Convert set_char to 8 byte bitmap, add support for bignum
 * Convert set_char of drivers bayrad, lb216, lcterm, mtc_s16209x, wirz-sli
 * CFontzPacket: Fix not using select() even if its available
 * CFontzPacket: Use cellwidth = 5 on CFA-631 and CFA-635 too
 * t6963: Convert font to 8 byte bitmap
 * convert i2500vfd and mdm166a driver to use 8 byte bitmap custom character
 * ula200: convert custom characters to 8 byte bitmap
 * Build system: Fix trying to install svga driver twice
 + imon: Add bignum support (Y. Scheglyuk)
 * hd44780/ethlcd: fix LCDd hang up on network error (exits now) (M. Bialonczyk)
 + New common 5x8 font for t6963, sed1520, mdm166a and i2500vfd
 * t6963: Size has now to be configured in pixels instead of characters
 + Build system: --enable-extra-charmaps option adds language specific charmaps
 * imonlcd: Fix spinning of disc icon (E. Pooch)
 * SureElec: Change port initialization (SF 3212891)
 * curses: Fix missed keystrokes (from Debian)
 * curses: Correct the name of the info function.
 * Update LCDd.conf to be better understood by Config::Model (D. Dumont)
 * CFontz: Fix display of bars

v0.5.4
 * Update driver includes and LDFLAGS (fixed glk and bayrad binding error)
 * Modify all drivers to use server/drivers/report.h instead of shared/report.h
 * Make all drivers using parallel port fail if they cannot get IO permission
 * Include <errno.h> instead of <sys/errno.h> everywhere (found by S. Klauer)
 * configure: Fix de-selecting of drivers
 * Build system: Add a check testing if SA_RESTART flag is available.
 * picolcd driver: Fix RC-5 for picoLCD 20x2 (A. van Schie)
 * Introduce shared/defines.h to collect commonly used macros
 + new driver: SureElec for devices made by SURE Electronics (Laurent Latil)
 * hd44780-4bit: Fix bug not checking for keypad rows 7-10 correctly
 * Modify serial drivers to use cfmakeraw
 + hd44780: Add a 'none' charmap which does not replace any character
 * hd44780: Change mapping for spanish 'n with tilde' characters
 * hd44780: Exclude pin for switchable backlight from keypad scanning
 * server core: New network input buffering
 * hd44780: Extended subdriver API and performance improvement for lcd2usb
 * hd44780: Add KOI8-R character mapping (Yura Scheglyuk)
 * shuttleVFD: Add support for devices with USB VID 0x1308 again
 * CFontzPacket: Add CFA-533. Use HD44780 character mapping for CFA533/633.
 * CFontzPacket: Report cell size correctly, use block character for icon.
 * CFontz633: Deprecate this driver. Use CFontzPacket with Model=633 instead!
 - Remove native win32 support.
 * picolcd: Use module output function to control key LEDs (Martin Jones)
 + hd44780: Added 'usbtiny' connection type (Siarhei Herasimchuk)
 * SureElec: Fix stall on startup
 + new driver: MDM166A for Futaba/Targa USB VFD (Christoph Rasim)
 * picolcd: Fix backlight and contrast handling (M. T. Jones)
 * picolcd: Fix heartbeat icon messing up with vbar
 * imonlcd: Fix possible lockup in output (icons) function (E. Pooch)
 * pyramid: Fix buffer overflow in set_leds, incorrect escaping, and icons

v0.5.3
 + lcdexec: notification when called program finishes
 + xosd driver: offset from top-left corner
 + LCDd: Hello= config option for startup message
 * glk driver: bignum support fixed
 * update local snprintf to the last one, which is GPL
 - server core: cleanup - remove unnecessary cruft
 * server core: refactor rendering code
 + LCDd: config options TitleSpeed= and Heartbeat=
 * overhauled CwLnx driver (Dave Platt)
 * overhauled picolcd driver (Nicu Pavel)
 + add ServerScreen=blank option value
 * MtxOrb driver: fix VFD/VKD support (Ethan Dicks)
 + new driver lis for VLSystem L.I.S VFD (Daryl Fonseca-Holt)
 * serialPOS driver: input support, cleanup (Eric Pooch)
 + new driver shuttleVFD for USB-based Shuttle VFDs (Thien Vu)
 * enable building lcdproc client on FreeBSD AMD64 platform (M. Dolze)
 * Autoconf fixes for Net/FreeBSD (M. Dolze)
 * fixes for the lcpdroc client iface screen on *BSD (M.Dolze)
 + key support for hd44780/lcd2usb (M. Dolze)
 + option Priority for NoritakeVFD (idea: Richard Muratti)
 * overhauled NoritakeVFD driver (idea: Richard Muratti)
 * allow wrapping slider menu items with <= 4 keys (M. Dolze)
 * a bit more Doxygen-ation and documentation
 * hd44780 driver: make ConnectionType 4bit more fool-proof (M. Dolze)
 * picolcd driver: overhaul USB init for more portability (M. Dolze)
 + curses driver: new option DrawBorder (Bruno Schwander)
 * avoid sending duplicate "success" messages in response to menu_add_item
 + lcdexec: parameter support using widgets
 * server core: display values in menu for all input MenuEntry types
 * hd44780 driver: reorganize ConnectionType recognition
 + hd44780 driver: new ConnectionType mplay
 + hd44780 driver: get/set contrast & brightness support for lcd2usb (M. Dolze)
 + hd44780 driver: reporting in ConnectionType functions (Thomas Jarosch)
 * clean up build system (T. Jarosch, with tests/fixes by M. Dolze)
 + hd44780 driver: new ConnectionType ftdi (Thomas Jarosch)
 * shuttleVFD driver: fix VendorID, support newer models (Miska Sulander)
 + i2500vfd driver for a graphical Noritake VFD (Thomas Jarosch)
 * hd44780 driver: more complete initialization (idea by Pierre Ossman)
 * hd44780 driver: "euro" character mapping table (Markus Dolze)
 * CwLnx driver: support for CW12832
 + new driver mx5000 for Logitech MX5000 keyboards (Christian Jodar)
 * update debian init scripts to allow multiple instances of clients
 + serialVFD driver: support for 4 more display types (Stefan Herdler)
 * IOWarrior driver: fixed 4-line display use
 + IOWarrior driver: add ExtendedMode option
 * lcdproc: unbreak compilation on OS X 1.5 and higher (Topher Fangio)
 * server: fix interpretation of numeric priorities (Andrew M. Bishop)
 + hd44780 driver: new ConnectionType ethlcd (Mariusz Bialonczyk)
 + picolcd driver: support to send IR commands to LIRC (Jack Cleaver)
 + picolcd driver: support for 20x4 picoLCDs (Nicu Pavel)
 + hd44780 driver: new ConnectionType uss720 (Eric Pooch)
 * new make targets to compile & install server & clients separately
 * autoconf fixes: check for pkg-config macros
 + new driver irtrans (Phant0m / Aron Parsons)
 + serialVFD driver: support Siemens/Wincor Nixdorf BA63/66 (Stefan Herdler)
 + new client lcdident.pl (Ethan Dicks)
 * server: prevent stalling if time goes backwards (M. Vallevand)
 * hd44780 driver: update the backlight setting only on change
 + hd44780 driver: add lineaddress option to support ST7036 (Malte Poeggel)
 + lcdproc: Retrieve per CPU usage statistics on FreeBSD (M. Dolze)
 * Change drivers to start in background by default: bayrad, CFontz, ea65, glk
 * imon driver: Change hbars to make last char in-/decrease vertically
 + hd44780 driver: port usblcd connection type from 0.4.5 to 0.5 (M. Dolze)
 * server: Fix interpretation of named screen priorities
 + new driver imonlcd for SoundGraph iMON LCD (Dean Harding, Jonathan Kyler)
 * MtxOrb driver: Fix backlight for old firmware
 * Add checks for null pointer / failed malloc to menu code
 * lcdproc, lcdexec, lcdvc clients: Use our included getopt.h
 * documentation: Fix XML errors in user and developer guide
 + documentation: Add man page for lcdvc

v.0.5.2
 * fix switching on/off the Load screen in lcdproc client using the menu
 * refactor adv_bignum: support height > 4, loadable chars with offset
 * ged rid of global variables buffer & tmp in lcdproc client
 + add init-script templates for LSB 3.x conforming systems
 * fix cellwidth in iMon driver
 + graphics character support in iMon driver (John Saunders)
 * try harder to find a shell in lcdexec
 + big number support in CwLnx driver via bignum library
 * do not copy more bytes than reserved in get_fs() in lcdproc's machine_*.c
 * update lcdproc's About screen with credits
 * fixes to ula200 build environment and documentation (B. Walle)
 * update g15 driver to be compatible with g15daemon > 1.2 (Anthony J. Mirabella)
 * allow compiling with LDFLAG "--as-needed" (Robert Buchholz)
 * optimize MtxOrb flush()
 * make MtxOrb use Brightness and OffBrightness like CFontzPacket
 * make Brightness & OffBrightness run-time configurable in CFontz & MtxOrb
 * add support for Iface and ProcSize screens for OS X / Darwin (Eric Pooch)
 * update & fix serialVFD driver (Stefan Herdler)
 + Hitachi SP14Q002 support & ConnectionType setting for sed1330 (Benjamin Wiedmann)
 * replace obsolete index() by strchr() (Guillaume LECERF)
 * fixes for CwLinux driver (Gideon Tsang)
 + new ConnectionType vdr-wakeup in hd44780 driver (originally Frank Jepsen)
 * fixes for the hd44780-serial drivers (Matteo Pillon)
 + new options for the hd44780 driver (Frank Jepsen)
 + new driver for text based serial Point of Sale displays (Eric Pooch)
 + new 2-line bignum mode using 6 user defined characters (Robin Gilks)
 * fix custom characters [with NewFirmware=yes] & backlight in CFontz
 + ea65 - driver for VFD in AOpen XC Cube AV EA65 barebones (Karsten Festag)
 + picolcd - driver for Mini-box.com USB LCD picoLCD (Gatewood Green)
 + new ConnectionType pertelian in hd44780 driver (Matteo Pillon)
 + new ConnectionType lcd2usb in hd44780 driver
 + make LCDd's GoodBye message configurable
 - remove compile-time option to read configuration from LDAP

v.0.5.1
 + config file support in lcdproc client (Andrew Foss)
 * install server & client config files in $(sysconfdir)
 + new client lcdvc: virtual console on LCD
 * convert adv_bignum library to bitwise set_char()
 * adapt drivers to modified bignum library:
   + CFontz
   + CFontz633
   + CFontzPacket
   + hd44780
   + IOWarrior
   + MtxOrb
   + serialVFD
   + tyan
 + add iface screen(s) to lcdproc client (Andrew Foss/Markus Dolze)
 * update sample Perl clients
 * fix LCDd crash on shutdown of clients using "menu_set_main" (Andrew Foss)
 * fix sock_connect() to allow 0 as legal socket (Frederick Nacino)
 * improve serialVFD driver (Stefan Herdler)
 + add MD8800 driver for LCDs in Medion MD8800 PCs (Stefan Herdler/Martin Møller)
 * make bigclock client screen more flexible: centered, require only 14 chars
 * correct axis mapping logic for joystick driver (Jannis Achstetter)
 * fixes to the CFontz driver
 + new unified serial sub-driver for hd44780 (Matteo Pillon)
 - obsolete hd44780 subdrivers-removed: lcdserializer, picanlcd
 * character mapping for special HD44780 modules (Matteo Pillon)
 + g15 - driver for LCD in Logitech G15 gaming keyboards (Anthony J. Mirabella)
 + EyeboxOne - driver for the LCD display on the EyeboxOne (Cedric TESSIER)
 * more flexible parsing of config files: allow spaces around sections and keys
 * updated & extended documentation
 * command line options more synchronizd between server and clients
 * CFontzPacket & CFontz633 support big & little endian systems
 * MtxOrb driver completely rewritten to support used-defined characters
 * CwLnx driver completely rewritten to support Cw1602 LCDs in addition
   to the CW12232 LCDs
 * more options in lcdproc's config file lcdproc.conf:
   + time and date formats can be changed using strftime formats
   + Load screen's LowLoad & HighLoad thresholds
 * revamped lcdexec's menu configuration
 * various little fixes

v0.5.0
This version has split off from unstable-0.4.3. Includes major changes.
 + LCDd now does dynamic loading of driver modules
 * New API in use
 + LCDd will use fill-in functions for drivers that don't support a certain
   function. Available for: vbar, hbar, bignum, heartbeat, icon, cursor.
 * Server internal functions cleaned up, moved to other files etc.
 * Menu system rewritten
 + Client-supplied menus are now supported
 * Key support improved: now keys have descriptive names, not just a letter
   anymore.
 + Reloading of configuration and drivers by sending a SIGHUP. Probably
   needs improvement ;)
 * HD44780 output register support on winamp wiring
 * Reporting levels are now used more consistently
 + lcdexec client added
 * Display update frequency is now exactly 8Hz. Key scan frequency increased
   to 32Hz.
 * Priority scheme changed. Priority classes are now used, that indicate what
   kind of a screen we're dealing with. More useful for interactive clients.
 + better support for operating systems other than Linux: *BSD, Darwin
 + menu support for lcdproc client (configure option)
 + Added CFontz633 driver for CrytalFontz CFA633
 + Added CFontzPacket driver fro CrystalFontz CFA-633, CFA-631, CFA-635
 + Added glcdlib meta-driver for all LCDs supported by graphlcd-base
 + Added icp_a106 driver for A106 alarm/LCD boards in 19" racks by ICP
 + Added imon driver for iMON IR/VFD Modules from Silverstone, Uneed, ...
 + Added IOWarrior driver for IOWarrior USB controllers
 + Added lcterm driver for serial LCD terminal from Neumark Elektronik
 + Added ms6931 driver for MSI-6931 displays in 1U rack servers by MSI
 + Added mtc_s16209x driver for MTC-S16209x by Microtips Technology Inc
 + Added NoritakeVFD driver for Noritake VFD Device CU20045SCPB-T28A
 + Added pyramid driver for Pyramid LC-Displays
 + Added serialVFD driver for NEC FIPC8367 based serial VFDs
 + Added tyan driver for LCDs in Tyan Barebone GS series
 + Added ula200 driver for the USB board ULA-200 from ELV
 + Added xosd driver for On Screen Display on X
 + Added HD44780 subdriver bwctusb for USB-to-LCD converters from BWCT
 + Added HD44780 subdriver lcdserializer for an serial-LCD converter
 + Added HD44780 subdriver i2c to conned LCDs using the I2C bus
 + Added HD44780 subdriver lis2 for the LIS2 serial-to-USB converter

v0.4.3
 - Removed possibility of passing arguments to the drivers from the
   command line, which can be done in the configuration file now
 + Added configuration file support
 + Added ability of dropping root privileges to LCDd
 + Added LCDM001 driver (kernelconcepts.de)
 + Added Toshiba T6963 driver
 + Added Seiko-Epson SED 1520 driver
 + Added Seiko-Epson SED 1330 driver
 + Added STV5730 driver composite TV signal character generation driver
 * Modified the CFontz driver so that the new ROM version is
   supported
 + Added ASCII emulation of BigNum to the drivers that did not
   support custom characters
 + Added ability of configuring (rearranging) the keypad of MtxOrb
   displays
 + Added .lircrc location to the options of the lirc driver
 * Fixed backlight code (partly rewritten)
 * Fixed heartbeat code (completely rewritten analogous to
   the backlight code)
 + Added init scripts for debian and Redhat/Mandrake
 + Added EPM list file so that you can use EPM
   (http://www.easysw.com/epm) to generate packages of LCDproc
 + Added documentation in docbook format
   (LCDproc User's Guide - docs/lcdproc-user)
 * Ported some screens of lcdproc to *BSD.
 * Ported the parallel port functions in port.h to *BSD.

v0.4.2:
 - Removed multiple output driver support - to simplify life
 * Removed use of sprintf() (in favor of snprintf) for security
 + Added ability to use defined port and address to listen on
   (for security) - now defaults to 127.0.0.1
 + Revamped curses driver: now supports color; has border; and
   can be (with recompile) placed anywhere on screen it fits
 + Added --waittime server argument.  You can now specify how long
   a screen should stay on the display.  The default is 64 ticks or
   8 seconds.
 + Fixed compile problems on non-Linux platforms where cfmakeraw,
   CBAUD, and ioperm don't exist.  All drivers now use the same code
   to initialize a serial port.

v0.4.1:
 + Changed protocol to 0.3 because we now accept both -foo and foo
   as argument introducers.  in 0.4-pre10, these were changed
   from foo to -foo and this broke many clients.  This change
   should fix that and be backward/forward compatible.
 + Added a noop function to the protocol.  This is useful for writing
   shell script clients of LCDproc.
 + Changed the shared sock_send_string function to NOT send the
   ending NUL ('\0') byte with the string.  This was confusing
   in Perl clients which saw NUL's as the first character of
   each reply.  WARNING: The LCDPROC client depended on this
   behavior, your client may too.  Use the newline instead.
   See the code in clients/lcdproc/main.c (look for "switch(buf[i])")
   to see one way to handle this.  This should allow new clients
   to connect to old servers and vice-versa.
 + Messed with include structure in .c files.  It should no longer
   be necessary to specify ../.. to get to shared code headers.
   Also, since we are using automake, the global config.h can be
   found with just #include "config.h" as it's location is included
   in a -I to the compilers.  As a result "make distcheck" now works.
 + Added support for client_add_key and friends.  Clients MUST now
   request keys to get notified.  Keys are NO LONGER broadcast to all
   clients.  Keys are now tied to a SCREEN and CLIENT.  There are new
   commands (screen_add_key, screen_del_key) to bind keys to a
   screen. If the current screen has requested the key just pressed,
   the client gets the key and no other client gets it.  If the current
   screen has not requested the current key, then the key is broadcast
   to all clients who requested that key with client_add_key.  Clients
   might want to request a "hotkey" with client_add_key so they will
   be notified of that key no matter what screen is currently on the
   display, and then request whatever other keys a screen uses with
   screen_add_key.
 + Added driver for Matrix Orbital GLK12232-25 graphical display.
   Display is run in text mode since there isn't enough information in
   the widgets.  If we knew the max width/height of an hbar/vbar, we
   could use graphical vbar/hbar's.  Oh well.  BigNums' look really
   cool though.  You'll need some additional characters in the "Small Font"
   font (and the BigNumbers font) for this to work.
 + Rewrote Big Number Clock function in chrono.c.  Many problems with
   it have been fixed.  It now uses number value 10 to display a colon
   in between the digits.  Don't know what this will do on a the MtxOrb
   driver.  GLK driver draws a colon.
 + Cleaned up the CPU GRAPH (G) function in cpu.c and made get_load take
   a 'struct load' from the caller to save the last time values.  This
   allows two callers (CPU GRAPH and CPU LOAD) to call get_load and
   both get reaosonable values.  Without this, CPU GRAPH was always
   getting 0 because it was called immediatly after CPU LOAD called
   get_load and no clock ticks had gone by.
 + Changed --enable-debug autoconf option to be processed correctly.
   Debug now turns on -g and turns off -O6
 + Lots of other little fixes.

v0.4-pre10:
 + Metar client added (Weather information)
 + Linux infrared control (LIRC) driver added.
 + "rotating slash" heartbeat mode (looks weird on CFontz displays)
   Use "screen_set s -heartbeat slash" to use this.
 * Switched to client-server protocol V0.2, so arguments can come in
   any order, and require a "-" on the front.  Instead of
   "widget_set s w 1 2 foo", use "widget_set s w -x 1 -y 2 -text foo".
 * Server info screen no longer enabled by default.
 * Merged Andrew McMeikan's HD44780 driver changes
 - Removed WHATSNEW
 + Added ChangeLog (okay, fine, I renamed WHATSNEW and typed a few
   entries into it :)

V0.4-pre9:
 * small fixes for irix
 * Added flag in LCDd to shut off server screen:
 	"-i off" or "--serverinfo off"
 + Wirz SLI driver
 + 16x2 support (server only)
 + MtxOrb generic output support in driver
 * Misc libc5 fixes
 + support for arbitrary display sizes; anything 16-80 characters
   wide, and 2-25 characters tall.  (doesn't work if you try to use
   a display size bigger than your LCD..  :)
 * curses driver looks slightly better (titles)
 * sizes > 20x4 no longer crash

V0.4-pre8:
 * LCDd gives more updated info in the connect string:
	cell size, lcd dimensions.
 * bargraphs are now the correct length on CFontz displays, in the main
   lcdproc client.  For other clients, please parse the connect string!
 * MtxOrb driver supports simultaneous vbars/hbars now, with a
   mechanism to automatically use custom characters for as many
   as possible.
 + 20x2 support in LCDd!  Use "--type 20x2".
	(LCDd --type 20x2 -d MtxOrb )
 * LCDd can change backlight brightness instead of shutting it "off"
 * HD44780 should work again (LCDd forks earlier now.. oops  :)
 + 20x2 support in lcdproc client for the following screens:
	C G T M X D B S U A
   Support will not be added for K and O screens.
 + BSD support (?)
 * 20x4 screens should display correctly on 40x4 displays

V0.4-pre7: new driver
 + CrystalFontz (CFontz) LCD driver added
 * Backlight control improved
 + Brightness control added

V0.4-pre6:
 + Backlight control added.  Modes so far are "on", "off", and "open".
   Client commands are "on", "off", "toggle", "blink", and "flash".
 * Main clients' screen titles now include hostname
 + New sample client: fortune.pl
 * Added changes from David Glaude:
 + IRman support
 + Big Digit support (MtxOrb driver doesn't display it correctly yet)
 + Big cloc[K] screen
 * MtxOrb serial speed is configurable with "--speed"
 * misc bugfixes
 + ... more

V0.4-pre5:
 * Fixed bug which could take server down with a dead client
 * Fixed small scroller bug
 + Added "frame" widgets..  use "-in <frame_name>" after widget_add
   to put something in a frame.
 + The disk screen is back!
 - Frame rendering isn't finished..  the renderer probably needs a
   complete rewrite.  Any volunteers?  :)

V0.4-pre4:
 + Server menus (fairly small so far)
 + Ability to shut down, reboot, change some options, etc (menu only)
 * Driver arguments no longer required
 * Curses driver doesn't start server in background any more...
 * Argument handling fixed...
 * Drivers support "--help" for command-line info

V0.4-pre3:
 + Scroller widgets
 * Proper errno messages...
 * "make install" should work now
 * Joystick driver allows key remapping
 + Screen "durations" are actually *used* now.  (oops)
 + Initial support for screen priorities (strict priority queue)
 * Misc (but annoying!) bugs fixed, which hadn't yet affected anything
 + Drivers support parameters now (MtxOrb, joy, at least)


V0.4-pre2:
 * Memory leaks fixed
 + Memory "top" screen (S)
 + Titles now autoscroll
 * Fixed small screen-visibility bug  (clients weren't always
   notified of screen cycles)
 + Heartbeat modes implemented.  Clients can remove it with
	     widget_del my_screen heartbeat
   or turn it back on with
	     widget_add my_screen heartbeat " "
   Note that the client only has control over this when the heartbeat
   is in "open" mode.  (currently, it always is)


V0.4-pre1:
 * Too much to mention...

