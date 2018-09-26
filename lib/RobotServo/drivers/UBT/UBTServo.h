#ifndef _UBT_Servo_H_
#define _UBT_Servo_H_

#include "baseServo.h"

#define UBT_COMMAND_BUFFER_SIZE 10
#define UBT_RETURN_BUFFER_SIZE 	20  // Actually, 10 is enough, just for saftey
#define UBT_COMMAND_WAIT_TIME	5   // 2ms is mroe than enough as it should be returned within 400us

const byte UBT_SERVO_CMD[] = {0xFA, 0xAF,0x00,0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0xED};
// const byte JIMU_VERSION[] = {0xFC, 0xCF,0x00,0xAA,0x41, 0x16, 0x51, 0x01, 0x00, 0xED};


class UBTServo : public baseServo
{
    public:

        UBTServo();
        ~UBTServo();

        // Methos MUST be implemented
        //
        // Methods without default implementation, MUST be overrided
        byte servoType() override { return BS_TYPE_UBT; }

        bool reset() override;
        uint32_t getVersion(byte id) override;

        bool resetServo() override { return false; }
        bool resetServo(byte id) override { return false; }

        bool move(byte id, int16_t pos, uint16_t time) override;
        bool setLED(byte id, bool mode) override;
        bool setLED(bool mode) override { return setLED(0, mode); }

        int16_t getPos(byte id) override;
        int16_t getPos(byte id, bool lockAfterGet) override;

        // bool isLocked(byte id) override;
        bool lock(byte id) override;
        bool unlock(byte id) override;

        uint16_t getAdjAngle(byte id) override;
        uint16_t setAdjAngle(byte id, uint16 adjValue) override;
        byte servoCommand(byte *cmd) override { return 0; }

        byte setAngle(byte id, byte angle, byte minor) override { return 0; }

        // Methods can be overrided (optional)
        //
        // void initBus() override;
        void showInfo() override;

        // bool validId(byte id) override;
        // bool validPos(int16_t pos) override;

        // bool moveX(byte cnt, byte *data) override;

        // bool isLocked(byte id) override;
        // bool lockAll() override;
        // bool unlockAll() override;
        
        // bool moveX(byte cnt, byte *data) override;
        // bool goAngle(byte id, int16_t angle, uint16_t time) override;

        // uint16_t lastPos(byte id)  override;
        // uint16_t lastAngle(byte id) override;

        // uint16_t getAngle(byte id) override;
        // uint16_t getAngle(byte id, bool lockAfterGet) override;


    private:
        
        byte _buf[UBT_COMMAND_BUFFER_SIZE];
        byte _retBuf[UBT_RETURN_BUFFER_SIZE];  
        byte _retCnt = 0;

        inline bool sendCommand() { return sendCommand(true); }
        bool sendCommand(bool expectReturn);
        void showCommand();
        bool checkReturn();
        inline void resetCommandBuffer();
        inline void resetReturnBuffer();

};

#endif
