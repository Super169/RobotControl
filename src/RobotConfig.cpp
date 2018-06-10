#include "RobotConfig.h"

RobotConfig::RobotConfig() {
    initObject(NULL);
}


RobotConfig::RobotConfig(HardwareSerial *hsDebug) {
    initObject(hsDebug);
}

void RobotConfig::setUint16_t(uint8_t offset, uint16_t value) {
    _data[offset] = (value >> 8);
    _data[offset+1] = (value & 0xFF);  // just for safety, it can be assigned directly
}

uint16_t RobotConfig::getUint16_t(uint8_t offset) {
    uint16_t value = 0;
    value = (_data[offset] << 8) | _data[offset+1];
    return value;
}

void RobotConfig::initObject(HardwareSerial *hsDebug) {
    initConfig();
    _dbg = hsDebug;
    setDebug((_dbg != NULL));
}

void RobotConfig::initConfig() {

    memset(_data, 0, RC_RECORD_SIZE);

    _data[0] = 0xA9;
    _data[1] = 0x9A;
    _data[2] = RC_CONFIG_DATA_SIZE;
    _data[RC_RECORD_SIZE-1] = 0xED;
    
    setDebug(DEFAULT_ENABLE_DEBUG);
    setRouter(DEFAULT_CONNECT_ROUTER);
    setOLED(DEFAULT_ENABLE_OLED);
    
    setVoltage(DEFAULT_REF_VOLTAGE, DEFAULT_MIN_VOLTAGE, DEFAULT_MAX_VOLTAGE, DEFAULT_ALARM_VOLTAGE, DEFAULT_ALARM_MP3, DEFAULT_ALARM_INTERVAL);

    setMaxServo(DEFAULT_MAX_SERVO);
    setMaxDetectRetry(DEFAULT_MAX_DETECT_RETRY);
    setMaxCommandWaitMs(DEFAULT_MAX_COMMAND_WAIT_MS);
    setMaxCommandRetry(DEFAULT_MAX_COMMAND_RETRY);

    setMp3Enabled(DEFAULT_MP3_ENABLED);
    setMp3Volume(DEFAULT_MP3_VOLUME);
    setMp3Startup(DEFAULT_MP3_STARTUP);

    setAutoStand(DEFAULT_AUTO_STAND, DEFAULT_AUTO_FACE_UP, DEFAULT_AUTO_FACE_DOWN);

}

bool RobotConfig::readConfig() {

    if (enableDebug()) _dbg->printf("readConfig");

    if (!SPIFFS.begin()) return false;
    
    if (!SPIFFS.exists(_configFileName)) {
        if (enableDebug()) _dbg->printf("#### config file %s not found\n", _configFileName);
        return false;
    }
    File configFile = SPIFFS.open(_configFileName, "r");
    if (!configFile) return false;

    if (enableDebug()) _dbg->printf("#### Read from: %s\n", _configFileName);

    // * partial read is allowed

    initConfig();
	size_t cnt = configFile.readBytes((char *)_data, RC_RECORD_SIZE);    

    configFile.close();

    // make sure the header information is filled
    _data[0] = 0xA9;
    _data[1] = 0x9A;
    _data[2] = RC_CONFIG_DATA_SIZE;
    _data[RC_RECORD_SIZE-1] = 0xED;

    return true;

}

byte RobotConfig::writeConfig() {

    if (enableDebug()) _dbg->printf("writeConfig");

    if (!SPIFFS.begin()) return RESULT::ERR::SPIFFS;

    byte configSaved = RESULT::ERR::UNKNOWN;
    File configFile = SPIFFS.open(_configFileName, "w");
    if (configFile) {

        if (enableDebug()) _dbg->printf("#### Write to: %s\n", _configFileName);
        size_t cnt = configFile.write((uint8_t *) _data, RC_RECORD_SIZE);
        configFile.close();
        configSaved = (cnt == RC_RECORD_SIZE ? RESULT::SUCCESS : RESULT::ERR::FILE_WRITE_COUNT);
    }

    SPIFFS.end();
    return configSaved;    

}

bool RobotConfig::setDebug(bool debug) {
	if (_dbg == NULL) debug = false; 
    _data[RC_ENABLE_DEBUG] = debug;
	return debug;
}

bool RobotConfig::setRouter(bool router) {
    _data[RC_CONNECT_ROUTER] = router;
	return router;
}

bool RobotConfig::setOLED(bool oled) {
    _data[RC_ENABLE_OLED] = oled;
	return oled;
}

void RobotConfig::setRefVoltage(uint16_t refVoltage) {    
    setUint16_t(RC_REF_VOLTAGE, refVoltage);
}

void RobotConfig::setMinVoltage(uint16_t minVoltage) {    
    setUint16_t(RC_MIN_VOLTAGE, minVoltage);
}

void RobotConfig::setMaxVoltage(uint16_t maxVoltage) {    
    setUint16_t(RC_MAX_VOLTAGE, maxVoltage);
}

void RobotConfig::setAlarmVoltage(uint16_t alarmVoltage) {    
    setUint16_t(RC_ALARM_VOLTAGE, alarmVoltage);
}

void RobotConfig::setVoltageAlarmMp3(uint8_t mp3) {    
    setUint16_t(RC_ALARM_MP3, mp3);
}

void RobotConfig::setVoltageAlarmInterval(uint8_t interval) {    
    setUint16_t(RC_ALARM_MP3, interval);
}

void RobotConfig::setVoltage(uint16_t refVoltage, uint16_t minVoltage, uint16_t maxVoltage, uint16_t alarmVoltage, uint8_t mp3, uint8_t interval) { 
    setRefVoltage(refVoltage); 
    setMinVoltage(minVoltage); 
    setMaxVoltage(maxVoltage); 
    setAlarmVoltage(alarmVoltage);
    setVoltageAlarmMp3(mp3);
    setVoltageAlarmInterval(interval);
};


void RobotConfig::setMaxServo(uint8_t maxServo) {
    _data[RC_MAX_SERVO] = maxServo;
}

void RobotConfig::setMaxDetectRetry(uint8_t maxDetectRetry) {
    _data[RC_MAX_DETECT_RETRY] = maxDetectRetry;
}

void RobotConfig::setMaxCommandWaitMs(uint8_t maxCommandWaitMs) {
    _data[RC_MAX_COMMAND_WAIT_MS] = maxCommandWaitMs;
}

void RobotConfig::setMaxCommandRetry(uint8_t maxCommandRetry) {
    _data[RC_MAX_COMMAND_RETRY] = maxCommandRetry;
}

void RobotConfig::setMp3Enabled(bool enabled) {
    _data[RC_MP3_ENABLED] = enabled;
}

void RobotConfig::setMp3Volume(uint8_t volume) {
    _data[RC_MP3_VOLUME] = volume;
}

void RobotConfig::setMp3Startup(uint8_t mp3) {
    _data[RC_MP3_STARTUP] = mp3;
}

void RobotConfig::setAutoStand(bool autoStand) {
    _data[RC_AUTO_STAND] = autoStand;
}

void RobotConfig::setAutoFaceUp(uint8_t faceUp) {
    _data[RC_AUTO_FACE_UP] = faceUp;
}

void RobotConfig::setAutoFaceDown(uint8_t faceDown) {
    _data[RC_AUTO_FACE_DOWN] = faceDown;
}
