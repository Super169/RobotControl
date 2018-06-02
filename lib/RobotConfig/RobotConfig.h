#ifndef _ROBOT_CONFIG_H_
#define _ROBOT_CONFIG_H_

#include <ESP8266WiFi.h>
#include <FS.h>


#define DEFAULT_ENABLE_DEBUG    false
#define DEFAULT_REF_VOLTAGE     3200        // aroud 3.2v for A0 of D1 mini, 1.0v for ADC of ESP-12
#define DEFAULT_MIN_VOLTAGE     220
#define DEFAULT_MAX_VOLTAGE     246
#define DEFAULT_ALARM_VOLTAGE   225
#define DEFAULT_MAX_SERVO       16

#define DEFAULT_MAX_DETECT_RETRY        2
#define DEFAULT_MAX_COMMAND_WAIT_MS     2
#define DEFAULT_MAX_COMMAND_RETRY       10

#define DEFAULT_MP3_ENABLED     true
#define DEFAULT_MP3_VOLUME      20

#define RC_DATA_SIZE            64
#define RC_VERSION              0
#define RC_ENABLE_DEBUG         1
#define RC_REF_VOLTAGE          10
#define RC_MIN_VOLTAGE          12
#define RC_MAX_VOLTAGE          14
#define RC_ALARM_VOLTAGE        16
#define RC_MAX_SERVO            21
#define RC_MAX_DETECT_RETRY     22
#define RC_MAX_COMMAND_WAIT_MS  23
#define RC_MAX_COMMAND_RETRY    24
#define RC_MP3_ENABLED          31
#define RC_MP3_VOLUME           32


class RobotConfig {

    const char* _configFileName = "/robot/config.dat";

    public:
        RobotConfig();
        RobotConfig(HardwareSerial *hsDebug);
        void initConfig();
        bool readConfig();
        bool writeConfig();

        bool setDebug(bool debug);
        void setRefVoltage(uint16_t refVoltage); 
        void setMinVoltage(uint16_t minVoltage); 
        void setMaxVoltage(uint16_t maxVoltage); 
        void setAlarmVoltage(uint16_t alarmVoltage); 
        void setVoltage(uint16_t refVoltage, uint16_t minVoltage, uint16_t maxVoltage, uint16_t alarmVoltage);
        void setMaxServo(uint8_t maxServo);
        void setMaxCommandWaitMs(uint8_t maxCommandWaitMs);
        void setMaxCommandRetry(uint8_t maxCommandRetry);
        void setMaxDetectRetry(uint8_t maxDetectRetry);
        void setMp3Enabled(bool enabled);
        void setMp3Volume(uint8_t volume);

        bool enableDebug() { return _data[RC_ENABLE_DEBUG]; }
        uint16_t refVoltage() { return getUint16_t(RC_REF_VOLTAGE); }
        uint16_t minVoltage() { return getUint16_t(RC_MIN_VOLTAGE); }
        uint16_t maxVoltage() { return getUint16_t(RC_MAX_VOLTAGE); }
        uint16_t alarmVoltage() { return getUint16_t(RC_ALARM_VOLTAGE); }
        uint8_t maxServo() { return _data[RC_MAX_SERVO]; }
        uint8_t maxDetectRetry() { return _data[RC_MAX_DETECT_RETRY]; }
        uint8_t maxCommandWaitMs() { return _data[RC_MAX_COMMAND_WAIT_MS]; }
        uint8_t maxCommandRetry() { return _data[RC_MAX_COMMAND_RETRY]; }
        bool mp3Enabled() { return _data[RC_MP3_ENABLED]; }
        uint8_t mp3Volume() { return _data[RC_MP3_VOLUME]; }

    private:
        void initObject(HardwareSerial *hsDebug);
        void setUint16_t(byte offset, uint16_t value);
        uint16_t getUint16_t(byte offset);

        HardwareSerial *_dbg;
        
        byte _data[RC_DATA_SIZE];

};


#endif