[![Build Status](https://travis-ci.org/lcdproc/lcdproc.svg?branch=master)](https://travis-ci.org/lcdproc/lcdproc)

This is the official repository for the LCDproc project.

_Note: this project is currently being migrated from Sourceforge. Documentation
is still being updated to reflect this and we're still getting our bearings here
on GitHub, so please excuse the dust while we set up shop._

# Introduction

LCDproc is a client/server suite including drivers for all kinds of nifty LCD
devices. The server works with different display sizes and supports several
serial devices: Matrix Orbital, Crystal Fontz, Bayrad, LB216, LCDM001
(kernelconcepts.de), Wirz-SLI and PIC-an-LCD; some devices connected to the
parallel port: HD44780, STV5730, T6963, SED1520 and SED1330; and also some
displays connected via USB: CFontzPacket, CwLnx, IOWarrior, LIS2 and BWCT.

Various clients are available that display things like CPU load, system load,
memory usage, uptime, and a lot more.

LCDproc also supports key or remote control input for controlling the clients.

The client and the server use a TCP connection to communicate, so it is
possible to have a client on a box in Sweden showing its stats on a LCD device
in the United States.

Visit our [website](http://lcdproc.org/) for more information. The project's
repository, issue tracking and pull requests can be found at
https://github.com/lcdproc/lcdproc. Our mailing list can be found at
https://lists.lcdproc.org/wws/info/lcdproc

LCDproc was originally written by [William Ferrell](mailto:willfe@gmail.com)
and [Selene Scriven](mailto:lcdproc@toykeeper.net).

# Installation

Refer to [INSTALL](INSTALL.md) (included with this archive) for installation
instructions (including how to connect an LCD to your system).

# Changes

Refer to [ChangeLog.md](ChangeLog.md) file for details.

# Contributing

If you've found a bug or want to add new functionality to LCDproc, feel free to
fork the repository, make your changes and send us a pull request. They're
always welcome!

If you want to help but need some inspiration, take a look at the project's
[open issues](https://github.com/lcdproc/lcdproc/issues).

To learn more about how LCDproc works, first have a look at the docs/
directory. Some important things are documented there, but because this is
still somewhat a big hacking project, some documentation will be missing ;)

Please don't hesitate to ask us for help if you're having trouble getting LCDproc
working with your hardware or you'd like to contribute to the project but need
guidance on how something works or how things fit together. We're a friendly
bunch!

## Mailing List

We also maintain a [mailing list](https://lists.lcdproc.org/wws/info/lcdproc)
for development and user discussions. To join, follow the above link. You
will need to have JavaScript enabled in your browser.

## Code of Merit

This project adopts the Code of Merit, version 1.0, originally published at
[codeofmerit.org](https://codeofmerit.org/). Refer to [CODE_OF_MERIT.md](CODE_OF_MERIT.md) for
details, but in short, we focus on writing software and leave politics out of
it.

# Special Thanks

LCDproc wouldn't be possible without the ongoing patience and efforts by its
amazing community of developers and users. We'd like to recognize everyone's
contributions to the project, and we've tried to list them all in
[CREDITS](CREDITS.md). If we've overlooked your contributions, please let us know
so we can attribute your work immediately!

_From William Ferrell, original developer:_ This project is almost twenty years
old as I write this, and I'm so proud of what it's become over the years thanks
in no small part to the people (past and present) who not only contributed
their work and ideas to the project, but helped to manage and maintain it when
I couldn't. To see this project start as a small single-system utility program
supporting a single type of device and flourish into a client/server daemon
used around the world to drive all sorts of display devices is beyond anything
I could have ever hoped for, and I'm eternally grateful to everyone who's
contributed over the years. Thank you all for being part of this project and
for making it amazing. Here's to another twenty years! :)

# Legal Stuff

LCDproc is Copyright (C) 1998-2016 William Ferrell, Selene Scriven and many
other contributors.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

The file COPYING contains the GNU General Public License.  You should have
received a copy of the GNU General Public License along with this program; if
not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
Floor, Boston, MA 02110-1301, USA.
