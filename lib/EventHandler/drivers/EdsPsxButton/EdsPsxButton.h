#ifndef _EDS_PSX_BUTTON_H_
#define _EDS_PSX_BUTTON_H_

// Keyword: EPB_

#include "EventHandler.h"
#include "EventDataSource.h"
#include "SSBoard.h"

#define EPB_NO_BUTTON           0xFFFF
// #define EPB_CONTINUE_CHECK_MS   20
// #define EPB_NO_EVENT_MS         100
// #define EPB_IGNORE_REPEAT_TIME  200

/*
*   Event Data Source for PSX Button
*
*       Get Data: send "A8 8A 02 01 03 ED" to bus 
*                 wait for return "A8 8A 0B 01 -- -- -- ?? ?? -- -- -- -- -- ED" 
*/
/*
*   New sub-system board command:
*   
*       Get Data: A8 8A 06 01 00 02 28 02 33 ED
*         Return: A8 8A 08 01 00 02 28 02  [ EF 00 ] 24 ED 
*                  0  1  2  3  4  5  6  7     8  9
*
*
*       Shock: A8 8A 07 01 00 03 2E 01 01 3C ED wrong
*              A8 8A 07 01 00 03 2F 01 01 3C ED
*
*/


class EdsPsxButton : public EventDataSource {
    public:
        EdsPsxButton(EventData *data, MyDebugger *dbg, byte devId = 0);
        ~EdsPsxButton();

        // void Initialize(EventData *data) override;
        void Setup(SSBoard *ssb, uint8_t normalCheckMs, uint8_t noEventMs, uint16_t ignoreRepeatMs);
        bool GetData() override;
        void PostHandler(bool eventMatched, bool isRelated, bool pending) override;
        void Shock();

    private:
        SSBoard *_ssb;
        bool Ping();

        uint8_t _normalCheckMs = 0;
        uint8_t _noEventMs = 0;
        uint16_t _ignoreRepeatMs = 0;


        uint16_t _lastReportValue = 0;
};

#endif