#include "robot.h"

// Command start with A9 9A

// {len} = cmd[2] = count( {len} xx .. xx )
// {sum} = cmd[{len} + 2] = sum( {len} xx .. xx )
// Size = A9 9A [{len} ...] {sum} ED = [{len} ... ] + 4 = len + 4
// minimum : A9 9A 2 {cmd} {sum} ED = 6

// Command set:
//
//   01 - Reset (fix)    		 	: A9 9A 03 01 00 04 ED
//   02 - Set Debug (fix) 			: A9 9A 03 02 01 06 ED
//   03 - Set DevMode (fix) 		: A9 9A 03 03 01 07 ED
//   11 - Get Angle (fix) 			: A9 9A 02 11 13 ED
//   12 - Get One Angle (fix)		: A9 9A 03 12 01 16 ED
//   13 - Get Adj Angle (fix)		: A9 9A 02 13 15 ED
//   14 - Get One Adj Angle (fix) 	: A9 9A 03 14 01 18 ED
//	 21 - Lock servo (var)			: A9 9A 06 21 01 02 03 04 31 ED
//	 22 - Unlock servo (var)		: A9 9A 05 22 01 02 03 2D ED
//   F1 - Read SPIFFS (fix) 		: A9 9A 02 F1 F3 ED
//   F2 - Write SPIFFS (fix) 		: A9 9A 02 F2 F4 ED


#define V2_CMD_RESET			0x01
#define V2_CMD_DEBUG			0x02
#define V2_CMD_DEVMODE      	0x03
#define V2_CMD_SERVOANGLE		0x11
#define V2_CMD_ONEANGLE			0x12
#define V2_CMD_SERVOADJANGLE	0x13
#define V2_CMD_ONEADJANGLE		0x14

#define V2_CMD_LOCKSERVO		0x21
#define V2_CMD_UNLOCKSERVO		0x22

#define V2_CMD_READSPIFFS   	0xF1
#define V2_CMD_WRITESPIFFS  	0xF2


bool V2_Command() {

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
			DEBUG.printf("%02X ", cmd[i]);
		}
	}

	if (cmd[len+3] != 0xED) {
		if (debug) DEBUG.println(F(" => Invalid end code"));
		return cmdSkip(true);
	}

	byte sum = CheckVarSum(cmd);
	if (debug) {
		DEBUG.printf("=> %02X\n", sum);
	}

	// bypass checksum in devMode
	if (!devMode) {
		if (cmd[len+2] != sum) {
			if (debug) 	DEBUG.println(F("Invalid checksum"));
			return cmdSkip(true);
		}
	}

	// header, checksum, endcode passed
	cmdBuffer.skip(len+4);

	switch (cmd[3]) {

		case V2_CMD_RESET:
			V2_Reset(cmd);
			break;

		case V2_CMD_DEBUG:
			V2_SetDebug(cmd);
			break;

		case V2_CMD_DEVMODE:			
			V2_SetDevMode(cmd);
			break;

		case V2_CMD_SERVOANGLE:
			V2_GetServoAngle();
			break;

		case V2_CMD_ONEANGLE:
			V2_GetOneAngle(cmd);
			break;

		case V2_CMD_SERVOADJANGLE:
			V2_GetServoAdjAngle();
			break;

		case V2_CMD_ONEADJANGLE:
			V2_GetOneAdjAngle(cmd);
			break;

		case V2_CMD_LOCKSERVO:
			V2_LockServo(cmd, true);
			break;

		case V2_CMD_UNLOCKSERVO:
			V2_LockServo(cmd, false);
			break;


		case V2_CMD_READSPIFFS:
			V2_ReadSPIFFS();
			break;

		case V2_CMD_WRITESPIFFS:
			V2_WriteSPIFFS();
			break;

		default:
			if (debug) {
				DEBUG.printf("Undefined command: %02X\n", cmd[3]);
			}
	}

	return true;
}

void V2_SendResult(byte *result) {
	uint16_t len = result[2];
	uint16_t size = len + 4;
	result[0] = 0xA9;
	result[1] = 0x9A;
	result[len+2] = CheckVarSum(result);
	result[len+3] = 0xED;
	Serial.write(result, size);
}

#pragma region V2_CMD_RESET / V2_CMD_DEBUG / V2_CMD_DEVMODE

void V2_Reset(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_Reset]"));
	servo.end();
	delay(200);
	servo.begin();
	byte showAngle = 0;
	if (cmd[2] > 2) showAngle = cmd[4];
	if ((showAngle) && (showAngle != '0')) V2_GetServoAngle();
}

void V2_SetDebug(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_SetDebug]"));
	byte status = (cmd[4] ? 1 : 0);
	SetDebug(cmd[4]);
}

void V2_SetDevMode(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_SetDevMode]"));
	devMode = (cmd[4] ? 1 : 0);
}

#pragma endregion

