#ifndef _EVENTHANDLER_H_
#define _EVENTHANDLER_H_
#include <ESP8266WiFi.h>
#include "EventData.h"
#include <FS.h>
#define FILE_READ	"r"
#define FILE_WRITE	"w"

class EventHandler {

    public:

        enum class CHECK_MODE : uint8_t {
            match = 1, greater = 2, less = 3, button = 4
        };

        enum class MPU6050_TARGET : uint8_t {
            x = 0, y  = 1, z = 2
        };

        enum class ACTION_TYPE : uint8_t {
            na = 0, playAction = 1, stopAction = 2, headLed = 3, mp3PlayMp3 = 4, mp3PlayFile = 5, mp3Stop = 6, gpio = 7, system_action = 8, servo = 9, sonic = 10
        };

        enum class EVENT_TYPE : uint8_t {
            na = 0, handler = 1, preCond = 2
        };


        union CONDITION {
            uint8_t buffer[6];
            struct {
                uint8_t device;
                uint8_t devId;
                uint8_t target;
                uint8_t checkMode;
                int16_t value;                    
            } data;
        };

        union ACTION {
            uint8_t buffer[4];
            struct {
                uint8_t type;
                uint8_t parm_1;
                uint8_t parm_2;
                uint8_t parm_3;
            } data;
            struct {
                uint8_t type;
                uint8_t parm_1;
                uint16_t parm_u16;
            } u16data;
        };

        union EVENT {
            uint8_t buffer[12];
            struct {
                uint8_t seq;
                uint8_t type;
                CONDITION condition;
                ACTION action;
            } data;
        };

        EventHandler(EventData *data);
        ~EventHandler();

        void ReleaseMemory();
        void SetCount(uint8_t count);
        void Reset(uint8_t count);
        bool FillData(uint8_t startIdx, uint8_t count, byte *buffer);
        bool Clone(EventHandler *source);
        bool IsValid();

        bool LoadData(String filename);
        void LoadDummyData();
        bool SaveData(String filename);
        bool IsRequired(uint8_t device, uint8_t devId);
        bool LastEventRelated(uint8_t device, uint8_t devId);
        bool IsPending(uint8_t device, uint8_t devId);

        EVENT CheckEvents();
        uint16_t Count() { return _evtCount; }
        EVENT *Events() { return _events; }

        void ResetEventLastMs();
        void DumpEvents(Stream *output);

    private:
        uint16_t _evtCount;
        EVENT *_events;
        unsigned long *_eventLastMs;

        EventData *_data;
        bool *_isRequired;
        void CheckEventsRequirement();  // mark flag in _isRequired

        // bool _lastEventRelated[ED_MAX_DEVICE + 1];
        bool *_lastEventRelated; 
        bool MatchCondition(uint16_t idx, CONDITION cod);

        size_t FileSize(const char *fileName);
        bool ReadFile(const char *fileName, uint8_t *buffer, size_t size);
        bool DeleteFile(const char *fileName, bool mustExists = false);
        bool WriteFile(const char *fileName, uint8_t *buffer, size_t size);
};

#endif
