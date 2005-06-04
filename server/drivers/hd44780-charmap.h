/*
 * Character mapping for HD44780 devices by Mark Haemmerling <mail@markh.de>.
 *
 * Translates ISO 8859-1 to HD44780 charset.
 * HD44780 charset reference: http://markh.de/hd44780-charset.png
 *
 * Initial table taken from lcd.o Linux kernel driver by
 * Nils Faerber <nilsf@users.sourceforge.net>. Thanks!
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * The following translations are being performed:
 * - map umlaut accent characters to the corresponding umlaut characters
 * - map other accent characters to the characters without accents
 * - map beta (=sharp s), micro and Yen
 *
 * Alternative mappings:
 * - #112 ("p") -> #240 (large "p"),  orig. mapped -> #112
 * - #113 ("q") -> #241 (large "q"),  orig. mapped -> #113
 *
 * HD44780 misses backslash
 *
 */

const unsigned char HD44780_charmap[] = {
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
  160,  33, 236, 237, 164,  92, 124, 167,
   34, 169, 170, 171, 172, 173, 174, 175,
  223, 177, 178, 179,  39, 249, 247, 165,
   44, 185, 186, 187, 188, 189, 190,  63,
  /* #192 */
   65,  65,  65,  65, 225,  65,  65,  67,
   69,  69,  69,  69,  73,  73,  73,  73,
   68,  78,  79,  79,  79,  79, 239, 120,
   48,  85,  85,  85, 245,  89, 240, 226,
  /* #224 */
   97,  97,  97,  97, 225,  97,  97,  99,
  101, 101, 101, 101, 105, 105, 105, 105,
  111, 110, 111, 111, 111, 111, 239, 253,
   48, 117, 117, 117, 245, 121, 240, 255
};
