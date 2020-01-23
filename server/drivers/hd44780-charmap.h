/** \file server/drivers/hd44780-charmap.h
 * Character mapping for HD44780 devices. Translates ISO 8859-1 to known
 * HD44780 (and compatible) ROM character sets.
 *
 * \note  All supported display controllers have their custom characters
 *        available as codes 0x00 (0) to 0x0F (15). Theses character codes
 *        MUST NOT be mapped.
 * \note  By convention drivers (hd44780, IOWarrior, ula200) use codes
 *        0x1A (26) and 0x1B (27) to access left and right arrow characters.
 *        Map these to the corresponding characters of the controller CGROM.
 */

/*-
 * Copyright (C) 2001 Mark Haemmerling <mail@markh.de>
 *               2006 Pillon Matteo <matteo.pillon@email.it>
 *               2007 Frank Jepsen <vdr_at_jepsennet.de>
 *               2009-2011 Markus Dolze <bsdfan@nurfuerspam.de>
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 */

#include "../elektragen.h"


/*
 * The 'none' charmap does map each character to itself, not replacing
 * anything. Most useful for testing purpose.
 */
const unsigned char none_charmap[] = {
        /* #0 */
          0,   1,   2,   3,   4,   5,   6,   7,
          8,   9,  10,  11,  12,  13,  14,  15,
         16,  17,  18,  19,  20,  21,  22,  23,
         24,  25,  26,  27,  28,  29,  30,  31,
        /* #32 */
         32,  33,  34,  35,  36,  37,  38,  39,
         40,  41,  42,  43,  44,  45,  46,  47,
         48,  49,  50,  51,  52,  53,  54,  55,
         56,  57,  58,  59,  60,  61,  62,  63,
        /* #64 */
         64,  65,  66,  67,  68,  69,  70,  71,
         72,  73,  74,  75,  76,  77,  78,  79,
         80,  81,  82,  83,  84,  85,  86,  87,
         88,  89,  90,  91,  92,  93,  94,  95,
        /* #96 */
         96,  97,  98,  99, 100, 101, 102, 103,
        104, 105, 106, 107, 108, 109, 110, 111,
        112, 113, 114, 115, 116, 117, 118, 119,
        120, 121, 122, 123, 124, 125, 126, 127,
        /* #128 */
        128, 129, 130, 131, 132, 133, 134, 135,
        136, 137, 138, 139, 140, 141, 142, 143,
        144, 145, 146, 147, 148, 149, 150, 151,
        152, 153, 154, 155, 156, 157, 158, 159,
        /* #160 */
        160, 161, 162, 163, 164, 165, 166, 167,
        168, 169, 170, 171, 172, 173, 174, 175,
        176, 177, 178, 179, 180, 181, 182, 183,
        184, 185, 186, 187, 188, 189, 190, 191,
        /* #192 */
        192, 193, 194, 195, 196, 197, 198, 199,
        200, 201, 202, 203, 204, 205, 206, 207,
        208, 209, 210, 211, 212, 213, 214, 215,
        216, 217, 218, 219, 220, 221, 222, 223,
        /* #224 */
        224, 225, 226, 227, 228, 229, 230, 231,
        232, 233, 234, 235, 236, 237, 238, 239,
        240, 241, 242, 243, 244, 245, 246, 247,
        248, 249, 250, 251, 252, 253, 254, 255
};

/*
 * HD44780 table
 *
 * Initial table taken from lcd.o Linux kernel driver by
 * Nils Faerber <nilsf@users.sourceforge.net>. Thanks!
 *
 * The following translations are being performed:
 * - map umlaut accent characters to the corresponding umlaut characters
 * - map other accent characters to the characters without accents
 * - map beta (=sharp s), micro and Yen
 * - map 'n/N with tilde' to 'n with bar above'
 *
 * HD44780 misses backslash and tilde
 *
 */
