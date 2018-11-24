#ifndef _BASE_SERVO_H_
#define _BASE_SERVO_H_

#include <ESP8266WiFi.h>
#include "MyDebugger.h"

#define BS_TYPE_UNKNOWN 0
#define BS_TYPE_UBT     1
#define BS_TYPE_HaiLzd  2


struct BS_MOVEPARM {
    byte id;
    int16_t pos;
    uint16_t time;
};

class baseServo {
    
    protected:

        Stream *_bus = NULL;
        MyDebugger _dbg;

        void (*_enableTxCallback)(bool);
        void enableTx(bool);

        // servo depended constant; to be setup by servo
        int16_t _MIN_POS = 0;
        int16_t _MAX_POS = 180;
        int16_t _MIN_ANGLE = 0;
        int16_t _MAX_ANGLE = 180;
        int16_t _INVALID_POS = 999;
        bool    _MULTI_SERVO_COMMAND = false;

        bool _arrayReady = false;
        byte _maxId = 0;
        bool* _servo = NULL;
        byte* _led = NULL;
        bool* _isLocked = NULL;
        int16_t* _lastPos = NULL;
        bool* _isServo = NULL;
        byte _servoCnt = 0;
        
        byte _maxRetry = 0;

    public:
        void setEnableTxCalback(void (*enableTxCallback)(bool));

        baseServo();
        virtual ~baseServo();
        bool begin(Stream *busPort, Stream *debugPort);

        bool init(byte maxId, byte maxRetry);
        bool setBaud(uint32_t baud);
        bool end();
        void enableDebug(bool value);

        bool detectServo();
        bool exists(byte id) { return (validId(id) ? _servo[id] : false); }

        int16_t minPos() { return _MIN_POS; }
        int16_t maxPos() { return _MAX_POS; }
        int16_t minAngle() { return _MIN_ANGLE; }
        int16_t maxAngle() { return _MAX_ANGLE; }
        int16_t servoCnt() { return _servoCnt; }
        

        // Method can be overrided
        virtual void initBus() { return; }
        virtual bool initServo(byte id) { return true; }
        virtual void showInfo();

        virtual bool validId(byte id);
        virtual bool validPos(int16_t);

        virtual uint16_t angle2pos(uint16_t angle) { return map(angle, _MIN_ANGLE, _MAX_ANGLE, _MIN_POS, _MAX_POS); }
        virtual uint16_t pos2angle(uint16_t pos) { return map(pos, _MIN_POS, _MAX_POS, _MIN_ANGLE, _MAX_ANGLE); }

        virtual bool isLocked(byte id);
        virtual bool lockAll();
        virtual bool unlockAll();
        virtual bool moveX(byte cnt, byte *data);
        virtual bool goAngle(byte id, int16_t angle, uint16_t time) { return move(id, angle2pos(angle), time); }

        virtual uint16_t lastPos(byte id)  { return (validId(id) ? _lastPos[id] : _INVALID_POS); }
        virtual uint16_t lastAngle(byte id) { return (validId(id) ? pos2angle(_lastPos[id]) : _INVALID_POS); }

        virtual uint16_t getAngle(byte id) { return (validId(id) ? pos2angle(getPos(id)) : _INVALID_POS); }
        virtual uint16_t getAngle(byte id, bool lockAfterGet) { return (validId(id) ? pos2angle(getPos(id, lockAfterGet)) : _INVALID_POS); }

                // Methods MUST be overrided
        virtual byte servoType() = 0;
        virtual bool reset() = 0;
        virtual uint32_t getVersion(byte id) = 0;


        virtual bool resetServo() = 0;
        virtual bool resetServo(byte id) = 0;

        virtual bool move(byte id, int16_t pos, uint16_t time) = 0;
        virtual bool setLED(byte id, bool mode) = 0;
        virtual bool setLED(bool mode) = 0;

        virtual int16_t getPos(byte id) = 0;
        virtual int16_t getPos(byte id, bool lockAfterGet) = 0;

        virtual bool lock(byte id) = 0;
        virtual bool unlock(byte id) = 0;

        virtual uint16_t getAdjAngle(byte id) = 0;
        virtual uint16_t setAdjAngle(byte id, uint16 adjValue) = 0;
        virtual byte servoCommand(byte* cmd) = 0;

        virtual byte setAngle(byte id, byte angle, byte minor) = 0;

    private:

};

#endif
