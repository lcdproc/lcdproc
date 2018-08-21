## General

- [ ] Some ports have ranges like this: 0x200 - 0x400
    - Is there also ABCDEF possible? 

## server

- [ ] DriverPath
    - Where exactly are those? `server/drivers` is not available on my system. Maybe /usr/lib/x86_64-linux-gnu/lcdproc?
- [ ] Hello & Goodbye:
    - Any validation? Maybe maximum number of characters
- [ ] Frameinterval
    - Any validation? `[default: 125000 meaning 8Hz]` is all which is given
- [ ] Wait time 
    - max/min?
- [ ] Keys(Up, Enter, Escape, etc.)
    - Which values are allowed?

## menu

- [ ] MenuKey, EnterKey, etc.
    - What are all allowed Keys?

## curses

- [ ] Foreground, Background, Backlight
    - What are valid colors? Hex values also allowed?

## CwLnx

- [ ] KeyMap_A/B/C/D/E/F
    - What values are allowed?


## EyeboxOne

- [ ] LeftKey/ RightKey/ UpKey/ DownKey/ EscapeKey
    - What values are allowed?


## glcd

- [ ] ConnectionType
    - t6963, serdisp, lcdp2usb, png .... these are all?
- [ ] KeyMap_A/B/C/D
    - What values are allowed?
    - "There may be up to 16 keys numbered 'A' to 'Z'." Alphabet has 26 letters though?
- [ ] serdisp_name: "Name of the underlying serdisplib driver"
    - what are allowed values?
    - http://serdisplib.sourceforge.net/docs/index.html gives nothing concerning that
- [ ] serdisp_options
    - Is validating possible except for requiring quotes?
- [ ] x11_PixelSize
    - What kind of validation should i do there? any number? or [0-9]+\+[0-9]+ as regex?
- [ ] x11_Border
    - Any allowed range? I took 0-100 for now

## glcdlib

- [ ] Driver: "see /etc/graphlcd.conf for possible drivers"
    - I dont have such a file on my PC
    - What values could be possible?
- [ ] CharEncoding
    - Any list of all allowed encodings?
- [ ] PixelShiftX/Y
    - Any range limit?
    - No description at all

## hd44780

- [ ] ConnectionType: "See documentation for available types"
    - What are all allowed enumerations? The documentation is very confusing to me.
- [ ] Model: "Available: standard (default), extended, winstar_oled, pt6314_vfd"
    - Documentation also mentions: hd66712, ks0073, weh00xxyya... what is correct now?
- [ ] Speed
    - What are allowed values?
- [ ] BacklightCmdOn/BacklightCmdOff
    - Validation 0x and four numbers ranging between 0-9? eg. 0x9999 is max and 0x0000 is min?
- [ ] vspan
    - what kind of validation is possible here? [0-9]*,[0-9]* as regex?
- [ ] KeyMatrix_x_y
    - allowed values?
    - max x and y? eg KeyMatrix_102_193 also possible?

## IOWarrior

- [ ] ExtendedMode
    - Default value? I took "no"
- [ ] SerialNumber
    - Any validation possible? eg. max number of characters

## joy

- [ ] Map_Axis1neg, Map_Axis1pos, Map_Axis2neg, Map_Axis2pos, Map_Button1, Map_Button2
    - allowed values?

## lcdm001

- [ ] PauseKey, BackKey, ForwardKey, MainMenuKey
    - allowed values?

## linux_input
- [ ] `#key=1,Escape .. #key=28,Enter .. `
    - I have no idea what this is and how it is set
    - Please give a more detailed explanation of
        - What are possible keys, `#key` is a number or keyboard char or ..?
        - What are possible values? Is it always of the form `[number],[key]`?
        - Enter, Escape, Left etc. ... What are allowed values?

## lirc
- [ ] prog: `Must be the same as in your lircrc`
    - What is meant by that?
    - Any validation possible?

## lis
- [ ] VendorID/ ProductID
    - validation possible? eg regex?

## MtxOrb
- [ ] KeyMap_A, KeyMap_B, KeyMap_C, KeyMap_D, KeyMap_E, KeyMap_F
    - Allowed values?

## MtxOrb
- [ ] WaitAfterRefresh
    - Allowed Range?

## picolcd
- [ ] LircFlushThreshold: `legal: 1000 -`
    - Unbounded upper value?

## rawserial
- [ ] Speed
    - What values?

## serialPOS
- [ ] Custom_chars
    - What range should be allowed?

## serialVFD
- [ ] Custom-Characters
    - What range should be allowed?
    - Why once "Custom-Characters" and once "Custom_chars"?

## svga
- [ ] Mode
    - Any validation possible? Any ranges?

## ula200
- [ ] KeyMap_A, KeyMap_B, KeyMap_C, KeyMap_D, KeyMap_E, KeyMap_F
    - Allowed values?

## xosd
- [ ] Offset
    - Any range? eg max. 1000x1000
- [ ] Font
    - Any validation possible?
    