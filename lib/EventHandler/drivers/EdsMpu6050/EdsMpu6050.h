#ifndef _EDS_MPU6050_H_
#define _EDS_MPU6050_H_

// Keyword: EMPU_

#include <Wire.h>
#include "EventHandler.h"
#include "EventDataSource.h"

#define EMPU_DEFAULT_ADDR   0x68
#define EMPU_DATA_SIZE      14
#define EMPU_RESULT_SIZE    20
#define EMPU_DELAY_CHECK_MS 5000

class EdsMpu6050 : public EventDataSource {
    public:
        EdsMpu6050(EventData *data, MyDebugger *dbg, byte devId = 0);
        ~EdsMpu6050();

        void Setup(uint8_t i2cAddr, uint16_t threadhold, uint16_t elapseMs);
        bool GetData() override;
        // void PostHandler(bool eventMatched, bool isRelated, bool pending) override;

        bool GetMpuData();
        uint8_t *MpuBuffer() { return (uint8_t *) _mpuBuffer; }

        inline int16_t ax() { return _ax; }
        inline int16_t ay() { return _ay; }
        inline int16_t az() { return _az; }

        inline int16_t gx() { return _gx; }
        inline int16_t gy() { return _gy; }
        inline int16_t gz() { return _gz; }

    private:
        uint8_t _i2cAddr = 0x68;
        uint8_t _mpuBuffer[EMPU_DATA_SIZE];
        int16_t _ax, _ay, _az;
        int16_t _gx, _gy, _gz;
        int16_t _tmp;
        int8_t  _actionSign;
        uint16_t _threadhold;
};


#endif
