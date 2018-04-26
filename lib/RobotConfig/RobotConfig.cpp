#include "RobotConfig.h"

RobotConfig::RobotConfig() {
    initObject(NULL);
}


RobotConfig::RobotConfig(HardwareSerial *hsDebug) {
    initObject(hsDebug);
}

void RobotConfig::initObject(HardwareSerial *hsDebug) {
    initConfig();
    _dbg = hsDebug;
    _enableDebug = (_dbg != NULL);
}

void RobotConfig::initConfig() {

    _max_servo = DEFAULT_MAX_SERVO;
    _max_retry = DEFAULT_MAX_RETRY;

    _mp3_enabled = DEFAULT_MP3_ENABLED;
    _mp3_volume = DEFAULT_MP3_VOLUME;


}

bool RobotConfig::setDebug(bool debug) {
	if (_dbg == NULL) return false;
	_enableDebug = debug;
	return _enableDebug;
}

bool RobotConfig::readConfig() {

    if (_enableDebug) _dbg->printf("readConfig");

    if (!SPIFFS.begin()) return false;
    if (!SPIFFS.exists(_configFileName)) {
        if (_enableDebug) _dbg->printf("#### config file %s not found\n", _configFileName);
        return false;
    }
    File configFile = SPIFFS.open(_configFileName, "r");
    if (!configFile) return false;

    if (_enableDebug) _dbg->printf("#### Read from: %s\n", _configFileName);

    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(configFile);
    configFile.close();

    if (!json.success()) return false;

    initConfig();


    if (json.containsKey(JSON_MAX_SERVO)) {
        _max_servo = json[JSON_MAX_SERVO];
    }

    if (json.containsKey(JSON_MAX_RETRY)) {
        _max_retry = json[JSON_MAX_RETRY];
    }

    if (json.containsKey(JSON_MP3_ENABLED)) {
        _mp3_enabled = json[JSON_MP3_ENABLED];
    }

    if (json.containsKey(JSON_MP3_VOLUME)) {
        _mp3_volume = json[JSON_MP3_VOLUME];
    }

    return true;


}

bool RobotConfig::writeConfig() {

    if (_enableDebug) _dbg->printf("writeConfig");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();

    json[JSON_MAX_SERVO] = _max_servo;
    json[JSON_MAX_RETRY] = _max_retry;
    json[JSON_MP3_ENABLED] = _mp3_enabled;
    json[JSON_MP3_VOLUME] = _mp3_volume;

    bool configSaved = false;
    File configFile = SPIFFS.open(_configFileName, "w");
    if (configFile) {

        if (_enableDebug) _dbg->printf("#### Write to: %s\n", _configFileName);
        json.printTo(configFile);
        configSaved = true;
    }
    configFile.close();    
    return configSaved;    

}

void RobotConfig::setMaxServo(uint8_t maxServo) {
    _max_servo = maxServo;
}

void RobotConfig::setMaxRetry(uint8_t maxRetry) {
    _max_retry = maxRetry;
}

void RobotConfig::setMp3Enabled(bool enabled) {
    _mp3_enabled = enabled;
}

void RobotConfig::setMp3Volume(uint8_t volume) {
    _mp3_volume = volume;
}