const unsigned char HD44780_charmap[] = {
        /* #0 */
          0,   1,   2,   3,   4,   5,   6,   7,
          8,   9,  10,  11,  12,  13,  14,  15,
         16,  17,  18,  19,  20,  21,  22,  23,
         24,  25, 126, 127,  28,  29,  30,  31,
        /* #32 */
         32,  33,  34,  35,  36,  37,  38,  39,
         40,  41,  42,  43,  44,  45,  46,  47,
         48,  49,  50,  51,  52,  53,  54,  55,
         56,  57,  58,  59,  60,  61,  62,  63,
        /* #64 */
         64,  65,  66,  67,  68,  69,  70,  71,
         72,  73,  74,  75,  76,  77,  78,  79,
         80,  81,  82,  83,  84,  85,  86,  87,
         88,  89,  90,  91,  47,  93,  94,  95,
        /* #96 */
         96,  97,  98,  99, 100, 101, 102, 103,
        104, 105, 106, 107, 108, 109, 110, 111,
        112, 113, 114, 115, 116, 117, 118, 119,
        120, 121, 122, 123, 124, 125, 126, 127,
        /* #128 */
        128, 129, 130, 131, 132, 133, 134, 135,
        136, 137, 138, 139, 140, 141, 142, 143,
        144, 145, 146, 147, 148, 149, 150, 151,
        152, 153, 154, 155, 156, 157, 158, 159,
        /* #160 */
        160,  33, 236, 237, 164,  92, 124, 167,
         34, 169, 170, 171, 172, 173, 174, 175,
        223, 177, 178, 179,  39, 228, 247, 165,
         44, 185, 186, 187, 188, 189, 190,  63,
        /* #192 */
         65,  65,  65,  65, 225,  65,  65,  67,
         69,  69,  69,  69,  73,  73,  73,  73,
         68, 238,  79,  79,  79,  79, 239, 120,
         48,  85,  85,  85, 245,  89, 240, 226,
        /* #224 */
         97,  97,  97,  97, 225,  97,  97,  99,
        101, 101, 101, 101, 105, 105, 105, 105,
        111, 238, 111, 111, 111, 111, 239, 253,
         48, 117, 117, 117, 245, 121, 240, 255
};

/*
 * Table for HD44780 controller with ROM mask supporting
 * the european charset (ROM code A02).
 *
 * Replacements:
 * 160 => 32, 168 => 34, 180 => 39, 184 => 44
 */
const unsigned char HD44780_euro_charmap[] = {
        /* #0 */
          0,   1,   2,   3,   4,   5,   6,   7,
          8,   9,  10,  11,  12,  13,  14,  15,
         16,  17,  18,  19,  20,  21,  22,  23,
         24,  25,  26,  27,  28,  29,  30,  31,
        /* #32 */
         32,  33,  34,  35,  36,  37,  38,  39,
         40,  41,  42,  43,  44,  45,  46,  47,
         48,  49,  50,  51,  52,  53,  54,  55,
         56,  57,  58,  59,  60,  61,  62,  63,
        /* #64 */
         64,  65,  66,  67,  68,  69,  70,  71,
         72,  73,  74,  75,  76,  77,  78,  79,
         80,  81,  82,  83,  84,  85,  86,  87,
         88,  89,  90,  91,  92,  93,  94,  95,
        /* #96 */
         96,  97,  98,  99, 100, 101, 102, 103,
        104, 105, 106, 107, 108, 109, 110, 111,
        112, 113, 114, 115, 116, 117, 118, 119,
        120, 121, 122, 123, 124, 125, 126, 127,
        /* #128 */
        128, 129, 130, 131, 132, 133, 134, 135,
        136, 137, 138, 139, 140, 141, 142, 143,
        144, 145, 146, 147, 148, 149, 150, 151,
        152, 153, 154, 155, 156, 157, 158, 159,
        /* #160 */
         32, 161, 162, 163, 164, 165, 166, 167,
         34, 169, 170, 171, 172, 173, 174, 175,
        176, 177, 178, 179,  39, 181, 182, 183,
         44, 185, 186, 187, 188, 189, 190, 191,
        /* #192 */
        192, 193, 194, 195, 196, 197, 198, 199,
        200, 201, 202, 203, 204, 205, 206, 207,
        208, 209, 210, 211, 212, 213, 214, 215,
        216, 217, 218, 219, 220, 221, 222, 223,
        /* #224 */
        224, 225, 226, 227, 228, 229, 230, 231,
        232, 233, 234, 235, 236, 237, 238, 239,
        240, 241, 242, 243, 244, 245, 246, 247,
        248, 249, 250, 251, 252, 253, 254, 255
};

