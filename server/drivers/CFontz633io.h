#ifndef CFONTZ633IO_H
#define CFONTZ633IO_H
/* ====================================================================
 * 635 WinTest Code.
 * SERIAL.C: Windows 32 packet based example code
 * Copyright 2001, Crystalfontz America, Inc. Written by Brent A. Crosby
 * www.crystalfontz.com, brent@crystalfontz.com
 * ====================================================================
 */

#define CF633_Ping_Command					0
#define CF633_Get_Hardware_And_Firmware_Version			1
#define CF633_Write_User_Flash_Area				2
#define CF633_Read_User_Flash_Area				3
#define CF633_Store_Current_State_As_Boot_State			4
#define CF633_Reboot						5
#define CF633_Clear_LCD_Screen					6
#define CF633_Set_LCD_Contents_Line_One				7
#define CF633_Set_LCD_Contents_Line_Two				8
#define CF633_Set_LCD_Special_Character_Data			9
#define CF633_Read_Height_Bytes_of_LCD_Memory			10
#define CF633_Set_LCD_Cursor_Position				11
#define CF633_Set_LCD_Cursor_Style				12
#define CF633_Set_LCD_Contrast					13
#define CF633_Set_LCD_And_Keypad_Backlight			14
#define CF633_Query_Last_Fan_Pulse_Count			15
#define CF633_Set_Up_Fan_Reporting				16
#define CF633_Set_Fan_Power					17
#define CF633_Read_DOW_Device_Information			18
#define CF633_Set_Up_Temperature_Reporting			19
#define CF633_Arbitrary_DOW_Transaction				20
#define CF633_Set_Up_Live_Fan_or_Temperature_Display		21
#define CF633_Send_Command_Directly_to_the_LCD_Controller	22
#define CF633_Configure_Key_Reporting				23
#define CF633_Read_Keypad_Polled_Mode				24
#define CF633_Set_Fan_Power_Failsafe				25
#define CF633_Set_Fan_Tachometer_Glitch_Filter			26
#define CF633_Query_Fan_Power_and_Failsafe_Mask			27
#define CF633_Set_ATX_Power_Switch_Functionality		28
#define CF633_Enable_Disable_and_Reset_the_Watchdog		29
#define CF633_Read_Reporting_and_Status				30
#define CF633_Send_Data_to_LCD					31
#define CF633_Key_Legends					32

/* Those are all the possible keys we know off */
#define KEY_UP_PRESS		1
#define KEY_DOWN_PRESS		2
#define KEY_LEFT_PRESS		3
#define KEY_RIGHT_PRESS		4
#define KEY_ENTER_PRESS		5
#define KEY_EXIT_PRESS		6
#define KEY_UP_RELEASE		7
#define KEY_DOWN_RELEASE	8
#define KEY_LEFT_RELEASE	9
#define KEY_RIGHT_RELEASE	10
#define KEY_ENTER_RELEASE	11
#define KEY_EXIT_RELEASE	12


typedef unsigned char ubyte;
typedef signed char sbyte;
typedef unsigned short word;
typedef unsigned long dword;
typedef union {
    unsigned char as_bytes[2];
    word as_word;
} WORD_UNION;

/* KeyRing management */
void EmptyKeyRing(void);
int AddKeyToKeyRing(unsigned char key);
unsigned char GetKeyFromKeyRing(void);


void send_bytes_message(int fd, int len, int msg, char *framebuf);
void send_onebyte_message(int fd, int msg, int value);
void send_zerobyte_message(int fd, int msg);

void           EmptyReceiveBuffer(void);
int            Serial_Init(int port, int baud_rate);
void           Uninit_Serial();
void           SendByte(int fd, unsigned char datum);
void           Sync_Read_Buffer(int fd, unsigned char expected_bytes);
int	       BytesAvail(void);
unsigned char  GetByte(void);
int            PeekBytesAvail(void);
void           Sync_Peek_Pointer(void);
void           AcceptPeekedData(void);
unsigned char  PeekByte(void);
void           Clear_Buffer(void);
void           Buffer_Character(int fd, unsigned char datum);
void           Buffer_String(int fd, char *input);
void           Send_Buffer(int fd);

int            test_packet(int fd);

#define MAX_DATA_LENGTH 16
#define MAX_COMMAND 32

typedef struct {
    ubyte command;
    ubyte data_length;
    ubyte data[MAX_DATA_LENGTH];
    WORD_UNION CRC;
} COMMAND_PACKET;
int get_crc(char * bufptr, int len, int seed);
extern COMMAND_PACKET incoming_command;
extern COMMAND_PACKET outgoing_response;
unsigned char check_for_packet(int fd, unsigned char expected_length);
void print_packet(COMMAND_PACKET *packet);
void send_packet(int fd);

#endif
