#ifndef CFONTZ633IO_H
#define CFONTZ633IO_H
/* ====================================================================
 * 635 WinTest Code.
 * SERIAL.C: Windows 32 packet based example code
 * Copyright 2001, Crystalfontz America, Inc. Written by Brent A. Crosby
 * www.crystalfontz.com, brent@crystalfontz.com
 * ====================================================================
 */

#define CF633_Ping_Command 0
#define CF633_Get_Hardware_And_Firmware_Version 1
#define CF633_Write_User_Flash_Area 2
#define CF633_Read_User_Flash_Area 3
#define CF633_Store_Current_State_As_Boot_State 4
#define CF633_Reboot 5
#define CF633_Clear_LCD_Screen 6
#define CF633_Set_LCD_Contents_Line_One 7
#define CF633_Set_LCD_Contents_Line_Two 8
#define CF633_Set_LCD_Special_Character_Data 9
#define CF633_Read_Height_Bytes_of_LCD_Memory 10
#define CF633_Set_LCD_Cursor_Position 11
#define CF633_Set_LCD_Cursor_Style 12
#define CF633_Set_LCD_Contrast 13
#define CF633_Set_LCD_And_Keypad_Backlight 14
#define CF633_Query_Last_Fan_Pulse_Count 15
#define CF633_Set_Up_Fan_Reporting 16
#define CF633_Set_Fan_Power 17
#define CF633_Read_DOW_Device_Information 18
#define CF633_Set_Up_Temperature_Reporting 19
#define CF633_Arbitrary_DOW_Transaction 20
#define CF633_Set_Up_Live_Fan_or_Temperature_Display 21

typedef unsigned char ubyte;
typedef signed char sbyte;
typedef unsigned short word;
typedef unsigned long dword;
typedef union {
    ubyte as_bytes[2];
    word as_word;
} WORD_UNION;

void send_bytes_message(int fd, int len, int msg, char *framebuf);
void send_onebyte_message(int fd, int msg, int value);
void send_zerobyte_message(int fd, int msg);

void            EmptyReceiveBuffer(void);
int            Serial_Init(int port, int baud_rate);
void           Uninit_Serial();
void           SendByte(int fd, unsigned char datum);
void           Sync_Read_Buffer(ubyte expected_bytes);
int	       BytesAvail(void);
char           GetByte(void);
int            PeekBytesAvail(void);
void           Sync_Peek_Pointer(void);
void           AcceptPeekedData(void);
ubyte PeekByte(void);
void         Clear_Buffer(void);
void           Buffer_Character(int fd, unsigned char datum);
void           Buffer_String(int fd, char *input);
void           Send_Buffer(int fd);

#define MAX_DATA_LENGTH 16
#define MAX_COMMAND 21

typedef struct {
    ubyte command;
    ubyte data_length;
    ubyte data[MAX_DATA_LENGTH];
    WORD_UNION CRC;
} COMMAND_PACKET;
int get_crc(char * bufptr, int len, int seed);
extern COMMAND_PACKET incoming_command;
extern COMMAND_PACKET outgoing_response;
ubyte check_for_packet(ubyte expected_length);
void send_packet(int fd);

#endif
