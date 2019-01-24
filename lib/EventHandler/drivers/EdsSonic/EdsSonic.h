#ifndef _EDS_SONIC_H_
#define _EDS_SONIC_H_

// Keyword: EPB_

#include "EventHandler.h"
#include "EventDataSource.h"
#include "SSBoard.h"

/*
*   Event Data Source for PSX Button
*
*
*       Ping:
*          Send:    A8 8A 04 02 00 01 07 ED
*          Return:  A8 8A 05 02 00 01 00 08 ED
*
*
*       Get Data:
*          Send:    A8 8A 06 02 00 02 28 02 34 ED	
*          Return:  A8 8A 08 02 00 02 28 02 00 B4 EA ED （B4，180cm）
*/


class EdsSonic : public EventDataSource {
    public:
        EdsSonic(EventData *data, MyDebugger *dbg, byte devId = 0);
        ~EdsSonic();

        void Setup(SSBoard *ssb, unsigned long continueCheckms, unsigned long delayCheckMs);
        bool GetData() override;
        // void PostHandler(bool eventMatched, bool isRelated, bool pending) override;

    private:
        SSBoard *_ssb;
        bool Ping();
        // uint8_t _normalCheckMs = 0;
        // uint8_t _noEventMs = 0;
        // uint16_t _ignoreRepeatMs = 0;
        // uint16_t _lastReportValue = 0;
};

#endif