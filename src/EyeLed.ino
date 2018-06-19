#include "robot.h"

unsigned long eyeBlinkTime;
boolean eyeStatus;
boolean eyeBlinkStatus ,eyeBreathStatus;
unsigned long eyeBreathTime;
uint8_t breathPwm = 0;
int8_t breathRatio = 3;

void ReserveEyeBlink(){
  eyeBlinkStatus = !eyeBlinkStatus;
  for(int i=0;i<4;i++){
      digitalWrite(HEAD_LED_GPIO,i%2);
      delay(50);
    };
}

void ReserveEyeBreath(){
  eyeBreathStatus = !eyeBreathStatus;
  for(int i=0;i<4;i++){
      digitalWrite(HEAD_LED_GPIO,i%2);
      delay(50);
    };
}

void EyeBlink(){
  if(millis()-eyeBlinkTime>300 && eyeBlinkStatus ){
      digitalWrite(HEAD_LED_GPIO,eyeStatus);
      eyeStatus = !eyeStatus;
      eyeBlinkTime = millis();
    }
}

void EyeBreath(){
  if(millis()-eyeBreathTime>40 && eyeBreathStatus){
    breathPwm += breathRatio;
    if(breathPwm>250){
        breathRatio=-breathRatio;
        breathPwm=250;
      }
    else if(breathPwm<=0){
        breathRatio= breathRatio;
        breathPwm=0;
    }
    analogWrite(HEAD_LED_GPIO,breathPwm);
    eyeBreathTime = millis();
  }
}

void EyeLedHandle(){
  EyeBreath();
  EyeBlink();
}
