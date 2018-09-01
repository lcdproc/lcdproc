## General

- [x] Some ports have ranges like this: 0x200 - 0x400
    - Is there also ABCDEF possible? 
    - **yes**

## server

- [x] DriverPath
    - Where exactly are those? `server/drivers` is not available on my system. Maybe /usr/lib/x86_64-linux-gnu/lcdproc?
    - **Just for testing, users have to set it themselves correctly, relative path: No validation possible!**
- [ ] Hello & Goodbye:
    - Any validation? Maybe maximum number of characters
- [ ] Frameinterval
    - Any validation? `[default: 125000 meaning 8Hz]` is all which is given
- [x] Wait time 
    - max/min?
    - **No borders definable -> will take regex requiring any positive number**
- [x] Keys(Up, Enter, Escape, etc.)
    - Which values are allowed?
    - **Everything, even keys which do not exist**

## menu

- [x] MenuKey, EnterKey, etc.
    - What are all allowed Keys?
    - **Everything, even keys which do not exist**

## curses

- [x] Foreground, Background, Backlight
    - What are valid colors? Hex values also allowed?
    - **legal: red, black, green, yellow, blue, magenta, cyan, white according to curses.docbook**

## CwLnx

- [x] KeyMap_A/B/C/D/E/F
    - What values are allowed?
    - **Everything, even keys which do not exist**


## EyeboxOne

- [x] LeftKey/ RightKey/ UpKey/ DownKey/ EscapeKey
    - What values are allowed?
    - **Everything, even keys which do not exist**


## glcd

- [x] ConnectionType
    - t6963, serdisp, lcdp2usb, png .... these are all?
    - **No: t6963, png, serdisplib, glcd2usb, x11, picolcdgfx, xyz**
- [x] KeyMap_A/B/C/D
    - "There may be up to 16 keys numbered 'A' to 'Z'." Alphabet has 26 letters though? 
    - **I assume arbitrary keys, cannot limit the number of keys though**
- [ ] serdisp_name: "Name of the underlying serdisplib driver"
    - what are allowed values?
    - http://serdisplib.sourceforge.net/docs/index.html gives nothing concerning that
    - Whats a default?
- [ ] serdisp_options
    - Is validating possible except for requiring quotes?
- [x] x11_PixelSize
    - What kind of validation should i do there? any number? or [0-9]+\+[0-9]+ as regex?
    = **Yes, number+number according to docbook**
- [x] x11_Border
    - Any allowed range? I took 0-100 for now
    - **Took unbounded positive number**
    
## glcdlib

- [x] Driver: "see /etc/graphlcd.conf for possible drivers"
    - I dont have such a file on my PC
    - What values could be possible?
    - **docbook says: avrctl, framebuffer, gu140x32f, gu256x64-372, gu256x64C-3xx0, hd61830, image, ks0108, noritake800, sed1330, sed1520, serdisp, simlcd, t6963c.
    As it is variable i will take a string only and not enum**
- [ ] CharEncoding
    - Any list of all allowed encodings?
- [x] PixelShiftX/Y
    - Any range limit?
    - No description at all
    - **Took unbounded positive number**

## hd44780

- [x] ConnectionType: "See documentation for available types"
    - What are all allowed enumerations? The documentation is very confusing to me.
    - **I took all entries from [here](https://github.com/lcdproc/lcdproc/blob/master/docs/lcdproc-user/drivers/hd44780.docbook)**
- [x] Model: "Available: standard (default), extended, winstar_oled, pt6314_vfd"
    - Documentation also mentions: hd66712, ks0073, weh00xxyya... what is correct now?
    - **I took all entries from [here](https://github.com/lcdproc/lcdproc/blob/master/docs/lcdproc-user/drivers/hd44780.docbook)**
- [ ] Speed
    - What are allowed values?
- [x] BacklightCmdOn/BacklightCmdOff
    - Validation 0x and four numbers ranging between 0-9? eg. 0xFFFF is max and 0x0000 is min?
    - **"4 bytes encoded as hex in big endian-order": so 0000-FFFF**
- [x] vspan
    - what kind of validation is possible here? [0-9]*,[0-9]* as regex?
    - **I took `^[1-9][0-9]*(,[1-9][0-9]*)*$` as regex. Forms like this:
    `1,1` or `2,1,3` are allowed but not `0,3` or `1,`**
- [x] KeyMatrix_x_y
    - allowed values?
    - **Everything, even keys which do not exist**

## IOWarrior

- [ ] ExtendedMode
    - Default value? I took "no"
- [ ] SerialNumber
    - Any validation possible? eg. max number of characters

## joy

- [x] Map_Axis1neg, Map_Axis1pos, Map_Axis2neg, Map_Axis2pos, Map_Button1, Map_Button2
    - allowed values?
    - **Everything, even keys which do not exist**

## lcdm001

- [x] PauseKey, BackKey, ForwardKey, MainMenuKey
    - allowed values?
    - **Everything, even keys which do not exist**

## linux_input
- [x] `#key=1,Escape .. #key=28,Enter .. `
    - I have no idea what this is and how it is set
    - Please give a more detailed explanation of
        - What are possible keys, `#key` is a number or keyboard char or ..?
        - What are possible values? Is it always of the form `[number],[key]`?
        - Enter, Escape, Left etc. ... What are allowed values?
        - **Like elektra arrays with [number], [key] as values. Will
        need to rewrite code though to work like this**

## lirc
- [ ] prog: `Must be the same as in your lircrc`
    - What is meant by that?
    - Any validation possible?

## lis
- [x] VendorID/ ProductID
    - validation possible? eg regex?
    - **0x0000-0xFFFF**

## MtxOrb
- [x] KeyMap_A, KeyMap_B, KeyMap_C, KeyMap_D, KeyMap_E, KeyMap_F
    - Allowed values?
    - **Everything, even keys which do not exist**

## mx5000
- [x] WaitAfterRefresh
    - Allowed Range?
    - **No borders definable -> will take regex requiring any positive number**

## picolcd
- [x] LircFlushThreshold: `legal: 1000 -`
    - Unbounded upper value?
    - **No borders definable -> will take regex requiring any positive number above 1000**

## rawserial
- [ ] Speed
    - What values?

## serialPOS
- [x] Custom_chars
    - What range should be allowed?
    - **No borders definable -> will take regex requiring any positive number**

## serialVFD
- [x] Custom-Characters
    - What range should be allowed?
    - Why once "Custom-Characters" and once "Custom_chars"? **guess due to inconsistency**
    - **No borders definable -> will take regex requiring any positive number**

## svga
- [x] Mode
    - Any validation possible? Any ranges?
    - **docbook says: "can be any legal mode string for svgalib"**

## ula200
- [x] KeyMap_A, KeyMap_B, KeyMap_C, KeyMap_D, KeyMap_E, KeyMap_F
    - Allowed values?
    - **Everything, even keys which do not exist**

## xosd
- [ ] Offset
    - Any range? eg max. 1000x1000
- [ ] Font
    - Any validation possible?
    