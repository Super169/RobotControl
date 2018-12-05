#ifndef _EDS_BATTERY_H_
#define _EDS_BATTERY_H_

// Keyword: EBAT

#include "EventHandler.h"
#include "EventDataSource.h"
#include "SSBoard.h"

#define EBAT_THREADHOLD 1000

/*
*   Event Data Source for Battery
*
*/

class EdsBattery : public EventDataSource {
    public:
        EdsBattery(EventData *data, MyDebugger *dbg, byte devId = 0);
        ~EdsBattery();

        // void Initialize(EventData *data) override;
        void Setup(uint16_t minVoltage, uint16_t maxVoltage, uint16_t normalCheckMs, uint16_t alarmIntervalMs);
        bool GetData() override;
        // void PostHandler(bool eventMatched, bool isRelated, bool pending) override;

        byte GetPower(uint16_t v);

    private:

        uint16_t _minVoltage = 0;
        uint16_t _maxVoltage = 0;
        uint16_t _lastReportReading = 0;
        uint16_t _lastReportLevel = 0;

};

#endif