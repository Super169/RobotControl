#ifndef _EDS_TOUCH_H_
#define _EDS_TOUCH_H_


// Keyword: EBAT

#include "EventHandler.h"
#include "EventDataSource.h"
#include "SSBoard.h"

#define TOUCH_NONE      0
#define TOUCH_SINGLE    1
#define TOUCH_DOUBLE    2
#define TOUCH_TRIPLE    3
#define TOUCH_LONG      0xFF

/*
*   Event Data Source for Battery
*
*/

class EdsTouch : public EventDataSource {
    public:
        EdsTouch(EventData *data);
        ~EdsTouch();

        void Setup(uint8_t gpioPin, unsigned long touchDetectPeriod, unsigned touchReleasePeriod);
        void GetData() override;
        // void PostHandler(bool eventMatched, bool isRelated) override;

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
