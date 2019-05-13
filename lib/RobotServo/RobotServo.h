#ifndef _ROBOT_SERVO_H_
#define _ROBOT_SERVO_H_

#include "baseServo.h"

#define _UBTServo_
//#define _HLServo_

#if defined(_UBTServo_)
    #include <drivers/UBT/UBTServo.h>
#elif defined(_HLServo_)
    #include <drivers/HaiLzd/HLServo.h>
#endif

// Just a dummy class to control the interface for baseServo.
// You can skip this class and use the servo class directly.
class RobotServo {
    
    public:

        void setEnableTxCalback(void (*enableTxCallback)(bool)) { return _servo.setEnableTxCalback(enableTxCallback); }

        RobotServo();
        bool begin(Stream *busPort, Stream *debugPort) { return _servo.begin(busPort, debugPort); }
        ~RobotServo();

        byte servoType() { return _servo.servoType(); }

        void showInfo() { return _servo.showInfo(); }
        bool init(byte maxId, byte maxRetry) { return _servo.init(maxId, maxRetry); }
        bool initServo(byte id) { return _servo.initServo(id); }

        bool end() { return _servo.end(); }
        bool reset() { return _servo.reset(); }

        void enableDebug(bool value) { return _servo.enableDebug(value);}

        uint32_t getVersion(byte id) { return _servo.getVersion(id); }
        bool detectServo() { return _servo.detectServo(); }
        bool resetServo(byte id) { return _servo.resetServo(id); }

        bool exists(byte id) { return _servo.exists(id); }

        bool move(byte id, uint16_t pos, uint16_t time) { return _servo.move(id, pos, time); }
        bool move(byte cnt, byte *data) { return _servo.moveX(cnt, data); }
        bool goAngle(byte id, int16_t angle, uint16_t time) { return _servo.goAngle(id, angle, time); }
        
        uint16_t getAdjAngle(byte id) { return _servo.getAdjAngle(id); }
		uint16_t setAdjAngle(byte id, uint16 adjValue) {return _servo.setAdjAngle(id, adjValue); }

        byte servoCommand(byte *cmd) { return _servo.servoCommand(cmd); }

        byte setAngle(byte id, byte angle, byte minor) { return _servo.setAngle(id, angle, minor); } 


        uint16_t lastPos(byte id) { return _servo.lastPos(id); }
        uint16_t lastAngle(byte id) { return _servo.lastAngle(id); }
        
        bool setLED(byte id, bool mode) { return _servo.setLED(id, mode); }
        bool setLED(bool mode) { return _servo.setLED(mode); }

        uint16_t getPos(byte id) { return _servo.getPos(id); }
        uint16_t getPos(byte id, bool lockAfterGet) { return _servo.getPos(id, lockAfterGet); }
        uint16_t getAngle(byte id) { return _servo.getAngle(id); }
        uint16_t getAngle(byte id, bool lockAfterGet) { return _servo.getAngle(id, lockAfterGet); }

        bool isLocked(byte id) { return _servo.isLocked(id); }
        bool lock(byte id) { return _servo.lock(id); }
        bool lock() { return _servo.lockAll(); }
        bool unlock(byte id) { return _servo.unlock(id); }
        bool unlock() { return _servo.unlockAll(); }

        int16_t minPos() { return _servo.minPos(); }
        int16_t maxPos() { return _servo.maxPos(); }
        int16_t minAngle() { return _servo.minAngle(); }
        int16_t maxAngle() { return _servo.maxAngle(); }
        
        int16_t servoCnt() { return _servo.servoCnt(); }

    private:

        #if defined(_UBTServo_)
            UBTServo   _servo;
        #elif defined(_HLServo_)
            HLServo    _servo;
        #endif
};

#endif