/*
 * Electronic Assembly's KS0073 based LCDs table
 * http://www.lcd-module.de/eng/pdf/doma/dip204-4e.pdf
 *
 * This map is more similar to a ISO-8859-15, but with fractions from
 * ISO-8859-1 (0xBC, 0xBD).
 *
 * ~ (126) is mapped to right arrow, even though it exists, for
 * compatibility with some clients. 127 is left arrow.
 *
 * There's no backtick, substituded with '.
 *
 * | is substituted with another similar charachter as the code is
 * used by some serial drivers.
 *
 * Charset of the display offers a nice set of icons, they are mapped
 * from 128 to 159. I mapped these intervals to, in order: 16-28,
 * 140-151, 180-182, 187, 207, 222, 224.
 *
 * (C) 2006 Pillon Matteo <matteo.pillon@email.it>
 *
 */
const unsigned char EA_KS0073_charmap[] = {
        /* #0 */
          0,   1,   2,   3,   4,   5,   6,   7,
          8,   9,  10,  11,  12,  13,  14,  15,
         16,  17,  18,  19,  20,  21,  22,  23,
         24,  25, 223, 225,  28,  29,  30,  31,
        /* #32 */
         32,  33,  34,  35,  36,  37,  38,  39,
         40,  41,  42,  43,  44,  45,  46,  47,
         48,  49,  50,  51,  52,  53,  54,  55,
         56,  57,  58,  59,  60,  61,  62,  63,
        /* #64 */
         64,  65,  66,  67,  68,  69,  70,  71,
         72,  73,  74,  75,  76,  77,  78,  79,
         80,  81,  82,  83,  84,  85,  86,  87,
         88,  89,  90, 250, 251, 252,  29, 196,
        /* #96 */
         39,  97,  98,  99, 100, 101, 102, 103,
        104, 105, 106, 107, 108, 109, 110, 111,
        112, 113, 114, 115, 116, 117, 118, 119,
        120, 121, 122, 253, 218, 255, 206, 127,
        /* #128 */
         16,  17,  18,  19,  20,  21,  22,  23,
         24,  25,  26,  27,  28, 140, 141, 142,
        143, 144, 145, 146, 147, 148, 149, 150,
        151, 180, 181, 182, 187, 207, 222, 224,
        /* #160 */
        160,  64, 177, 161,  36, 163, 243,  95,
        248,  67, 170,  20, 172, 173,  82, 175,
        128, 140, 130, 131, 249, 143, 182, 221,
        244, 129, 128,  21, 139, 138, 190,  96,
        /* #192 */
        174, 226, 174, 174,  91, 174, 188, 169,
        197, 191, 198,  69,  73,  73,  73,  73,
         68,  93, 168, 228, 236,  79,  92, 120,
        171, 238, 229, 238,  94, 230, 178, 190,
        /* #224 */
        127, 231, 175, 175, 123, 175, 189, 200,
        164, 165, 199, 101, 167, 232, 105, 105,
        111, 125, 168, 233, 237, 111, 124,  58,
        172, 166, 234, 239, 126, 235, 178, 255
};

/*
 * Character mapping by Frank Jepsen <vdr_at_jepsennet.de>.
 */
const unsigned char SED1278F_0B_charmap[] = {
        /* #0 */
          0,   1,   2,   3,   4,   5,   6,   7,
          8,  32,  32,  32,  32,  32,  14,  15,
         16,  17,  18,  19,  20,  21,  22,  23,
         24,  25, 199, 200,  28,  29,  30,  31,
        /* #32 */
         32,  33,  39,  35,  36,  37,  38,  39,
         40,  41,  42,  43,  44,  45,  46,  47,
         48,  49,  50,  51,  52,  53,  54,  55,
         56,  57,  58,  59,  60,  61,  62,  63,
        /* #64 */
         64,  65,  66,  67,  68,  69,  70,  71,
         72,  73,  74,  75,  76,  77,  78,  79,
         80,  81,  82,  83,  84,  85,  86,  87,
         88,  89,  90,  91,  92,  93,  94,  95,
        /* #96 */
         96,  97,  98,  99, 100, 101, 102, 103,
        104, 105, 106, 107, 108, 109, 110, 111,
        112, 113, 114, 115, 116, 117, 118, 119,
        120, 121, 122, 123, 124, 125, 126, 127,
        /* #128 */
         64,  65,  66,  67,  68,  69,  70,  71,
         72,  73,  74,  75,  76,  77,  78,  79,
         80,  81,  82,  83,  84,  85,  86,  87,
         88,  89,  90,  91,  92,  93,  94,  95,
        /* #160 */
        124, 169, 164, 166, 175, 166, 124, 210,
        177, 207, 178, 187,  44,  44, 206, 191,
        178,  16,  30,  31, 180, 234, 211, 205,
         44, 180, 178, 188, 182, 181, 245, 159,
        /* #192 */
         65,  65,  65, 170, 142, 143, 146, 128,
         69, 144,  69,  69,  73,  73,  73,  73,
        245, 156,  79,  79,  79, 172, 153, 183,
        174,  85,  85,  85, 154,  89, 220, 224,
        /* #224 */
        133, 160, 131, 171, 132, 134, 145, 135,
        138, 130, 136, 137, 141, 161, 140, 139,
         32, 155, 149, 162, 147, 173, 148, 184,
        175, 151, 163, 150, 129, 121,  32, 253
};

