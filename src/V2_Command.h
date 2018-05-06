#ifndef _V2_COMMAND_H_
#define _V2_COMMAND_H_

#include "robot.h"
#include "math.h"

bool V2_ActionPlaying = false;
byte V2_ActionCombo = 0; // combo = 0 reserved for single action play
byte V2_NextAction = 0;
byte V2_NextPose = 0;
unsigned long V2_NextPlayMs = 0;
int V2_ServoTimeRatio = 20;

#define V2_CMD_RESET			0x01
#define V2_CMD_DEBUG			0x02
#define V2_CMD_DEVMODE      	0x03

#define V2_CMD_ENABLE			0x0A

#define V2_CMD_SERVOANGLE		0x11
#define V2_CMD_ONEANGLE			0x12
#define V2_CMD_SERVOADJANGLE	0x13
#define V2_CMD_ONEADJANGLE		0x14

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

#define V2_CMD_UPD_ADHEADER		0x71
#define V2_CMD_UPD_ADPOSE		0x72
#define V2_CMD_UPD_ADNAME		0x74


#define V2_CMD_READSPIFFS   	0xF1
#define V2_CMD_WRITESPIFFS  	0xF2

bool deepDebug = true;


#endif