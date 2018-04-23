#ifndef _ROBOT_CONFIG_H_
#define _ROBOT_CONFIG_H_

#include <ESP8266WiFi.h>
#include <FS.h>
#include <ArduinoJson.h>

#define DEFAULT_MP3_ENABLED true
#define DEFAULT_MP3_VOLUME  20

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
        bool setMp3Enabled(bool enabled);

        bool mp3Enabled() { return _mp3_enabled; }
        uint8_t mp3Volume() { return _mp3_volume; }

    private:
        void initObject(HardwareSerial *hsDebug);
        bool _enableDebug;
        HardwareSerial *_dbg;
        bool _mp3_enabled;
        uint8_t _mp3_volume;

};


#endif