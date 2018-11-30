#ifndef _EDS_MPU6050_H_
#define _EDS_MPU6050_H_

// Keyword: EMPU_

#include <Wire.h>
#include "EventHandler.h"
#include "EventDataSource.h"

#define EMPU_DATA_SIZE   14
#define EMPU_RESULT_SIZE 20

class EdsMpu6050 : public EventDataSource {
    public:
        EdsMpu6050(EventData *data, MyDebugger *dbg, byte devId = 0);
        ~EdsMpu6050();

        void Setup(uint8_t i2cAddr = 0x68);
        bool GetData() override;
        // void PostHandler(bool eventMatched, bool isRelated) override;

    private:
        uint8_t _i2cAddr = 0x68;
        uint8_t _mpuBuffer[EMPU_DATA_SIZE];
        int16_t _ax, _ay, _az;
        int16_t _gx, _gy, _gz;
        int16_t _tmp;
        int8_t  _actionSign;
};


#endif
