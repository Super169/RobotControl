#include "MP3TF16P.h"

MP3TF16P::MP3TF16P(SoftwareSerial *ssData) 
{
    initObject(ssData, NULL);
}

MP3TF16P::MP3TF16P(SoftwareSerial *ssData, HardwareSerial *hsDebug) 
{
	initObject(ssData, hsDebug);
}
        
void MP3TF16P::initObject(SoftwareSerial *ssData, HardwareSerial *hsDebug) {
    _ss = ssData;
	_dbg = hsDebug;
	_enableDebug = (_dbg != NULL);
}        

MP3TF16P::~MP3TF16P() {
    _ss = NULL;
	_dbg = NULL;
}

bool MP3TF16P::setDebug(bool debug) {
	if (_dbg == NULL) return false;
	_enableDebug = debug;
	return _enableDebug;
}

void MP3TF16P::begin() {
    _ss->begin(MP3_BAUD);
	delay(100);
}

void MP3TF16P::end() {
    _ss->end();
}

void MP3TF16P::showCommand()  {
    if (!_enableDebug) return;
    _dbg->printf("%08ld MP3 OUT>>", millis());
    for (int i = 0; i < 10; i++) {
        _dbg->printf(" %02X", _buf[i]);
    }
    _dbg->println();
}

bool MP3TF16P::sendCommand(bool expectReturn) {
	uint16_t sum = 0;
	for (int i = 1; i < 7; i++) {
		sum += _buf[i];
	}
    sum = 0 - sum;
    _buf[7] = sum >> 8;
    _buf[8] = sum & 0xFF;

	if (_enableDebug) showCommand();

	clearRxBuffer();
	_ss->flush();
	//_ss->enableTx(true);
	//delay(1);
	_ss->write(_buf, 10);
	//_ss->enableTx(false);
	if (expectReturn) return checkReturn();
	clearRxBuffer();
	return true;
}

void MP3TF16P::clearRxBuffer() {
    while (_ss->available() ) {
        _ss->read();
        delay(2);
    }
}

bool MP3TF16P::checkReturn() {
    if (_enableDebug) _dbg->printf("%08ld MP3 checkReturn\n", millis());
    unsigned long startMs = millis();
    resetReturnBuffer();
    byte ch;
    while ( ((millis() - startMs) < MP3_COMMAND_WAIT_TIME) && (!_ss->available()) ) delay(1);
	// unsigned long endMs = millis();
	// if (_enableDebug) _dbg->printf("MP3 wait return from %d to %d\n", startMs, endMs);

    if (!_ss->available()) {
	    // if (_enableDebug) _dbg->printf("MP3 no return after %d ms\n", MP3_COMMAND_WAIT_TIME);
		return false;
	}
    if (_enableDebug) {
        _dbg->printf("%08ld MP3 IN>>>", millis());
    }
	// 10 ms is almost good for 10byte data
	delay(10);
    while (_ss->available()) {
        ch =  (byte) _ss->read();
        _retBuf[_retCnt++] = ch;
        if (_enableDebug) {
			_dbg->printf(" %02X", ch);
			/*
            _dbg->print((ch < 0x10 ? " 0" : " "));
            _dbg->print(ch, HEX);
			*/
        }
		// extra delay to make sure transaction completed 
		// ToDo: check data end?
		//       But what can i do if not ended, seems not logical as only few bytes returned. 
		//       1ms is already more than enough.
		if (!_ss->available()) delay(2);
    }
    if (_enableDebug) _dbg->println();
    return true;
}

void MP3TF16P::sendSingleByteCommand(byte cmd) {
	resetCommandBuffer();
	_buf[3] = cmd;
	sendCommand();    
}

void MP3TF16P::sendCommand(byte cmd, byte p1, byte p2) {
	resetCommandBuffer();
	_buf[3] = cmd;
	_buf[5] = p1;
	_buf[6] = p2;
	sendCommand();    
}

void MP3TF16P::sendCommand(byte cmd, uint16_t parm) {
	byte p1 = parm >> 8;
	byte p2 = parm;
	sendCommand(cmd, p1, p2);
}

void MP3TF16P::playFile(uint16_t seq) {
	resetCommandBuffer();
	_buf[3] = 0x03;
	_buf[5] = seq >> 8;
	_buf[6] = seq;
	sendCommand();    
}

void MP3TF16P::setVol(byte vol) {
    if (vol > 30) vol = 30;
	resetCommandBuffer();
	_buf[3] = 0x06;
	_buf[6] = vol;
	sendCommand();    
	_vol = vol;
}

void MP3TF16P::playFolderFile(byte folder, byte seq) {
	resetCommandBuffer();
	_buf[3] = 0x0F;
    _buf[5] = folder;
	_buf[6] = seq;
	sendCommand();    
}

// 7E FF 06 12 00 xx xx ?? ?? EF
void MP3TF16P::playMp3File(uint16_t seq) {
	resetCommandBuffer();
	_buf[3] = 0x12;
	_buf[5] = seq >> 8;
	_buf[6] = seq & 0xFF;
	sendCommand();    
}

void MP3TF16P::playAdFile(byte seq) {
	resetCommandBuffer();
	_buf[3] = 0x13;
	_buf[6] = seq;
	sendCommand();    
}

uint8_t MP3TF16P::getVol() {
	return _vol;
	// Function will return local record instead of reading from MP3 module as it has been changed to single wire mode, no return from MP3 module
	/*
	resetCommandBuffer();
	_buf[3] = 0x43;
	sendCommand(true);
	if (_retCnt == 10) {
		if ((_retBuf[0] == 0x7E) && (_retBuf[1] == 0xFF) && (_retBuf[2] == 0x06) && (_retBuf[3] == 0x43) && (_retBuf[9] == 0xEF)) {
			return _retBuf[6];
		}
	}
	return 0xFF;
	*/
}

void MP3TF16P::adjVol(int diff) {
	if (diff == 0) return;
	uint8_t currVol = getVol();
	if (currVol == 0xFF) return;
	if ((currVol == 0) && (diff < 0)) return;
	if ((currVol >= 30) && (diff > 0)) return;
	int iVol = currVol;
	iVol += diff;
	if (iVol < 0) iVol = 0;
	if (iVol > 30) iVol = 30;
	currVol = iVol;
	setVol(currVol);
}