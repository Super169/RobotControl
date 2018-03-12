#ifndef _UBTSERVO_H_
#define _UBTSERVO_H_

#include <ESP8266WiFi.h>
#include "HardwareSerial.h"
#include "SoftwareSerial.h"

#define SERVO_BAUD 115200
#define MAX_SERVO_ID 16

#define COMMAND_BUFFER_SIZE 10
#define RETURN_BUFFER_SIZE 	20  // Actually, 10 is enough, just for saftey

#define COMMAND_WAIT_TIME		400
#define DEFAULT_RETRY_GETPOS	3

const byte SERVO_CMD[] = {0xFA, 0xAF,0x00,0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0xED};
const byte JIMU_VERSION[] = {0xFC, 0xCF,0x00,0xAA,0x41, 0x16, 0x51, 0x01, 0x00, 0xED};

class UBTServo {
    public:
        UBTServo(SoftwareSerial *ssData);
		UBTServo(SoftwareSerial *ssData, HardwareSerial *hsDebug);
		UBTServo(byte id, SoftwareSerial *ssData, HardwareSerial *hsDebug);
        ~UBTServo();
		bool setDebug(bool debug);
        void begin();
        void end();
        void getVersion(byte id);
        void move(byte id, byte angle, byte time);
		byte lock(byte id) { return getPos(id, true); }
		byte unlock(byte id) { return getPos(id, false); }
        byte getPos(byte id) { return getPos(id, _isLocked[id], DEFAULT_RETRY_GETPOS); }
		byte getPos(byte id, bool lockAfterGet) { return getPos(id, lockAfterGet, DEFAULT_RETRY_GETPOS); }
		byte getPos(byte id, bool lockAfterGet, int retryCount);
        void setLED(byte id, byte mode);
		inline void setLedOn(byte id) { setLED(id, 0); }
		inline void setLedOff(byte id) { setLED(id, 1); }
		inline void detectServo() { detectServo(1, MAX_SERVO_ID); }
		inline void detectServo(byte max) { detectServo(1, max); }
		void detectServo(byte min, byte max);
		bool exists(byte id);
		byte* retBuffer() { return _retBuf; }
		byte retCount() { return _retCnt; }
		bool isLocked(byte id) { return _isLocked[id]; }
		byte lastAngle(byte id) { return (_isLocked ? _lastAngle[id] : 0xFF); }
		bool isServo(byte id) { return _isServo[id]; }
		int execute(byte cmd[], byte result[]);
		uint16 getAdjAngle(byte id);
		uint16 setAdjAngle(byte id, uint16 adjValue);

    private:
        void initObject(byte id, SoftwareSerial *ssData, HardwareSerial *hsDebug);
        inline bool sendCommand() { sendCommand(true); }
        bool sendCommand(bool expectReturn);
        void showCommand();
        bool checkReturn();
        inline void resetCommandBuffer() { memcpy(_buf, SERVO_CMD, COMMAND_BUFFER_SIZE); }
        inline void resetReurnBuffer() { memset(_retBuf, 0, RETURN_BUFFER_SIZE); _retCnt = 0; }

		byte _id;

        SoftwareSerial *_ss;
        HardwareSerial *_hs;
        bool _enableDebug;
        byte _buf[COMMAND_BUFFER_SIZE];

        byte _retBuf[RETURN_BUFFER_SIZE];  
        byte _retCnt;
		bool _servo[MAX_SERVO_ID + 1];
		bool _isLocked[MAX_SERVO_ID + 1];
		byte _lastAngle[MAX_SERVO_ID + 1];
		bool _isServo[MAX_SERVO_ID + 1];
		uint16 _adjAngle[MAX_SERVO_ID + 1];
};

#endif