#ifdef EXTRA_CHARMAPS
/*
 * Table for HD44780 controller with ROM mask supporting
 * the european charset (ROM code A02).
 *
 * Russian KOI8-R codepage as input
 */
const unsigned char HD44780_KOI8R_charmap[] = {
        /* #0 */
          0,   1,   2,   3,   4,   5,   6,   7,
          8,   9,  10,  11,  12,  13,  14,  15,
         16,  17,  18,  19,  20,  21,  22,  23,
         24,  25,  26,  27,  28,  29,  30,  31,
        /* #32 */
         32,  33,  34,  35,  36,  37,  38,  39,
         40,  41,  42,  43,  44,  45,  46,  47,
         48,  49,  50,  51,  52,  53,  54,  55,
         56,  57,  58,  59,  60,  61,  62,  63,
        /* #64 */
         64,  65,  66,  67,  68,  69,  70,  71,
         72,  73,  74,  75,  76,  77,  78,  79,
         80,  81,  82,  83,  84,  85,  86,  87,
         88,  89,  90,  91,  92,  93,  94,  95,
        /* #96 */
         96,  97,  98,  99, 100, 101, 102, 103,
        104, 105, 106, 107, 108, 109, 110, 111,
        112, 113, 114, 115, 116, 117, 118, 119,
        120, 121, 122, 123, 124, 125, 126, 127,
        /* #128 */
         45, 124,  43,  43,  43,  43,  43,  43,       /* ─ │ ┌ ┐ └ ┘ ├ ┤ */
         43,  43,  43,  32,  32,  32,  32,  32,       /* ┬ ┴ ┼ ▀ ▄ █ ▌ ▐ */
        144, 145, 146, 147, 148, 149, 150, 126,       /*               ≈ */
         28,  29, 154, 155, 176, 157, 183, 247,       /* ≤ ≥     °   · ÷ */
        /* #160 */
        160, 161, 162, 203, 164, 165, 166, 167,       /*       ё         */
        168, 169, 170, 171, 172, 173, 174, 175,
        176, 177, 178, 203, 180, 181, 182, 183,       /*       Ё         */
        184, 185, 186, 187, 188, 189, 190, 169,       /*               © */
        /* #192 */
        172,  65, 128, 137, 129,  69, 216, 146,       /* ю а б ц д е ф г */
         88, 132, 133,  75, 134,  77,  72,  79,       /* х и й к л м н о */
        135, 173,  80,  67,  84, 136, 130,  66,       /* п я р с т у ж в */
         98, 142, 131, 139, 143, 140, 138, 141,       /* ь ы з ш э щ ч ъ */
        /* #224 */
        172,  65, 128, 137, 129,  69, 216, 146,       /* Ю А Б Ц Д Е Ф Г */
         88, 132, 133,  75, 134,  77,  72,  79,       /* Х И Й К Л М Н О */
        135, 173,  80,  67,  84, 136, 130,  66,       /* П Я Р С Т У Ж В */
         98, 142, 131, 139, 143, 140, 138, 141        /* Ь Ы З Ш Э Щ Ч Ъ */
};

/*
 * Table for HD44780 controller with ROM mask supporting
 * the european charset (ROM code A02).
 *
 * Russian CP-1251 codepage as input
 */
