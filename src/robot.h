#ifndef _robot_h_
#define _robot_h_

#include "ESP8266WiFi.h"
#include "WiFiClient.h"
#include <WiFiUDP.h>

#include <Wire.h>
#include "OLED12864.h"

#include "FS.h"
#include "Buffer.h"
#include "ComboData.h"
#include "ActionData.h"
#include "MP3TF16P.h"
#include "RobotConfig.h"

#include "message.h"
#include "RESULT.h"

#include "SimpleWiFiManager.h"
SimpleWiFiManager SWFM;

#include "RobotServo.h"
#include "V2_Command.h"

#define VERSION_MAJOR   2
#define VERSION_MINOR   1
#define VERSION_SUB     1
#define VERSION_FIX     1

// Start a TCP Server on port 6169
uint16_t port = 6169;
String localSegment;
byte localSegmentStart;
// WiFiServer server(port);
// WiFiClient client;
bool isConnected = false;

// UDP related settings
uint16_t udpReceiveport = 9012;
uint16_t udpSendport = 9020;
WiFiUDP udpClient;


#define NETWORK_NONE        0
#define NETWORK_ROUTER      1
#define NETWORK_AP          2
uint8_t NetworkMode = 0;

char *AP_Name = (char *) "Alpha 1S";
char *AP_Password = (char *) "12345678";



#define DEBUG Serial1

#define CMD_BUFFER_SIZE 128

Buffer cmdBuffer(CMD_BUFFER_SIZE);

RobotConfig config(&DEBUG);

#ifdef ENABLE_OTA
//OTA Setting
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
const char* ssid = "wuhulanren";          
const char* password = "wuhulanren";
#define EN_OTA true
#endif

/*
//Touch Setting
*/
#define TOUCH_NONE      0
#define TOUCH_SINGLE    1
#define TOUCH_DOUBLE    2
#define TOUCH_TRIPLE    3
#define TOUCH_LONG      0xFF

//MPU6050 Setting
const uint8_t MPU_addr=0x68;  // I2C address of the MPU-6050
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t tmp;
int8_t actionSign;
int8_t getFaceDown , getFaceUp;
bool mpuActionBegin = false;
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
bool enable_HAILZD = true;

ComboData comboTable[CD_MAX_COMBO];
ActionData actionData;

#define MAX_ACTION      26
#define MAX_POSES       30 
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


struct {
    uint8_t rx_pin          = 12;
    uint8_t tx_pin          = 12;
    unsigned long baud      = 115200;
    bool inverse_logic      = false;
    uint16_t buffer_size    = 64;
} busConfig;

SoftwareSerial robotPort(busConfig.rx_pin, busConfig.tx_pin, busConfig.inverse_logic, busConfig.buffer_size);
RobotServo rs;

// SoftwareSerial ubt_ss(12, 12, false, 256);
// UBTech servo(&ubt_ss, &DEBUG);  // Debug on Serial1

int servoCnt = 0;
// byte *retBuffer;

byte ledMode = 0;

byte ch, cmd;

long lastCmdMs = 0;

bool debug = true;
bool devMode = false;

//BoardDefine

#define ROBOT_ARM_BOARD

#define MP3_RXD_GPIO    14
#define MP3_TXD_GPIO    16  
#define HEAD_LED_GPIO   15
#define TOUCH_GPIO      13

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

// Command_Generic
bool SendGenericCommand(byte *cmd, byte sendCnt);

// HILZD
bool HAILZD_Command();

// OTA.ino
void ArduinoOTASetup();

// Mpu6050
bool MpuInit();
void MpuGetActionHandle();

// Touch.ino
uint8_t DetectTouchMotion();
boolean ButtonIsPressed();

// TouchV2.ino
uint8_t CheckTouchAction();

// EyeLed.ino
void ReserveEyeBlink();
void ReserveEyeBreath();
void EyeBlink();
void EyeBreath();
void EyeLedHandle();

#endif
