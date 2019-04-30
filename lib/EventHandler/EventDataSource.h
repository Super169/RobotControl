#ifndef _EVENT_DATA_SOURCE_H_
#define _EVENT_DATA_SOURCE_H_

#include "ESP8266WiFi.h"
#include "EventData.h"
#include "MyDebugger.h"

// Default setting for checking frequency, don't make it too busy
#define EDS_PENDING_CHECK_MS    20
#define EDS_CONTINUE_CHECK_MS   50
#define EDS_DELAY_CHECK_MS      1000
#define EDS_HANDLED_CHECK_MS    1000

class EventDataSource {

    protected:
        EventData *_data = NULL;
        MyDebugger *_dbg;

        bool _isAvailable = false;
        bool _isEnabled = true;
        bool _isSuspended = false;
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
        bool _thisDataError = false;

        unsigned long _lastReportMS = 0;
        bool _lastValueHandled = false;
        unsigned long _reportInterval = 1000;   // by default, each sensor will be reported once per second
        unsigned long _nextReportMs = 0;

        unsigned long _pendingCheckMs = EDS_PENDING_CHECK_MS;
        unsigned long _continueCheckMs = EDS_CONTINUE_CHECK_MS;
        unsigned long _delayCheckMs = EDS_DELAY_CHECK_MS;

        uint8_t _Device = 0;
        uint8_t _DevId = 0;

        void Config(EventData *data, MyDebugger *dbg, byte devId = 0);
        void SetNextReportTime();

    public:
        EventDataSource() {}
        ~EventDataSource() {}

        uint8_t Device() { return _Device; }
        uint8_t DevId() { return _DevId; }

        bool SetEnabled(bool enabled);
        bool IsAvailable();
        bool IsEnabled();
        bool IsReady();
        void Suspend(bool suspend);
        virtual bool GetData() = 0;
        bool ForceGetData() { 
            _nextReportMs = 0;
            return GetData();
        }
        virtual void PostHandler(bool eventMatched, bool isRelated, bool pending);

    private:


};

#endif

