#include "UBTServo.h"

UBTServo::UBTServo() {
	_MIN_POS = 0;
	_MAX_POS = 240;
    _MIN_ANGLE = 0;
    _MAX_ANGLE = 240;
	_INVALID_POS = 999;
	_MULTI_SERVO_COMMAND = false;
}

UBTServo::~UBTServo() {
    
}

void UBTServo::showInfo() {
    if (_dbg.isEnabled()){
        _dbg.println("I am UBT Servo");
        baseServo::showInfo();
    } 
}

void UBTServo::resetCommandBuffer() { 
	memcpy(_buf, UBT_SERVO_CMD, UBT_COMMAND_BUFFER_SIZE); 
}

void UBTServo::resetReturnBuffer() { 
	while (_bus->available()) _bus->read();
	memset(_retBuf, 0, UBT_RETURN_BUFFER_SIZE);  
	_retCnt = 0; 
}

bool UBTServo::sendCommand(bool expectReturn) {
	byte sum = 0;
	for (int i = 2; i < 8; i++) {
		sum += _buf[i];
	}
	_buf[8] = sum;
	if (_dbg.isEnabled()) showCommand();

	// clear reutnr buffer before sending commands
	resetReturnBuffer();
	_bus->flush();
	enableTx(true);
	_bus->write(_buf, 10);
	enableTx(false);
	if (expectReturn) return checkReturn();
	return true;
}

void UBTServo::showCommand()  {
    if (!_dbg.isEnabled()) return;
    _dbg.printf("%08ld OUT>>", millis());
    for (int i = 0; i < 10; i++) {
        _dbg.print( (_buf[i] < 0x10 ? " 0" : " "));
        _dbg.print(_buf[i], HEX);
    }
    _dbg.println();
}

bool UBTServo::checkReturn() {
    unsigned long endMs = millis() + UBT_COMMAND_WAIT_TIME;
    while ( (millis() < endMs) && (!_bus->available()) ) ;
    if (!_bus->available()) {
		return false;
	}
    byte ch;

    if (_dbg.isEnabled()) {
        _dbg.printf("%08ld IN>>>", millis());
    }
    while (_bus->available()) {
        ch =  (byte) _bus->read();
        _retBuf[_retCnt++] = ch;
        if (_dbg.isEnabled()) {
			_dbg.printf(" %02X", ch);
        }
		// extra delay to make sure transaction completed 
		// ToDo: check data end?
		//       But what can i do if not ended, seems not logical as only few bytes returned. 
		//       1ms is already more than enough.
		// if (!_bus->available()) delay(1);
    }
	// TODO: Think about any better solution to initiate the bus.
	// Special handling for missing frist byte.
	// In some situation, espeically right after reset, the first return byte will be missing.
	// This is a temporary solution to handle FC / FA return with missing byte.
	if ((_retCnt == 9) && (_retBuf[8]==0xED)) {
		byte b1 = 0;

		// Now, only handle those 0x?F
		if ((_retBuf[0] % 0x10) == 0x0F) {
			b1 = 0xF0 + (_retBuf[0] >> 4);
		}
		// add handling for other code  if needed

		if (b1) {
			for (int i = _retCnt; i > 0; i--) {
				_retBuf[i] = _retBuf[i-1];
			}
			_retBuf[0] = b1;
			_retCnt++;
			if (_dbg.isEnabled()) _dbg.printf("  **Missing byte added: [%02X]",b1);
		}
	}

    if (_dbg.isEnabled()) _dbg.println();
    return true;
}

bool UBTServo::reset() {
    _dbg.msg("UBTServo: Reset not yet implemented.");
    return false;
}

