#ifndef SED1520FM_H
#define SED1520FM_H

#define b______ 0x00
#define b_____O 0x01
#define b____O_ 0x02
#define b____OO 0x03
#define b___O__ 0x04
#define b___O_O 0x05
#define b___OO_ 0x06
#define b___OOO 0x07
#define b__O___ 0x08
#define b__O__O 0x09
#define b__O_O_ 0x0a
#define b__O_OO 0x0b
#define b__OO__ 0x0c
#define b__OO_O 0x0d
#define b__OOO_ 0x0e
#define b__OOOO 0x0f
#define b_O____ 0x10
#define b_O___O 0x11
#define b_O__O_ 0x12
#define b_O__OO 0x13
#define b_O_O__ 0x14
#define b_O_O_O 0x15
#define b_O_OO_ 0x16
#define b_O_OOO 0x17
#define b_OO___ 0x18
#define b_OO__O 0x19
#define b_OO_O_ 0x1a
#define b_OO_OO 0x1b
#define b_OOO__ 0x1c
#define b_OOO_O 0x1d
#define b_OOOO_ 0x1e
#define b_OOOOO 0x1f
#define b_OOOOO 0x1f
#define bOOOOOO 0x3f

char fontmap[256][8];
char *fontbignum[10][24];
char *fontbigdp[24];

#endif
