#ifndef _EDS_PSX_BUTTON_H_
#define _EDS_PSX_BUTTON_H_

// Keyword: EPB_

#include "EventHandler.h"
#include "EventDataSource.h"
#include "SSBoard.h"

#define EPB_NO_BUTTON           0xFFFF
#define EPB_CONTINUE_CHECK_MS   20
#define EPB_IGNORE_REPEAT_TIME  200

/*
*   Event Data Source for PSX Button
*
*       Get Data: send "A8 8A 02 01 03 ED" to bus 
*                 wait for return "A8 8A 0B 01 -- -- -- ?? ?? -- -- -- -- -- ED" 
*/

class EdsPsxButton : public EventDataSource {
    public:
        EdsPsxButton(EventData *data, MyDebugger *dbg, byte devId = 0);
        ~EdsPsxButton();

        // void Initialize(EventData *data) override;
        void Setup(SSBoard *ssb);
        bool GetData() override;
        void PostHandler(bool eventMatched, bool isRelated, bool pending) override;
        void Shock();

    private:
        SSBoard *_ssb;

        uint16_t _lastReportValue = 0;
};

#endif