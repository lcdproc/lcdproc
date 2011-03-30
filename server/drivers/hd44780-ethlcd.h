#ifndef HD_ETHLCD_H
#define HD_ETHLCD_H

/* initialise this particular driver */
int hd_init_ethlcd(Driver *drvthis);

#define ETHLCD_DRV_NAME      "ethlcd"
#define DEFAULT_ETHLCD_PORT  2425
#define ETHLCD_TIMEOUT       5

/* ethlcd protocol constants: */
#define ETHLCD_SEND_INSTR               0x01
#define ETHLCD_SEND_DATA                0x02
#define ETHLCD_GET_BUTTONS              0x03
#define ETHLCD_SET_BACKLIGHT            0x04
#define ETHLCD_SET_BEEP                 0x05
#define ETHLCD_GET_FIRMWARE_VERSION     0x06
#define ETHLCD_GET_PROTOCOL_VERSION     0x07
#define ETHLCD_GET_ENC_REVISION         0x08
#define ETHLCD_CLOSE_CONN               0x09
#define ETHLCD_UNRECOGNIZED_COMMAND     0x0A

#define ETHLCD_BACKLIGHT_ON             0x01
#define ETHLCD_BACKLIGHT_HALF           0x02
#define ETHLCD_BACKLIGHT_OFF            0x03

#endif
