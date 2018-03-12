#include "robot.h"

/*
	Servo command can be passed to servo directly

*/

#pragma region "UBTech Command"
void cmd_UBTCommand(byte b1) {
	delay(1);  // just for safety
	byte cmd[10];
	byte result[10];
	cmd[0] = b1;
	for (int i = 1; i < 10; i++) {
		if (!Serial.available()) return;  // skip for incomplete command, it should be ready after delay(1);
		cmd[i] = Serial.read();
	}
	if (cmd[9] != 0xED) return;  // invalid end code
	if (((cmd[0] == 0xFA) && (cmd[1] != 0xAF)) || ((cmd[0] == 0xFC) && (cmd[1] != 0xCF))) return;  // invalid start code
	byte sum = 0;
	for (int i = 2; i < 8; i++) {
		sum += cmd[i];
	}
	if (cmd[8] != sum) return;  // invalid checksum
	int cnt =  (servo.execute(cmd, result));
	if (cnt > 0) {
		Serial.write(result, cnt);
	}
}
#pragma endregion