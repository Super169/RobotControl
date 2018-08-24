#include "HLServo.h"

HLServo::HLServo() {
	_MIN_POS = 500;
	_MAX_POS = 2500;
    _MIN_ANGLE = 0;
    _MAX_ANGLE = 180;
	_INVALID_POS = -9999;
	_MULTI_SERVO_COMMAND = true;
}

HLServo::~HLServo() {
}

void HLServo::initBus() {
    // For HaiLzd servo, send empty command to reset the _bus for initialization
	resetReturnBuffer();
   	_bus->flush();
    enableTx(true);
    _bus->write(0x0d);
    _bus->write(0x0a);
    enableTx(false);
	checkReturn();
	resetReturnBuffer();
	return;
}

void HLServo::showInfo() {
    if (_dbg.isEnabled()) {
        _dbg.println("I am HaiLzd Servo");
        baseServo::showInfo();
    }
}

void HLServo::showCommand() {
    if (_dbg.isEnabled()) {
        _dbg.printf("%08ld OUT>> ", millis());
        _dbg.println(_buffer);
    }
}

void HLServo::resetReturnBuffer() { 
	while (_bus->available()) _bus->read();
    memset(_retBuf, 0, HL_RETURN_BUFFER_SIZE);  
    _retCnt = 0; 
}

bool HLServo::sendCommand(bool expectReturn) {

  	if (_dbg.isEnabled()) showCommand();

	resetReturnBuffer();
   	_bus->flush();

    enableTx(true);
    _bus->print(_buffer);
    _bus->write(0x0d);
    _bus->write(0x0a);
    enableTx(false);

	if (expectReturn) return checkReturn();
	return true;

}

bool HLServo::sendUntilOK() {
    int tryCnt = 0;
    while (tryCnt++ < _maxRetry) {
        if (sendCommand(true) && isReturnOK()) return true;
        delay(1);
    }
    return false;
}

bool HLServo::checkReturn() {
    unsigned long endMs = millis() + HL_COMMAND_WAIT_TIME;
    while ( (millis() < endMs) && (!_bus->available()) ) ;
    if (!_bus->available()) {
		return false;
	}
    byte ch;    
    if (_dbg.isEnabled()) {
        _dbg.printf("%08ld IN>>> ", millis());
    }
    while (_bus->available()) {
        ch =  (byte) _bus->read();
        _retBuf[_retCnt++] = ch;
        if (_dbg.isEnabled()) {
            // _dbg.printf(" %02X", ch);
            _dbg.print((char) ch);
        }

        // Extra 1ms to make sure the communization is ended
        if (!_bus->available()) delay(2);

    }
    if (_dbg.isEnabled()) {
        _dbg.println();
    }
    
    if (_retCnt < 2) return false;  // should be always >= 2 as it should end with 0x0d 0x0a, in fact, it should be more than 2
    
    if ((_retBuf[_retCnt-2] != 0x0d) || (_retBuf[_retCnt-1] != 0x0a)) {
        if (_dbg.isEnabled()) _dbg.print("-- Invalid return\n");
        return false;  // should always end with 0x0d 0x0a
    }
    return true;

}

bool HLServo::isReturnOK() {
    if (_retCnt < 5) return false;
    return ((_retBuf[0] == '#') && (_retBuf[1] == 'O') && (_retBuf[2] == 'K') && (_retBuf[3] == 0x0D) && (_retBuf[4] == 0x0A));
}

bool HLServo::getRetNum(int16_t *data, byte start, byte len, byte minLen) {
    if (_retCnt < start + minLen) return false;
    if (len == 0) return false;
    byte cnt = 0;
    bool nev  = false;
    int16_t value = 0;
    byte endPos = start + len;
    if (endPos > _retCnt) endPos = _retCnt;
    for (int i = start; i < endPos; i++) {
        char ch = (char) _retBuf[i];
        if ((ch >= '0')  && (ch <= '9')) {
            value *= 10;
            value += (ch - '0');
        } else if (i == start) {
            if (ch == '-') {
                nev = true;
            } else if (ch != '+') {
                return false;
            }

        } else {
            return false;
        }
        cnt++;
    }
    if (cnt < minLen) return false;
    if (nev) value *= -1;
    *data = value;
    return true;
}

bool HLServo::reset() {
    _dbg.msg("HLServo: Reset not yet implemented.");
    return false;
}