const unsigned char HD44780_CP1251_charmap[] = {
        /* #0 */
          0,   1,   2,   3,   4,   5,   6,   7,
          8,   9,  10,  11,  12,  13,  14,  15,
         16,  17,  18,  19,  20,  21,  22,  23,
         24,  25,  26,  27,  28,  29,  30,  31,
        /* #32 */
         32,  33,  34,  35,  36,  37,  38,  39,
         40,  41,  42,  43,  44,  45,  46,  47,
         48,  49,  50,  51,  52,  53,  54,  55,
         56,  57,  58,  59,  60,  61,  62,  63,
        /* #64 */
         64,  65,  66,  67,  68,  69,  70,  71,
         72,  73,  74,  75,  76,  77,  78,  79,
         80,  81,  82,  83,  84,  85,  86,  87,
         88,  89,  90,  91,  47,  93,  94,  95,
        /* #96 */
         96,  97,  98,  99, 100, 101, 102, 103,
        104, 105, 106, 107, 108, 109, 110, 111,
        112, 113, 114, 115, 116, 117, 118, 119,
        120, 121, 122, 123, 124, 125, 126, 127,
        /* #128 */
        141, 146,  44, 146,  34,  95,  43,  35,       /* Ђ Ѓ ‚ ѓ „ … † ‡ */
        158,  37, 134,  60,  72,  75,  72, 137,       /* € ‰ Љ ‹ Њ Ќ Ћ Џ */
        144, 145, 146, 147, 148, 149, 150, 151,
        152,  84, 156,  62, 156, 157, 158, 159,       /*   ™   ›         */
        /* #160 */
        160, 161, 162, 163, 164, 165, 124, 167,       /*         ¤   ¦ § */
        203, 169,  69, 171,  32,  32, 174, 207,       /* Ё © Є «     ® Ї */
        176, 177, 178, 179, 180, 181, 182, 183,       /* ° ±       µ ¶ · */
        203,  78,  69, 187, 188, 189, 190, 207,       /* ё № є »       ї */
        /* #192 */
         65, 128,  66, 146, 129,  69, 130, 131,       /* А Б В Г Д Е Ж З */
        132, 133,  75, 134,  77,  72,  79, 135,       /* И Й К Л М Н О П */
         80,  67,  84, 136, 216,  88, 137, 138,       /* Р С Т У Ф Х Ц Ч */
        139, 140, 141, 142,  98, 143, 172, 173,       /* Ш Щ Ъ Ы Ь Э Ю Я */
        /* #224 */
         65, 128,  66, 146, 129,  69, 130, 131,       /* а б в г д е ж з */
        132, 133,  75, 134,  77,  72,  79, 135,       /* и й к л м н о п */
         80,  67,  84, 136, 216,  88, 137, 138,       /* р с т у ф х ц ч */
        139, 140, 141, 142,  98, 143, 172, 173        /* ш щ ъ ы ь э ю я */
};

/*
 * Table for HD44780 controller with ROM mask supporting
 * the european charset (ROM code A02).
 *
 * Russian ISO 8859-5 codepage as input
 */
