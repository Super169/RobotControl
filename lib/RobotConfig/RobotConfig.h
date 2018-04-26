#ifndef _ROBOT_CONFIG_H_
#define _ROBOT_CONFIG_H_

#include <ESP8266WiFi.h>
#include <FS.h>
#include <ArduinoJson.h>

#define DEFAULT_MAX_SERVO   16
#define DEFAULT_MAX_RETRY   2
#define DEFAULT_MP3_ENABLED true
#define DEFAULT_MP3_VOLUME  20

#define JSON_MAX_SERVO      "maxServo"
#define JSON_MAX_RETRY      "maxRetry"
#define JSON_MP3_ENABLED    "mp3Enabled"
#define JSON_MP3_VOLUME     "mp3Volume"

class RobotConfig {

    const char* _configFileName = "/robot/config.json";

    public:
        RobotConfig();
        RobotConfig(HardwareSerial *hsDebug);
        void initConfig();
        bool setDebug(bool debug);
        bool readConfig();
        bool writeConfig();

        void setMaxServo(uint8_t maxServo);
        void setMaxRetry(uint8_t maxRetry);
        void setMp3Enabled(bool enabled);
        void setMp3Volume(uint8_t volume);

        uint8_t maxServo() { return _max_servo; }
        uint8_t maxRetry() { return _max_retry; }
        bool mp3Enabled() { return _mp3_enabled; }
        uint8_t mp3Volume() { return _mp3_volume; }

    private:
        void initObject(HardwareSerial *hsDebug);
        bool _enableDebug;
        HardwareSerial *_dbg;

        uint8_t _max_servo;
        uint8_t _max_retry;
        bool _mp3_enabled;
        uint8_t _mp3_volume;

};


#endif