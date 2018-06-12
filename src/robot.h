#ifndef _robot_h_
#define _robot_h_

#include "ESP8266WiFi.h"
#include "WiFiClient.h"
#include "WiFiManager.h"

#include <Wire.h>
#include "OLED12864.h"

#include "UBTech.h"
#include "FS.h"
#include "Buffer.h"
#include "ComboData.h"
#include "ActionData.h"
#include "MP3TF16P.h"
#include "RobotConfig.h"

#include "message.h"
#include "RESULT.h"


WiFiManager wifiManager;

#define DEBUG Serial1

#define CMD_BUFFER_SIZE 64

Buffer cmdBuffer(CMD_BUFFER_SIZE);

RobotConfig config(&DEBUG);


//OTA Setting
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
const char* ssid = "wuhulanren";          
const char* password = "wuhulanren";
#define EN_OTA true

//Touch Setting
#define LONG_TOUCH 255
#define NONE_MOTION 0
#define SINGLE_CLICK 1
#define DOUBLE_CLICK 2
#define TRIPLE_CLICK 3

//MPU6050 Setting
// #define EN_MPU6050 true
#define MPU_CHECK_TIMES 10
const uint8_t MPU_addr=0x68;  // I2C address of the MPU-6050
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t tmp;
int8_t actionSign;
int8_t getFaceDown , getFaceUp;
bool mpuActionBegin = false;;
// #define FACE_DOWN_ID 5
// #define FACE_UP_ID 6

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
bool enable_HILZD = false;

ComboData comboTable[CD_MAX_COMBO];
ActionData actionData;

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

#pragma region "Global variables"

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

#define MP3_RXD_GPIO    14
#define MP3_TXD_GPIO    16  
#define HEAD_LED_GPIO   15
#define PIN_SETUP       13

bool headLed = false;

SoftwareSerial mp3_ss(MP3_RXD_GPIO, MP3_TXD_GPIO, false, 256);
MP3TF16P mp3(&mp3_ss, &DEBUG);

uint8_t mp3_Vol = 0xff;

#pragma endregion

#pragma region "Local Functions"

byte GetPower(uint16_t v);
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
void V2_GoAction(byte actionId, bool v2, byte *cmd);

void RobotMaintenanceMode();


// HILZD
bool HILZD_Command();

// OTA.ino
void ArduinoOTASetup();

// Mpu6050
void MpuGetActionHandle();

// Touch.ino
uint8_t DetectTouchMotion();
boolean ButtonIsPressed();

// EyeLed.ino
void ReserveEyeBlink();
void ReserveEyeBreath();
void EyeBlink();
void EyeBreath();
void EyeLedHandle();

#endif