const unsigned char HD44780_ISO_8859_5_charmap[] = {
        /* #0 */
          0,   1,   2,   3,   4,   5,   6,   7,
          8,   9,  10,  11,  12,  13,  14,  15,
         16,  17,  18,  19,  20,  21,  22,  23,
         24,  25,  26,  27,  28,  29,  30,  31,
        /* #32 */
         32,  33,  34,  35,  36,  37,  38,  39,
         40,  41,  42,  43,  44,  45,  46,  47,
         48,  49,  50,  51,  52,  53,  54,  55,
         56,  57,  58,  59,  60,  61,  62,  63,
        /* #64 */
         64,  65,  66,  67,  68,  69,  70,  71,
         72,  73,  74,  75,  76,  77,  78,  79,
         80,  81,  82,  83,  84,  85,  86,  87,
         88,  89,  90,  91,  47,  93,  94,  95,
        /* #96 */
         96,  97,  98,  99, 100, 101, 102, 103,
        104, 105, 106, 107, 108, 109, 110, 111,
        112, 113, 114, 115, 116, 117, 118, 119,
        120, 121, 122, 123, 124, 125, 126, 127,
        /* #128 */
        128, 129, 130, 131, 132, 133, 134, 135,
        136, 137, 138, 139, 140, 141, 142, 143,
        144, 145, 146, 147, 148, 149, 150, 151,
        152, 153, 154, 155, 156, 157, 158, 159,
        /* #160 */
        160, 203, 162, 163, 164, 165, 166, 167,       /*   Ё             */
        168, 169, 170, 171, 172, 173, 174, 175,
         65, 128,  66, 146, 129,  69, 130, 131,       /* А Б В Г Д Е Ж З */
        132, 133,  75, 134,  77,  72,  79, 135,       /* И Й К Л М Н О П */
        /* #192 */
         80,  67,  84, 136, 216,  88, 137, 138,       /* Р С Т У Ф Х Ц Ч */
        139, 140, 141, 142,  98, 143, 172, 173,       /* Ш Щ Ъ Ы Ь Э Ю Я */
         65, 128,  66, 146, 129,  69, 130, 131,       /* а б в г д е ж з */
        132, 133,  75, 134,  77,  72,  79, 135,       /* и й к л м н о п */
        /* #224 */
         80,  67,  84, 136, 216,  88, 137, 138,       /* р с т у ф х ц ч */
        139, 140, 141, 142,  98, 143, 172, 173,       /* ш щ ъ ы ь э ю я */
         78, 203, 242, 243, 244, 245, 246, 247,       /* № ё             */
        248, 249, 250, 251, 252, 167, 254, 255        /*           §     */
};

/*
 * Mapping table for NEC µ PD16314 controller. This is a VFD controller
 * that understands the same command set as HD44780 driver and comes with (at
 * least) one of two character sets in ROM:
 *
 * - ROM code 001 is the same charset as hd44780_euro.
 * - ROM code 002 is the same charset as hd44780_default (with some additional
 *   characters in the otherwise unused areas.
 *
 * The table below is for ROM code 002 and identical to hd44780_default but
 * takes advantage of some additional international characters.
 */
const unsigned char uPD16314_charmap[] = {
        /* 00h - 1Fh : Control characters */
          0,   1,   2,   3,   4,   5,   6,   7,
          8,   9,  10,  11,  12,  13,  14,  15,
         16,  17,  18,  19,  20,  21,  22,  23,
         24,  25, 126, 127,  28,  29,  30,  31,
        /* 20h - 3Fh : Numbers and punctuation */
         32,  33,  34,  35,  36,  37,  38,  39,
         40,  41,  42,  43,  44,  45,  46,  47,
         48,  49,  50,  51,  52,  53,  54,  55,
         56,  57,  58,  59,  60,  61,  62,  63,
        /* 40h - 7Fh : Upper and lower-case letters */
         64,  65,  66,  67,  68,  69,  70,  71,
         72,  73,  74,  75,  76,  77,  78,  79,
         80,  81,  82,  83,  84,  85,  86,  87,
         88,  89,  90,  91, 140,  93,  94,  95,
         96,  97,  98,  99, 100, 101, 102, 103,
        104, 105, 106, 107, 108, 109, 110, 111,
        112, 113, 114, 115, 116, 117, 118, 119,
        120, 121, 122, 123, 124, 125, 142,  32,
        /* 80h - 9Fh : Control characters */
        128, 129, 130, 131, 132, 133, 134, 135,
        136, 137, 138, 139, 140, 141, 142, 143,
        144, 145, 146, 147, 148, 149, 150, 151,
        152, 153, 154, 155, 156, 157, 158, 159,
        /* A0h - BFh : Special characters */
         32,  33, 236, 146, 164,  92, 152, 143,
         34, 169, 170, 171, 172, 173, 174, 175,
        223, 177, 178, 179,  39, 228, 247, 165,
         44, 185, 186, 187, 188, 189, 190,  63,
        /* C0h - FFh : International characters */
        129, 129,  65,  65, 128, 130, 144, 153,
         69,  69,  69,  69,  73,  73,  73,  73,
         68, 238,  79,  79,  79,  79, 134, 120,
        136,  85,  85,  85, 138,  89, 240, 226,
         97, 131,  97,  97, 225, 132, 145, 153,
        101, 101, 101, 101, 105, 105, 105, 105,
        111, 238, 111, 111, 111, 111, 135, 253,
        137, 117, 117, 117, 139, 121, 240, 255
};

