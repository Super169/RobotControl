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
//   41 - Play Action               : A9 9A 03 41 00 43 ED
//   42 - Play Combo                : A9 9A 03 42 00 44 ED
//   4F - Stop playing              : A9 9A 02 4F 51 ED
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

#define V2_CMD_PLAYACTION		0x41
#define V2_CMD_PLAYCOMBO		0x42
#define V2_CMD_STOPPLAY			0x4F

#define V2_CMD_GET_ADHEADER		0x61
#define V2_CMD_GET_ADPOSE		0x62
#define V2_CMD_GET_ADDATA		0x63

#define V2_CMD_UPD_ADHEADER		0x71
#define V2_CMD_UPD_ADPOSE		0x72
#define V2_CMD_UPD_ADNAME		0x74


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

	// Special handle for stop play as there has no need to check for action playing
	if (cmd[3] == V2_CMD_STOPPLAY) {
		V2_ResetAction();
		V2_SendSingleByteResult(cmd, 0);
		return true;
	}

	// No command except STOP is allowed when action playing
	// Even some command will not affect the action, but for safety, do not play any of them.
	// In fact, there shoudl have such command sending from controller when action is playing.
	// All return the generic result:
	//   A9 9A 03 {cmd} FF {sum} ED
	if (V2_ActionPlaying) {
		V2_SendSingleByteResult(cmd, 0xFF);
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
			V2_GetServoAngle(cmd);
			break;

		case V2_CMD_ONEANGLE:
			V2_GetOneAngle(cmd);
			break;

		case V2_CMD_SERVOADJANGLE:
			V2_GetServoAdjAngle(cmd);
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

		case V2_CMD_PLAYACTION:
			V2_PlayAction(cmd);
			break;

		case V2_CMD_PLAYCOMBO:
			V2_PlayCombo(cmd);
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
		
		case V2_CMD_UPD_ADHEADER:
			V2_UpdateAdHeader(cmd);
			break;

		case V2_CMD_UPD_ADPOSE:
			V2_UpdateAdPose(cmd);
			break;

		case V2_CMD_UPD_ADNAME:
			V2_UpdateAdName(cmd);
			break;

		case V2_CMD_READSPIFFS:
			V2_ReadSPIFFS(cmd);
			break;

		case V2_CMD_WRITESPIFFS:
			V2_WriteSPIFFS(cmd);
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

void V2_SendSingleByteResult(byte *cmd, byte data) {
	byte result[7];
	result[2] = 3;
	result[3] = cmd[3];
	result[4] = data;
	V2_SendResult(result);
}

#pragma region V2_CMD_RESET / V2_CMD_DEBUG / V2_CMD_DEVMODE

void V2_Reset(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_Reset]"));
	servo.end();
	delay(200);
	servo.begin();
	byte showAngle = 0;
	if (cmd[2] > 2) showAngle = cmd[4];
	if ((showAngle) && (showAngle != '0')) {
		V2_GetServoAngle(cmd);
	} else {
		V2_SendSingleByteResult(cmd, 0);
	}
}

void V2_SetDebug(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_SetDebug]"));
	byte status = (cmd[4] ? 1 : 0);
	if (debug && !status) DEBUG.printf("Disable debug mode\n");
	SetDebug(status);
	if (debug) DEBUG.printf("Debug mode %s\n", (status ? "enabled" : "disabled"));
	V2_SendSingleByteResult(cmd, 0);
}

void V2_SetDevMode(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_SetDevMode]"));
	devMode = (cmd[4] ? 1 : 0);
	if (debug) DEBUG.printf("Developer mode %s\n", (devMode ? "enabled" : "disabled"));
	V2_SendSingleByteResult(cmd, 0);
}

void V2_CommandEnable(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_CommandEnable]"));
	byte result[11];
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
	result[2] = 7;
	result[3] = cmd[3];
	result[4] = (enable_V1 ? 1 : 0);
	result[5] = (enable_V2 ? 1 : 0);
	result[6] = (enable_UBTBT ? 1 : 0);
	result[7] = (enable_UBTCB ? 1 : 0);
	result[8] = (enable_UBTSV ? 1 : 0);

	V2_SendResult(result);
	// Serial.write(result, 5);
}

#pragma endregion

#pragma region V2_CMD_SERVOANGLE / V2_CMD_ONEANGLE

void V2_GetServoAngle(byte *cmd) {
	
	if (debug) DEBUG.println(F("[V2_GetServoAngle]"));
	uint16_t len = 34;
	byte result[len+4];
	
	result[2] = len;
	result[3] = cmd[3];
	byte *ptr = result + 4;
	
	UBT_GetServoAngle(ptr);
	V2_SendResult(result);
	
}

