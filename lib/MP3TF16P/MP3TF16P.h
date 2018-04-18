#ifndef _MP3_TF_16P_H_
#define _MP3_TF_16P_H_

#include <ESP8266WiFi.h>
#include "HardwareSerial.h"
#include "SoftwareSerial.h"

#define MP3_BAUD 9600
#define COMMAND_BUFFER_SIZE 10

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
        inline void playNext()      { sendSingleByteCommand(0x01); }
        inline void playPrev()      { sendSingleByteCommand(0x02); }
        void playFile(byte seq);    // 0x03
        void setVol(byte vol);      // 0x06
        inline void resetDevice()   { sendSingleByteCommand(0x0C); }
        inline void play()          { sendSingleByteCommand(0x0D); }
        inline void pause()         { sendSingleByteCommand(0x0E); }
        void playFolderFile(byte folder, byte seq); // 0x0F
        inline void playAllLoop()   { sendSingleByteCommand(0x11); }
        void playMp3File(byte seq); // 0x12
        void playAdFile(byte seq);  // 0x13
        inline void stopPlayAd()    { sendSingleByteCommand(0x15); }
        inline void stop()          { sendSingleByteCommand(0x16); }
        inline void playRandom()    { sendSingleByteCommand(0x18); }


    private:
        void initObject(SoftwareSerial *ssData, HardwareSerial *hsDebug);
        inline void resetCommandBuffer() { memcpy(_buf, MP3_CMD, COMMAND_BUFFER_SIZE); }
        inline bool sendCommand() { sendCommand(false); }
        bool sendCommand(bool expectReturn);
        void showCommand();
        bool checkReturn();
        SoftwareSerial *_ss;
        HardwareSerial *_dbg;
        bool _enableDebug;
        byte _buf[COMMAND_BUFFER_SIZE];

};

#endif