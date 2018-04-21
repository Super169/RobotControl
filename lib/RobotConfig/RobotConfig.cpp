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
    if (!SPIFFS.exists(_configFileName)) return false;

    File configFile = SPIFFS.open(_configFileName, "r");
    if (!configFile) return false;

    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(configFile);
    configFile.close();

    if (!json.success()) return false;

    initConfig();

    if (json.containsKey(JSON_MP3_VOLUME)) {
        _mp3_volume = json[JSON_MP3_VOLUME];
    }

    return true;


}

bool RobotConfig::writeConfig() {

    if (_enableDebug) _dbg->printf("writeConfig");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();

    json[JSON_MP3_VOLUME] = _mp3_volume;

    bool configSaved = false;
    File configFile = SPIFFS.open(_configFileName, "w");
    if (configFile) {

        #ifdef _SERIAL_DEBUG_
            json.printTo(DEBUG);
            DEBUG.println();
        #endif        
            
        json.printTo(configFile);
        configSaved = true;
    }
    configFile.close();    
    return configSaved;    

}