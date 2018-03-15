#ifndef _robot_h_
#define _robot_h_

#include "UBTech.h"
#include "FS.h"
#include "Buffer.h"

#define DEBUG Serial1

#define BUFFER_SIZE 64

Buffer cmdBuffer(BUFFER_SIZE);

#define MAX_ACTION 26
#define MAX_POSES 30 
// #define MAX_POSES_SIZE 40
#define MAX_POSES_SIZE 20
// PoseInfo (20 bytes)
// 0 : enabled
// 1~16 : 16 servo angle, 0xFF = no change
// 17 : Time in servo command
// 18~19 : waiting time in millis

// 26 * 30 * 20 = 15600
#define ACTION_TABLE_SIZE 15600

#define ENABLE_FLAG    0
#define ID_OFFSET      0
#define EXECUTE_TIME   17
#define WAIT_TIME_HIGH 18
#define WAIT_TIME_LOW  19

#define MAX_COMBO 10
#define MAX_COMBO_SIZE 100

#define READ_OK 			 0x00
#define READ_ERR_NOT_FOUND   0x01
#define READ_ERR_OPEN_FILE   0x02
#define READ_ERR_FILE_SIZE   0x03
#define READ_ERR_READ_FILE   0x04

#define WRITE_OK 			 0x00
#define WRITE_ERR_OPEN_FILE  0x01
#define WRITE_ERR_WRITE_FILE 0x02

#define MOVE_OK 				0x00
#define MOVE_ERR_PARM_CNT		0x01
#define MOVE_ERR_PARM_VALUE     0x02
#define MOVE_ERR_PARM_CONTENT   0x03
#define MOVE_ERR_PARM_END		0x04
#define MOVE_ERR_PARM_ALL_CNT	0x11
#define MOVE_ERR_PARM_ALL_ANGLE	0x12
#define MOVE_ERR_PARM_ONE_ID	0x21
#define MOVE_ERR_PARM_ONE_ANGLE	0x22
#define MOVE_ERR_PARM_DUP_ID	0x23

#define UPLOAD_OK 				0x00
#define UPLOAD_CLEAR_OK			0x00
#define UPLOAD_ERR_HEADER		0x01
#define UPLOAD_ERR_ACTION		0x02
#define UPLOAD_ERR_POSE         0x03
#define UPLOAD_ERR_POSE_DATA    0x04
#define UPLOAD_ERR_CLEAR_POSE   0x05

#define MAX_WAIT_CMD 			100

#pragma region "Global variables"

/*
byte cmdBuf[BUFFER_SIZE];
int bufReadPtr = 0;
int bufWritePtr = 0;
bool bufOverflow = false;
// if bufReadPtr = bufWritePtr : no data
// when bufWritePtr = bufReadPtr - 1; buffer full, don't write to it
*/


byte actionTable[MAX_ACTION][MAX_POSES][MAX_POSES_SIZE];
byte comboTable[MAX_COMBO][MAX_COMBO_SIZE];

char* actionDataFile = (char *) "/robot.dat";

SoftwareSerial ss(12, 12, false, 256);

UBTech servo(&ss, &DEBUG);  // Debug on Serial1

int servoCnt = 0;
byte *retBuffer;

byte ledMode = 0;

byte ch, cmd;

long lastCmdMs = 0;

bool debug = true;
bool devMode = false;

#pragma endregion

#pragma region "Local Functions"

void cmd_ReadSPIFFS();
void ReadSPIFFS(bool sendResult);

bool UBTBT_Command();
bool UBTCB_Command();
bool UBTSV_Command();

bool V1_Command();  // To be obsolete
bool V2_Command();

#pragma endregion


#pragma region "UTIL.ino"

void SetDebug(bool mode);

byte CheckSum(byte *cmd);
byte CheckVarSum(byte *cmd);
byte CheckFullSum(byte *cmd);

void clearInputBuffer();
bool cmdSkip(bool flag);

void DebugShowSkipByte();

#pragma endregion


void UBT_GetServoAngle(byte *result);
void UBT_ReadSPIFFS(byte cmdCode);
void UBT_WriteSPIFFS(byte cmdCode);
#endif