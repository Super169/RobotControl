#include "SSBoard.h"

SSBoard::SSBoard() {
}

SSBoard::~SSBoard() {
}

void SSBoard::Begin(Stream *busPort, Stream *debugPort) {
    _bus = busPort;
    _dbg.setOutput(debugPort);
    _retBuf.init(SSB_BUFFER_SIZE);
}

void SSBoard::SetEnableTxCalback(void (*enableTxCallback)(bool)) {
	_enableTxCallback = enableTxCallback;
}

void SSBoard::EnableTx(bool mode) {
	if (_enableTxCallback != NULL) {
        _enableTxCallback(mode);
        delayMicroseconds(10);
    }
}

void SSBoard::ShowCommand()  {
    if (!_enableDebug) return;
    _dbg.msgh("SSB OUT>>");
    byte count = _buf[2] + 4;
    for (int i = 0; i < count; i++) {
        _dbg.msgf(" %02X", _buf[i]);
    }
    _dbg.msgf("\n");
}


void SSBoard::ClearRxBuffer() {
    while (_bus->available() ) {
        _bus->read();
        delay(2);
    }
}

bool SSBoard::IsReturnCompleted() {
    // Must start with A8 8A
    while ((_retBuf.available() > 1) && (_retBuf.peek() != 0xA8) && (_retBuf.peek(1) != 0x8A)) {
        _retBuf.skip();
    } 
    // At least 6 byte: A8 8A 02 {cmd} {sum} ED
    if (_retBuf.available() < 6) return false;
    byte count = _retBuf.peek(2);
    if (_retBuf.available() < count + 4) return false;
    if (_retBuf.peek(count + 3) != 0xED) {
        // Skip header and check again later
        _retBuf.skip(2);
        return false;
    }
    // checksume
	uint16_t sum = 0;
	for (int i = 0; i < count; i++) {
		sum += _retBuf.peek(2 + i);
	}
    byte checkSum = sum & 0xFF;
    if (checkSum != _retBuf.peek(count + 2)) {
        // _dbg.msg("Sum not match: %02X vs %02X", checkSum, _retBuf.peek(count + 2));
        // Skip header and check again later
        _retBuf.skip(2);
        return false;
    }
    return true;
}

bool SSBoard::SendCommand(byte *cmd, bool expectReturn) {
    _buf[0] = 0xA8;
    _buf[1] = 0x8A;
    byte count = cmd[2];
    _buf[2] = cmd[2];
	uint16_t sum = 0;
	for (int i = 0; i < count; i++) {
        _buf[2 + i] = cmd[2 + i];
		sum += _buf[2 + i];
	}
    _buf[count+2] = sum;
    _buf[count+3] = 0xED;

	if (_enableDebug) ShowCommand();

	ClearRxBuffer();
	_bus->flush();
	EnableTx(true);
	_bus->write(_buf, count + 4);
	EnableTx(false);
	if (expectReturn) return CheckReturn();
	ClearRxBuffer();
	return true;
}

bool SSBoard::CheckReturn() {
    // if (_enableDebug) _dbg.msg("SSB CheckReturn");
    unsigned long startMs = millis();
    ResetReturnBuffer();
    byte ch;
    while ( ((millis() - startMs) < SSB_COMMAND_WAIT_TIME) && (!_bus->available()) ) delay(1);
	// unsigned long endMs = millis();
	// if (_enableDebug) _dbg.msgf("SSB wait return from %d to %d\n", startMs, endMs);

    if (!_bus->available()) {
	    if (_enableDebug) _dbg.msg("SSB no return after %d ms", SSB_COMMAND_WAIT_TIME);
		return false;
	}
    if (_enableDebug) {
        _dbg.msgh("SSB IN>>>");
    }
	// 10 ms is almost good for 10byte data
    bool returnCompleted = false;
	delay(10);
    while (_bus->available()) {
        ch =  (byte) _bus->read();
        _retBuf.write(ch);
        if (_enableDebug) {
			_dbg.msgf(" %02X", ch);
        }
        if (IsReturnCompleted()) {
            returnCompleted = true;
            break;
        }
		// extra delay to make sure transaction completed 
		// ToDo: check data end?
		//       But what can i do if not ended, seems not logical as only few bytes returned. 
		//       1ms is already more than enough.
		if (!_bus->available()) delay(2);
    }
    if (_enableDebug) _dbg.msgf("\n");
    return returnCompleted;
}
