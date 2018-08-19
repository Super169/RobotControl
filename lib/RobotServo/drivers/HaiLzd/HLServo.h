#ifndef _HL_Servo_H_
#define _HL_Servo_H_

// Profile for Hailzd Servo
//
#include "baseServo.h"
#define HL_RETURN_BUFFER_SIZE 	64  
#define HL_COMMAND_WAIT_TIME	5   // 3ms is more than enough

class HLServo : public baseServo
{
    public:
        HLServo();
        ~HLServo();

        // Methos MUST be implemented
        //
        bool reset() override;
        uint32_t getVersion(byte id) override;

        bool resetServo() override { return false; }
        bool resetServo(byte id) override { return false; }
        bool move(byte id, int16_t pos, uint16_t time) override;


        bool setLED(byte id, bool mode) override;
        bool setLED(bool mode) override;

        inline int16_t getPos(byte id) override { return getPos(id, false); }
        int16_t getPos(byte id, bool lockAfterGet) override;

        bool lock(byte id) override;
        bool unlock(byte id) override;


        // Methods can be overrided (optional)
        //
        void initBus() override;
        void showInfo() override;

        // bool validId(byte id);
        // bool validPos(int16_t pos);

        // bool isLocked(byte id) override;
        bool lockAll() override;
        bool unlockAll() override;
        
        bool moveX(byte cnt, byte *data) override;
        // bool goAngle(byte id, int16_t angle, uint16_t time) override;

        // uint16_t lastPos(byte id)  override;
        // uint16_t lastAngle(byte id) override;


    private:

        String _buffer;
        byte _retBuf[HL_RETURN_BUFFER_SIZE];  
        byte _retCnt = 0;
        
        inline bool sendCommand() { return sendCommand(true); }
        bool sendCommand(bool expectReturn);
        bool sendUntilOK();

        void showCommand();
        bool checkReturn();
        bool isReturnOK();
        void resetReturnBuffer();

        inline bool getRetNum(int16_t *data, byte start, byte len) { return getRetNum(data, start, len, len); }
        bool getRetNum(int16_t *data, byte start, byte len, byte minLen); 

};

#endif
