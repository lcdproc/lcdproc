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
#define CF633_Read_Eight_Bytes_of_LCD_Memory			10
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
#define CF633_Set_Baud_Rate					33
#define CF633_Set_GPIO_Pin					34
#define CF633_Read_GPIO_Pin					35

/* Key events for 533, 633 and 635 */
#define CFP_KEY_UP		1
#define CFP_KEY_DOWN		2
#define CFP_KEY_LEFT		3
#define CFP_KEY_RIGHT		4
#define CFP_KEY_ENTER		5
#define CFP_KEY_ESCAPE		6
#define CFP_KEY_UP_RELEASE	7
#define CFP_KEY_DOWN_RELEASE	8
#define CFP_KEY_LEFT_RELEASE	9
#define CFP_KEY_RIGHT_RELEASE	10
#define CFP_KEY_ENTER_RELEASE	11
#define CFP_KEY_ESCAPE_RELEASE	12
/* Key events for 631 */
#define CFP_KEY_UL_PRESS	13
#define CFP_KEY_UR_PRESS	14
#define CFP_KEY_LL_PRESS	15
#define CFP_KEY_LR_PRESS 	16
#define CFP_KEY_UL_RELEASE	17
#define CFP_KEY_UR_RELEASE	18
#define CFP_KEY_LL_RELEASE	19
#define CFP_KEY_LR_RELEASE	20


typedef unsigned char ubyte;
typedef signed char sbyte;
typedef unsigned short word;
typedef unsigned long dword;


/* KeyRing management */
#define KEYRINGSIZE	16

typedef struct {
	unsigned char contents[KEYRINGSIZE];
	int head;
	int tail;
} KeyRing;


/* receive buffer management */
#define RECEIVEBUFFERSIZE	512

typedef struct _reveivebuffer {
	unsigned char contents[RECEIVEBUFFERSIZE];
	int head;
	int tail;
	int peek;
} ReceiveBuffer;


/* command management */
#define MAX_DATA_LENGTH	22	/* CF635 spec says 0..22 */
#define MAX_COMMAND	35	/* CF635 spec says 0..35 */

typedef struct {
	ubyte command;
	ubyte data_length;
	ubyte data[MAX_DATA_LENGTH+1];
	word crc;
} COMMAND_PACKET;


void          EmptyKeyRing(KeyRing *kr);
int           AddKeyToKeyRing(KeyRing *kr, unsigned char key);
unsigned char GetKeyFromKeyRing(KeyRing *kr);

void          send_bytes_message(int fd, unsigned char msg, int len, unsigned char *data);
void          send_onebyte_message(int fd, unsigned char msg, unsigned char value);
void          send_zerobyte_message(int fd, unsigned char msg);

void          EmptyReceiveBuffer(ReceiveBuffer *rb);
void          SyncReceiveBuffer(ReceiveBuffer *rb, int fd, unsigned int number);
int           BytesAvail(ReceiveBuffer *rb);
unsigned char GetByte(ReceiveBuffer *rb);
int           PeekBytesAvail(ReceiveBuffer *rb);
void          SyncPeekPointer(ReceiveBuffer *rb);
void          AcceptPeekedData(ReceiveBuffer *rb);
unsigned char PeekByte(ReceiveBuffer *rb);


/* global variables */
extern KeyRing keyring;
extern ReceiveBuffer receivebuffer;


#endif /* CFONTZ633IO_H */
