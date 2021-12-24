LCDproc Configuration
=====================

Configure LCDd
==============

As mentioned in the [introduction](#introduction) LCDd, the LCDproc
server, has its own configuration file, which is usually
`/etc/LCDd.conf`.

> **Note**
>
> If you have not installed LCDproc from the sources the configuration
> file might have a different location. You should be able to find it by
> making your system's package manager list all the files in the LCDproc
> package.

The format of the `/etc/LCDd.conf` is INI-file like.

It is divided into sections that start at declarations that look like
`[section]`; i.e. an opening square bracket, followed by the section
name, and terminated by a closing square bracket, on a line by itself.
Section names are case insensitive.

Parameters are grouped into sections and have the form `key=value`; i.e.
a key, also known as the configuration option, followed by an equality
sign and finally the value for the option. All three elements must occur
together on one line. The key, which is case insensitive, may be
surrounded by spaces, but is must be one word (i.e. a sequence of
non-space characters) not containing the equality sign. A similar rule
applies to the value: it may be surrounded by spaces, but it must be
either one word or enclosed within double quotes (`"`), which are not
considered as part of value. When quoted, the following character
sequences are evaluated as in literal C strings:

| escape sequence | character              |
|-----------------|------------------------|
| `\a`            | alert (bell) character |
| `\b`            | backspace              |
| `\f`            | formfeed               |
| `\n`            | newline                |
| `\r`            | carriage return        |
| `\t`            | horizontal tab         |
| `\v`            | vertical tab           |
| `\\`            | backslash              |

All other occurrences of `\` within quoted values will be ignored.

Comments are all line-based, and may start with '`#`' or '`;`'.
Everything including and behind the character starting the comment up to
the end of the line is ignored.

The server has a 'central' section named `[Server]`. Further each driver
has a section which defines how the driver acts. Those sections start
with `[drivername]`.

The drivers are activated by specifying them in a `Driver=` line in the
server section, like:

```
[Server]
Driver=curses
```
This tells LCDd to use the `curses` driver.

The drivers read their own options from the config file. For this
purpose they use the config sections that are named like the driver.

`LCDd.conf`: The `[Server]`
-----------------------------------

The `[Server]` section of the `LCDd.conf` contains the settings for the
LCDproc server LCDd.

```
DriverPath = DRIVERPATH
```
Tells the server where to look for the driver files. See
    [above](#which-driver) for details. If not specified DRIVERPATH
    defaults to the empty string, resulting in drivers being searched in
    the directory LCDd is started in.

```
Driver = DRIVERNAME
```
Tells the server which driver(s) to use. The first driver specified
    here that is capable of output functionality will be used as the
    *master* output driver, defining display properties and
    capabilities. All other drivers specified can only serve as input
    drivers or slave output drivers. If not specified `DRIVERNAME`
    defaults to `curses`, a driver that is supposed to work on any
    half-way decent UNIX console.

This setting can be overridden on LCDd's command line using the
    `-d DRIVER` option. When the command line option is used, only the
    one driver given there will be loaded, and all drivers specified in
    the configuration file are ignored.

```
Bind = ADDRESS
```
Tells the server to bind to the given local IP address and listen
    for incoming client connections. The default value for `ADDRESS` is
    `127.0.0.1`, which is actually the safest variant, as it allows
    connections only from the local machine and forbids connections from
    remote systems.

This setting can be overridden on LCDd's command line using the
    `-a ADDRESS` option.

```
Port = PORTNUMBER
```
Tells the server to listen to this specified port. If not specified
    `PORTNUMBER` defaults to `13666`.

This setting can be overridden on LCDd's command line using the
    `-p PORTNUMBER` option.

```
ReportLevel = LEVEL
```
Sets the reporting level. Legal values for LEVEL range from `0`
    (only critical errors) to `5` (everything including debugging
    information). If not specified it defaults to `2` (warnings and
    errors only).

This setting can be overridden on LCDd's command line using the
    `-r LEVEL` option.

```
ReportToSyslog = { *yes* | no }
```
Should we report to `syslog` (`yes`) instead of `stderr` (`no`)?
    Default value is `no`.

This setting can be overridden on LCDd's command line using the
    `-s NUMBER` option. Passing `-s 1` on the command line enables
    reporting to `syslog` while `-s 0` disables it.

> **Warning**
>
> If LCDd is started automatically by an init script using the
> `curses` driver, it will lock `/dev/tty1`! So, be careful about
> what you are doing here.

```
User = USER
```
User to run as. When started as root LCDd will drop its privileges,
    and run as `USER` instead. Defaults to `nobody`.

This setting can be overridden on LCDd's command line using the
    `-u USER` option.

```
Foreground = { true | false }
```
The server will stay in the foreground if set to true. Otherwise the
    server will fork to background and report to syslog. Defaults to
    `no`.

This setting can be overridden on LCDd's command line with the `-f`
    option that forces foreground mode.

```
Hello = HELLOMSG
```

Define the startup message printed on the screen when LCDd starts.
    This message will stay on the screen until the first client
    connects. If not given, it defaults to the built-in server screen
    that tells how many clients are connected and how many screens these
    clients are using in total. If it is given, each `Hello=` directive
    represents a line on the display.

The HELLOMSGs will be printed on the display one after each other
    starting on the beginning of each line. So, the definition of

```
Hello="   Welcome to"
Hello="    LCDproc!"
```

prints a nice 2-line welcome message to the display.

To simply disable the default built-in server screen on startup, and
    start with a blank screen a single `Hello=""` is sufficient.

```
GoodBye = GOODBYEMSG
```
Define the message left on the screen when LCDd exits. If not given,
    it defaults to the built-in `Thanks for using LCDproc!`. If it is
    given, each `GoodBye=` directive represents a line on the display.

The `GOODBYEMSG`s will be printed on the display one after each other
    starting on the beginning of each line. So, the definition of

```
GoodBye="       So Long,"
GoodBye="         and"
GoodBye="Thanks for All the Fish!"
```

prints the well known dolphin's message on the first 3 lines of the
    display (which obviously needs to be 24 columns wide to show the
    full last line).

To simply disable the default built-in message, and leave the screen
    blank a single `GoodBye=""` suffices.

```
FrameInterval = MICROSECONDS
```
Sets the interval in microseconds for updating the display. If not
    specified the default value for `MICROSECONDS` is `125000`.

```
WaitTime = SECONDS
```
Sets the default time in seconds to display a screen. If not
    specified the default value for `SECONDS` is `4`.

This setting can be overridden on LCDd's command line with the
    `-w SECONDS` option.

```
AutoRotate = { *yes* | no }
```

If set to `no`, LCDd will start with screen rotation disabled. This
    has the same effect as if the ToggleRotateKey had been pressed.
    Rotation will start if the ToggleRotateKey is pressed.

> **Note**
>
> This setting does not turn off priority sorting of screens.
> Therefore the client or LCDd may still show a different screen if
> it assigns it a higher priority than any other screen. Due to the
> way priority sorting works the screen shown when the first client
> connects may not be that clients first screen. If the client sets
> up more than two screens it will be the next to last one (this is
> not considered a bug).

```
ServerScreen = { *yes* | no | blank }
```
Control the behaviour of the server screen, that usually shows the
    number of active clients and screens. When set to its default value
    `yes`, the server screen is included into the screen rotation scheme
    when other screens exist. Whet set to `no`, the server screen only
    shows up when no other screen exists. The special value `blank` is
    similar to `no`, but instead of displaying the current number of
    clients and screens, only a blank screen is displayed.

This setting can be partially overridden on LCDd's command line
    using the `-i NUMBER` option. Passing `-i 1` on the command line
    enables server screen rotation, while `-i 0` disables it.

> **Note**
>
> Using the command line, it is not possible to set the server
> screen to `blank` mode.

```
Backlight = { off | *open* | on }
```
Set the master backlight setting. If set to the default value
    `open`, then the backlight setting of the display can be influenced
    by the clients. When set to `off` or `on`, the backlight is set to
    the appropriate value without the clients being able to change the
    value.

```
Heartbeat = { off | *open* | on }
```
Set the master heartbeat, the oscillating icon in the top right
    corner of the display, setting. If set to the default value `open`,
    then the heartbeat setting of the display can be influenced by the
    clients. When set to `off` or `on`, the heartbeat is turned on or
    off without the clients being able to change the value.

```
TitleSpeed = SPEED
```
Set the speed how fast over-long title lines shall scroll. Legal
    values are `0` to `10`, where `0` means that no scrolling takes
    place and `10` stands for fastest scrolling. Default is `10`, where
    no artificial delay is inserted.

The `HELLIPKey` lines define what the server does with keypresses that
don't go to any client.

```
ToggleRotateKey = KEY
```
Defaults to `Enter`.

```
PrevScreenKey = KEY
```
Defaults to `Left`.

```
NextScreenKey = KEY
```
Defaults to `Right`.

```
ScrollUpKey = KEY
```
Defaults to `Up`.

```
ScrollDownKey = KEY
```
Defaults to `Down`.

`LCDd.conf`: The `[Menu]`
---------------------------------

The `[Menu]` section enables you to set some general ("global") options
related to the way LCDd handles input "events".

The menu is a special LCDproc client built into LCDd that allows
changing server and display settings as well as extending it with
entries from client applications.

You can configure what keys the menu should use.
```
MenuKey = KEY
```
The key that switches into menu mode (=open the main menu). In menu
    mode it cancels any operation. Cancelling the main menu means
    returning to the regular display mode. It has no default, but a
    natural candidate is `Menu`.

> **Note**
>
> The `MenuKey` will be reserved exclusively, while the others work
> in shared mode and can thus be used by a client application when
> not in the menu.

```
EnterKey = KEY
```
The key to enter a sub menu, to select an entry and/or to confirm
    the value of an input field. If the `RightKey` is not defined, it is
    also used to move right in input fields. In this case the value of
    the input field is not confirmed, until the right end of the input
    has been reached. It is not set by default, but a natural candidate
    is `Enter`.

```
UpKey = KEY
```
The key to move to the previous item in a menu and/or to select the
    previous value in input fields (e.g. the previous character
    available for the current position). If the `DownKey` is not set,
    moving up before the first entry automatically wraps around to the
    last entry. It is not set by default, but a natural candidate is
    `Up`.

```
DownKey = KEY
```
The key to move to the next item in a menu and/or to select the next
    value in input fields (e.g. the next character available for the
    current position). If the `UpKey` is not set, moving down below the
    last entry automatically wraps around to the first entry. It has no
    default, but a natural candidate is `Down`.

```
LeftKey = KEY
```
If defined, this optional key is used to to move left in input
    fields and to select submenu entries. It is not set by default, but
    if you have more than 4 keys, a natural candidate is `Left`.

```
RightKey = KEY
```
If defined, this optional key is used to to move right in input
    fields. It is not set by default, but if you have more than 4 keys,
    a natural candidate is `Right`.

The minimal keys required for the menu work correctly are the `MenuKey`,
the `EnterKey` and one of `UpKey` or `DownKey`. With these 3 keys the
menus can be operated. Of course with only 3 keys the navigation gets a
bit awkward. So if you have 4 or more keys, you better use them.
Especially the `LeftKey` and `RightKey` make a big difference in user
experience.

`LCDd.conf`: The Driver
-------------------------------

As mentioned earlier, each driver has its own section in the
`LCDd.conf`.

Although the settings are more or less self-explanatory, they are
explained in the next chapter in the section for each driver. So, read
through the section of your driver and change everything necessary.

The LCDproc Init Scripts
========================

The LCDproc distribution contains init scripts for LSB 3.1 (Linux
Standard Base 3.1) conforming GNU/Linux distributions. In addition to
those it contains init scripts for older RedHat- and Debian-based
distributions that do not adhere to LSB 3.1. You can find all of them in
the `scripts/` directory of the LCDproc sources.

> **Note**
>
> The init scripts are generated using autoconf. So, again it is
> important that you have run `./configure` with the correct options for
> your system.

Refer to your system's manual on how to install the scripts.

init-LCDd
---------

The file `scripts/init-LCDd.*` is the init script for the LCDproc server
LCDd. It does not require modification.

init-lcdproc
------------

The file `scripts/init-lcdproc.*` is the init script for the LCDproc
"main" client lcdproc.

> **Note**
>
> You can retrieve a listing of all options of lcdproc running
> `lcdproc --help`.

init-lcdexec
------------

The file `scripts/init-lcdexec.*` is the init script for the LCDproc
lcdexec client, which can execute predefined commands via the menu
feature.

init-lcdvc
----------

The file `scripts/init-lcdvc.*` is the init script for the LCDproc lcdvc
client, a simple terminal.
