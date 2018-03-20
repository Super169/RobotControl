#include "robot.h"

// Command start with A9 9A

// {len} = cmd[2] = count( {len} xx .. xx )
// {sum} = cmd[{len} + 2] = sum( {len} xx .. xx )
// Size = A9 9A [{len} ...] {sum} ED = [{len} ... ] + 4 = len + 4
// minimum : A9 9A 2 {cmd} {sum} ED = 6

// Command set:
//
//   01 - Reset (fix)    		 	: A9 9A 02 01 03 ED
//   02 - Set Debug (fix) 			: A9 9A 03 02 00 05 ED
//                                    A9 9A 03 02 01 06 ED    
//   03 - Set DevMode (fix) 		: A9 9A 03 03 00 06 ED
//									  A9 9A 03 03 01 07 ED
//   0A - Command Enable (fix)		: A9 9A 02 0A 0C ED
//                                    A9 9A 07 0A 00 01 01 01 01 15 ED
//   11 - Get Angle (fix) 			: A9 9A 02 11 13 ED
//   12 - Get One Angle (fix)		: A9 9A 03 12 01 16 ED
//   13 - Get Adj Angle (fix)		: A9 9A 02 13 15 ED
//   14 - Get One Adj Angle (fix) 	: A9 9A 03 14 01 18 ED
//	 21 - Lock servo (var)			: A9 9A 06 21 01 02 03 04 31 ED
//	 22 - Unlock servo (var)		: A9 9A 05 22 01 02 03 2D ED
//	 23 - Servo move (var)			: A9 9A 05 23 00 5A A0 22 ED
//									: A9 9A 08 23 01 5A A0 02 00 A0 C8 ED 
//	 31 - Set LED (var)				: A9 9A 04 31 00 01 36 ED
//                                  : A9 9A 06 31 01 00 02 01 3B ED
//   61 - Read Action Header (fix) 	: A9 9A 03 61 01 65 ED
//   62 - Read Action Data (fix)	: A9 9A 03 62 01 65 ED
//   63 - Read Action Post (fix)	: A9 9A 03 61 01 65 ED


//   F1 - Read SPIFFS (fix) 		: A9 9A 02 F1 F3 ED
//   F2 - Write SPIFFS (fix) 		: A9 9A 02 F2 F4 ED


#define V2_CMD_RESET			0x01
#define V2_CMD_DEBUG			0x02
#define V2_CMD_DEVMODE      	0x03

#define V2_CMD_ENABLE			0x0A

#define V2_CMD_SERVOANGLE		0x11
#define V2_CMD_ONEANGLE			0x12
#define V2_CMD_SERVOADJANGLE	0x13
#define V2_CMD_ONEADJANGLE		0x14

#define V2_CMD_LOCKSERVO		0x21
#define V2_CMD_UNLOCKSERVO		0x22

#define V2_CMD_SERVOMOVE		0x23

#define V2_CMD_LED				0x31

#define V2_CMD_GET_ADHEADER		0x61
#define V2_CMD_GET_ADDATA		0x62
#define V2_CMD_GET_ADPOSE		0x63

#define V2_CMD_SAVE_ADHEADER	0x71
#define V2_CMD_SAVE_ADPOSE		0x72


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

	// V2_CMD_ENABLE should always available
	if ((!enable_V2) && (cmd[3] != V2_CMD_ENABLE)) {
		if (debug) {
			DEBUG.print(F("V2 Command disabled:"));
			for (int i = 0; i < len + 4; i++) {
				DEBUG.printf(" %02X", cmd[i]);
			}
			DEBUG.println();
		}  
		return true;
	}

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

		case V2_CMD_ENABLE:
			V2_CommandEnable(cmd);
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

		case V2_CMD_SERVOMOVE:
			V2_ServoMove(cmd);
			break;

		case V2_CMD_LED:
			V2_SetLED(cmd);
			break;

		case V2_CMD_READSPIFFS:
			V2_ReadSPIFFS();
			break;

		case V2_CMD_WRITESPIFFS:
			V2_WriteSPIFFS();
			break;

		case V2_CMD_GET_ADHEADER:
			V2_GetAdHeader(cmd);
			break;

		case V2_CMD_GET_ADDATA:
			V2_GetAdData(cmd);
			break;

		case V2_CMD_GET_ADPOSE:
			V2_GetAdPose(cmd);
			break;
		
		case V2_CMD_SAVE_ADHEADER:
			V2_SaveAdHeader(cmd);
			break;

		case V2_CMD_SAVE_ADPOSE:
			V2_SaveAdPose(cmd);
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
	SetDebug(status);
	if (debug) DEBUG.printf("Debug mode %s\n", (status ? "enabled" : "disabled"));
}

void V2_SetDevMode(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_SetDevMode]"));
	devMode = (cmd[4] ? 1 : 0);
	if (debug) DEBUG.printf("Developer mode %s\n", (devMode ? "enabled" : "disabled"));
}

