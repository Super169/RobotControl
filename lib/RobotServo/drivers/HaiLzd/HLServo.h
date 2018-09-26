#ifndef _HL_Servo_H_
#define _HL_Servo_H_

// Profile for Hailzd Servo
//
#include "baseServo.h"
#define HL_RETURN_BUFFER_SIZE 	64  
#define HL_COMMAND_WAIT_TIME    5       // 3ms is more than enough

#define HL_SERVO_MODE           1       // 180 degree
#define HL_POS_MAX              2500    // This is a limitation due to old version action file, if servo pos > POS_MAX, move it to POS_MAX
#define HL_ANGLE_MAX            180     // This is a limitation due to old version action file, if servo angle > ANGLE_MAX, move it to ANGLE_MAX

class HLServo : public baseServo
{
    public:
        HLServo();
        ~HLServo();

        // Methos MUST be implemented
        //
        byte servoType() override { return BS_TYPE_HaiLzd; }

        bool reset() override;
        uint32_t getVersion(byte id) override;

        bool resetServo() override { return false; }
        bool resetServo(byte id) override;
        bool move(byte id, int16_t pos, uint16_t time) override;


        bool setLED(byte id, bool mode) override;
        bool setLED(bool mode) override;

        inline int16_t getPos(byte id) override { return getPos(id, _isLocked[id]); }
        int16_t getPos(byte id, bool lockAfterGet) override;

        bool lock(byte id) override;
        bool unlock(byte id) override;

        uint16_t getAdjAngle(byte id) override { return 0; }
        uint16_t setAdjAngle(byte id, uint16 adjValue) override { return 0; }
        byte servoCommand(byte *cmd) override;

        byte setAngle(byte id, byte angle, byte minor) override;

        // Methods can be overrided (optional)
        //
        void initBus() override;
        bool initServo(byte id)  override;
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

        // uint16_t getAngle(byte id) override;
        // uint16_t getAngle(byte id, bool lockAfterGet) override;


    private:

        String _buffer;
        byte _retBuf[HL_RETURN_BUFFER_SIZE];  
        byte _retCnt = 0;
        
        inline bool sendCommand() { return sendCommand(true); }
        bool sendCommand(bool expectReturn, unsigned long waitMs = HL_COMMAND_WAIT_TIME);
        bool sendUntilOK(unsigned long waitMs = HL_COMMAND_WAIT_TIME);

        void showCommand();
        bool checkReturn(unsigned long waitMs = HL_COMMAND_WAIT_TIME);
        bool isReturnOK();
        void resetReturnBuffer();

        inline bool getRetNum(int16_t *data, byte start, byte len) { return getRetNum(data, start, len, len); }
        bool getRetNum(int16_t *data, byte start, byte len, byte minLen); 

        byte getServoMode(int id);

};

#endif
