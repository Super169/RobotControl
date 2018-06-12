#include "robot.h"

unsigned long buttonTime,motionBeginTime;
uint8_t pressedTimes,clickTimes,releasedTimes;
boolean buttonState,lastButtonState;
uint8_t buttonMotion;

uint8_t DetectTouchMotion(){
   if(millis() - buttonTime > 20){
      buttonTime = millis();
      buttonState = ButtonIsPressed();
      
      if(buttonState && !lastButtonState && releasedTimes>15){
          releasedTimes = 0;
          motionBeginTime = millis();
        }
      if(buttonState && lastButtonState){
          pressedTimes++;
        }
      if(!buttonState && lastButtonState){
          clickTimes++;
        }  
      if(!buttonState && !lastButtonState){
          releasedTimes++;
          pressedTimes = 0;
        }  
      lastButtonState = buttonState;

      if(millis() - motionBeginTime > 1000){
          if(pressedTimes > 40){
            buttonMotion = LONG_TOUCH;
            clickTimes = 0;
            DEBUG.println(F("LongTouch Detected!!"));
            myOLED.print(0,6,"LongTouch Detected!!");
            myOLED.show();
          }
          else if(pressedTimes < 10 && clickTimes != 0){
            buttonMotion = clickTimes;
            if(clickTimes == 1){
              DEBUG.println(F("SINGLE_CLICK Detected!!"));
              myOLED.print(0,6,"SINGLE_CLICK Detected!!");
              myOLED.show();
            }
            else if(clickTimes == 2){
              DEBUG.println(F("DOUBLE_CLICK Detected!!"));
              myOLED.print(0,6,"DOUBLE_CLICK Detected!!");
              myOLED.show();
            }
            else {
              DEBUG.println(F("TRIPLE_CLICK Detected!!"));
              myOLED.print(0,6,"TRIPLE_CLICK Detected!!");
              myOLED.show();
            }
          }
          if(releasedTimes > 5){
              releasedTimes = 0;
              clickTimes = 0;
              pressedTimes = 0;
              buttonMotion = NONE_MOTION;
              myOLED.print(0,6,"                        ");
              myOLED.show();
            }
        }
    }
    return buttonMotion;
}

boolean ButtonIsPressed(){
  pinMode(PIN_SETUP,INPUT);
  return  digitalRead(PIN_SETUP);
}