uint32_t UBTServo::getVersion(byte id) {
	if (!validId(id)) return 0;
	int tryCnt = 0;
	while (tryCnt++ <= _maxRetry) {
		resetCommandBuffer();
		_buf[0] = 0xFC;
		_buf[1] = 0xCF;
		_buf[2] = id;
		_buf[3] = 0x01;
		sendCommand();

		if ((_retCnt > 0) && (_retBuf[2] == id) && (_retBuf[3] == 0xAA)) {
			uint32_t ver;
			byte *ptr1 = (byte *) &ver;
			byte *ptr2 = _retBuf + 4;
			memcpy(ptr1, ptr2, 4);
			return ver;
		}
		delay(1);	// wait 1ms before retry
	}
    return 0;
}

bool UBTServo::move(byte id, int16_t pos, uint16_t time)  {
	if (!validId(id)) return false;
	if (pos < 0)  return false;
	int tryCnt = 0;
	while (tryCnt++ <= _maxRetry) {
		resetCommandBuffer();
		_buf[2] = id;
		_buf[3] = 0x01;
		_buf[4] = (pos & 0xFF);
		_buf[5] = (time / 20);
		_buf[6] = 0x00;
		_buf[7] = (time / 20);
		sendCommand();

		if ((_retCnt == 1) && (_retBuf[0] == (0xAA + id))) {
			_isLocked[id] = true;
			_lastPos[id] = pos;
			return true;
		}		
	}
	return false;
}

bool UBTServo::setLED(byte id, bool mode)  {
	if (!validId(id)) return 0;
	int tryCnt = 0;
	while (tryCnt++ <= _maxRetry) {
		resetCommandBuffer();
		_buf[2] = id;
		_buf[3] = 0x04;
		_buf[4] = (mode ? 0 : 1);
		sendCommand();

		if (id == 0) {
			// assume OK for all servo command and it cannot check return
			memset(_led, _maxId + 1, mode);
			return true;
		}

		if ((_retCnt == 1) && (_retBuf[0] == (0xAA + id))) {
			_led[id] = mode;
			return true;
		}
	}
	return false;
}

int16_t UBTServo::getPos(byte id) {
	if (!validId(id)) return _INVALID_POS;
	return getPos(id, _isLocked[id]);
}


int16_t UBTServo::getPos(byte id, bool lockAfterGet) {
	if (!validId(id)) return _INVALID_POS;
	if (isLocked(id) && lockAfterGet) {
		return _lastPos[id];
	}

	int tryCnt = 0;
	while (tryCnt++ <= _maxRetry) {
		resetCommandBuffer();
		_buf[2] = id;
		_buf[3] = 0x02;
		sendCommand();
		if (_retCnt == 10) break;
	}
	if (_retCnt != 10) return _INVALID_POS;

	byte pos = _retBuf[7];

	if (lockAfterGet) {
		move(id, pos, 0);
	} else {
		// servo will be unlocked after fire a get position command
		_isLocked[id] = false;
		_lastPos[id] = pos;
	}

	return (int16_t) pos;
}

bool UBTServo::lock(byte id) {
	if (!validId(id)) return false;
	if (isLocked(id)) return true;
	uint16_t pos = getPos(id, true);
	return (isLocked(id));
} 

bool UBTServo::unlock(byte id) {
	if (!validId(id)) return false;
	// since the system is default to unlock, so not checking and go ahead to unlock
	getPos(id, false);
	return (!_isLocked[id]);
}

uint16_t UBTServo::getAdjAngle(byte id) {
	if (!validId(id)) return 0x7F7F;
	int tryCnt = 0;
	while (tryCnt++ <= _maxRetry) {
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
	uint16_t _adjAngle = _retBuf[6] * 256 + _retBuf[7];
	return _adjAngle;
}

uint16 UBTServo::setAdjAngle(byte id, uint16 adjValue) {
	if (!validId(id)) return 0x7F7F;
	int tryCnt = 0;
	while (tryCnt++ < _maxRetry) {
		resetCommandBuffer();
		_buf[2] = id;
		_buf[3] = 0xD2;
		_buf[6] = adjValue / 256;
		_buf[7] = adjValue % 256;
		sendCommand();
		if (_retCnt == 10) break;
	}
	if (_retCnt != 10) {
		// What can I do if it has not return the position
		return 0x7F7F;
	}
	return getAdjAngle(id);
}
