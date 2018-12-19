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
    
    setMaxServo(DEFAULT_MAX_SERVO);
    setMaxDetectRetry(DEFAULT_MAX_DETECT_RETRY);
    setMaxCommandWaitMs(DEFAULT_MAX_COMMAND_WAIT_MS);
    setMaxCommandRetry(DEFAULT_MAX_COMMAND_RETRY);

    setBattery(DEFAULT_BATTERY_REF_VOLTAGE, DEFAULT_BATTERY_MIN_VALUE, DEFAULT_BATTERY_MAX_VALUE, DEFAULT_BATTERY_NORMAL_SEC, DEFAULT_BATTERY_ALARM_SEC);
    setTouch(DEFAULT_TOUCH_ENABLED, DEFAULT_TOUCH_DETECT_PERIOD, DEFAULT_TOUCH_RELEASE_PERIOD);
    setMp3(DEFAULT_MP3_ENABLED, DEFAULT_MP3_VOLUME, DEFAULT_MP3_STARTUP);
    setMpu(DEFAULT_MPU_ENABLED, DEFAULT_MPU_CHECK_FREQ, DEFAULT_MPU_POSITION_CHECK_FREQ);
    setPsx(DEFAULT_PSX_ENABLED, DEFAULT_PSX_CHECK_MS, DEFAULT_PSX_IGNORE_REPEAT_MS);

}

bool RobotConfig::readConfig() {

    if (enableDebug()) _dbg->printf("RobotConfig::readConfig\n");

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

    checkConversion();
    checkConfig();

    return true;

}

void RobotConfig::checkConversion() {
    if (_data[RC_VERSION] == CURRENT_VERSION) return;

    // Conversion from V0
    if (_data[RC_VERSION] == 0) {
    	if (_dbg != NULL) {
            _dbg->printf("Conversion from  V0 configuration file\n");
        }

        // Default battery noraml check frequency in 5 seconds
        _data[RC_BATTERY_NORAML_SEC] = 5;

        _data[RC_TOUCH_ENABLED] = _data[V0_ENABLE_TOUCH];

        _data[RC_MPU_CHECK_FREQ] = _data[V0_MPU_CHECK_FREQ];
        _data[RC_MPU_POSITION_CHECK_FREQ] = _data[V0_POSITION_CHECK_FREQ];
        _data[RC_VERSION] = CURRENT_VERSION;
    }
}

