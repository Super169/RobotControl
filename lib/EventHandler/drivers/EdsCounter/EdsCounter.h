#ifndef _EDS_COUNTER_H_
#define _EDS_COUNTER_H_

#define ECNT_MAX 10

#include "EventHandler.h"
#include "EventDataSource.h"

/*
*   Event Data Source for Counter
*
*/

class EdsCounter : public EventDataSource {
    public:
        EdsCounter(EventData *data, MyDebugger *dbg, byte devId = 0);
        ~EdsCounter();

        // void Initialize(EventData *data) override;
        void Setup();
        bool GetData() override;
        bool SetCounter(uint8_t target, int16_t value);

        byte GetPower(uint16_t v);

    private:

        int16_t _counter[ECNT_MAX];

};

#endif