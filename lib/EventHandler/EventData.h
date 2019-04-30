#ifndef _EVENTDATA_H_
#define _EVENTDATA_H_
#include <ESP8266WiFi.h>

#define ED_INVALID_DATA     -32768

#define ED_SIZE_MPU         3
#define ED_SIZE_TOUCH       1
#define ED_SIZE_PSXBUTTON   1
#define ED_SIZE_BATTERY     2
#define ED_SIZE_SONIC       1
#define ED_SIZE_MAZE        1

#define ED_COUNT_MPU        1
#define ED_COUNT_TOUCH      1
#define ED_COUNT_PSXBUTTON  1
#define ED_COUNT_BATTERY    1
#define ED_COUNT_SONIC      4
#define ED_COUNT_MAZE       1

/*
#define ED_OFFSET_MPU       0
#define ED_OFFSET_TOUCH     3
#define ED_OFFSET_PSXBUTTON 4
#define ED_OFFSET_BATTERY   5
#define ED_OFFSET_SONIC     7
*/

#define ED_MAX_DEVICE       6
#define ED_DATA_SIZE        64      


class EventData {

    public:

        enum class DEVICE : uint8_t {
            mpu = 1, touch = 2, psx_button = 3, battery = 4, sonic = 5, maze = 6
        };

        enum class MPU_TARGET : uint8_t {
            x = 0, y = 1, z = 2
        };

        enum class BATTERY_TARGET : uint8_t {
            reading = 0, level = 1
        };

        EventData();
        ~EventData();

        bool IsValid(uint8_t device, uint8_t devId, uint8_t target);
        inline bool IsValid(uint8_t device, uint8_t devId) { return IsValid(device, devId, 0); }
        bool IsReady(uint8_t device, uint8_t devId, uint8_t target);
        bool MarkReady(uint8_t device, uint8_t devId, uint8_t target, bool ready);
        void Clear();


        bool SetData(uint8_t device, uint8_t devId, uint8_t target, int16_t value);
        int16_t GetData(uint8_t device, uint8_t devId, uint8_t target);

        bool SetData(DEVICE device, uint8_t devId, uint8_t target, int16_t value) {
            return SetData((uint8_t) device, devId, target, value);
        }
        int16_t GetData(DEVICE device, uint8_t devId, uint8_t target) {
            return GetData((uint8_t) device, devId, target);
        }

        // In the first version, devId is not used, so let's allow to skip this parameter in common interface first
        bool SetData(DEVICE device, uint8_t target, int16_t value) { 
            return SetData((uint8_t) device, 0, target, value); 
        }
        int16_t GetData(DEVICE device, uint8_t target) { 
            return GetData((uint8_t) device, 0, target); 
        }

        uint8_t DeviceDataSize(uint8_t device);

        uint8_t Offset(uint8_t device, uint8_t devId, uint8_t target);
        uint8_t DevOffset(uint8_t device, uint8_t devId);
        uint8_t IdCount(uint8_t device);
        uint8_t MaxId(uint8_t device);
        uint8_t DevCount() { return _devCount; }

        bool SetThreadhold(uint8_t device, uint8_t devId, uint16_t threadhold);
        uint16_t Threadhold(uint8_t device, uint8_t devId);

        void DumpData(Stream *output);
        void DumpData(Stream *output, uint8_t device);

        // ------
        // TODO: remove these method once all program can handle multiple ID for device
        bool SetThreadhold(uint8_t device, uint16_t threadhold) {
            return SetThreadhold(device, 0, threadhold);
        }
        uint16_t Threadhold(uint8_t device) {
            return Threadhold(device, 0);
        }
        // ------


    private:
        const uint8_t _size[ED_MAX_DEVICE + 1] = 
            {0, ED_SIZE_MPU, ED_SIZE_TOUCH, ED_SIZE_PSXBUTTON, ED_SIZE_BATTERY, ED_SIZE_SONIC, ED_SIZE_MAZE};
        const uint8_t _idCount[ED_MAX_DEVICE + 1] = 
            {0, ED_COUNT_MPU, ED_COUNT_TOUCH, ED_COUNT_PSXBUTTON, ED_COUNT_BATTERY, ED_COUNT_SONIC, ED_COUNT_MAZE};
        // const uint8_t _offset[ED_MAX_DEVICE + 1] = {0, ED_OFFSET_MPU, ED_OFFSET_TOUCH, ED_OFFSET_PSXBUTTON, ED_OFFSET_BATTERY, ED_OFFSET_SONIC };
        // uint16_t _threadhold[ED_MAX_DEVICE + 1] = {0, 0, 0, 0, 0,0 };

        uint8_t _offset[ED_MAX_DEVICE + 1];
        uint8_t _devOffset[ED_MAX_DEVICE + 1];


        uint8_t _devCount;      // actual device, not the count of device type
        uint8_t _dataSize;


        int16_t _data[ED_DATA_SIZE];

        // Control flags: use bool array instead of bit control table at this moment
        // bool     _ready[ED_DATA_SIZE];
        // uint16_t _threadhold[ED_MAX_DEVICE + 1];  
        bool *_ready;
        uint16_t *_threadhold;


        void ShowValue(Stream *output, uint8_t idx, uint8_t mode = 0);

};


#endif
