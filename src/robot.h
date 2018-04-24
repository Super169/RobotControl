#ifndef _robot_h_
#define _robot_h_

#include "ESP8266WiFi.h"
#include "WiFiClient.h"
#include "WiFiManager.h"

#include "OLED12864.h"

#include "UBTech.h"
#include "FS.h"
#include "Buffer.h"
#include "ActionData.h"
#include "MP3TF16P.h"
#include "RobotConfig.h"

#include "message.h"

WiFiManager wifiManager;

#define DEBUG Serial1

#define CMD_BUFFER_SIZE 64

Buffer cmdBuffer(CMD_BUFFER_SIZE);

RobotConfig config(&DEBUG);

// OLED Settings

boolean ENABLE_OLED_BUFFER = true;
boolean ENABLE_OLED_DIRECTDRAW = false;

//  OLED_1306i2c - 0.96" OLED
//  OLED_1106i2c - 1.3" OLED
OLED12864 myOLED(OLED_1306i2c, ENABLE_OLED_BUFFER, ENABLE_OLED_DIRECTDRAW);

bool enable_V1 = true;
bool enable_V2 = true;
bool enable_UBTBT = true;
bool enable_UBTCB = true;
bool enable_UBTSV = true;

ActionData actionData;

#define MAX_SERVO		16
#define MAX_ACTION 		26
#define MAX_POSES 		30 
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

byte actionTable[MAX_ACTION][MAX_POSES][MAX_POSES_SIZE];
byte comboTable[MAX_COMBO][MAX_COMBO_SIZE];

char* actionDataFile = (char *) "/robot.dat";

SoftwareSerial ubt_ss(12, 12, false, 256);

UBTech servo(&ubt_ss, &DEBUG);  // Debug on Serial1

int servoCnt = 0;
byte *retBuffer;

byte ledMode = 0;

byte ch, cmd;

long lastCmdMs = 0;

bool debug = true;
bool devMode = false;

#define MY_PCB

#ifdef MY_PCB
    // My PCB
    #define MP3_RXD_GPIO    14
    #define MP3_TXD_GPIO    13  
    #define HEAD_LED_GPIO   15
#else
    // L's PCB
    #define MP3_RXD_GPIO    14
    #define MP3_TXD_GPIO    16  
    #define HEAD_LED_GPIO   13
#endif

bool headLed = false;

SoftwareSerial mp3_ss(MP3_RXD_GPIO, MP3_TXD_GPIO, false, 256);
MP3TF16P mp3(&mp3_ss, &DEBUG);

uint8_t mp3_Vol = 0xff;

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
void SetHeadLed(bool status);

byte CheckSum(byte *cmd);
byte CheckVarSum(byte *cmd);
byte CheckFullSum(byte *cmd);

void clearInputBuffer();
bool cmdSkip(bool flag);

void DebugShowSkipByte();

#pragma endregion

// UBT_Command.ino
void UBT_GetServoAngle(byte *result);
void UBT_GetServoAdjAngle(byte *result);

// V2_CheckAction.ino
void V2_ResetAction();

void UBT_ReadSPIFFS(byte cmdCode);
void UBT_WriteSPIFFS(byte cmdCode);

void V1_UBT_ReadSPIFFS(byte cmdCode);
void V2_CheckAction();

void RobotMaintenanceMode();

#endif