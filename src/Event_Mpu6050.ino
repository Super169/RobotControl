#include "robot.h"
#include "V2_Command.h"

bool MpuInit(){
  Wire.begin();
  // Check if MPU available
  Wire.beginTransmission(MPU_addr);
  uint8_t i2cError = Wire.endTransmission();
  mpuExists = (i2cError == 0);
  if (DEBUG) {
    DEBUG.printf("\n====================\n");
    DEBUG.printf("i2c 0x%02X - %d\n",MPU_addr,i2cError);
    DEBUG.printf("\n====================\n");
  }
  if (mpuExists) {
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B);  // PWR_MGMT_1 register
    Wire.write(0);     // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);
    if (DEBUG) DEBUG.println("MPU 6050 Initialized");
  } else {
    if (DEBUG) DEBUG.println("MPU 6050 not available");
    return false;
  }
  return true;
}


bool MpuGetData() {
  if (!mpuExists)  return false;
  ax = ay = az = tmp = gx = gy = gz = 0;
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, (size_t) MPU_DATA_SIZE,true);  // request a total of 14 registers
  memset(mpuBuffer, 0, MPU_DATA_SIZE);
  for (int i = 0; i < MPU_DATA_SIZE; i++) {
    if (Wire.available()) {
      mpuBuffer[i] = Wire.read();
    } else {
      if (DEBUG) DEBUG.printf("Incomplete MPU data, only %d bytes returned\n", i);
      memset(mpuBuffer, 0, MPU_DATA_SIZE);
      return false;
    }
  }
  ax = mpuBuffer[0] << 8 | mpuBuffer[1];
  ay = mpuBuffer[2] << 8 | mpuBuffer[3];
  az = mpuBuffer[4] << 8 | mpuBuffer[5];
  gx = mpuBuffer[8] << 8 | mpuBuffer[9];
  gy = mpuBuffer[10] << 8 | mpuBuffer[11];
  gz = mpuBuffer[12] << 8 | mpuBuffer[13];
  return true;
}
/*
 *  16384                                  -16383        
 *   AX              -16383    AY   16384    AZ
 *   -16383                                16384
 */
uint8_t detectTimes = 0;

unsigned long lastMs = 0;

void MpuGetActionHandle(){
    if (!config.autoStand()) return;
    if (!mpuExists) return;
    actionSign = 0;
    
    if (!MpuGetData()) {
      if (DEBUG) DEBUG.println("Error getting MPU Data");
      return;
    }
    
    if ((debug) && (MPU_DEBUG)) {
      // Don't send too much, just 1 / sec
      if (millis() - lastMs > 1000) {
        DEBUG.printf("x,y,z: %d,%d, %d ; faceUp: %d, faceDown: %d\n", ax, ay, az, config.faceUpAction(), config.faceDownAction());
        lastMs = millis();
      }
    }


   //Serial.print(" | AcZ = ");
   //Serial.println(az);
  if (config.enableOLED()) {
    myOLED.print(64,5,"AZ: ");
    myOLED.printNum( 85 , 5 , (long)az , 10 , 6 , true);
    myOLED.show();
  }
  
  if(az < -14000 || az > 14000)detectTimes++;
  else if(!(az < -14000 || az > 14000) && detectTimes > 0 )detectTimes = 0;
  
  if( !debug && detectTimes >= config.mpuCheckFreq() ){
    if(az < -14000 ){
      // actionSign = FACE_DOWN_ID;
      actionSign = config.faceDownAction();
    }
    else if(az > 14000 ){
      // actionSign = FACE_UP_ID;hu9
      actionSign = config.faceUpAction();
    }
    if (actionSign) {
      V2_ResetAction();
      MpuAutoAction(actionSign);
      detectTimes = 0;
    }
  }
}

void MpuAutoAction(int8_t actionId) {
  V2_GoAction(actionId, false, NULL);
}

