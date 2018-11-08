#ifndef _EVENT_MPU6050_H_
#define _EVENT_MPU6050_H_

//MPU6050 Setting
#define MPU_DEBUG       false

#define MPU_DATA_SIZE   14
#define MPU_RESULT_SIZE 20
const uint8_t MPU_addr=0x68;  // I2C address of the MPU-6050
uint8_t mpuBuffer[MPU_DATA_SIZE];
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t tmp;
int8_t actionSign;
int8_t getFaceDown , getFaceUp;
bool mpuActionBegin = false;
// #define FACE_DOWN_ID 5
// #define FACE_UP_ID 6

// Mpu6050
bool mpuExists = false;

bool MpuInit();
bool MpuGetData();
void MpuGetActionHandle();
void MpuAutoAction(int8_t actionId);

#endif