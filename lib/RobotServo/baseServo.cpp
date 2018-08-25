#include "baseServo.h"

baseServo::baseServo() {
}

baseServo::~baseServo() {

}

void baseServo::setEnableTxCalback(void (*enableTxCallback)(bool)) {
	_enableTxCallback = enableTxCallback;
}

void baseServo::enableTx(bool mode) {
	if (_enableTxCallback != NULL) _enableTxCallback(mode);
}


bool baseServo::begin(Stream *busPort, Stream *debugPort) {
	_bus = busPort;
	_dbg.setOutput(debugPort);
	_arrayReady = false;
	initBus();
	return true;
}

bool baseServo::end() {
	return true;
}

bool baseServo::init(byte maxId, byte maxRetry) {
	if (!_arrayReady) {
		_maxId = maxId;
		_maxRetry = maxRetry;
		
		int arraySize = _maxId + 1;
		_servo = new bool[arraySize];
		_led = new byte[arraySize];
		_isLocked = new bool[arraySize];
		_lastPos = new int16_t[arraySize];
		_isServo = new bool[arraySize];

		memset(_servo, false, arraySize);
		memset(_led, false, arraySize);
		memset(_isLocked, false, arraySize);
		for (int i = 0; i < arraySize; i++) _lastPos[i] = _INVALID_POS;
		memset(_isServo, false, arraySize);

		_servoCnt = 0;
		_arrayReady = true;
	}	
	return true;
}

void baseServo::enableDebug(bool value) {
	return _dbg.enableDebug(value);
}	

void baseServo::showInfo() {
    if (_dbg.isEnabled()) {
        if (_arrayReady) {
            _dbg.println("-- Setup is ready");
			_dbg.printf("-- Max ID: %d\n-- Max Retry: %d\n", _maxId, _maxRetry);
			_dbg.printf("-- Position: %d - %d [%d]  (%s)\n", _MIN_POS, _MAX_POS, _INVALID_POS,
												 		   (_MULTI_SERVO_COMMAND ? "allow multiple sevo command" : "single servo command only"));
			_dbg.printf("-- %d servo detected: \n", _servoCnt);
			if (_servoCnt) {
				for (byte id = 1; id <= _maxId; id++) {
					if (_servo[id]) {
						_dbg.printf("id: %02d (", id);
						if (validPos(_lastPos[id])) {
							_dbg.print(_lastPos[id]);
						} else {
							_dbg.print("Unknown");
						}
						_dbg.printf(") %s ; LED: %s \n", (_isLocked[id] ? "locked" : "unlocked"), (_led[id] ? "ON" : "OFF"));
					}
				}
				_dbg.println();
			} 
        } else {
            _dbg.println("-- Setup is not yet completed");
        }
    }	
}


bool baseServo::detectServo() {
	
	memset(_servo, 0, _maxId + 1);
	_servoCnt = 0;
	for (byte id = 1; id <= _maxId; id++) {
		if (getVersion(id)) {
			_servo[id] = true;
			_servoCnt++;
			initServo(id);
			getPos(id, true);
		}
	}
	return true;
}

bool baseServo::validId(byte id) {
	if (!_arrayReady) return false;
	if (id > _maxId) return false;
	return true;
}

bool baseServo::validPos(int16_t pos) {
	if (pos == _INVALID_POS) return false;
	if ((pos < _MIN_POS) || (pos > _MAX_POS)) return false;
	return true;
}

bool baseServo::isLocked(byte id) {
	if (!validId(id)) return false;
	if (_isLocked[id] && validPos(_lastPos[id])) return true;
	return false;
} 

bool baseServo::lockAll() {
	bool success = true;
	for (byte id = 1; id <= _maxId; id++) {
		if (_servo[id]) {
			success &= lock(id);
		}
	}
	return success;
}

bool baseServo::unlockAll() {
	bool success = true;
	for (byte id = 1; id <= _maxId; id++) {
		if (_servo[id]) {
			success &= unlock(id);
		}
	}
	return success;
}


bool baseServo::moveX(byte cnt, byte *data)  {
	if (cnt == 0) return true;
	int moveCnt = 0;
	BS_MOVEPARM *mp;
	for (int i = 0; i < cnt; i++) {
		mp = (BS_MOVEPARM *) (data + i * sizeof(BS_MOVEPARM));
		if (_dbg.isEnabled()) _dbg.printf("%d: id: %d ; pos=%d ; time=%d \n", i, mp->id, mp->pos, mp->time);
		if (move(mp->id, mp->pos, mp->time)) moveCnt ++;
	}
	return (moveCnt == cnt);
}
