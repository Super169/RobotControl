#include "robot.h"

// Command start with A9 9A

// {len} = cmd[2] = count( {len} xx .. xx )
// {sum} = cmd[{len} + 2] = sum( {len} xx .. xx )
// Size = A9 9A [{len} ...] {sum} ED = [{len} ... ] + 4 = len + 4

void V2_CommandSet(byte b1) {
	delay(1);
	if (!Serial.available()) return;
	byte b2 = Serial.read();
	if (b2 != 0x9A) return;
	if (!Serial.available()) return;
	int len = Serial.read();
	if (!len) return;  // len should always > 0
	byte cmd[len+4];
	cmd[0] = b1;
	cmd[1] = b2;
	cmd[2] = len;
	// Remaining = xx .. xx {sum} ED : {len + 1} bytes
	for (int i = 0; i < len + 1; i++) {
		if (!Serial.available()) return;
		cmd[3+i] = Serial.read();
	}
	byte sum = CheckVarSum(cmd);
	if (cmd[len+2] != sum) return;
	if (cmd[len+3] != 0xED) return;

	DEBUG.println();
	for (int i = 0; i < len + 4; i++) {
		DebugPrintByte(cmd[i]);
		DEBUG.print(" ");
	}
	DEBUG.print(" => ");
	DebugPrintByte(sum);
	DEBUG.println("\n");
	
}