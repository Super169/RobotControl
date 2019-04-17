#ifndef _MP3_TF_16P_H_
#define _MP3_TF_16P_H_

#include <ESP8266WiFi.h>
#include "HardwareSerial.h"
#include "SoftwareSerial.h"

#define MP3_BAUD 9600
#define MP3_COMMAND_BUFFER_SIZE 10
#define MP3_RETURN_BUFFER_SIZE 	20  // Actually, 10 is enough, just for saftey

#define MP3_COMMAND_WAIT_TIME	500

const byte MP3_CMD[] = {0x7E, 0xFF, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF};

class MP3TF16P {
    public:
        MP3TF16P(SoftwareSerial *ssData);
        MP3TF16P(SoftwareSerial *ssData, HardwareSerial *hsDebug);
        ~MP3TF16P();
		bool setDebug(bool debug);
        void begin();
        void end();
        void sendSingleByteCommand(byte cmd);
        void sendCommand(byte cmd, byte p1, byte p2);
        void sendCommand(byte cmd, uint16_t parm);
        inline void playNext()      { sendSingleByteCommand(0x01); }
        inline void playPrev()      { sendSingleByteCommand(0x02); }
        void playFile(uint16_t seq);    // 0x03
        void setVol(byte vol);      // 0x06
        inline void resetDevice()   { sendSingleByteCommand(0x0C); }
        inline void play()          { sendSingleByteCommand(0x0D); }
        inline void pause()         { sendSingleByteCommand(0x0E); }
        void playFolderFile(byte folder, byte seq); // 0x0F
        inline void playAllLoop()   { sendSingleByteCommand(0x11); }
        void playMp3File(uint16_t seq); // 0x12
        void playAdFile(byte seq);  // 0x13
        inline void stopPlayAd()    { sendSingleByteCommand(0x15); }
        inline void stop()          { sendSingleByteCommand(0x16); }
        inline void playRandom()    { sendSingleByteCommand(0x18); }
        uint8_t getVol();
        void adjVol(int diff);
        inline void volUp()         { adjVol(1); }
        inline void volDown()       { adjVol(-1); }


    private:
        void initObject(SoftwareSerial *ssData, HardwareSerial *hsDebug);
        inline void resetCommandBuffer() { memcpy(_buf, MP3_CMD, MP3_COMMAND_BUFFER_SIZE); }
        inline void resetReturnBuffer() { memset(_retBuf, 0, MP3_RETURN_BUFFER_SIZE); _retCnt = 0; }
        inline bool sendCommand() { return sendCommand(false); }
        bool sendCommand(bool expectReturn);
        void showCommand();
        void clearRxBuffer();
        bool checkReturn();
        SoftwareSerial *_ss;
        HardwareSerial *_dbg;
        bool _enableDebug;
        byte _buf[MP3_COMMAND_BUFFER_SIZE];
        byte _retBuf[MP3_RETURN_BUFFER_SIZE];  
        byte _retCnt;
        byte _vol = 0xFF;

};

#endif