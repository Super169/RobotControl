#include "robot.h"

void SetDebug(bool mode) {
	debug = mode;
	_dbg->enableDebug(mode);
	_dbg->setLogLevel((debug ? DEFAULT_LOG_LEVEL : 0));
	rs.enableDebug(mode);
	mp3.setDebug(mode);
	config.setDebug(mode);
	SWFM.enableDebug(mode);
}


void SetHeadLed(bool status) {
	headLed = status;
	if (status) {
		#ifdef ROBOT_ARM_BOARD
		  digitalWrite(HEAD_LED_GPIO, LOW);
		#else
		  digitalWrite(HEAD_LED_GPIO, HIGH);
		#endif
	} else {
		#ifdef ROBOT_ARM_BOARD
		  digitalWrite(HEAD_LED_GPIO, HIGH);
		#else
		  digitalWrite(HEAD_LED_GPIO, LOW);
		#endif
	}
}

// XX XX {id} {cmd} xx xx xx xx {sum} ED
// {sum} = cmd[8] = sum ( {id} {cmd} xx xx xx xx )
byte CheckSum(byte *cmd) {
	byte sum = 0;
	for (int i = 2; i < 8; i++) {
		sum += cmd[i];
	}
	return sum;
}

// XX XX {len} xx .. xx {sum} ED
// {len} = cmd[2] = count( {len} xx .. xx )
// {sum} = cmd[{len} + 2] = sum( {len} xx .. xx )
// e.g.  F3 3F
byte CheckVarSum(byte *cmd) {
	byte sum = 0;
	int len = cmd[2];
	int endPos = len + 2;
	for (int i = 2; i < endPos; i++) {
		sum += cmd[i];
	}
	return sum;
}


// XX XX {len} {cmd} xx .. xx {sum} ED
// {len} = count( XX XX {len} {cmd} xx .. xx {sum} )
// {sum} = cmd[{len} - 1] = sum( {len} {cmd} xx .. xx )
// e.g.  F3 3F
byte CheckFullSum(byte *cmd) {
	byte sum = 0;
	int len = cmd[2];
	int endPos = len - 1;
	for (int i = 2; i < endPos; i++) {
		sum += cmd[i];
	}
	return sum;
}

void clearInputBuffer() {
	while (Serial.available()) {
		Serial.read();
		if (!Serial.available()) delay(1);  // make sure no more data coming, add 1ms delay when no data.
	}
}

bool cmdSkip(bool flag) {
	DebugShowSkipByte();
	cmdBuffer.skip();
	return flag;
}

void DebugShowSkipByte() {
	if (debug) {
		DEBUG.printf("Skip byte: %02X\n", cmdBuffer.peek());
	}
}