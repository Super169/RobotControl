#ifndef _EVENT_DATA_SOURCE_H_
#define _EVENT_DATA_SOURCE_H_

#include "ESP8266WiFi.h"
#include "EventData.h"
#include "MyDebugger.h"

class EventDataSource {

    protected:
        EventData *_data = NULL;
        MyDebugger _dbg;

        bool _isEnabled = false;
        unsigned long _lastReportMS = 0;
        unsigned long _reportInterval = 1000;   // by default, each sensor will be reported once per second
        unsigned long _nextReportMs = 0;

        uint8_t _Device = 0;
        uint8_t _DevId = 0;

        void SetNextReportTime() { _nextReportMs = millis() + _reportInterval; }

    public:
        EventDataSource() {}
        ~EventDataSource() {}

        bool IsEnabled() { return _isEnabled; }
        bool IsReady() { return (_isEnabled && (millis() > _nextReportMs)); }
        virtual void Initialize(EventData *data);
        virtual void GetData();
        virtual void PostHandler();

    private:


};

#endif

