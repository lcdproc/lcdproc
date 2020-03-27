# REALLY-QUICK START

If you're in a desperate hurry type:
```
sh ./autogen.sh
./configure --enable-drivers=all
make
```

And if you start wondering why it doesn't work, come back and read the
rest of the file. 8)


# PREREQUISITES

First read the [README](README.md) if you haven't already.

Please take a few minutes to read the user guide located at
http://lcdproc.sourceforge.net/docs/

For the details on supported platforms, see the PLATFORM SPECIFIC section
of this file.

In order to compile LCDproc, you'll need the following programs:

* A C compiler which supports C99, we recommend GCC. Most Linux or BSD systems
 come with GCC.

* GNU Make. It is available for all major distributions. If you want to
 compile it yourself, see http://www.gnu.org/software/make/make.html .

* The GNU autotools, that is automake and autoconf. They are only required
 if you want to build LCdproc directory from Git.
  The GNU autotools are available for all major distributions. If you want
  to compile them yourself, see
  http://www.gnu.org/software/autoconf/ and
  http://www.gnu.org/software/automake/.

Depending on the ```./configure``` options and your choice of drivers, you will
need some additional programs or libraries installed:

* DocBook XML converter ```xmlto``` if you want to create the documentation
 or Doxygen when used with ```--enable-doxygen```, also see
  http://cyberelk.net/tim/software/xmlto/ and
  http://www.stack.nl/~dimitri/doxygen/

* ```libusb``` if you ```--enable-libusb```, see http://libusb.sourceforge.net/

* G15daemon, ```libg15``` and ```libg15render``` (>= 1.1.1) for use with the ```g15``` driver,
  see http://www.g15tools.com/

* GraphLCD and GLCDprocDriver for use with the ```glcdlib``` driver, see
  http://projects.vdr-developer.org/projects/graphlcd/ and
  http://lucianm.github.com/GLCDprocDriver/

* ```libirman``` for use with the ```irman``` driver, see
 http://www.evation.com/libirman/libirman.html for the home page and
 http://www.lirc.org/software/snapshots/ for current downloads

* LIRC for use with the ```lirc``` driver, see http://www.lirc.org/

* ```ncurses``` for use with the ```curses``` driver, see
  http://www.gnu.org/software/ncurses/

* ```svgalib``` for use with the ```svga``` driver, see http://www.svgalib.org/

* ```libftdi``` and ```libusb``` for use with the ```ula200```, ```lis```, and ```i2500vfd``` driver,
 see http://www.intra2net.com/en/developer/libftdi/ and
 http://libusb.sourceforge.net/

* ```XOSD``` for use with the ```xosd``` driver, see http://sourceforge.net/projects/libxosd/,
 as well as an x11 implementation, more precisely libX11 and ```libXext```.

# DISPLAYS

Many different displays (or better said: output devices) are supported.
Some of these devices also support input, for example with a keypad.
There are drivers for input-only devices, too.

For LCDd (the server) to use the device, it needs to load a driver. The
drivers are so called 'shared modules', that usually have an extension
of ```.so```. The drivers to be loaded should be specified in the config file
(by one or more ```Driver=``` lines), or on the command line. The command line
should only be used to override things in the config file. The drivers
should be in a directory that is indicated by the ```DriverPath=``` line in
the configfile.

Depending on what kind of LCD display that you have, there are several
sources for information. If your LCD display came with a manual, this
is a great place to start. If you don't have a manual, then you must
find out what kind of display this is, and check the related
information:

HD44780 and compatible:
  read the HD44780 section in the LCDproc User's Guide ([docs/lcdproc-user](docs/lcdproc-user))
  or alternatively see http://www.robijn.net/lcdproc/

If you still have problems you could take a look at the comments in the
driver source code in server/drivers/. You may have some good indications
there.

You may want to take a look at LCDproc's hardware page at
http://lcdproc.org/hardware.php3 for details. Several
different types of displays are now supported.


# BUILDING LCDPROC

## Preparing a Git distro
If you retrieved these files from the Git, you will first need to run:
```
sh ./autogen.sh
```

## Configuration
The simplest way of doing it is with:
```
./configure
```
But it may not do what you want, so please take a few seconds to type:
```
./configure --help
```
And read the available options, especially ```--enable-drivers```

## Compilation
Run make to build the server and all clients
```
make
```
If you only want to compile the clients, you can omit to compile the
server:
```
make clients
```

Similarly, if you only want to compile the server, you can omit to
compile the clients:
```
make server
```

Depending on your system, LCDproc will build in a few seconds to a
few minutes. It's not very big.

