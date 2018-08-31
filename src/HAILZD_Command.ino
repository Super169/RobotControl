/*
	UBT Bluetooth Coomunization Command

*/
#include "robot.h"

bool HAILZD_Command() {

	if (debug) DEBUG.print(F("HAILZD Command detected\n"));

	// #?P???\r\d - at lease 8 bytes
	if (cmdBuffer.available() < 8) {
		return true;
	}

    // command ended with 0x0d 0x0a
	byte lastEndPos = cmdBuffer.available() - 1;
	byte endPos = 6;
	bool findEnd = false;

	while (endPos < lastEndPos) {
		findEnd = ((cmdBuffer.peek(endPos) == 0x0D) && (cmdBuffer.peek(endPos+1) == 0x0A));
		if (findEnd) break;
		endPos++;
	}

	if (!findEnd) return true;

	byte cmd[endPos+2];
	cmdBuffer.read(cmd, endPos+2);

	if (!enable_HAILZD) return true;

	if (debug) {
		DEBUG.printf("HaiLzd Servo Command sent (%d): ", endPos+2);
		for (int i = 0; i < endPos; i++) {
			DEBUG.print((char) cmd[i]);
		}
		DEBUG.println();
	}
	SendGenericCommand(cmd, endPos + 2);
	return true;	
}