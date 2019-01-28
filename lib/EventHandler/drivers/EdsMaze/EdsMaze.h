#ifndef _EDS_MAZE_H_
#define _EDS_MAZE_H_

// Keyword: EPB_

#include "EventHandler.h"
#include "EventDataSource.h"
#include "SSBoard.h"

#define EDS_MAZE_FRONT_ID   0
#define EDS_MAZE_LEFT_ID    1
#define EDS_MAZE_RIGHT_ID   2

#define EDS_MAZE_GO_FRONT   0
#define EDS_MAZE_GO_LEFT    1
#define EDS_MAZE_GO_RIGHT   2
#define EDS_MAZE_TURN_LEFT  3

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


class EdsMaze : public EventDataSource {
    public:
        EdsMaze(EventData *data, MyDebugger *dbg, byte devId = 0);
        ~EdsMaze();

        void Setup(SSBoard *ssb, unsigned long continueCheckms, unsigned long delayCheckMs);
        bool GetData() override;
        // void PostHandler(bool eventMatched, bool isRelated, bool pending) override;

    private:
        uint16_t _openDistance = 100;
        SSBoard *_ssb;

        bool Ping();
        bool Ping(byte id);
        bool ReadSensor(byte id, uint16 *data);
        // uint8_t _normalCheckMs = 0;
        // uint8_t _noEventMs = 0;
        // uint16_t _ignoreRepeatMs = 0;
        // uint16_t _lastReportValue = 0;
};

#endif