/*
	UBT Servo Coomunization Command

*/
#include "robot.h"

#pragma region "UBTech Command"
bool UBTSV_Command() {	

	if (debug) DEBUG.print(F("UBT Servo Command detected\n"));

	// if (cmdBuffer.available() < 10) return false;
	// It's not logicaly that 10 byte cannot be received within 1 ms, so consider as invalid input directly
	if (cmdBuffer.available() < 10) {
		if (debug) DEBUG.print(F("Invalid length\n"));
		return cmdSkip(true);
	}

	byte cmd[10];
	byte result[10];

	cmdBuffer.peek(cmd, 10);  // Don't read command, just check the data first
	if (cmd[9] != 0xED) {
		if (debug) DEBUG.print(F("Invalid end code\n"));
		return cmdSkip(true);
	}

	if (((cmd[0] == 0xFA) && (cmd[1] != 0xAF)) || ((cmd[0] == 0xFC) && (cmd[1] != 0xCF))) {
		if (debug) DEBUG.print(F("Invalid start code\n"));
		return cmdSkip(true);
	}

	byte sum = CheckSum(cmd);
	if (cmd[8] != sum) {
		if (debug) {
			if (debug) DEBUG.print(F("Invalid checksum\n"));
		}
		return cmdSkip(true);
	}
	// Now a complete command received, clear data from buffer
	cmdBuffer.skip(10);

	if (!enable_UBTSV) return true;

	if (debug) {
		DEBUG.print(F("UBT Servo Command sent: "));
		for (int i = 0; i < 10; i++) {
			DEBUG.printf("%02X ", cmd[i]);
		}
		DEBUG.println();
	}

	SendGenericCommand(cmd, 10);
	return true;

}
#pragma endregion