void RobotConfig::checkConfig() {
    
    // Just for safety, to check for possible invalid interval to prevent continue trigger event
   
    if (touchDetectPeriod() < MIN_TOUCH_PERIOD) setTouchDetectPeriod(DEFAULT_TOUCH_DETECT_PERIOD);
    if (touchReleasePeriod() < MIN_TOUCH_PERIOD) setTouchReleasePeriod(DEFAULT_TOUCH_RELEASE_PERIOD);


    // if (voltageAlarmInterval() < 10) setVoltageAlarmInterval(DEFAULT_ALARM_INTERVAL);

    if ((mpuCheckFreq() < MIN_BATTERY_CHECK_FREQ) || (mpuCheckFreq() > MAX_BATTERY_CHECK_FREQ)) {
        setMpuCheckFreq(DEFAULT_MPU_CHECK_FREQ);
    }

    if ((positionCheckFreq() < MIN_BATTERY_CHECK_FREQ) || (positionCheckFreq() > MAX_BATTERY_CHECK_FREQ)) {
        setMpuPositionCheckFreq(DEFAULT_MPU_POSITION_CHECK_FREQ);
    } 
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


void RobotConfig::dumpConfig() {
	if (_dbg == NULL) return;
	_dbg->printf("\n\nRobot Config V%d:\n", _data[RC_VERSION]);
	_dbg->printf("Debug: %s\n", (enableDebug() ? "Enabled" : "Disabled"));
    _dbg->printf("Maximum Servo: %d, Detection Retry: %d\n", maxServo(), maxDetectRetry());
    _dbg->printf("Default command wait(ms): %d, Maximum Retry: %d\n", maxCommandWaitMs(), maxCommandRetry());

    _dbg->printf("Battery ref: %d, min value: %d, max value: %d\n",
                 batteryRefVoltage(), batteryMinValue(), batteryMaxValue());
    _dbg->printf("Battery normal check: %d s, alarm check %d s\n",
                 batteryNormalSec(), batteryAlarmSec());

	_dbg->printf("Router: %s\n", (connectRouter() ? "Enabled" : "Disabled"));
 	_dbg->printf("OLED: %s\n", (enableOLED() ? "Enabled" : "Disabled"));
 	_dbg->printf("Touch Sensor: %s, Detect Period: %d, Release Period: %d\n", 
                  (enableTouch() ? "Enabled" : "Disabled"), touchDetectPeriod(), touchReleasePeriod());

 	_dbg->printf("MPU6050: %s, Check Freq: %d, Position Confirm Freq: %d\n", 
                  (enableMpu() ? "Enabled" : "Disabled"), mpuCheckFreq(), positionCheckFreq());

 	_dbg->printf("PSX Button: %s, Check Interval: %d, Ignore Repeat Interval: %d\n", 
                  (enablePsxButton() ? "Enabled" : "Disabled"), psxCheckMs(), psxIgnoreRepeatMs());

	_dbg->println();
}

bool RobotConfig::setDebug(bool debug) {
	if (_dbg == NULL) debug = false; 
    _data[RC_ENABLE_DEBUG] = debug;
	return enableDebug();
}

bool RobotConfig::setRouter(bool value) {
    _data[RC_CONNECT_ROUTER] = value;
	return connectRouter();
}

bool RobotConfig::setOLED(bool value) {
    _data[RC_ENABLE_OLED] = value;
	return enableOLED();
}

bool RobotConfig::setTouchEnabled(bool value) {
    _data[RC_TOUCH_ENABLED] = value;
	return enableTouch();
}

/*
void RobotConfig::setTouchAction(uint8_t id, uint8_t value) {
	if (id >= RC_TOUCH_ACTION_CNT) return;
	_data[RC_TOUCH_ACTION + id] = value;
}
*/

bool RobotConfig::enableTouch() {
	if (!_data[RC_TOUCH_ENABLED]) return false;
    return true;  // For new event handler, touch enabled if events required.
    /*
	bool enabled = false;
	for (uint8_t id = 0; id < RC_TOUCH_ACTION_CNT; id++) {
		enabled |= _data[RC_TOUCH_ACTION + id];
	}
	return enabled;
    */
}

/*
uint8_t RobotConfig::touchAction(uint8_t id) {
	if (id >= RC_TOUCH_ACTION_CNT) return DEFAULT_TOUCH_NO_ACTION;
	return (_data[RC_TOUCH_ACTION + id]);
}
*/

void RobotConfig::setTouchDetectPeriod(uint16_t detectPeriod) {
    setUint16_t(RC_TOUCH_DETECT_PERIOD, detectPeriod);
}

void RobotConfig::setTouchReleasePeriod(uint16_t releasePeriod) {
    setUint16_t(RC_TOUCH_RELEASE_PERIOD, releasePeriod);
}

void RobotConfig::setBatteryRefVoltage(uint16_t refVoltage) {    
    setUint16_t(RC_BATTERY_REF_VOLTAGE, refVoltage);
}

void RobotConfig::setBatteryMinValue(uint16_t minVoltage) {    
    setUint16_t(RC_BATTERY_MIN_VALUE, minVoltage);
}

void RobotConfig::setBatteryMaxValue(uint16_t maxVoltage) {    
    setUint16_t(RC_BATTERY_MAX_VALUE, maxVoltage);
}

void RobotConfig::setBatteryNormalSec(uint8_t value) {
    _data[RC_BATTERY_NORAML_SEC] = value;
}

void RobotConfig::setBatteryAlarmSec(uint8_t value) {
    _data[RC_BATTERY_ALARM_SEC] = value;
}

void RobotConfig::setBattery(uint16_t refVoltage, uint16_t minVoltage, uint16_t maxVoltage, uint8_t normalSec, uint8_t alarmSec)  {
    setBatteryRefVoltage(refVoltage); 
    setBatteryMinValue(minVoltage); 
    setBatteryMaxValue(maxVoltage); 
    setBatteryNormalSec(normalSec);
    setBatteryAlarmSec(alarmSec);
}


/*
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
    setBatteryRefVoltage(refVoltage); 
    setBatteryMinValue(minVoltage); 
    setBatteryMaxValue(maxVoltage); 
    setAlarmVoltage(alarmVoltage);
    setVoltageAlarmMp3(mp3);
    setVoltageAlarmInterval(interval);
};
*/

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

void RobotConfig::setMpuEnabled(bool enabled) {
    _data[RC_MPU_ENABLED] = enabled;
}

/*
void RobotConfig::setAutoStand(bool autoStand) {
    _data[RC_AUTO_STAND] = autoStand;
}

void RobotConfig::setAutoFaceUp(uint8_t faceUp) {
    _data[RC_AUTO_FACE_UP] = faceUp;
}

void RobotConfig::setAutoFaceDown(uint8_t faceDown) {
    _data[RC_AUTO_FACE_DOWN] = faceDown;
}
*/

void RobotConfig::setMpuCheckFreq(uint8_t checkFreq) {
    _data[RC_MPU_CHECK_FREQ] = checkFreq;
}

void RobotConfig::setMpuPositionCheckFreq(uint8_t positionCheckFreq) {
    _data[RC_MPU_POSITION_CHECK_FREQ] = positionCheckFreq;
}


void RobotConfig::setPsxEnabled(bool enabled) {
    _data[RC_PSX_ENABLED] = enabled;
}

void RobotConfig::setPsxCheckMs(uint16_t value) {
    setUint16_t(RC_PSX_CHECK_MS, value);
}

void RobotConfig::setPsxIgnoreRepeatMs(uint16_t value) {
    setUint16_t(RC_PSX_IGNORE_REPEAT_MS, value);
}