void V2_GetOneAngle(byte *cmd) {

	if (debug) DEBUG.println(F("[V2_GetOneAngle]"));
	byte result[9];
	result[2] = 4;
	result[3] = cmd[3];

	if (cmd[2] == 3) {
		byte id = cmd[4];
		result [4] = id;
		if ((id) && (id <= 16) && (servo.exists(id))) {
				if (servo.isLocked(id)) {
					result[5] = servo.lastAngle(id);
					result[6] = 1;
				} else {
					result[5] = servo.getPos(id);
					result[6] = 0;
				}
		} else {
			result[5] = 0xff;
			result[6] = 0xff;
		}
	} else {
		result[4] = 0x00;
		result[5] = 0xff;
		result[6] = 0xff;
	}
	V2_SendResult(result);

}


#pragma endregion

#pragma region V2_CMD_SERVOADJANGLE / V2_CMD_ONEADJANGLE

void V2_GetServoAdjAngle(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_GetServoAdjAngle]"));
	uint16_t len = 34;
	byte result[len+4];
	result[2] = len;
	result[3] = cmd[3];
	byte *ptr = result + 4;
	UBT_GetServoAdjAngle(ptr);
	V2_SendResult(result);

}

void V2_GetOneAdjAngle(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_GetOneAdjAngle]"));
	byte result[9];
	result[2] = 4;
	result[3] = cmd[3];

	if (cmd[2] == 3) {
		byte id = cmd[4];
		result [4] = id;
		if (cmd[2] > 2) id = cmd[4];
		if ((id) && (id <= 16) && (servo.exists(id))) {
			uint16_t  adjAngle = servo.getAdjAngle(id);
			result[5] = adjAngle / 256;
			result[6] = adjAngle % 256;
		} else {
			result[5] = 0x7F;
			result[6] = 0x7F;
		}
	} else {
		result[4] = 0x00;
		result[5] = 0x7F;
		result[6] = 0x7F;
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
	byte result[55]; // max: A9 9A {len} {cmd} {cnt} (3 * 16) {sum} ED = 48 + 7 = 55
	for (int id = 1; id <= 16; id++) {
		pos = 2 * (id - 1);
		if (moveParm[pos] != 0xFF) {
			int resultPos = 5 + 3 * moveCnt;
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
	result[3] = cmd[3];
	result[4] = moveCnt;
	V2_SendResult(result);
	
}

#pragma endregion


#pragma region V2_CMD_LED

void V2_SetLED(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_SetLED]"));
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
	V2_SendSingleByteResult(cmd, 0);
}

#pragma endregion


#pragma region Play Action / Combo 

void V2_PlayAction(byte *cmd) {
	V2_ResetAction();
	if (cmd[2] != 3) {
		V2_SendSingleByteResult(cmd, 1);
		return;
	}
	byte actionId = cmd[4];
	if (actionData.id() != actionId) {
		// need to load actionData
		if (!actionData.ReadSPIFFS(actionId)) {
			V2_SendSingleByteResult(cmd, 2);
			return;
		}
	}
	// Just for safety, check the poseCnt again.  
	// May skip this step if those information is always updated.
	actionData.RefreshActionInfo();

	if (actionData.PoseCnt() > 0) {
		V2_ActionCombo = 0;
		V2_NextAction = actionId;
		V2_NextPose = 0;
		V2_NextPlayMs = millis();
		V2_ActionPlaying = true;
	}
	V2_SendSingleByteResult(cmd, 0);
}

void V2_PlayCombo(byte *cmd) {
	
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
	actionData.Header()[3] = cmd[3];
	if (!V2_CheckActionId(aId)) {
		V2_SendSingleByteResult(cmd, 0x01);
		return;	
	}
	V2_SendResult((byte *) actionData.Header());

}

void V2_GetAdData(byte *cmd) {
	
	if (debug) DEBUG.println(F("[V2_GetAdData]"));
	byte aId = cmd[4];	
	if (!V2_CheckActionId(aId)) {
		V2_SendSingleByteResult(cmd, 0x01);
		return;	
	}
	// Data size can over 256, single byte of len cannot be used_block
	// Special handle for this result set or do not allow such retrieval
	// - add special logic, make len = 0
	// - then acual len will be stored in [4] [5] with Hing-Low order
	// - data strt at offset 6 instead of 4
	//
	byte poseCnt = actionData.PoseCnt();
	int dataSize = poseCnt * AD_POSE_SIZE;
	int len = dataSize + 6;  // {len} {cmd} {len_H} {len_L} {aId} {poseCnt} {data} => datasize + 6
	byte len_H = (byte) (len / 256);
	byte len_L = (byte) (len % 256);
	byte d1[8] = { 0xA9, 0x9A, 0x00, cmd[3], len_H, len_L, aId, poseCnt};
	byte sum = len + cmd[3] + len_H + len_L + aId + poseCnt;
	byte *data = actionData.Data();
	for (int i = 0; i < dataSize; i++) {
		sum += data[i];
	}
	Serial.write(d1, 8);
	Serial.write(data, dataSize);
	Serial.write(sum);
	Serial.write(0xED);
}


void V2_GetAdPose(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_GetAdPose]"));
	byte aId = cmd[4];
	if (!V2_CheckActionId(aId)) {
		V2_SendSingleByteResult(cmd, 0x01);
		return;	
	}
	byte pId = cmd[5];
	byte result[40];
	byte len = AD_POSE_SIZE + 4; // {len} {cmd} {aId} {pId} {poseData} = pose_size + 4
	result[2] = len;
	result[3] = cmd[3];
	result[4] = aId;
	result[5] = pId;

	byte * toPos = result + 6;
	byte * fromPos = actionData.Data() + AD_POSE_SIZE * pId;
	memcpy(toPos, fromPos, AD_POSE_SIZE);
	V2_SendResult(result);
}

