#include "robot.h"

void SetDebug(bool mode) {
	debug = mode;
	servo.setDebug(mode);
}

void DebugPrintByte(byte data) {
	if (data < 0x10) Serial.print("0");
	DEBUG.print(data, HEX);
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
		DEBUG.print("Skip byte: ");
		DebugPrintByte(cmdBuffer.peek());
		DEBUG.println();
	}
}