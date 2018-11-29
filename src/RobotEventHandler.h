#ifndef _ROBOTEVENTHANDLER_H_
#define _ROBOTEVENTHANDLER_H_

#include "EventData.h"
#include "EventHandler.h"
#include "EdsPsxButton.h"
#include "EdsBattery.h"


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

struct {
    uint8_t rx_pin          = 14;
    uint8_t tx_pin          = 14;
    unsigned long baud      = 115200;
    bool inverse_logic      = false;
    uint16_t buffer_size    = SSB_BUFFER_SIZE;
} ssbConfig;

SoftwareSerial ssbPort(ssbConfig.rx_pin, ssbConfig.tx_pin, ssbConfig.inverse_logic, ssbConfig.buffer_size);

EventData eData;
EventHandler eIdle(&eData);
EventHandler eBusy(&eData);
EventHandler eTemp(&eData);

SSBoard ssb;
EdsPsxButton edsPsxButton(&eData);
EdsBattery edsBattery(&eData);

void InitEventHandler();
void RobotEventHandler();
void CheckPosition();
void CheckTouch();
void CheckVoltage() ;
byte GetPower(uint16_t v);



#endif