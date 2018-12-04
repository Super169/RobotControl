#ifndef _EDS_TOUCH_H_
#define _EDS_TOUCH_H_


// Keyword: ETU_

#include "EventHandler.h"
#include "EventDataSource.h"
#include "SSBoard.h"

#define ETU_TOUCH_NONE      0
#define ETU_TOUCH_SINGLE    1
#define ETU_TOUCH_DOUBLE    2
#define ETU_TOUCH_TRIPLE    3
#define ETU_TOUCH_LONG      0xFF

/*
*   Event Data Source for Battery
*
*/

class EdsTouch : public EventDataSource {
    public:
        EdsTouch(EventData *data, MyDebugger *dbg, byte devId = 0);
        ~EdsTouch();

        void Setup(uint8_t gpioPin, unsigned long touchDetectPeriod, unsigned long touchReleasePeriod);
        bool GetData() override;
        void PostHandler(bool eventMatched, bool isRelated, bool pending) override;

    private:
        uint8_t _gpioPin;

        unsigned long _touchDetectPeriod = 1500;
        unsigned long _touchReleasePeriod = 1000;
        bool _lastStatus = 0;
        unsigned long _touchStartMs = 0;
        unsigned long _touchReleaseMs = 0;
        unsigned long _touchCount = 0;
        bool _waitRelease = false;

        uint16_t _lastReportValue = 0;

        void ResetTouchAction();
        bool IsTouchPressed();
        uint8_t CheckTouchAction();
};

#endif
