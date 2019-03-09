#ifndef _EDS_MAZE_H_
#define _EDS_MAZE_H_

// Keyword: EPB_

#include "EventHandler.h"
#include "EventDataSource.h"
#include "SSBoard.h"
#include "RobotServo.h"

#define EDS_MAZE_FRONT_ID   0
#define EDS_MAZE_LEFT_ID    1
#define EDS_MAZE_RIGHT_ID   2

#define EDS_MAZE_GO_FRONT   0
#define EDS_MAZE_GO_LEFT    1
#define EDS_MAZE_GO_RIGHT   2
#define EDS_MAZE_TURN_BACK  3



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

        void Setup(SSBoard *ssb, RobotServo *rs, uint8_t wallDistance, 
                   uint8_t servoId, bool servoReversed, uint16_t servoMoveMs, uint16_t servoWaitMs,
                   unsigned long continueCheckms, unsigned long delayCheckMs);
        bool GetData() override;
        // void PostHandler(bool eventMatched, bool isRelated, bool pending) override;

    private:
        uint8_t _servoId = 0;
        uint8_t _wallDistance = 100;
        SSBoard *_ssb;
        RobotServo *_rs;

        bool Ping();
        bool Ping(byte id);
        bool ReadSensor(byte id, uint16 *data);

        bool GetOneSensorData(uint16 *disFront, uint16 *disLeft, uint16 *disRight);
        bool GetThreeSensorData(uint16 *disFront, uint16 *disLeft, uint16 *disRight);

        unsigned long _lastMsgMs = 0;

        bool _servoReversed = false;
        unsigned long _servoMoveMs = 500;
        unsigned long _servoWaitMs = 2000;


        // uint8_t _normalCheckMs = 0;
        // uint8_t _noEventMs = 0;
        // uint16_t _ignoreRepeatMs = 0;
        // uint16_t _lastReportValue = 0;
};

#endif