void V2_CommandEnable(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_CommandEnable]"));
	byte result[10];
	// Should have only 2 options:
	//   cmd[2] = 7 : set all 5 flags
	//   cmd[2] = 2 : enquiry  (just assume all non-7 length is for enquiry)
	if (cmd[2] == 7) {
		enable_V1 = cmd[4];
		enable_V2 = cmd[5];
		enable_UBTBT = cmd[6];
		enable_UBTCB = cmd[7];
		enable_UBTSV = cmd[8];
	} 
	result[2] = 6;
	result[3] = (enable_V1 ? 1 : 0);
	result[4] = (enable_V2 ? 1 : 0);
	result[5] = (enable_UBTBT ? 1 : 0);
	result[6] = (enable_UBTCB ? 1 : 0);
	result[7] = (enable_UBTSV ? 1 : 0);

	V2_SendResult(result);
	// Serial.write(result, 5);
}

#pragma endregion

#pragma region V2_CMD_SERVOANGLE / V2_CMD_ONEANGLE

void V2_GetServoAngle() {
	if (debug) DEBUG.println(F("[V2_GetServoAngle]"));
	uint16_t len = 33;
	byte result[len+4];
	result[2] = len;
	byte *ptr = result + 3;
	UBT_GetServoAngle(ptr);
	V2_SendResult(result);
}

void V2_GetOneAngle(byte *cmd) {

	if (debug) DEBUG.println(F("[V2_GetOneAngle]"));
	byte result[8];
	result[2] = 4;

	if (cmd[2] == 3) {
		byte id = cmd[4];
		result [3] = id;
		if ((id) && (id <= 16) && (servo.exists(id))) {
				if (servo.isLocked(id)) {
					result[4] = servo.lastAngle(id);
					result[5] = 1;
				} else {
					result[4] = servo.getPos(id);
					result[5] = 0;
				}
		} else {
			result[4] = 0xff;
			result[5] = 0xff;
		}
	} else {
		result[3] = 0x00;
		result[4] = 0xff;
		result[5] = 0xff;
	}

	V2_SendResult(result);

}


#pragma endregion

#pragma region V2_CMD_SERVOADJANGLE / V2_CMD_ONEADJANGLE

void V2_GetServoAdjAngle() {
	if (debug) DEBUG.println(F("[V2_GetServoAdjAngle]"));
	uint16_t len = 33;
	byte result[len+4];
	result[2] = len;
	byte *ptr = result + 3;
	UBT_GetServoAdjAngle(ptr);
	V2_SendResult(result);

}

void V2_GetOneAdjAngle(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_GetOneAdjAngle]"));
	byte result[8];
	result[2] = 4;

	if (cmd[2] == 3) {
		byte id = cmd[4];
		result [3] = id;
		if (cmd[2] > 2) id = cmd[4];
		if ((id) && (id <= 16) && (servo.exists(id))) {
			uint16_t  adjAngle = servo.getAdjAngle(id);
			result[4] = adjAngle / 256;
			result[5] = adjAngle % 256;
		} else {
			result[4] = 0x7F;
			result[5] = 0x7F;
		}
	} else {
		result[3] = 0x00;
		result[4] = 0x7F;
		result[5] = 0x7F;
	}
	V2_SendResult(result);
}

#pragma endregion

#pragma region V2_CMD_LOCKSERVO / V2_CMD_LOCKSERVO

