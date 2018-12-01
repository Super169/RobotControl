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

        bool _isEnabled = true;
        bool _enableDebug = true;
        /*
            _lastDataReady vs _thisDataReady
            In most situation, _lastDataReady should be the same as _thisDataReady as the current value will be updated to last value.
            But for some special case, it need to check if the previous value has been handled, and it may skip current reading under some condition.
            E.g. PSX Button
               Since it will keep checking the button status, while this status will repeat for 1s if no other button pressed.
               When it checked again within 1s, it need to ignore this button, but it need to keep the status of some value handled.
               In this case, _lastDataReady is true, but _thisDataReady is false.
        */
        bool _lastDataReady = false;
        bool _thisDataReady = false;


        unsigned long _lastReportMS = 0;
        bool _lastValueHandled = false;
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
        virtual bool GetData() = 0;
        virtual void PostHandler(bool eventMatched, bool isRelated);

    private:


};

#endif