#pragma region V2_CMD_SERVOANGLE / V2_CMD_ONEANGLE

void V2_GetServoAngle() {
	if (debug) DEBUG.println(F("[V2_GetServoAngle]"));
	uint16_t len = 34;
	byte result[len+4];
	result[2] = len;
	result[3] = V2_CMD_SERVOANGLE;

	byte *ptr = result + 4;
	UBT_GetServoAngle(ptr);
	V2_SendResult(result);
}

void V2_GetOneAngle(byte *cmd) {

	if (debug) DEBUG.println(F("[V2_GetOneAngle]"));
	byte result[2] = {0xff, 0x00};

	if (cmd[2] == 3) {
		byte id = cmd[4];
		if ((id) && (id <= 16) && (servo.exists(id))) {
				if (servo.isLocked(id)) {
					result[0] = servo.lastAngle(id);
					result[1] = 1;
				} else {
					result[0] = servo.getPos(id);
					result[1] = 0;
				}
		} 
	}
	
	Serial.write(result, 2);

}


#pragma endregion

#pragma region V2_CMD_SERVOADJANGLE / V2_CMD_ONEADJANGLE

void V2_GetServoAdjAngle() {
	if (debug) DEBUG.println(F("[V2_GetServoAdjAngle]"));
	uint16_t len = 34;
	byte result[len+4];
	result[2] = len;
	result[3] = V2_CMD_SERVOADJANGLE;
	for (int id = 1; id <= 16; id++) {
		int pos = 4 + 2 * (id - 1);
		if (servo.exists(id)) {
			uint16  adjAngle = servo.getAdjAngle(id);
			result[pos] = adjAngle / 256;
			result[pos+1] = adjAngle % 256;
		} else {
			result[pos] = 0x7F;
			result[pos+1] = 0x7F;
		}
	}
	V2_SendResult(result);
}

void V2_GetOneAdjAngle(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_GetOneAdjAngle]"));
	byte result[2] = {0x7f, 0xff};
	if (cmd[2] == 3) {
		byte id = 0;
		if (cmd[2] > 2) id = cmd[4];
		if ((id) && (id <= 16) && (servo.exists(id))) {
			uint16_t  adjAngle = servo.getAdjAngle(id);
			result[0] = adjAngle / 256;
			result[1] = adjAngle % 256;
		}
	}
	Serial.write(result, 2);
}

#pragma endregion

#pragma region V2_CMD_LOCKSERVO / V2_CMD_LOCKSERVO

void V2_LockServo(byte *cmd, bool goLock) {
	if (debug) DEBUG.println(F("[V2_LockServo]"));

	byte result[40];  // max: A9 9A len cmd n {2 * n} {sum} ED - n <= 16 => max: 39
	result[3] = cmd[3];
	byte cnt = 0;
	if ((cmd[2] == 2) || ((cmd[2] == 3) && (cmd[4] == 0))) {
		// All servo
		for (int id = 1; id <= 16; id++)  {
			byte pos = 5 + 2 * cnt;
			if (servo.exists(id)) {
				result[pos] = id;
				if (servo.isLocked(id) && goLock) {
					// prevent lock servo if already locked, and request to lock
					result[pos + 1] = servo.lastAngle(id);
				} else  {
					result[pos + 1] = servo.getPos(id, goLock);	
				}
				cnt++;
			}
		}
		result[4] = cnt;
	} else {
		int reqCnt = cmd[2] - 2;
		for (int i = 0; i < reqCnt; i++) {
			byte id = cmd[4 + i];
			if (id) {
				// check if already exists
				for (int j = 0; j < i; i++) {
					if (cmd[4 + j] == id) {
						if (debug) {
							DEBUG.printf("Duplicate ID: %d \n", id);
						}
						id = 0; // clear the id if already exists
						break;
					}
				}
			}
			if (id) {
				byte pos = 5 + 2 * cnt;
				if (servo.exists(id)) {
					result[pos] = id;
					if (servo.isLocked(id) && goLock) {
						// prevent lock servo if already locked, and request to lock
						result[pos + 1] = servo.lastAngle(id);
					} else  {
						result[pos + 1] = servo.getPos(id, goLock);	
					}
					cnt++;
				}
			}
		}
	}
	result[2] = 3 +  result[4] * 2;
	V2_SendResult(result);
}

#pragma region SPIFFS: V2_CMD_READSPIFFS / V2_CMD_WRITESPIFFS

void V2_ReadSPIFFS() {
	if (debug) DEBUG.println(F("[V2_ReadSPIFFS]"));
	UBT_ReadSPIFFS(V2_CMD_READSPIFFS);
}

void V2_WriteSPIFFS() {
	if (debug) DEBUG.println(F("[V2_WriteSPIFFS]"));
	UBT_WriteSPIFFS(V2_CMD_WRITESPIFFS);
}
#pragma endregion