void V2_LockServo(byte *cmd, bool goLock) {
	if (debug) DEBUG.printf("[V2_LockServo - %s]\n", (goLock ? "Lock" : "Unlock"));

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
		if (debug) DEBUG.printf("All servo: %d\n",cnt);
	} else {
		int reqCnt = cmd[2] - 2;
		for (int i = 0; i < reqCnt; i++) {
			byte id = cmd[4 + i];
			if (id) {
				// check if already exists
				for (int j = 0; j < i; j++) {
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
		result[4] = cnt;
		if (debug) DEBUG.printf("Selected %d servos : %d\n", reqCnt, cnt);
	}
	result[2] = 3 +  result[4] * 2;
	V2_SendResult(result);
}

#pragma endregion

#pragma region  V2_CMD_SERVOMOVE

void V2_ServoMove(byte *cmd) {
	byte moveAngle, moveTime;
	if (debug) DEBUG.println(F("[V2_ServoMove]"));
	int cnt = (cmd[2] - 2) / 3;
	byte moveParm[32];
	memset(moveParm, 0xFF, 32);
	int pos;

	if ((cnt == 1) && (cmd[4] == 0)) {
		moveAngle = cmd[5];
		moveTime = cmd[6];
		for (byte id = 1; id <= 16; id++ ) {
			if (servo.exists(id)) {
				pos = 2 * (id - 1);
				moveParm[pos] = moveAngle;
				moveParm[pos+1] = moveTime;
			}
		}
	} else {
		byte id;
		for (int i = 0; i < cnt; i++) {
			pos = 4 + 3 * i;
			id = cmd[pos];
			if ((id != 0) && servo.exists(id) && (moveParm[2*(id-1)] == 0xFF)) {
				moveAngle = cmd[pos+1];
				moveTime = cmd[pos+2];
				pos = 2 * (id - 1);
				moveParm[pos] = moveAngle;
				moveParm[pos+1] = moveTime;
			} else {
				if (debug) DEBUG.printf("Servo ID: %02d is invalid/duplicated\n", id);
			}
		}
	}
	
	if (debug) {
		DEBUG.println("move parameters:");
		for (int i=0; i < 32; i++) {
			DEBUG.printf("%02X ", moveParm[i]);
		}
		DEBUG.println();
	}

	int moveCnt = 0;
	byte result[54]; // max: A9 9A {len} {cnt} (3 * 16) {sum} ED = 48 + 6 = 54
	for (int id = 1; id <= 16; id++) {
		pos = 2 * (id - 1);
		if (moveParm[pos] != 0xFF) {
			int resultPos = 4 + 3 * moveCnt;
			result[resultPos] = id;
			moveAngle = moveParm[pos];
			moveTime = moveParm[pos+1];
			result[resultPos+1] = moveAngle;
			result[resultPos+2] = moveTime;
			if (debug) DEBUG.printf("Move servo %02d to %d [%02X], time: %d [%02X]\n", id, moveAngle, moveAngle, moveTime, moveTime);
			servo.move(id, moveAngle, moveTime);
			moveCnt++;
		}
	}
	result[2] = 2 + 3 * moveCnt;
	result[3] = moveCnt;
	V2_SendResult(result);
	
}

#pragma endregion


#pragma region V2_CMD_LED

void V2_SetLED(byte *cmd) {
	DEBUG.print(F("[V2_SetLED]"));
	byte id, mode;
	if ((cmd[2] == 4) && (cmd[4] == 0)) {
		mode = (cmd[5] ? 0 : 1);
		for (int id = 1; id <= 16; id++) {
			if (servo.exists(id)) {
				if (debug) DEBUG.printf("Turn servo %02d LED %s\n", id, (mode ? "OFF" : "ON"));
				servo.setLED(id, mode);
			} 
		}
	} else {
		int cnt = (cmd[2] - 2) / 2;
		for (int i = 0; i < cnt; i++) {
			int pos = 4 + 2 * i;
			id = cmd[pos];
			if (servo.exists(id)) {
				mode = (cmd[pos+1] ? 0 : 1);
				if (debug) DEBUG.printf("Turn servo %02d LED %s\n", id, (mode ? "OFF" : "ON"));
				servo.setLED(id, mode);
			}
		}
	}
}

#pragma endregion

#pragma region ActionTable

bool V2_CheckActionId(byte actionId) {

	if (debug) DEBUG.printf("Current Id: %d; requested Id: %d\n", actionData.id(), actionId);
	if (actionId != actionData.id() ) {
		if (!actionData.ReadSPIFFS(actionId)) {
			// anything to do if still fail to read
			if (debug) DEBUG.println("Fail to get Id matched.");
			return false;
		}
	} 
	return true;
}


void V2_GetAdHeader(byte *cmd) {
	if (debug) DEBUG.println("[V2_GetAdHeader]");
	byte aId = cmd[4];	
	if (!V2_CheckActionId(aId)) {
		return;	
	}
	V2_SendResult((byte *) actionData.Header());

}

void V2_GetAdData(byte *cmd) {
	
	DEBUG.print(F("[V2_GetAdData]"));
	byte aId = cmd[4];	
	if (!V2_CheckActionId(aId)) {
		return;	
	}
	byte poseCnt = actionData.PoseCnt();
	byte dataSize = poseCnt * AD_POSE_SIZE;
	byte len = dataSize + 2;
	byte d1[4] = { 0xA9, 0x9A, len, V2_CMD_GET_ADDATA};
	byte sum = len + V2_CMD_GET_ADDATA;
	byte *data = actionData.Data();
	for (int i = 0; i < dataSize; i++) {
		sum += data[i];
	}
	Serial.write(d1, 4);
	Serial.write(data, dataSize);
	Serial.write(sum);
	Serial.write(0xED);
	
}


void V2_GetAdPose(byte *cmd) {
	DEBUG.print(F("[V2_GetAdPose]"));
	byte aId = cmd[4];
	byte aPose = cmd[5];
}

void V2_SaveAdHeader(byte *cmd) {
	DEBUG.print(F("[V2_SaveAdHeader]"));
	byte aId = cmd[4];	
}

void V2_SaveAdPose(byte *cmd) {
	DEBUG.print(F("[V2_SaveAdPose]"));
	byte aId = cmd[4];
	byte aPose = cmd[5];
}

#pragma endregion


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


