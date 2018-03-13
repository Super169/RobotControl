#include "robot.h"

// Command start with A9 9A

// {len} = cmd[2] = count( {len} xx .. xx )
// {sum} = cmd[{len} + 2] = sum( {len} xx .. xx )
// Size = A9 9A [{len} ...] {sum} ED = [{len} ... ] + 4 = len + 4
// minimum : A9 9A 1 {cmd} {sum} ED = 6

bool V2_CommandSet() {

	if (cmdBuffer.available() < 6) return false;
	byte header[3];
	cmdBuffer.peek(header, 3);
	if (header[1] != 0x9A) {
		if (debug) DEBUG.println(F("Invalid start code"));
		return cmdSkip(true);
	}
	int len = header[2];
	if (cmdBuffer.available() < len + 4) return false;

	byte cmd[len+4];
	cmdBuffer.peek(cmd, len+4);

	if (debug) {
		for (int i = 0; i < len + 4; i++) {
			DebugPrintByte(cmd[i]);
			DEBUG.print(" ");
		}
	}

	if (cmd[len+3] != 0xED) {
		if (debug) DEBUG.println(F(" => Invalid end code"));
		return cmdSkip(true);
	}

	byte sum = CheckVarSum(cmd);
	if (debug) {
		DEBUG.print("=>  ");
		DebugPrintByte(sum);
		DEBUG.print("\n");
	}

	if (cmd[len+2] != sum) {
		if (debug) 	DEBUG.println(F("Invalid checksum"));
		return cmdSkip(true);
	}
	// header, checksum, endcode passed
	cmdBuffer.skip(len+4);

	DEBUG.println(F("Command processed"));
	return true;
}