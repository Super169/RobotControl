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
    _dbg.printf("%08ld: SSBoard OUT>>", millis());
    for (int i = 0; i < 10; i++) {
        _dbg.printf(" %02X", _buf[i]);
    }
    _dbg.printf("\n");
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
    byte count 
    

}

bool SSBoard::SendCommand(bool expectReturn) {
    _buf[0] = 0xA8;
    _buf[1] = 0x8A;
    byte count = _buf[2];
	uint16_t sum = 0;
	for (int i = 0; i < count; i++) {
		sum += _buf[2 + i];
	}
    _buf[count+2] = sum;
    _buf[count+3] = 0xED;

	if (_enableDebug) ShowCommand();

	ClearRxBuffer();
	_bus->flush();
	EnableTx(true);
	_bus->write(_buf, 10);
	EnableTx(false);
	if (expectReturn) return CheckReturn();
	ClearRxBuffer();
	return true;
}

bool SSBoard::CheckReturn() {
    if (_enableDebug) _dbg.msg("SSB CheckReturn", millis());
    unsigned long startMs = millis();
    ResetReturnBuffer();
    byte ch;
    while ( ((millis() - startMs) < SSB_COMMAND_WAIT_TIME) && (!_bus->available()) ) delay(1);
	// unsigned long endMs = millis();
	// if (_enableDebug) _dbg.printf("SSB wait return from %d to %d\n", startMs, endMs);

    if (!_bus->available()) {
	    if (_enableDebug) _dbg.msg("SSB no return after %d ms", SSB_COMMAND_WAIT_TIME);
		return false;
	}
    if (_enableDebug) {
        _dbg.printf("%08ld SSB IN>>>", millis());
    }
	// 10 ms is almost good for 10byte data
	delay(10);
    while (_bus->available()) {
        ch =  (byte) _bus->read();
        _retBuf.write(ch);
        if (_enableDebug) {
			_dbg.printf(" %02X", ch);
        }
		// extra delay to make sure transaction completed 
		// ToDo: check data end?
		//       But what can i do if not ended, seems not logical as only few bytes returned. 
		//       1ms is already more than enough.
		if (!_bus->available()) delay(2);
    }
    if (_enableDebug) _dbg.printf("\n");
    return true;
}
