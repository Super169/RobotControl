#ifndef _ROBOTEVENTHANDLER_H_
#define _ROBOTEVENTHANDLER_H_

#include "EventData.h"
#include "EventHandler.h"
#include "EdsDrivers.h"

#define EVENT_IDEL_FILE "/alpha/event/idle.event"
#define EVENT_BUSY_FILE "/alpha/event/busy.event"

#define EVENT_HANDLER_VERSION  1

#define EH_OFFSET_MODE          4
#define EH_OFFSET_VERSION       5
#define EH_OFFSET_COUNT         6
#define EH_OFFSET_ACTION        7

#define ED_OFFSET_MODE          4
#define ED_OFFSET_STARTIDX      5
#define ED_OFFSET_COUNT         6


#define EVENT_HANDLER_ELAPSE_MS 10

bool eventHandlerSuspended  = true;

struct {
    uint8_t rx_pin          = 14;
    uint8_t tx_pin          = 14;
    unsigned long baud      = 115200;
    bool inverse_logic      = false;
    uint16_t buffer_size    = SSB_BUFFER_SIZE;
} ssbConfig;

SoftwareSerial ssbPort(ssbConfig.rx_pin, ssbConfig.tx_pin, ssbConfig.inverse_logic, ssbConfig.buffer_size);

unsigned long nextHandlerMs = 0;
unsigned long nextShowMs = 0;
bool lastPlaying = false;

#define EDS_TOUCH_GPIO      13
#define EDS_MPU6050_I2CADDR 0x68  // I2C address of the MPU-6050

// Too bad, cannot put them here as it cannot reconized the _dbg in robot.h, so have to move to robot.h
// After Eds moved to robot.h, eData also need to mov e to robot.h, so all moved to robot.h

//EventData eData;
//EventHandler eIdle(&eData);
//EventHandler eBusy(&eData);
//EventHandler eTemp(&eData);

//SSBoard ssb;
//EdsPsxButton edsPsxButton(&eData, _dbg);
//EdsBattery edsBattery(&eData, _dbg);
//EdsTouch edsTouch(&eData, _dbg);

void InitEventHandler();
void RobotEventHandler();
void CheckPosition();
void CheckTouch();
void CheckVoltage() ;
byte GetPower(uint16_t v);


EventDataSource** eds;

void EnableSsbTxCallBack(bool send);
void USB_TTL(SoftwareSerial *ttl);
void USER_TTL(SoftwareSerial *ttl);

#endif