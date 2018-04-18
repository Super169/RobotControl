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
    _dbg->printf("%06d MP3>>", millis());
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

	_ss->flush();
	_ss->enableTx(true);
	_ss->write(_buf, 10);
	_ss->enableTx(false);
	if (expectReturn) return checkReturn();
    while (_ss->available() ) {
        _ss->read();
        delay(1);
    }
	return true;
}

bool MP3TF16P::checkReturn() {
    return true;
}

void MP3TF16P::sendSingleByteCommand(byte cmd) {
	resetCommandBuffer();
	_buf[3] = cmd;
	sendCommand();    
}

void MP3TF16P::playFile(byte seq) {
	resetCommandBuffer();
	_buf[3] = 0x03;
	_buf[6] = seq;
	sendCommand();    
}

void MP3TF16P::setVol(byte vol) {
    if (vol > 30) vol = 30;
	resetCommandBuffer();
	_buf[3] = 0x06;
	_buf[6] = vol;
	sendCommand();    
}

void MP3TF16P::playFolderFile(byte folder, byte seq) {
	resetCommandBuffer();
	_buf[3] = 0x0F;
    _buf[5] = folder;
	_buf[6] = seq;
	sendCommand();    
}

void MP3TF16P::playMp3File(byte seq) {
	resetCommandBuffer();
	_buf[3] = 0x12;
	_buf[6] = seq;
	sendCommand();    
}

void MP3TF16P::playAdFile(byte seq) {
	resetCommandBuffer();
	_buf[3] = 0x13;
	_buf[6] = seq;
	sendCommand();    
}
