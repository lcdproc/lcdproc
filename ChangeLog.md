v0.5dev (ongoing development)
  - [added] WINSTAR WEH001602A font bank 1 charmap and font bank selector
  - [fixed] HD44780: turn off display during initialization to not show garbage
  - [added] HD44780: support almost compatible WINSTAR OLED displays
  - [added] HD44780: support internal backlight mode of modern controllers

v0.5.9
  - [removed] scripts/debian (https://github.com/lcdproc/lcdproc/issues/39)
  - [fixed] spurious (direct) dependencies of ftdi drivers on libusb
  - [API] now drivers need to include "shared/report.h" instead of "report.h"
  - [fixed] autorotate setting - the old code was likely broken on most systems
  - [fixed] As default always use nanosleep() for delays
  - [fixed] Use libftdi1 if it is available instead of obsolete libftdi
  - [fixed] Make SEAMLESS_HBARS feature selectable by the drivers individually
  - [added] hd44780-gpio now supports dual controller displays
  - [added] percent glyph to 14-segment font
  - [fixed] Make display update interval selectable from LCDd.conf
  - [fixed] Move --enable-permissive-menu-goto from configure to LCDd.conf
  - [fixed] Segfault in LL_Find() on empty list
  - [fixed] Add C99 (https://github.com/lcdproc/lcdproc/issues/81)
  - [added] Add Portwell EZIO-100 and EZIO-300 support to serial HD44780 driver
  - [fixed] Removed stale variable in hd44780-i2c.c
  - [fixed] Removed stale screen_add_key_func
  - [fixed] Removed stale screen_del_key_func 

v0.5.8
  - [added] Add driver for the Futaba TOSD-5711BB VFDisplay on Elonex Artisan/Scaleo/FIC Spectra Media Centre PCs
  - [added] Add travis file for Auto build testing of lcdproc (https://github.com/lcdproc/lcdproc/pull/2)
  - [fixed] Update hd44780-i2c driver, pins defined via config file, rather than hardcoded (https://github.com/lcdproc/lcdproc/pull/1)
  - [fixed] Portability fixes (https://github.com/lcdproc/lcdproc/pull/4)
  - [added] Add driver for lcm162-module (by H. Rasch) (https://github.com/lcdproc/lcdproc/pull/6)
  - [added] New driver linux_input (https://github.com/lcdproc/lcdproc/pull/7)
  - [fixed] Cleanup I2C handling (https://github.com/lcdproc/lcdproc/pull/8)
  - [fixed] Add more patterns to .gitignore (https://github.com/lcdproc/lcdproc/pull/9)
  - [added] New driver for Olimex MOD-LCD1x9
  - [fixed] Allow multibyte characters (from string widget)
  - [added] New driver for YARD2 devices (https://github.com/lcdproc/lcdproc/pull/19)
  - [fixed] Handle backlight mode 'open' correctly (Bug#48 at sourceforge)
  - [fixed] lcdexec: make automatic action menus work (https://sourceforge.net/p/lcdproc/bugs/47/)
  - [added] Add support for icp_a125 to icp_a106 driver (https://github.com/lcdproc/lcdproc/pull/22)
  - [added] New hd44780 connection type using libugpio
  - [fixed] glcd driver: apply initial contrast setting (https://github.com/lcdproc/lcdproc/issues/12)
  - [API] removed report() pointer from struct lcd_logical_driver
  - [fixed] imonlcd: improve error logging
  - [fixed] NoritakeVFD: Fix cursor positioning (https://github.com/lcdproc/lcdproc/pull/30)
  - [fixed] NoritakeVFD: Increase # of Custom Characters (https://github.com/lcdproc/lcdproc/pull/29)
  - [fixed] hd44780: removed outdated semaphore code (https://github.com/lcdproc/lcdproc/issues/34)
  - [fixed] hd44780-rpi: Improve board detection (https://github.com/lcdproc/lcdproc/pull/42)
  - [fixed] Don't disable libusb-1.0 just because legacy libusb isn't available
  - [fixed] hd44780-rpi.c: Allow pin 2 on raspberry pi rev. 2 baords (https://github.com/lcdproc/lcdproc/issues/43)

v0.5.7
 - [fixed] Fix using the left key to change the ring and checkbox menu items
 - [fixed] sed1520: Add an option for inverted segment mapping (idea by R. Buchert)
 - [fixed] Fix build if compiled with --enable-permissive-menu-goto (#3599609)
 - [fixed] glcd driver: Add direct support for framebuffer with paged memory layout
 - [added] hd44780: Added 'raspberrypi' connection type (P. Corner)
 - [fixed] Build system: Rename configure.in to configure.ac
 - [added] hd44780: Add support for FreeBSD's iic I2C framework
 - [added] hd44780: New connection type 'piplate' (J. Brogdon)
 - [fixed] lcdproc client: Add option to turn off title on OldTime screen
 - [added] glcd driver: Connection type 'x11' that draws to a X window (S. Meharg)
 - [added] glcd driver: 'picolcdgfx' for picoLCD 256x64 from Mini-Box.com (S. Meharg)
 - [fixed] picolcd: Improvements to IR processing (M. Jones)
 - [added] hd44780: Added 'spi' connection type (S. Dawson)
 - [fixed] hd55780/serial: Change data escape handling (G. Smith)
 - [added] hd44780: New connection type 'pifacecad' (T. Preston)
 - [added] new driver: rawserial - dump framebuffer to serial port (C. Wolf)

v0.5.6
 - [removed] Remove deprecated CFontz633 driver. Use CFontzPacket with Model=633 instead!
 - [added] new driver: glcd unified graphic display driver (M. Dolze)
 - [added] new driver: vlsys_m428 driver for Moneual MonCaso 320 (W. Hauck)
 - [fixed] hd44780/serial: Fix handling of keys for LoS-Panel (M. Kirchner)
 - [fixed] hd44780/serial: Change backlight handling (see commit message)
 - [added] glcd driver: Add FreeType 2 rendering support (B. Walle and M. Dolze)
 - [added] glcd driver: New connection type 'png', writes framebuffer as PNG image
 - [added] hd44780: Fix vBars and block icon not working correctly on the same screen
 - [fixed] Build system: Link LCDd itself with pthread library if available
 - [fixed] lcdproc client: Unbreak build on OpenBSD non-i386 (E. Barrett)
 - [fixed] lcdproc client: Fix core dumps on FreeBSD if started as non-root
 - [added] glcd driver: new connection type 'serdisplib'
 - [added] glcd driver: new connection type 'glcd2usb'
 - [fixed] lcdproc client: On FreeBSD show only unique processes in 'S' screen
 - [fixed] picolcd: Use libusb-1.0 asynchronous transfers to fix missed keys (M. Jones)
 - [fixed] sed1520: Make it work without using an external inverter
 - [fixed] lis driver: Make the last pixel row work (add lastline option parsing)
 - [fixed] hd44780/ftdi: Fix error in setting backlight and add backlight for 4bit mode
 - [added] new driver: sdeclcd for Watchguard FireBox appliances (F. Mertz)
 - [added] hd44780: Added 'usb4all' connection type (T. Mohaupt)
 - [fixed] Fix RPM init script hanging on recent OS (#3488223)
 - [fixed] lcdproc client: Fix build on OpenBSD >= 5.0
 - [fixed] hd44780/serial: Add support for adjustable backlight
 - [fixed] Build system: Fix build with automake >= 1.11.3 (#3494755)
 - [fixed] Fix clang compiler warnings about obsolete GNU designated initializers
 - [fixed] Fix crash when adding a menu item without text and options (S. Dawson)

v0.5.5
 - [added] sed1330 driver: Add support for HG25504 (L. Lagendijk)
 - [fixed] Build system: Fix xosd detection
 - [fixed] Reset ccmode on screen clear for NoritakeVFD, tyan driver
 - [fixed] All drivers: Refactor CGmode enum into lcd.h
 - [fixed] hd44780: New (possible faster) screen update algorithm
 - [added] Add an option to start LCDd with screen rotation disabled (M. Zanetti)
 - [fixed] Rewrite of the t6963 driver fixing several problems (see commit note).
 - [fixed] picolcd: Add OffBrightness (S. Crane), more icons and fix bignum
 - [fixed] lcdproc: Fix a single dot appearing on line 5 of the D screen
 - [fixed] lcdproc: Make the iface screen more readable on 2x16 displays
 - [fixed] All drivers: sync functions with prototypes
 - [fixed] Port remaining drivers to 0.5 API (glk, lcdm001, stv5730)
 - [fixed] Pyramid: Only send LED state changes, doxygen-ize
 - [fixed] sed1520: Make it work with 68-family style interface, convert to 0.5 API
 - [fixed] hd44780/imon: Add charmap for NEC uPD16314 and make it configurable
 - [fixed] Pyramid: Convert set_char to 8 byte bitmap, add support for bignum
 - [fixed] Convert set_char of drivers bayrad, lb216, lcterm, mtc_s16209x, wirz-sli
 - [fixed] CFontzPacket: Fix not using select() even if its available
 - [fixed] CFontzPacket: Use cellwidth = 5 on CFA-631 and CFA-635 too
 - [fixed] t6963: Convert font to 8 byte bitmap
 - [fixed] convert i2500vfd and mdm166a driver to use 8 byte bitmap custom character
 - [fixed] ula200: convert custom characters to 8 byte bitmap
 - [fixed] Build system: Fix trying to install svga driver twice
 - [added] imon: Add bignum support (Y. Scheglyuk)
 - [fixed] hd44780/ethlcd: fix LCDd hang up on network error (exits now) (M. Bialonczyk)
 - [added] New common 5x8 font for t6963, sed1520, mdm166a and i2500vfd
 - [fixed] t6963: Size has now to be configured in pixels instead of characters
 - [added] Build system: --enable-extra-charmaps option adds language specific charmaps
 - [fixed] imonlcd: Fix spinning of disc icon (E. Pooch)
 - [fixed] SureElec: Change port initialization (SF 3212891)
 - [fixed] curses: Fix missed keystrokes (from Debian)
 - [fixed] curses: Correct the name of the info function.
 - [fixed] Update LCDd.conf to be better understood by Config::Model (D. Dumont)
 - [fixed] CFontz: Fix display of bars

v0.5.4
 - [fixed] Update driver includes and LDFLAGS (fixed glk and bayrad binding error)
 - [fixed] Modify all drivers to use server/drivers/report.h instead of shared/report.h
 - [fixed] Make all drivers using parallel port fail if they cannot get IO permission
 - [fixed] Include <errno.h> instead of <sys/errno.h> everywhere (found by S. Klauer)
 - [fixed] configure: Fix de-selecting of drivers
 - [fixed] Build system: Add a check testing if SA_RESTART flag is available.
 - [fixed] picolcd driver: Fix RC-5 for picoLCD 20x2 (A. van Schie)
 - [fixed] Introduce shared/defines.h to collect commonly used macros
 - [added] new driver: SureElec for devices made by SURE Electronics (Laurent Latil)
 - [fixed] hd44780-4bit: Fix bug not checking for keypad rows 7-10 correctly
 - [fixed] Modify serial drivers to use cfmakeraw
 - [added] hd44780: Add a 'none' charmap which does not replace any character
 - [fixed] hd44780: Change mapping for spanish 'n with tilde' characters
 - [fixed] hd44780: Exclude pin for switchable backlight from keypad scanning
 - [fixed] server core: New network input buffering
 - [fixed] hd44780: Extended subdriver API and performance improvement for lcd2usb
 - [fixed] hd44780: Add KOI8-R character mapping (Yura Scheglyuk)
 - [fixed] shuttleVFD: Add support for devices with USB VID 0x1308 again
 - [fixed] CFontzPacket: Add CFA-533. Use HD44780 character mapping for CFA533/633.
 - [fixed] CFontzPacket: Report cell size correctly, use block character for icon.
 - [fixed] CFontz633: Deprecate this driver. Use CFontzPacket with Model=633 instead!
 - [removed] Remove native win32 support.
 - [fixed] picolcd: Use module output function to control key LEDs (Martin Jones)
 - [added] hd44780: Added 'usbtiny' connection type (Siarhei Herasimchuk)
 - [fixed] SureElec: Fix stall on startup
 - [added] new driver: MDM166A for Futaba/Targa USB VFD (Christoph Rasim)
 - [fixed] picolcd: Fix backlight and contrast handling (M. T. Jones)
 - [fixed] picolcd: Fix heartbeat icon messing up with vbar
 - [fixed] imonlcd: Fix possible lockup in output (icons) function (E. Pooch)
 - [fixed] pyramid: Fix buffer overflow in set_leds, incorrect escaping, and icons

v0.5.3
 - [added] lcdexec: notification when called program finishes
 - [added] xosd driver: offset from top-left corner
 - [added] LCDd: Hello= config option for startup message
 - [fixed] glk driver: bignum support fixed
 - [fixed] update local snprintf to the last one, which is GPL
 - [removed] server core: cleanup - remove unnecessary cruft
 - [fixed] server core: refactor rendering code
 - [added] LCDd: config options TitleSpeed= and Heartbeat=
 - [fixed] overhauled CwLnx driver (Dave Platt)
 - [fixed] overhauled picolcd driver (Nicu Pavel)
 - [added] add ServerScreen=blank option value
 - [fixed] MtxOrb driver: fix VFD/VKD support (Ethan Dicks)
 - [added] new driver lis for VLSystem L.I.S VFD (Daryl Fonseca-Holt)
 - [fixed] serialPOS driver: input support, cleanup (Eric Pooch)
 - [added] new driver shuttleVFD for USB-based Shuttle VFDs (Thien Vu)
 - [fixed] enable building lcdproc client on FreeBSD AMD64 platform (M. Dolze)
 - [fixed] Autoconf fixes for Net/FreeBSD (M. Dolze)
 - [fixed] fixes for the lcpdroc client iface screen on *BSD (M.Dolze)
 - [added] key support for hd44780/lcd2usb (M. Dolze)
 - [added] option Priority for NoritakeVFD (idea: Richard Muratti)
 - [fixed] overhauled NoritakeVFD driver (idea: Richard Muratti)
 - [fixed] allow wrapping slider menu items with <= 4 keys (M. Dolze)
 - [fixed] a bit more Doxygen-ation and documentation
 - [fixed] hd44780 driver: make ConnectionType 4bit more fool-proof (M. Dolze)
 - [fixed] picolcd driver: overhaul USB init for more portability (M. Dolze)
 - [added] curses driver: new option DrawBorder (Bruno Schwander)
 - [fixed] avoid sending duplicate "success" messages in response to menu_add_item
 - [added] lcdexec: parameter support using widgets
 - [fixed] server core: display values in menu for all input MenuEntry types
 - [fixed] hd44780 driver: reorganize ConnectionType recognition
 - [added] hd44780 driver: new ConnectionType mplay
 - [added] hd44780 driver: get/set contrast & brightness support for lcd2usb (M. Dolze)
 - [added] hd44780 driver: reporting in ConnectionType functions (Thomas Jarosch)
 - [fixed] clean up build system (T. Jarosch, with tests/fixes by M. Dolze)
 - [added] hd44780 driver: new ConnectionType ftdi (Thomas Jarosch)
 - [fixed] shuttleVFD driver: fix VendorID, support newer models (Miska Sulander)
 - [added] i2500vfd driver for a graphical Noritake VFD (Thomas Jarosch)
 - [fixed] hd44780 driver: more complete initialization (idea by Pierre Ossman)
 - [fixed] hd44780 driver: "euro" character mapping table (Markus Dolze)
 - [fixed] CwLnx driver: support for CW12832
 - [added] new driver mx5000 for Logitech MX5000 keyboards (Christian Jodar)
 - [fixed] update debian init scripts to allow multiple instances of clients
 - [added] serialVFD driver: support for 4 more display types (Stefan Herdler)
 - [fixed] IOWarrior driver: fixed 4-line display use
 - [added] IOWarrior driver: add ExtendedMode option
 - [fixed] lcdproc: unbreak compilation on OS X 1.5 and higher (Topher Fangio)
 - [fixed] server: fix interpretation of numeric priorities (Andrew M. Bishop)
 - [added] hd44780 driver: new ConnectionType ethlcd (Mariusz Bialonczyk)
 - [added] picolcd driver: support to send IR commands to LIRC (Jack Cleaver)
 - [added] picolcd driver: support for 20x4 picoLCDs (Nicu Pavel)
 - [added] hd44780 driver: new ConnectionType uss720 (Eric Pooch)
 - [fixed] new make targets to compile & install server & clients separately
 - [fixed] autoconf fixes: check for pkg-config macros
 - [added] new driver irtrans (Phant0m / Aron Parsons)
 - [added] serialVFD driver: support Siemens/Wincor Nixdorf BA63/66 (Stefan Herdler)
 - [added] new client lcdident.pl (Ethan Dicks)
 - [fixed] server: prevent stalling if time goes backwards (M. Vallevand)
 - [fixed] hd44780 driver: update the backlight setting only on change
 - [added] hd44780 driver: add lineaddress option to support ST7036 (Malte Poeggel)
 - [added] lcdproc: Retrieve per CPU usage statistics on FreeBSD (M. Dolze)
 - [fixed] Change drivers to start in background by default: bayrad, CFontz, ea65, glk
 - [fixed] imon driver: Change hbars to make last char in-/decrease vertically
 - [added] hd44780 driver: port usblcd connection type from 0.4.5 to 0.5 (M. Dolze)
 - [fixed] server: Fix interpretation of named screen priorities
 - [added] new driver imonlcd for SoundGraph iMON LCD (Dean Harding, Jonathan Kyler)
 - [fixed] MtxOrb driver: Fix backlight for old firmware
 - [fixed] Add checks for null pointer / failed malloc to menu code
 - [fixed] lcdproc, lcdexec, lcdvc clients: Use our included getopt.h
 - [fixed] documentation: Fix XML errors in user and developer guide
 - [added] documentation: Add man page for lcdvc

v.0.5.2
 - [fixed] fix switching on/off the Load screen in lcdproc client using the menu
 - [fixed] refactor adv_bignum: support height > 4, loadable chars with offset
 - [fixed] ged rid of global variables buffer & tmp in lcdproc client
 - [added] add init-script templates for LSB 3.x conforming systems
 - [fixed] fix cellwidth in iMon driver
 - [added] graphics character support in iMon driver (John Saunders)
 - [fixed] try harder to find a shell in lcdexec
 - [added] big number support in CwLnx driver via bignum library
 - [fixed] do not copy more bytes than reserved in get_fs() in lcdproc's machine_*.c
 - [fixed] update lcdproc's About screen with credits
 - [fixed] fixes to ula200 build environment and documentation (B. Walle)
 - [fixed] update g15 driver to be compatible with g15daemon > 1.2 (Anthony J. Mirabella)
 - [fixed] allow compiling with LDFLAG "--as-needed" (Robert Buchholz)
 - [fixed] optimize MtxOrb flush()
 - [fixed] make MtxOrb use Brightness and OffBrightness like CFontzPacket
 - [fixed] make Brightness & OffBrightness run-time configurable in CFontz & MtxOrb
 - [fixed] add support for Iface and ProcSize screens for OS X / Darwin (Eric Pooch)
 - [fixed] update & fix serialVFD driver (Stefan Herdler)
 - [added] Hitachi SP14Q002 support & ConnectionType setting for sed1330 (Benjamin Wiedmann)
 - [fixed] replace obsolete index() by strchr() (Guillaume LECERF)
 - [fixed] fixes for CwLinux driver (Gideon Tsang)
 - [added] new ConnectionType vdr-wakeup in hd44780 driver (originally Frank Jepsen)
 - [fixed] fixes for the hd44780-serial drivers (Matteo Pillon)
 - [added] new options for the hd44780 driver (Frank Jepsen)
 - [added] new driver for text based serial Point of Sale displays (Eric Pooch)
 - [added] new 2-line bignum mode using 6 user defined characters (Robin Gilks)
 - [fixed] fix custom characters [with NewFirmware=yes] & backlight in CFontz
 - [added] ea65 - driver for VFD in AOpen XC Cube AV EA65 barebones (Karsten Festag)
 - [added] picolcd - driver for Mini-box.com USB LCD picoLCD (Gatewood Green)
 - [added] new ConnectionType pertelian in hd44780 driver (Matteo Pillon)
 - [added] new ConnectionType lcd2usb in hd44780 driver
 - [added] make LCDd's GoodBye message configurable
 - [removed] remove compile-time option to read configuration from LDAP

v.0.5.1
 - [added] config file support in lcdproc client (Andrew Foss)
 - [fixed] install server & client config files in $(sysconfdir)
 - [added] new client lcdvc: virtual console on LCD
 - [fixed] convert adv_bignum library to bitwise set_char()
 - [fixed] adapt drivers to modified bignum library:
   - [added] CFontz
   - [added] CFontz633
   - [added] CFontzPacket
   - [added] hd44780
   - [added] IOWarrior
   - [added] MtxOrb
   - [added] serialVFD
   - [added] tyan
 - [added] add iface screen(s) to lcdproc client (Andrew Foss/Markus Dolze)
 - [fixed] update sample Perl clients
 - [fixed] fix LCDd crash on shutdown of clients using "menu_set_main" (Andrew Foss)
 - [fixed] fix sock_connect() to allow 0 as legal socket (Frederick Nacino)
 - [fixed] improve serialVFD driver (Stefan Herdler)
 - [added] add MD8800 driver for LCDs in Medion MD8800 PCs (Stefan Herdler/Martin Møller)
 - [fixed] make bigclock client screen more flexible: centered, require only 14 chars
 - [fixed] correct axis mapping logic for joystick driver (Jannis Achstetter)
 - [fixed] fixes to the CFontz driver
 - [added] new unified serial sub-driver for hd44780 (Matteo Pillon)
 - [removed] obsolete hd44780 subdrivers-removed: lcdserializer, picanlcd
 - [fixed] character mapping for special HD44780 modules (Matteo Pillon)
 - [added] g15 - driver for LCD in Logitech G15 gaming keyboards (Anthony J. Mirabella)
 - [added] EyeboxOne - driver for the LCD display on the EyeboxOne (Cedric TESSIER)
 - [fixed] more flexible parsing of config files: allow spaces around sections and keys
 - [fixed] updated & extended documentation
 - [fixed] command line options more synchronizd between server and clients
 - [fixed] CFontzPacket & CFontz633 support big & little endian systems
 - [fixed] MtxOrb driver completely rewritten to support used-defined characters
 - [fixed] CwLnx driver completely rewritten to support Cw1602 LCDs in addition
   to the CW12232 LCDs
 - [fixed] more options in lcdproc's config file lcdproc.conf:
   - [added] time and date formats can be changed using strftime formats
   - [added] Load screen's LowLoad & HighLoad thresholds
 - [fixed] revamped lcdexec's menu configuration
 - [fixed] various little fixes

v0.5.0
This version has split off from unstable-0.4.3. Includes major changes.
 - [added] LCDd now does dynamic loading of driver modules
 - [fixed] New API in use
 - [added] LCDd will use fill-in functions for drivers that don't support a certain
   function. Available for: vbar, hbar, bignum, heartbeat, icon, cursor.
 - [fixed] Server internal functions cleaned up, moved to other files etc.
 - [fixed] Menu system rewritten
 - [added] Client-supplied menus are now supported
 - [fixed] Key support improved: now keys have descriptive names, not just a letter
   anymore.
 - [added] Reloading of configuration and drivers by sending a SIGHUP. Probably
   needs improvement ;)
 - [fixed] HD44780 output register support on winamp wiring
 - [fixed] Reporting levels are now used more consistently
 - [added] lcdexec client added
 - [fixed] Display update frequency is now exactly 8Hz. Key scan frequency increased
   to 32Hz.
 - [fixed] Priority scheme changed. Priority classes are now used, that indicate what
   kind of a screen we're dealing with. More useful for interactive clients.
 - [added] better support for operating systems other than Linux: *BSD, Darwin
 - [added] menu support for lcdproc client (configure option)
 - [added] Added CFontz633 driver for CrytalFontz CFA633
 - [added] Added CFontzPacket driver fro CrystalFontz CFA-633, CFA-631, CFA-635
 - [added] Added glcdlib meta-driver for all LCDs supported by graphlcd-base
 - [added] Added icp_a106 driver for A106 alarm/LCD boards in 19" racks by ICP
 - [added] Added imon driver for iMON IR/VFD Modules from Silverstone, Uneed, ...
 - [added] Added IOWarrior driver for IOWarrior USB controllers
 - [added] Added lcterm driver for serial LCD terminal from Neumark Elektronik
 - [added] Added ms6931 driver for MSI-6931 displays in 1U rack servers by MSI
 - [added] Added mtc_s16209x driver for MTC-S16209x by Microtips Technology Inc
 - [added] Added NoritakeVFD driver for Noritake VFD Device CU20045SCPB-T28A
 - [added] Added pyramid driver for Pyramid LC-Displays
 - [added] Added serialVFD driver for NEC FIPC8367 based serial VFDs
 - [added] Added tyan driver for LCDs in Tyan Barebone GS series
 - [added] Added ula200 driver for the USB board ULA-200 from ELV
 - [added] Added xosd driver for On Screen Display on X
 - [added] Added HD44780 subdriver bwctusb for USB-to-LCD converters from BWCT
 - [added] Added HD44780 subdriver lcdserializer for an serial-LCD converter
 - [added] Added HD44780 subdriver i2c to conned LCDs using the I2C bus
 - [added] Added HD44780 subdriver lis2 for the LIS2 serial-to-USB converter

v0.4.3
 - [removed] Removed possibility of passing arguments to the drivers from the
   command line, which can be done in the configuration file now
 - [added] Added configuration file support
 - [added] Added ability of dropping root privileges to LCDd
 - [added] Added LCDM001 driver (kernelconcepts.de)
 - [added] Added Toshiba T6963 driver
 - [added] Added Seiko-Epson SED 1520 driver
 - [added] Added Seiko-Epson SED 1330 driver
 - [added] Added STV5730 driver composite TV signal character generation driver
 - [fixed] Modified the CFontz driver so that the new ROM version is
   supported
 - [added] Added ASCII emulation of BigNum to the drivers that did not
   support custom characters
 - [added] Added ability of configuring (rearranging) the keypad of MtxOrb
   displays
 - [added] Added .lircrc location to the options of the lirc driver
 - [fixed] Fixed backlight code (partly rewritten)
 - [fixed] Fixed heartbeat code (completely rewritten analogous to
   the backlight code)
 - [added] Added init scripts for debian and Redhat/Mandrake
 - [added] Added EPM list file so that you can use EPM
   (http://www.easysw.com/epm) to generate packages of LCDproc
 - [added] Added documentation in docbook format
   (LCDproc User's Guide - docs/lcdproc-user)
 - [fixed] Ported some screens of lcdproc to *BSD.
 - [fixed] Ported the parallel port functions in port.h to *BSD.

v0.4.2:
 - [removed] Removed multiple output driver support - to simplify life
 - [fixed] Removed use of sprintf() (in favor of snprintf) for security
 - [added] Added ability to use defined port and address to listen on
   (for security) - now defaults to 127.0.0.1
 - [added] Revamped curses driver: now supports color; has border; and
   can be (with recompile) placed anywhere on screen it fits
 - [added] Added --waittime server argument.  You can now specify how long
   a screen should stay on the display.  The default is 64 ticks or
   8 seconds.
 - [added] Fixed compile problems on non-Linux platforms where cfmakeraw,
   CBAUD, and ioperm don't exist.  All drivers now use the same code
   to initialize a serial port.

v0.4.1:
 - [added] Changed protocol to 0.3 because we now accept both -foo and foo
   as argument introducers.  in 0.4-pre10, these were changed
   from foo to -foo and this broke many clients.  This change
   should fix that and be backward/forward compatible.
 - [added] Added a noop function to the protocol.  This is useful for writing
   shell script clients of LCDproc.
 - [added] Changed the shared sock_send_string function to NOT send the
   ending NUL ('\0') byte with the string.  This was confusing
   in Perl clients which saw NUL's as the first character of
   each reply.  WARNING: The LCDPROC client depended on this
   behavior, your client may too.  Use the newline instead.
   See the code in clients/lcdproc/main.c (look for "switch(buf[i])")
   to see one way to handle this.  This should allow new clients
   to connect to old servers and vice-versa.
 - [added] Messed with include structure in .c files.  It should no longer
   be necessary to specify ../.. to get to shared code headers.
   Also, since we are using automake, the global config.h can be
   found with just #include "config.h" as it's location is included
   in a -I to the compilers.  As a result "make distcheck" now works.
 - [added] Added support for client_add_key and friends.  Clients MUST now
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
 - [added] Added driver for Matrix Orbital GLK12232-25 graphical display.
   Display is run in text mode since there isn't enough information in
   the widgets.  If we knew the max width/height of an hbar/vbar, we
   could use graphical vbar/hbar's.  Oh well.  BigNums' look really
   cool though.  You'll need some additional characters in the "Small Font"
   font (and the BigNumbers font) for this to work.
 - [added] Rewrote Big Number Clock function in chrono.c.  Many problems with
   it have been fixed.  It now uses number value 10 to display a colon
   in between the digits.  Don't know what this will do on a the MtxOrb
   driver.  GLK driver draws a colon.
 - [added] Cleaned up the CPU GRAPH (G) function in cpu.c and made get_load take
   a 'struct load' from the caller to save the last time values.  This
   allows two callers (CPU GRAPH and CPU LOAD) to call get_load and
   both get reaosonable values.  Without this, CPU GRAPH was always
   getting 0 because it was called immediatly after CPU LOAD called
   get_load and no clock ticks had gone by.
 - [added] Changed --enable-debug autoconf option to be processed correctly.
   Debug now turns on -g and turns off -O6
 - [added] Lots of other little fixes.

v0.4-pre10:
 - [added] Metar client added (Weather information)
 - [added] Linux infrared control (LIRC) driver added.
 - [added] "rotating slash" heartbeat mode (looks weird on CFontz displays)
   Use "screen_set s -heartbeat slash" to use this.
 - [fixed] Switched to client-server protocol V0.2, so arguments can come in
   any order, and require a "-" on the front.  Instead of
   "widget_set s w 1 2 foo", use "widget_set s w -x 1 -y 2 -text foo".
 - [fixed] Server info screen no longer enabled by default.
 - [fixed] Merged Andrew McMeikan's HD44780 driver changes
 - [removed] Removed WHATSNEW
 - [added] Added ChangeLog (okay, fine, I renamed WHATSNEW and typed a few
   entries into it :)

V0.4-pre9:
 - [fixed] small fixes for irix
 - [fixed] Added flag in LCDd to shut off server screen:
 	"-i off" or "--serverinfo off"
 - [added] Wirz SLI driver
 - [added] 16x2 support (server only)
 - [added] MtxOrb generic output support in driver
 - [fixed] Misc libc5 fixes
 - [added] support for arbitrary display sizes; anything 16-80 characters
   wide, and 2-25 characters tall.  (doesn't work if you try to use
   a display size bigger than your LCD..  :)
 - [fixed] curses driver looks slightly better (titles)
 - [fixed] sizes > 20x4 no longer crash

V0.4-pre8:
 - [fixed] LCDd gives more updated info in the connect string:
	cell size, lcd dimensions.
 - [fixed] bargraphs are now the correct length on CFontz displays, in the main
   lcdproc client.  For other clients, please parse the connect string!
 - [fixed] MtxOrb driver supports simultaneous vbars/hbars now, with a
   mechanism to automatically use custom characters for as many
   as possible.
 - [added] 20x2 support in LCDd!  Use "--type 20x2".
	(LCDd --type 20x2 -d MtxOrb )
 - [fixed] LCDd can change backlight brightness instead of shutting it "off"
 - [fixed] HD44780 should work again (LCDd forks earlier now.. oops  :)
 - [added] 20x2 support in lcdproc client for the following screens:
	C G T M X D B S U A
   Support will not be added for K and O screens.
 - [added] BSD support (?)
 - [fixed] 20x4 screens should display correctly on 40x4 displays

V0.4-pre7: new driver
 - [added] CrystalFontz (CFontz) LCD driver added
 - [fixed] Backlight control improved
 - [added] Brightness control added

V0.4-pre6:
 - [added] Backlight control added.  Modes so far are "on", "off", and "open".
   Client commands are "on", "off", "toggle", "blink", and "flash".
 - [fixed] Main clients' screen titles now include hostname
 - [added] New sample client: fortune.pl
 - [fixed] Added changes from David Glaude:
 - [added] IRman support
 - [added] Big Digit support (MtxOrb driver doesn't display it correctly yet)
 - [added] Big cloc[K] screen
 - [fixed] MtxOrb serial speed is configurable with "--speed"
 - [fixed] misc bugfixes
 - [added] ... more

V0.4-pre5:
 - [fixed] Fixed bug which could take server down with a dead client
 - [fixed] Fixed small scroller bug
 - [added] Added "frame" widgets..  use "-in <frame_name>" after widget_add
   to put something in a frame.
 - [added] The disk screen is back!
 - [removed] Frame rendering isn't finished..  the renderer probably needs a
   complete rewrite.  Any volunteers?  :)

V0.4-pre4:
 - [added] Server menus (fairly small so far)
 - [added] Ability to shut down, reboot, change some options, etc (menu only)
 - [fixed] Driver arguments no longer required
 - [fixed] Curses driver doesn't start server in background any more...
 - [fixed] Argument handling fixed...
 - [fixed] Drivers support "--help" for command-line info

V0.4-pre3:
 - [added] Scroller widgets
 - [fixed] Proper errno messages...
 - [fixed] "make install" should work now
 - [fixed] Joystick driver allows key remapping
 - [added] Screen "durations" are actually *used* now.  (oops)
 - [added] Initial support for screen priorities (strict priority queue)
 - [fixed] Misc (but annoying!) bugs fixed, which hadn't yet affected anything
 - [added] Drivers support parameters now (MtxOrb, joy, at least)

V0.4-pre2:
 - [fixed] Memory leaks fixed
 - [added] Memory "top" screen (S)
 - [added] Titles now autoscroll
 - [fix] Fixed small screen-visibility bug  (clients weren't always
   notified of screen cycles)
 - [added] Heartbeat modes implemented.  Clients can remove it with
	     widget_del my_screen heartbeat
   or turn it back on with
	     widget_add my_screen heartbeat " "
   Note that the client only has control over this when the heartbeat
   is in "open" mode.  (currently, it always is)

V0.4-pre1:
 * Too much to mention...
