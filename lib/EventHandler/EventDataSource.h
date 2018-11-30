#ifndef _EVENT_DATA_SOURCE_H_
#define _EVENT_DATA_SOURCE_H_

#include "ESP8266WiFi.h"
#include "EventData.h"
#include "MyDebugger.h"

// Default setting for continue check and delay check
#define EDS_CONTINUE_CHECK_MS   10
#define EDS_DELAY_CHECK_MS      1000

class EventDataSource {

    protected:
        EventData *_data = NULL;
        MyDebugger *_dbg;

        bool _isEnabled = false;
        bool _enableDebug = true;
        bool _lastDataReady = false;
        unsigned long _lastReportMS = 0;
        unsigned long _reportInterval = 1000;   // by default, each sensor will be reported once per second
        unsigned long _nextReportMs = 0;

        uint8_t _Device = 0;
        uint8_t _DevId = 0;

        void Config(EventData *data, MyDebugger *dbg, byte devId = 0);
        void SetNextReportTime();

    public:
        EventDataSource() {}
        ~EventDataSource() {}

        bool IsEnabled();
        bool IsReady();
        virtual void GetData() = 0;
        virtual void PostHandler(bool eventMatched, bool isRelated);

    private:


};

#endif

