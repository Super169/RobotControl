#ifndef _V2_COMMAND_H_
#define _V2_COMMAND_H_

#include "robot.h"
#include "math.h"

bool V2_ActionPlaying = false;
byte V2_ActionCombo = 0; // combo = 0 reserved for single action play
byte V2_NextAction = 0;
uint16_t V2_NextPose = 0;
unsigned long V2_GlobalTimeMs = 0;
unsigned long V2_NextPlayMs = 0;
byte V2_ActionPlayCount = 0;

int V2_ServoTimeRatio = 20;
bool V2_UseGlobalTime = true;


#define V2_CMD_RESET			0x01
#define V2_CMD_DEBUG			0x02
#define V2_CMD_DEVMODE      	0x03
#define V2_CMD_GETCONFIG     	0x04
#define V2_CMD_SETCONFIG      	0x05
#define V2_CMD_DEFAULTCONFIG    0x06

#define V2_CMD_ENABLE			0x0A
#define V2_CMD_CHECK_BATTERY	0x0B
#define V2_CMD_GET_NETWORK      0x0C
#define V2_CMD_GET_WIFI_CONFIG  0x0D
#define V2_CMD_SET_WIFI_CONFIG  0x0E

#define V2_CMD_SERVOANGLE		0x11
#define V2_CMD_ONEANGLE			0x12
#define V2_CMD_SERVOADJANGLE	0x13
#define V2_CMD_ONEADJANGLE		0x14
#define V2_CMD_SETADJANGLE      0x15

#define V2_CMD_LOCKSERVO		0x21
#define V2_CMD_UNLOCKSERVO		0x22

#define V2_CMD_SERVOMOVE		0x23
#define V2_CMD_LED				0x24

#define V2_CMD_SET_HEADLED      0x31
#define V2_CMD_MP3_STOP         0x32
#define V2_CMD_MP3_PLAYFILE     0x33
#define V2_CMD_MP3_PLAYMP3      0x34
#define V2_CMD_MP3_PLAYADVERT   0x35
#define V2_CMD_MP3_SETVOLUME    0x36

#define V2_CMD_PLAYACTION		0x41
#define V2_CMD_PLAYCOMBO		0x42
#define V2_CMD_STOPPLAY			0x4F

#define V2_CMD_GET_ADLIST		0x60
#define V2_CMD_GET_ADHEADER		0x61
#define V2_CMD_GET_ADPOSE		0x62

#define V2_CMD_GET_COMBO		0x68
#define V2_CMD_UPD_COMBO		0x69


#define V2_CMD_UPD_ADHEADER		0x71
#define V2_CMD_UPD_ADPOSE		0x72
#define V2_CMD_UPD_ADNAME		0x74
#define V2_CMD_DEL_ACTION		0x75

/*
#define V2_CMD_READSPIFFS   	0xF1
#define V2_CMD_WRITESPIFFS  	0xF2
*/

bool deepDebug = true;


#endif