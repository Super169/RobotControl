#include "robot.h"
#include "V2_Command.h"

bool MpuInit(){
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  return 1;
}

/*
 *  16384                                  -16383        
 *   AX              -16383    AY   16384    AZ
 *   -16383                                16384
 */
uint8_t detectTimes = 0;

void MpuGetActionHandle(){
    if (!config.autoStand()) return;
    actionSign = 0;
    az = 0;
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, (size_t) 14,true);  // request a total of 14 registers
    ax=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
    ay=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    az=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    gx=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    gy=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    gz=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
    // TODO: ask L why he comment out the endTransmission
  //Wire.endTransmission(true);
    
    if (debug) DEBUG.printf("x,y,z: %d,%d, %d ; faceUp: %d, faceDown: %d\n", ax, ay, az, config.faceUpAction(), config.faceDownAction());


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

void MpuAutoAction(int8_t actionId){
  V2_GoAction(actionId, false, NULL);
}