void V2_UpdateAdHeader(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_UpdateAdHeader]"));
	byte aId = cmd[4];	

	// Length should be header size - 4
	if (cmd[2] != (AD_HEADER_SIZE - 4)) {
		if (debug) DEBUG.printf("Invalid length: \n", cmd[2]);
		V2_SendSingleByteResult(cmd, 1);
		return;
	}

	// Action ID must be matched.  i.e. must GET before UPDATE
	if (cmd[4] != actionData.Header()[4]) {
		if (debug) DEBUG.printf("ID not matched: %d (current: %d)\n", cmd[4], actionData.Header()[4]);
		V2_SendSingleByteResult(cmd, 2);
		return;
	}
	for (int i = 0; i < AD_HEADER_SIZE; i++) {
		actionData.Header()[i] = cmd[i];
	}
	
	if (debug) DEBUG.printf("Action %d header updated\n", aId);
	V2_SendSingleByteResult(cmd, 0);
}

void V2_UpdateAdPose(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_UpdateAdPose]"));
	
	// Length should be {len} {actionId} {poseId} {data} => pose datasize + 3
	if (cmd[2] != (AD_POSE_SIZE + 3)) {
		if (debug) DEBUG.printf("Invalid length: \n", cmd[2]);
		V2_SendSingleByteResult(cmd, 1);
		return;
	}	

	byte aId = cmd[4];
	byte pId = cmd[5];

	// Action ID must be matched.  i.e. must GET before UPDATE
	if (aId != actionData.Header()[4]) {
		if (debug) DEBUG.printf("ID not matched: %d (current: %d)\n", aId, actionData.Header()[4]);
		V2_SendSingleByteResult(cmd, 2);
		return;
	}

	int startPos = AD_POSE_SIZE * pId;
	int dataPos = 6;
	for (int i = 0; i < AD_POSE_SIZE; i++) {
		actionData.Data()[startPos + i] = cmd[dataPos + i];
	}

	V2_SendSingleByteResult(cmd, 0);

}

void V2_UpdateAdName(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_UpdateAdName]"));
	byte id = cmd[4];
	if (actionData.id() != id) {
		V2_SendSingleByteResult(cmd, 1);
		return;
	}
	if (cmd[5] > AD_NAME_SIZE) {
		V2_SendSingleByteResult(cmd, 2);
		return;
	}
	byte * startPos = actionData.Header() + AD_OFFSET_NAME;
	memset(startPos, 0, AD_NAME_SIZE);
	byte len = cmd[5];

	byte * copyPos = cmd + 6;
	if (debug) DEBUG.print("Action Name: ");
	for (int i = 0; i < len; i++) { 
		if (debug) DEBUG.print((char) cmd[6 + i]);
		actionData.Header()[AD_OFFSET_NAME + i] = cmd[6 + i];
	}
	if (debug) DEBUG.println();
	V2_SendSingleByteResult(cmd, 0);
}

#pragma endregion


#pragma region SPIFFS: V2_CMD_READSPIFFS / V2_CMD_WRITESPIFFS

void V2_ReadSPIFFS(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_ReadSPIFFS]"));
	byte success = V2_UBT_ReadSPIFFS(cmd);
	V2_SendSingleByteResult(cmd, success);
}

byte V2_UBT_ReadSPIFFS(byte *cmd) {
	return 0;
}

void V2_WriteSPIFFS(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_WriteSPIFFS]"));
	byte success = V2_UBT_WriteSPIFFS(cmd);
	V2_SendSingleByteResult(cmd, success);
}

byte V2_UBT_WriteSPIFFS(byte *cmd) {
	return 0;
}

#pragma endregion