If you want to, you can install it (if you're root) by typing:
```
make install
```

This will install the binaries and the man pages in the directory you
specified in configure.
You may have to copy the configuration file (```LCDd.conf```) to ```/etc```
(or ```/usr/local/etc```) manually.


# PLATFORM SPECIFIC

The lcdproc crew has tried to support a number of platforms. Platforms
that are very non-standard are not and probably will never be supported.
Both, the server with its drivers and the clients, have their own
difficulties in porting. Here's are the prerequisites for both:

## Client
The client should run on any POSIX compliant system.
It has been successfully tested on:
Linux 2.2.x - 2.6.x, FreeBSD 6.x - 8.1, NetBSD 1.5 & 3.x, OpenBSD 3.0,
Darwin, Solaris and MacOS.

## Server
The server needs to talk to the LCD display. At this time, it works
on very little more than on the Intel i386 (PC Compatible)
architecture. It has been successfully tested on:
Linux 2.2.x - 2.6.x, FreeBSD 6.x - 8.1, NetBSD 1.5 & 3.x, OpenBSD 3.0,
Darwin, Solaris and MacOS.

Here are some comments specific to each platform that LCDproc has been
tested on.

## Linux
Nothing special to say, everything should go well.

You may not be able to compile all drivers on something else than i386
if your linux installation does not have ```ioperm```, ```inb``` and ```outb``` commands.
```./configure``` should leave all drivers that need those functions out
(drivers for parallel port displays), so that LCDd should compile anyway.

## NetBSD
You need to use GNU Make instead of NetBSD's make.
Remember to type gmake instead of make to compile LCDproc.
The server will probably only compile on NetBSD/i386.

If you try to run LCDd and you get this error:
```
Bus error (core dumped)
```
It is likely that you did not have the right permission to access the ports.
Try starting the program as root.

## OpenBSD
You need to use GNU Make instead of OpenBSD's make.
Remember to type gmake instead of make to compile LCDproc.
The server will probably only compile on OpenBSD/i386.

## FreeBSD
It should compile fine on i386. Must be started as root to be able to use
parallel and USB ports.

## Other platforms
If the build process fails, but you do know how to link a loadable
module by hand, you can add the appropriate flags et cetera into
acinclude.m4. Experience required. Please let us know if you got it
working on a previously unsupported system, so we can include it in a
next release.


# RUNNING LCDPROC

## Configuration file

The first thing that you need to do is to modify the configuration file
for your server. A example file (LCDd.conf) is available in the tarball.
The comments present in this file should give you indications on what
to do.

## Starting the server

If you're in the LCDproc source directory, and have just built it, run:
```
server/LCDd -c path/to/config/file
```
For security reasons, LCDd by default only accepts connections from
localhost (```127.0.0.1```), it will not accept connections from other computers on
your network / the Internet. You can change this behaviour in the
configuration file.

## Starting the client(s)

Then, you'll need some clients. LCDproc comes with a few, of which the
```lcdproc``` client is the main client:
```
clients/lcdproc/lcdproc -f  C M T L
```

This will run the LCDproc client, with the [C]pu, [M]emory,
[T]ime, and [L]oad screens.  The option ```-f``` causes it not to daemonize,
but run in the foreground.

By default, the client tries to connect to a server located on localhost
and listening to port 13666. To change this, use the -s and -p options.


# PUTTING LCDPROC IN SYSTEM STARTUP

It's nice to have LCDproc start when the computer boots, so here's how
to do it:

WARNING: Make sure you have modified the configuration file (/etc/LCDd.conf) so that LCDd enters the background mode. Otherwise LCDd will lock your system.

## Slackware
Add lines to your ```/etc/rc.d/rc.local```, such as the following:
```
echo "Starting LCDd..."
/usr/local/sbin/LCDd -c /etc/LCDd.conf
echo "Starting lcdproc..."
/usr/local/bin/lcdproc C M X &
```

## Debian (the hard way)
  - Copy (as root) the debian init script from the ```scripts/``` directory of the sources to ```/etc/init.d```
```
cp scripts/init-LCDd.debian /etc/init.d/LCDd && \
cp scripts/init-lcdproc.debian /etc/init.d/lcdproc
```
  - Run (as root):
```
update-rc.d LCDd defaults
```

This will create symlinks to start and stop the daemon, resp.	from ```/etc/rc[2-5].d/S50LCDd``` to ```/etc/init.d/LCDd```, and from
	```/etc/rc[016].d/K50LCDd``` to ```/etc/init.d/LCDd```.
  - Run (as root):
```
update-rc.d lcdproc defaults
```

This will create symlinks to start and stop the lcdproc client, resp. from ```/etc/rc[2-5].d/S50lcdproc``` to ```/etc/init.d/lcdproc```, and from
	```/etc/rc[016].d/K50lcdproc``` to ```/etc/init.d/lcdproc```.

## Debian (the easy way)
  - Install the previously created .deb package (as root)
```
dpkg -i lcdproc_VERSION-RELEASE_ARCH.deb
```

## Redhat / Mandrake
  - Copy the scripts:
```
cp scripts/init-lcdd.rpm /etc/rc.d/init.d/lcdd && \
cp scripts/init-lcdproc.rpm /etc/rc.d/init.d/lcdproc
```
  - enable the scripts with e.g. ```linuxconf``` or create the symlinks manually

## FreeBSD
  - If you install from the ports collection startup script are
	installed by default.

That's all the OS's we've actually done this with so far.
