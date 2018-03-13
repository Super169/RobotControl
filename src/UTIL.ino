#include "robot.h"

int BuferAvailable() {
	int count = bufWritePtr - bufReadPtr;
	if (count < 0) count += BUFFER_SIZE;
	return count;
}

byte BufferPeek() {
	if (BuferAvailable()) {
		return cmdBuf[bufReadPtr];
	}
	return 0;  // should not peek without checking available
}

byte BufferRead() {
	if (BuferAvailable()) {
		return cmdBuf[bufReadPtr++];
	}
	return 0;  // should not read without checking available
}

bool BufferWrite(byte data) {
	int newWritePtr = (bufWritePtr + 1) % BUFFER_SIZE;
	if (newWritePtr == bufReadPtr) return false;
	cmdBuf[bufWritePtr] = data;
	bufWritePtr = newWritePtr;
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