// #{id}PVER\r\n
uint32_t HLServo::getVersion(byte id) {
    int tryCnt = 0;
    while (tryCnt++ <= _maxRetry) {
        String code = "PVER";
        _buffer = "#" + String(id) + code;
        if (sendCommand(true) && (_retCnt >= 6)) {
            uint32_t ver;
            byte *ptr1 = (byte *) &ver;
            byte *ptr2 = _retBuf + (_retCnt - 6);
            memcpy(ptr1, ptr2, 4);
            return ver;
        }
        delay(1);
    }
    return 0;
}


// OUT>> #{id}P{pos}T{time}
bool HLServo::move(byte id, int16_t pos, uint16_t time)  {
    if (!validId(id)) return false;
    if (!validPos(pos)) return false;
    if (time > 50000) return false;

    _buffer = "#" + String(id) + "P" + String(pos) + "T" + String(time);
    if (!sendCommand(false)) return false;
    _lastPos[id] = pos;

    return true;
}

// OUT>> #{id}P{pos}T{time}#{id}P{pos}T{time}
bool HLServo::moveX(byte cnt, byte *data)  {
    _buffer = "";
	if (cnt == 0) return true;
	BS_MOVEPARM *mp;
	for (int i = 0; i < cnt; i++) {
		mp = (BS_MOVEPARM *) (data + i * sizeof(BS_MOVEPARM));
        if (validId(mp->id) && validPos(mp->pos)) {
            _buffer += "#" + String(mp->id) + "P" + String(mp->pos) + "T" + String(mp->time);
        }
	}
    if (!sendCommand(false)) return false;
	for (int i = 0; i < cnt; i++) {
		mp = (BS_MOVEPARM *) (data + i * sizeof(BS_MOVEPARM));
        if (validId(mp->id) && validPos(mp->pos)) {
                    _lastPos[mp->id] = mp->pos;
        }
	}
	return true;
}

bool HLServo::setLED(byte id, bool mode)  {
    _dbg.msg("**** HLServo: LED not available");
    return false;
}

bool HLServo::setLED(bool mode)  {
    _dbg.msg("**** HLServo: LED not available");
    return false;
}

// OUT>> #{id}PRAD
// IN << #nnnPaaaa
int16_t HLServo::getPos(byte id, bool lockAfterGet) {
    if (!validId(id)) return _INVALID_POS;

    _lastPos[id] = _INVALID_POS;

    String code = "PRAD";
    _buffer = "#" + String(id) + code;
    if (!sendCommand(true)) return _INVALID_POS;
    if (_retCnt < 9) return _INVALID_POS;
    int16_t value;
    if (!getRetNum(&value, 1, 3)) return _INVALID_POS;
    if (value != id) return _INVALID_POS;
    if (!getRetNum(&value, 5, 4)) return _INVALID_POS;
    if (!validPos(value)) return _INVALID_POS;
    _lastPos[id] = value;

    if (lockAfterGet) {
        lock(id);
    }
    return value;
}

// OUT>> #{id}PULR\r\n
// IN << OK
bool HLServo::lock(byte id) {
    if (!validId(id)) return false;

    String code = "PULR";
    _buffer = "#" + String(id) + code;
    
    if (!sendUntilOK()) return false;

    _isLocked[id] = true;
    return true;

} 

// OUT>> #255PULR\r\n
// IN << OK
bool HLServo::lockAll() {

    String code = "PULR";
    _buffer = "#255" + code;
    
    if (!sendCommand(false)) return false;

    for (byte id = 1; id <= _maxId; id++) {
        if (_servo[id]) _isLocked[id] = true;
    }
    return true;

} 

// OUT>> #{id}PULK\r\n
// IN << OK
bool HLServo::unlock(byte id) {
    if (!validId(id)) return false;
    String code = "PULK";
    _buffer = "#" + String(id) + code;
    
    if (!sendUntilOK()) return false;

    _isLocked[id] = false;
    return true;
}

// OUT>> #255PULK\r\n
// IN << OK
bool HLServo::unlockAll() {

    String code = "PULK";
    _buffer = "#255" + code;
    
    if (!sendCommand(false)) return false;

    for (byte id = 1; id <= _maxId; id++) {
        if (_servo[id]) _isLocked[id] = false;
    }
    return true;

} 


uint16_t HLServo::setPosMode(byte id, byte mode)  {
    if (!validId(id)) return 0;
    if ((mode < 1) || (mode > 2)) return 0;
    String code;
    switch (mode) {
        case 1:
            // center
            code = "CK";
            break;
        case 2:
            // min
            code = "MI";
            break;
        case 3:
            // max
            code = "MX";
            break;
    }
    _buffer = "#" + String(id) + "PS" + code;
    
    if (!sendUntilOK()) return 0;

    return mode;
}