/*
 * Mapping table for displays such as the WINSTAR WEH001602A
 * font bank 1 (Western Europe I)
 */
const unsigned char WEH001602A_1_charmap[] = {
	/* #0 */
          0,   1,   2,   3,   4,   5,   6,   7,
          8,   9,  10,  11,  12,  13,  14,  15,
         16,  17,  18,  19,  20,  21,  22,  23,
         24,  25, 126, 127,  28,  29,  30,  31,
	/* #32 */
	 32,  33,  34,  35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  58,  59,  60,  61,  62,  63,
	/* #64 */
	 64,  65,  66,  67,  68,  69,  70,  71,
	 72,  73,  74,  75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,  85,  86,  87,
	 88,  89,  90,  91,  92,  93,  94,  95,
	/* #96 */
	 96,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122, 123, 124, 125, 126,   0,
	/* #128 */
        128, 129, 130, 131, 132, 133, 134, 135,
        136, 137, 138, 139, 140, 141, 142, 143,
        144, 145, 146, 147, 148, 149, 150, 151,
        152, 153, 154, 155, 156, 157, 158, 159,
	/* #160 */
	 32,  33, 204, 179, 198,  32,  32, 176,
	209, 221,  32, 215,  32,  32, 220,  32,
	210, 177,  32,  32, 211, 200, 188,  32,
	 32,  32, 210, 216, 227, 226, 229, 143,
	/* #192 */
	152, 152,  65, 203, 153,  65, 175, 196,
	145, 146, 144, 147,  73,  73,  73,  73,
	194, 166, 136, 137, 135, 206,  79,  88,
	201, 129, 130, 128, 131,  89, 254, 195,
	/* #224 */
	156, 157, 155, 205, 158,  97,  32, 196,
	149, 150, 148, 151, 162, 163, 161, 164,
	111, 167, 140, 141, 139, 207, 142, 214,
	192, 133, 134, 132, 117, 121, 250, 202
};
#endif  /* EXTRA_CHARMAPS */

struct charmap {
	ElektraEnumHd44780Charmap elektraCharmap;	/**< Config value of the mapping table */
	const unsigned char *charmap;		        /**< Pointer to mapping table */
};

/** List of available character mappings. This list is sorted by relevance! */
const struct charmap available_charmaps[] = {
	{ ELEKTRA_ENUM_HD44780_CHARMAP_HD44780_DEFAULT, HD44780_charmap           },
	{ ELEKTRA_ENUM_HD44780_CHARMAP_HD44780_EURO,    HD44780_euro_charmap      },
	{ ELEKTRA_ENUM_HD44780_CHARMAP_EA_KS0073,       EA_KS0073_charmap         },
	{ ELEKTRA_ENUM_HD44780_CHARMAP_SED1278F_0B,     SED1278F_0B_charmap       },
#ifdef EXTRA_CHARMAPS
	{ ELEKTRA_ENUM_HD44780_CHARMAP_HD44780_KOI8_R,  HD44780_KOI8R_charmap     },
	{ ELEKTRA_ENUM_HD44780_CHARMAP_HD44780_CP1251,  HD44780_CP1251_charmap    },
	{ ELEKTRA_ENUM_HD44780_CHARMAP_HD44780_8859_5,  HD44780_ISO_8859_5_charmap},
	{ ELEKTRA_ENUM_HD44780_CHARMAP_UPD16314,        uPD16314_charmap          },
	{ ELEKTRA_ENUM_HD44780_CHARMAP_WEH001602A_1,    WEH001602A_1_charmap      },
#endif
};

/**
 * Gets the index of the requested charmap entry in the list of available
 * charmaps.
 * \param req_charmap  String containing the name of the requested charmap
 * \return  Index of charmap in available_charmaps array or -1 of not found.
 *
 * \note  As long as available_charmaps[] is still small a simple search is
 *        fine. Should the list become larger (>10 entries) a better search
 *        algorithm (e.g. binary search) should be used.
 */
static int
charmap_get_index(ElektraEnumHd44780Charmap req_charmap)
{
	int i;

	for (i = 0; i < (sizeof(available_charmaps)/sizeof(struct charmap)); i++) {
		if (req_charmap == available_charmaps[i].elektraCharmap) {
			return i;
		}
	}

	return -1;
}

/* EOF */
