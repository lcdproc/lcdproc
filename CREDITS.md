# Introduction

LCDproc wouldn't have been possible without the efforts and work of many
dedicated enthusiasts and dabblers who have contributed to its development
over the years. We acknowledge their contributions here.

Please help us keeping this file up to date. If you're submitting a pull
request for the project, be sure to update this file to include yourself
if you're not already listed. Include a brief bio if you'd like, and be sure
to highlight the contributions you've made to the project. It's your
spotlight!

If you're absent, misattributed or your name and/or contact information
is inaccurate, please let us know (or submit a pull request to fix it)!

If you'd like to be removed from this file for some reason, please let us
know so we can take care of it. You're also welcome to list only your
name (or profile name) with no email address or other contact information
as well. We love putting our contributors' names up in lights, but we're
flexible :)

# Contributors

(listed in order of appearance)

- [William Ferrell (willfe)](mailto:willfe@gmail.com)

  - Project founder, author of first versions and much documentation, design
	for V0.4, "benevolent dictator for life" (to shamelessly borrow a page
	from Linus Torvald's playbook).

  > It's willfe's fault that LCDproc exists. He wanted his computer to have a
  heartbeat, like some HP workstations and servers do. So he found a nice LCD,
  wrote some amazingly spiffy software for it, and life was good.

- [Selene Scriven (ToyKeeper)](mailto:lcdproc@toykeeper.net)

	- Wrote V0.3 and most of V0.4, and helped procrastinate the rest
	of the time...

  > ToyKeeper was the main programmer for LCDproc 0.2 - 0.4-pre9. She took
  willfe's idea and put it in its current form.

- Gareth Watts (Xeno42)

	- Patches, mailing list, ideas, web hosting, CVS, etc.


- [Matrix Orbital Corporation](http://www.matrixorbital.com/)

	- Free displays for many of the developers, and immeasurable help
	(especially early on).


- [Crystalfontz America, Inc.](http://www.crystalfontz.com/)

	- Sends us sample/test units to get LCDproc running on all their models
	- Rescued the lcdproc.org/com/net domains when their registrations
	  expired and they were "ransomed" by a domain squatter


- [Lorand Bruhacs](mailto:bruhacs@ip23.net)

	- Developed LCDproc v0.3.5, which added lots of nifty stuff while
	  keeping the old program model


- [Benjamin Tse](mailto:blt@Comports.com), [Matthias Prinke](mailto:m.prinke@trashcan.mcnet.de)

	- HD44780 driver and predecessors


- [Richard Rognlie](mailto:rrognlie@gamerz.net)

	- Work on HD44780 driver
	- GIF driver (lost code)


- Tom Wheeley

	- IRMan input driver


- Bjoern Andersson

	- Autoconf/automake build system
	- Curses driver


- [Andrew McMeikan](mailto:andrewm@engineer.com)

	- HD44780 driver


- David Glaude

	- Lirc driver (not the current version)
	- IRMAN driver,
  - lcdproc big clocK screen.
  - Matrix Orbital enhancement: Big Number, general-purpose output,
	dynamic custom char allocation, i2c support (lost)
	- Crystal Fontz drivers
	- Matrix orbital GLK driver


- Todd Porter

	- Windows 95/98 and NT version


- Jason Dale Woodward

	- widget scrolling


- Ethan Dicks

	- PIC-an-LCD original driver (later merged into HD44780 driver)
	- fixes & tests to the MtxOrb driver and some clients
	- lcdident.pl client
	- updated lcdmetar.pl client to pull METARs from the new NOAA site


- Michael Reinelt

	- SED1520 (copied work of him)


- Simon Harrison

	- SVGAlib driver


- Horizon Technologies

	- WIRZ-SLI driver


- Charles Steinkuehler

	- Work on HD44780 timing code


- Harald Klein

	- Lirc driver (initial version of the current code)
	- Metar Client

- Philip Pokorny

	- Crystal Fontz driver
	- Matrix Orbital driver
	- Matrix Orbital GLK driver
	- Curses driver


- Glen Gray

- David Douthitt

	- Crystal Fontz driver
	- Matrix Orbital driver
	- Matrix Orbital GLK driver
	- Curses driver

- Eddie Sheldrake

	- Crystal Fontz driver


- Rene Wagner

	- LCDM001 driver
	- ASCII emulation of BigNum
	- CFontz v2.0 patches merging,
	- LIRC driver updates (new API, merged David's code)
	- Lots of driver updates for v0.4.3 (configfile, reporting):
		- CFontz
		- curses
		- glk
		- MtxOrb
	- EPM list file for automatic package generation,
	- current documentation in docbook format
	- v0.4.3 backlight and heartbeat implementation
	- Lots of other stuff for the v0.4.3 release


- Andre Breiler

	- Matrix Orbital driver


- Joris Robijn

	- HD44780 driver maintenance and enhancement
	- Config file stuff
	- Reporting stuff
	- Work on API v0.5 architecture and implementation, loadable drivers.
	- Menu stuff of v0.5
	- SED1330 driver


- Guillaume Filion

	- Portability bug fixes on the curses, HD44780, SED1520 and
	  STV5730 drivers.
	- Partially implemented Thomas Runge's BSD port of lcdproc.
	- Ported the parallel functions in port.h to BSD.
	- Autoconf/automake stuff.
	- Various small 0.4.3 changes.


- Chris Debenham

	- Matrix Orbital GLK driver
	- LB216 driver


- Mark Haemmerling

	- HD44780 character conversion table

- Robin Adams

	- SED1520 driver
	- STV5730 driver


- Manuel Stahl

	- T6963 driver


- Mike Patnode

	- Crystal Fontz
	- Various 0.4.3 changes


- Peter Marschall

	- new drivers: IOWarrior, xosdlib
	- bwctusb connectiontype for the HD44780 driver
	- extensions to CFontz* drivers: support CF631 & CF635
	- various fixes in preparation of 0.5
	- lcd2usb connectiontype for the HD44780 driver


- Markus Dolze

	- project maintainer
	- various fixes (especially for *BSD)
	- merged and extended network interface statistics to lcdproc client
	- new LCDd network input buffering


- Volker Boerchers

	- fixes and extensions to the menu system

- Lucian Muresan

	- glcdlib meta driver

- Matteo Pillon

	- lcdserializer connectiontype to the hd44780 driver
	- various improvements to the serial connections of the hd44780 driver


- Laurent Arnal

	- connectiontype for LIS2 displays from VLSystem
	  (http://www.vlsys.co.kr)


- Simon Funke

	- driver for Noritake VFD


- Matthias Goebl <matthias.goebl@goebl.net>

	- I2C connectiontype for the HD44780 driver


- Stefan Herdler

	- serialVFD driver
	- bignum library


- Bernhard Walle

	- driver for the ULA-200 device from ELV (http://www.elv.de)


- Andrew Foss

	- menu support for lcdproc client


- Anthony J. Mirabella

	- Logitech g15 driver


- Cedric Tessier

	- EyeBoxOne driver


- John Sanders

	- iMon driver graphics support


- Eric Pooch

	- Mac OS X / Darwin support
	- serialPOS driver
	- uss720 connection type for hd44780 driver
	- driver for SoundGraph iMON OEM LCD Modules


- Benjamin Wiedmann

	- Added functionality for HITACHI SP14Q002 gLCD (320x240)
	- Implemented dynamic wiring scheme selection for SED1330 driver
	  ("ConnectionType")


- Frank Jepsen

	- vdr-wakeup support in hd44780 driver (originally for v0.4.5)
	- new options for the hd44780 driver

- Karsten Festag

	- ea65 driver


- Gatewood Green

	- picolcd driver


- Dave Platt

	- overhauled CwLnx driver


- Nicu Pavel

	- overhauled & extended picolcd driver


- Daryl Fonseca-Holt

	- lis driver


- Thien Vu

	- shuttleVFD driver


- Thomas Jarosch

	- support reporting in hd44780's ConnectionType functions
	- build-system clean up: use pkg-config for some libs
	- HD44780 connection type for a USB connection via a FTDI FT2232D chip
	- i2500vfd driver


- Christian Jodar

	- mx5000 driver


- Mariusz Bialonczyk

	- ethlcd connection type for HD44780


- Jack Cleaver

	- LIRC support in picolcd


- Aron Parsons (Phant0m)

	-IRtrans driver


- Malte Poeggel

	- support for ST7036 controller in hd44780


- Dean Harding, Christian Leuschen, Jonathan Kyler

	- driver for SoundGraph iMON OEM LCD Modules


- Laurent Latil

	- SureElec driver for devices made by SURE Electronics


- Christoph Rasim

	- MDM166A driver for Futaba/Targa USB VFD


- bjo81

	- Driver for lcm162-module (by H. Rasch)
	  (https://github.com/lcdproc/lcdproc/pull/6)


- [Harald Geyer](http://friends.ccbib.org/harald/supporting/)

  - Driver for the linux input subsystem
  - Driver and font for Olimex MOD-LCD1x9 14 segment display
  - Various minor and not-so-minor cleanups and fixes


- M. Feser and R. Geigenberger <yard2lcdproc (.A.T.) yard2usb.de>

  - Driver for YARD2 display devices
  

- Colin Munro (colinmunro)
- Shane Spencer (hardwire)
- Jim McCracken (merlin_jim)
- Luis Llorente	(luisllo)

- [Sam Bingner](mailto:sam@bingner.com)

	- Updated icp_a106 driver to support icp_a125 used in QNAP devices
	  and to add button support

- [Fatih Aşıcı](mailto:fatih.asici@gmail.com)

  - Driver for hd44780 connection type using linux GPIO interface

- Guillaume Membré

  - Documentation updates

- [Alex Wood](mailto:thetewood@gmail.com)

  - Driver for the Futaba TOSD-5711BB VFDisplay on Elonex Artisan/Scaleo/FIC
    Spectra Media Cenre PCs amoungst others

- [Francois Mertz](mailto:fireboxled@gmail.com)

  - Parallel Port LCD driver for the Watchguard/Lanner firewall appliances (sdeclcd)

  - Supplemented HD44780 serial driver to support Portwell EZIO-100 and EZIO-300 LCDs found in Portwell, Caswell and Check Point firewall appliances

- [Michał Skalski](mailto:mskalski13@gmail.com)

  - Support for HD44780-compatible Winstar OLED displays: WEH001604A, WEH002004A and similar in hd44780 driver

  - Added to HD44780 driver support for internal backlight handling (by means of levels of brightness)
    for Winsar OLED and PTC PT6314 VFD (for the latter credits go to trex2000 GitHub user).
