#include "UBTServo.h"

UBTServo::UBTServo(SoftwareSerial *ssData) 
{
    initObject(0, ssData, NULL);
}

UBTServo::UBTServo(SoftwareSerial *ssData, HardwareSerial *hsDebug) 
{
	initObject(0, ssData, hsDebug);
}

UBTServo::UBTServo(byte id, SoftwareSerial *ssData, HardwareSerial *hsDebug) 
{
	initObject(id, ssData, hsDebug);
}

void UBTServo::initObject(byte id, SoftwareSerial *ssData, HardwareSerial *hsDebug) {
	_id = id;
    _ss = ssData;
	_hs = hsDebug;
	_enableDebug = (_hs != NULL);
}

UBTServo::~UBTServo() {
    _ss = NULL;
}

bool UBTServo::setDebug(bool debug) {
	if (_hs == NULL) return false;
	_enableDebug = debug;
	return _enableDebug;
}


void UBTServo::begin() {
    _ss->begin(SERVO_BAUD);
	delay(100);
	detectServo();
}


void UBTServo::end() {
    _ss->end();
	for (int id =1; id <= MAX_SERVO_ID; id++) {
		_servo[id] = false;
		_isServo[id] = true;
		_isLocked[id] = false;
		_lastAngle[id] = 0xFF;
		_adjAngle[id] = 0x7F7F;
	}	
}


void UBTServo::detectServo(byte min, byte max) {
	memset(_servo, 0, MAX_SERVO_ID + 1);
	for (int id = min; id <= max; id++) {
		getVersion(id);
		if ((_retCnt > 0) && (_retBuf[2] == id) && (_retBuf[3] == 0xAA)) {
			// new servo detected, assume unlock
			if (!_servo[id]) {
				_servo[id] = true;
				_isServo[id] = true;
				_isLocked[id] = false;
				_lastAngle[id] = 0xFF;
				_adjAngle[id] = 0x7F7F;
			}
		} else {
			_servo[id] = false;
		}
	}
}


bool UBTServo::exists(byte id) {
	if (id > MAX_SERVO_ID) return false;
	return _servo[id];
}


bool UBTServo::sendCommand(bool expectReturn) {
	byte sum = 0;
	for (int i = 2; i < 8; i++) {
		sum += _buf[i];
	}
	_buf[8] = sum;
	if (_enableDebug) showCommand();

	_ss->flush();
	_ss->enableTx(true);
	_ss->write(_buf, 10);
	_ss->enableTx(false);
	if (expectReturn) return checkReturn();
	
	return true;
}

void UBTServo::showCommand()  {
    if (!_enableDebug) return;
    _hs->print(millis());
    _hs->print(" OUT>>");
    for (int i = 0; i < 10; i++) {
        _hs->print( (_buf[i] < 0x10 ? " 0" : " "));
        _hs->print(_buf[i], HEX);
    }
    _hs->println();
}

bool UBTServo::checkReturn() {
    unsigned long startMs = millis();
    resetReurnBuffer();
    byte ch;
    while ( ((millis() - startMs) < COMMAND_WAIT_TIME) && (!_ss->available()) ) ;
    if (!_ss->available()) return false;
    if (_enableDebug) {
        _hs->print(millis());
        _hs->print(" IN>>>");
    }
    while (_ss->available()) {
        ch =  (byte) _ss->read();
        _retBuf[_retCnt++] = ch;
        if (_enableDebug) {
            _hs->print((ch < 0x10 ? " 0" : " "));
            _hs->print(ch, HEX);
        }
		// extra delay to make sure transaction completed 
		// ToDo: check data end
		if (!_ss->available()) delay(1);
    }
    if (_enableDebug) _hs->println();
    return true;
}

void UBTServo::getVersion(byte id) {
	resetCommandBuffer();
	_buf[0] = 0xFC;
	_buf[1] = 0xCF;
	_buf[2] = id;
	_buf[3] = 0x01;
	sendCommand();
}

void UBTServo::move(byte id, byte angle, byte time) {
	resetCommandBuffer();
	_buf[2] = id;
	_buf[3] = 0x01;
	_buf[4] = angle;
	_buf[5] = time;
	_buf[6] = 0x00;
	_buf[7] = time;  
	sendCommand();
	// servo will be locked after fire a move command
	_isLocked[id] = true;
	_lastAngle[id] = angle;
}

byte UBTServo::getPos(byte id, bool lockAfterGet, int retryCount) {
	int tryCnt = 0;
	if (retryCount < 1) retryCount = DEFAULT_RETRY_GETPOS;
	while (tryCnt++ < retryCount) {
		resetCommandBuffer();
		_buf[2] = id;
		_buf[3] = 0x02;
		sendCommand();
		if (_retCnt == 10) break;
	}
	if (_retCnt != 10) {
		// What can I do if it has not return the position
		return 0xFF;
	}

	byte angle = _retBuf[7];

	if (lockAfterGet) {
		move(id, angle, 0);
	} else {
		// servo will be unlocked after fire a get position command
		_isLocked[id] = false;
	}
	return angle;
}

uint16 UBTServo::getAdjAngle(byte id) {
	int tryCnt = 0;
	int retryCount = DEFAULT_RETRY_GETPOS;
	while (tryCnt++ < retryCount) {
		resetCommandBuffer();
		_buf[2] = id;
		_buf[3] = 0xD4;
		sendCommand();
		if (_retCnt == 10) break;
	}
	if (_retCnt != 10) {
		// What can I do if it has not return the position
		return 0x7F7F;
	}
	_adjAngle[id] = _retBuf[6] * 256 + _retBuf[7];
	return _adjAngle[id];
}

uint16 UBTServo::setAdjAngle(byte id, uint16 adjValue) {
	int tryCnt = 0;
	int retryCount = DEFAULT_RETRY_GETPOS;
	while (tryCnt++ < retryCount) {
		resetCommandBuffer();
		_buf[2] = id;
		_buf[3] = 0xD2;
		_buf[6] = adjValue / 256;
		_buf[7] = adjValue % 256;
		sendCommand();
		if (_retCnt == 10) break;
	}
	return getAdjAngle(id);
}


void UBTServo::setLED(byte id, byte mode) {
	resetCommandBuffer();
	_buf[2] = id;
	_buf[3] = 0x04;
	_buf[4] = mode;
	sendCommand();
}

int UBTServo::execute(byte cmd[], byte result[]) {
	resetCommandBuffer();
	memcpy(_buf, cmd, 8);
	sendCommand();
	if (!_retCnt) return false;
	memcpy(result, _retBuf, _retCnt);
	return _retCnt;

}