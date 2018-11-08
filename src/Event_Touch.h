#ifndef _EVENT_TOUCH_H_
#define _EVENT_TOUCH_H_
#include "ESP8266WiFi.h"
/*
//Touch Setting
*/
#define TOUCH_NONE      0
#define TOUCH_SINGLE    1
#define TOUCH_DOUBLE    2
#define TOUCH_TRIPLE    3
#define TOUCH_LONG      0xFF

#define TOUCH_GPIO      13

// TouchV2.ino
uint8_t CheckTouchAction();
void ResetTouchAction();
bool IsTouchPressed();

#endif