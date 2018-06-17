#include "robot.h"
#include "V2_Command.h"

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
//   04 - Get Config (fix)          : A9 9A 02 04 06 ED
//   0A - Command Enable (fix)		: A9 9A 02 0A 0C ED
//                                    A9 9A 07 0A 00 01 01 01 01 15 ED
//   0B - Check battery (fix)       : A9 9A 02 0B 0D ED
//   11 - Get Angle (fix) 			: A9 9A 02 11 13 ED
//   12 - Get One Angle (fix)		: A9 9A 03 12 01 16 ED
//   13 - Get Adj Angle (fix)		: A9 9A 02 13 15 ED
//   14 - Get One Adj Angle (fix) 	: A9 9A 03 14 01 18 ED
//	 21 - Lock servo (var)			: A9 9A 06 21 01 02 03 04 31 ED
//	 22 - Unlock servo (var)		: A9 9A 05 22 01 02 03 2D ED
//	 23 - Servo move (var)			: A9 9A 05 23 00 5A A0 22 ED
//									: A9 9A 08 23 01 5A A0 02 00 A0 C8 ED 
//	 24 - Set LED (var)				: A9 9A 04 24 00 01 29 ED
//                                  : A9 9A 06 24 01 00 02 01 2E ED
//   31 - Set Head LED (fix)        : A9 9A 03 31 00 34 ED
//                                    A9 9A 03 31 01 35 ED
//	 32	- Stop MP3 (fix)			: A9 9A 02 32 34 ED
//   33 - Play File (fix)			: A9 9A 04 33 00 01 38 ED
//                                    A9 9A 04 33 FF 02 38 ED
//   34 - Play MP3 File (fix)		: A9 9A 03 34 01 38 ED
//   35 - Play Avert File (fix)		: A9 9A 03 35 01 39 ED
//   36 - MP3 Set Volume (fix)		: A9 9A 04 36 00 0F 49 ED
//                                    A9 9A 04 36 01 01 3C ED



//   41 - Play Action               : A9 9A 03 41 00 43 ED
//   42 - Play Combo                : A9 9A 03 42 00 44 ED
//   4F - Stop playing              : A9 9A 02 4F 51 ED
//   61 - Read Action Header (fix) 	: A9 9A 03 61 01 65 ED
//   62 - Read Action Data (fix)	: A9 9A 03 62 01 65 ED

//   71 - Update action header      : {Refer to ActionInfo}
//   72 - Update action pose        : {refer to PoseInfo}	
//   74 - Update action name	    : A9 9A 0B 74 01 07  44 65 66 61 75 6C 74 D7 ED
//   75 - Delete action file	    : A9 9A 03 75 00 78 ED



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
		case V2_CMD_DEBUG:
			V2_SetDebug(cmd);
			return true;
			break;
		
		case V2_CMD_DEVMODE:			
			V2_SetDevMode(cmd);
			return true;
			break;

		case V2_CMD_CHECK_BATTERY:
			V2_CheckBattery(cmd);
			return true;
			break;

		case V2_CMD_MP3_STOP:
			V2_Mp3Stop(cmd);
			return true;
			break;

		case V2_CMD_MP3_SETVOLUME:
			V2_Mp3SetVolume(cmd);
			return true;
			break;

		case V2_CMD_STOPPLAY:
			V2_ResetAction();
			if (config.mp3Enabled()) {
				mp3.begin();
				mp3.stop();
				mp3.end();
			}
			V2_SendSingleByteResult(cmd, 0);
			return true;
			break;

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

		case V2_CMD_ENABLE:
			V2_CommandEnable(cmd);
			break;

		case V2_CMD_GETCONFIG:
			V2_GetConfig(cmd);
			break;

		case V2_CMD_SETCONFIG:
			V2_SetConfig(cmd);
			break;

		case V2_CMD_DEFAULTCONFIG:
			V2_DefaultConfig(cmd);
			break;

		case V2_CMD_GET_NETWORK:
			V2_GetNetwork(cmd);
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

		case V2_CMD_SETADJANGLE:
			V2_SetAdjAngle(cmd);
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

		case V2_CMD_SET_HEADLED:
			V2_SetHeadLED(cmd);
			break;

		case V2_CMD_MP3_PLAYFILE:
			V2_Mp3PlayFile(cmd);
			break;

		case V2_CMD_MP3_PLAYMP3:
			V2_Mp3PlayMp3(cmd);
			break;

		case V2_CMD_MP3_PLAYADVERT:
			V2_Mp3PlayAdvert(cmd);
			break;



		case V2_CMD_PLAYACTION:
			V2_PlayAction(cmd);
			break;

		case V2_CMD_PLAYCOMBO:
			V2_PlayCombo(cmd);
			break;

		case V2_CMD_GET_ADLIST:
			V2_GetAdList(cmd);
			break;

		case V2_CMD_GET_ADHEADER:
			V2_GetAdHeader(cmd);
			break;

		case V2_CMD_GET_ADPOSE:
			V2_GetAdPose(cmd);
			break;
			
		case V2_CMD_GET_COMBO:
			V2_GetCombo(cmd);
			break;

		case V2_CMD_UPD_COMBO:
			V2_UpdateCombo(cmd);
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

		case V2_CMD_DEL_ACTION:
			V2_DeleteActionFile(cmd);
			break;
/*
		case V2_CMD_READSPIFFS:
			V2_ReadSPIFFS(cmd);
			break;

		case V2_CMD_WRITESPIFFS:
			V2_WriteSPIFFS(cmd);
			break;
*/
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

#pragma region V2_CMD_RESET / V2_CMD_DEBUG / V2_CMD_DEVMODE / V2_CMD_GETCONFIG / V2_CMD_SETCONFIG

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
		V2_SendSingleByteResult(cmd, RESULT::SUCCESS);
	}
}

void V2_SetDebug(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_SetDebug]"));
	byte status = (cmd[4] ? 1 : 0);
	if (debug && !status) DEBUG.printf("Disable debug mode\n");
	SetDebug(status);
	config.setDebug(status);
	config.writeConfig();
	if (debug) DEBUG.printf("Debug mode %s\n", (status ? "enabled" : "disabled"));
	V2_SendSingleByteResult(cmd, RESULT::SUCCESS);
}

void V2_SetDevMode(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_SetDevMode]"));
	devMode = (cmd[4] ? 1 : 0);
	if (debug) DEBUG.printf("Developer mode %s\n", (devMode ? "enabled" : "disabled"));
	V2_SendSingleByteResult(cmd, RESULT::SUCCESS);
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
}

void V2_GetConfig(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_GetConfig]"));
	config.Data()[3] = cmd[3];

	V2_SendResult((byte *) config.Data());

}

void V2_SetConfig(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_SetConfig]"));
	if (cmd[2] != RC_CONFIG_DATA_SIZE) {
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_SIZE);
		return;
	}

	memcpy((byte *) config.Data(), (byte *) cmd, RC_RECORD_SIZE);
	byte result = config.writeConfig();
	V2_SendSingleByteResult(cmd, result);
}

void V2_DefaultConfig(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_DefaultConfig]"));
	config.initConfig();
	byte result = config.writeConfig();
	V2_SendSingleByteResult(cmd, result);
}

#pragma endregion


void V2_CheckBattery(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_CheckBattery]"));
	byte result[9];
	result[2] = 5;
	result[3] = cmd[3];
	uint16_t v = analogRead(0);
	byte iPower = GetPower(v);
	result[4] = iPower;
	result[5] = v >> 8;
	result[6] = v & 0xFF;

	if (debug) DEBUG.printf("[V2_CheckBattery] - %d -> %d%%\n", v, iPower);

	V2_SendResult(result);
}

void V2_GetNetwork(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_GetNetwork]"));
	byte result[60];
	memset(result, 0, 60);
	result[2] = 56;
	result[3] = cmd[3];
	result[4] = NetworkMode;
	String tmp;
	char buf[20];
	
	switch (NetworkMode) {
		case NETWORK_ROUTER:
			memset(buf, 0, 20);
			tmp = WiFi.SSID();
			tmp.toCharArray(buf, 20);
			memcpy((byte *)(result + 5), buf, 20);
			tmp = WiFi.localIP().toString();
			tmp.toCharArray(buf, 20);
			memcpy((byte *)(result + 25), buf, 20);
			break;
		case NETWORK_AP:
			memcpy((byte *)(result + 5), AP_Name, strlen(AP_Name));
			/*
			for (size_t i = 0; i < strlen(AP_Name); i++ ) {
				result[5+i] = AP_Name[i];
			}
			*/
			tmp = WiFi.softAPIP().toString();
			tmp.toCharArray(buf, 20);
			memcpy((byte *)(result + 25), buf, 20);
			break;
	}
	
	result[45] = port >> 8;
	result[46] = port & 0xFF;
	V2_SendResult(result);
}



#pragma region V2_CMD_SERVOANGLE / V2_CMD_ONEANGLE

void V2_GetServoAngle(byte *cmd) {
	
	if (debug) DEBUG.println(F("[V2_GetServoAngle]"));
	uint16_t len = 2 * config.maxServo() + 2;
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
	result[2] = 5;
	result[3] = cmd[3];

	if (cmd[2] == 3) {
		byte id = cmd[4];
		result [4] = id;
		if ((id) && (id <= config.maxServo()) && (servo.exists(id))) {
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
	// uint16_t len = 34;
	uint16_t len = 2 * config.maxServo() + 2;
	byte result[len+4];
	result[2] = len;
	result[3] = cmd[3];
	byte *ptr = result + 4;
	UBT_GetServoAdjAngle(ptr);
	V2_SendResult(result);

}

// A9 9A 04 {cmd} {id} {high} {low} {sum} ED

void V2_GetOneAdjAngle(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_GetOneAdjAngle]"));
	byte result[9];
	result[2] = 5;
	result[3] = cmd[3];

	if (cmd[2] == 3) {
		byte id = cmd[4];
		result [4] = id;
		if (cmd[2] > 2) id = cmd[4];
		if ((id) && (id <= config.maxServo()) && (servo.exists(id))) {
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

void V2_SetAdjAngle(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_SetAdjAngle]"));
	byte id = cmd[4];
	if (cmd[2] == 5) {
		byte id = cmd[4];
		uint16_t adjSet = (cmd[5] << 8) | cmd[6];
		DEBUG.printf("Set angle: %d\n", adjSet);
		uint16_t adjResult = servo.setAdjAngle(id, adjSet);
		DEBUG.printf("Result angle: %d\n", adjResult);
		V2_SendSingleByteResult(cmd, (adjSet == adjResult ? 0 : 2));
	} else {
		V2_SendSingleByteResult(cmd, 1);
	}
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
		for (int id = 1; id <= config.maxServo(); id++)  {
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
	byte moveParm[2 * config.maxServo()];
	memset(moveParm, 0xFF, 2 * config.maxServo());
	int pos;

	if ((cnt == 1) && (cmd[4] == 0)) {
		moveAngle = cmd[5];
		moveTime = cmd[6];
		for (byte id = 1; id <= config.maxServo(); id++ ) {
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
		for (int i=0; i < 2 * config.maxServo(); i++) {
			DEBUG.printf("%02X ", moveParm[i]);
		}
		DEBUG.println();
	}

	int moveCnt = 0;
	// byte result[55]; // max: A9 9A {len} {cmd} {cnt} (3 * 16) {sum} ED = 48 + 7 = 55
	int arraySize = 3 * config.maxServo() + 7;
	byte result[arraySize]; // max: A9 9A {len} {cmd} {cnt} (3 * 16) {sum} ED = 48 + 7 = 55
	for (int id = 1; id <= config.maxServo(); id++) {
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
	result[2] = 3 + 3 * moveCnt;
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
		mode = (cmd[5] ? 1 : 0);
		for (int id = 1; id <= config.maxServo(); id++) {
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
				mode = (cmd[pos+1] ? 1 : 0);
				if (debug) DEBUG.printf("Turn servo %02d LED %s\n", id, (mode ? "OFF" : "ON"));
				servo.setLED(id, mode);
			}
		}
	}
	V2_SendSingleByteResult(cmd, 0);
}

#pragma endregion


#pragma region Set Head LED

void V2_SetHeadLED(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_SetHeadLED]"));
	bool status = (cmd[4] == 0x01);
	SetHeadLed(status);
	if (debug) DEBUG.printf("Turn Head LED %s\n", (status ? "ON" : "OFF"));
	V2_SendSingleByteResult(cmd, 0);
}

#pragma endregion

#pragma region MP3 Player

void V2_Mp3Stop(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_Mp3Stop]"));
	if (config.mp3Enabled()) {
		mp3.begin();
		mp3.stop();
		mp3.end();
	}
	V2_SendSingleByteResult(cmd, 0);
}

void V2_Mp3PlayFile(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_Mp3PlayFile]"));
	if (!config.mp3Enabled()) return;
	byte folderSeq = cmd[4];
	byte fileSeq = cmd[5];
	mp3.begin();
	if (folderSeq == 0xff) {
		mp3.playFile(fileSeq);
	} else {
		mp3.playFolderFile(folderSeq, fileSeq);
	}
	mp3.end();
	V2_SendSingleByteResult(cmd, 0);
}

void V2_Mp3PlayMp3(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_Mp3PlayMp3]"));
	if (!config.mp3Enabled()) return;
	byte fileSeq = cmd[4];
	mp3.begin();
	mp3.playMp3File(fileSeq);
	mp3.end();
	V2_SendSingleByteResult(cmd, 0);
}

void V2_Mp3PlayAdvert(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_Mp3PlayAdvert]"));
	if (!config.mp3Enabled()) return;
	byte fileSeq = cmd[4];
	mp3.begin();
	mp3.playAdFile(fileSeq);
	mp3.end();
	V2_SendSingleByteResult(cmd, 0);
}

void V2_Mp3SetVolume(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_Mp3SetVolume]"));
	if (!config.mp3Enabled()) return;
	byte mode = cmd[4];
	byte value = cmd[5];	
	int newVol = mp3_Vol;
	switch (mode) {
		case 1:
			newVol += value;
			break;
		case 2:
			newVol -= value;
			break;
		default:
			newVol = value;
			break;
	}
	if (newVol < 0) {
		newVol = 0;
	} else if (newVol > 30) {
		newVol = 30;
	} 
	mp3.begin();
	mp3.setVol((uint8_t) newVol);
	delay(1);
	mp3_Vol = mp3.getVol();
	mp3.end();
	V2_SendSingleByteResult(cmd, mp3_Vol);
}

#pragma endregion



#pragma region Play Action / Combo 

void V2_PlayAction(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_PlayAction]"));
	V2_ResetAction();
	if ((cmd[2] != 3) && (cmd[2] != 4)) {
		V2_SendSingleByteResult(cmd, 1);
		return;
	}
	byte actionId = cmd[4];
	byte playCount = 1;
	if (cmd[2] == 4) {
		playCount = cmd[5];
	}
	V2_GoAction(actionId, playCount, true, cmd);
}


// Function for both V1 and V2 and it need to keep V1 Play command
void V2_GoAction(byte actionId, bool v2, byte *cmd) {
	// Default play only once
	if (debug) DEBUG.printf("[V2_GoAction(%d, %s, [])]\n", actionId, (v2 ? "true" : "false"));
	V2_GoAction(actionId, 1, v2, cmd);
}

void V2_GoAction(byte actionId, byte playCount, bool v2, byte *cmd) {
	if (debug) DEBUG.printf("[V2_GoAction(%d, %d, %s, [])]\n", actionId, playCount,  (v2 ? "true" : "false"));
	if (actionData.id() != actionId) {
		// need to load actionData
		if (debug) DEBUG.printf("Read action %d from SPIFFS\n", actionId);
		if (int error = actionData.ReadSPIFFS(actionId)) {
			if (debug) DEBUG.printf("Fail to get Id matched - %d\n", error);
			if (v2) V2_SendSingleByteResult(cmd, 2);
			return;
		}
	}

	// Load starting with pId = 0
	uint16_t pId = 0;
	if (!actionData.IsPoseReady(pId)) {
		if (v2) V2_SendSingleByteResult(cmd, 3);
		return;	
	}
	

	// Just for safety, check the poseCnt again.  
	// May skip this step if those information is always updated.
	//
	// -- In new version, it cannot be checked as only partial action is loaded in memory
	//
	// if (debug) DEBUG.printf("Refresh action %d from SPIFFS\n", actionId);
	// actionData.RefreshActionInfo();

	if (actionData.PoseCnt() > 0) {
		V2_ActionCombo = 0;
		V2_NextAction = actionId;
		V2_ActionPlayCount = playCount;
		V2_NextPose = 0;
		V2_GlobalTimeMs = millis();
		V2_NextPlayMs = V2_GlobalTimeMs;
		V2_ActionPlaying = true;
	}
	if (v2) V2_SendSingleByteResult(cmd, 0);
	if (debug) DEBUG.printf("Ready to play action %d with %d steps, %d time(s)\n", actionId, actionData.PoseCnt(), V2_ActionPlayCount);

}

void V2_PlayCombo(byte *cmd) {
	
}

#pragma endregion


#pragma region ActionTable

bool V2_CheckActionId(byte actionId) {

	if (debug) DEBUG.printf("V2_CheckActionId - Current Id: %d (%d); requested Id: %d\n", actionData.id(), actionData.Header()[4], actionId);
	if (actionId != actionData.id() ) {
		if (int error = actionData.ReadSPIFFS(actionId)) {
			// anything to do if still fail to read
			if (debug) DEBUG.printf("Fail to get Id matched - %d\n", error);
			return false;
		}
		if (debug) DEBUG.printf("V2_CheckActionId - Current Id changed to %d (%d)\n", actionData.id(), actionData.Header()[4]);
	} 
	return true;
}

void V2_GetAdList(byte *cmd) {
	if (debug) DEBUG.println("[V2_GetAdList]");
	int dataSize = 32;  // 8 * 32 = 256 bit-wise flag
	byte result[dataSize + 6];
	memset(result, 0, dataSize + 6);
	result[2] = dataSize + 2;
	result[3] = cmd[3];

	SPIFFS.begin();
	Dir dir;
	dir = SPIFFS.openDir(ACTION_PATH);

	while (dir.next()) {
		int id = getActionId(dir);
		if (id >= 0) {
			int h, l;
			h = (id / 8);
			l = 7 - (id % 8);
			result[4 + h] |= (1 << l);
		}
	}
	V2_SendResult(result);
}

int getActionId(Dir dir) {

	String fName = dir.fileName();


	if (!fName.endsWith(".dat"))  return -1;

	int seq = 0;
	for (int i = 0; i < 3; i++) {
		byte c = fName[ACTION_POS + i];
		if ((c < '0') || (c > '9')) {
			seq = -1;
			break;
		}
		seq = seq * 10 + (c - '0');
	}
	if ((seq < 0) || (seq > 255)) return -1;
		
	File f= dir.openFile("r");
	int size = f.size();
	f.close();

	if (size < AD_HEADER_SIZE) return -1;

	size -= AD_HEADER_SIZE;
	int tail = (size % AD_POSE_SIZE);
	if (tail) return -1;

	return seq;
}

void V2_GetAdHeader(byte *cmd) {
	if (debug) DEBUG.println("[V2_GetAdHeader]");
	byte aId = cmd[4];	
	if (!V2_CheckActionId(aId)) {
		V2_SendSingleByteResult(cmd, 0x01);
		return;	
	}
	actionData.Header()[3] = cmd[3];
	V2_SendResult((byte *) actionData.Header());
}


void V2_GetAdPose(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_GetAdPose]"));

	#ifdef UBT_DUMP
		DEBUG.printf("\n\nV2_GetAdPose - A - Serial dump - first 60:\n");
		for (int i = 0; i < 60; i++) {
			DEBUG.printf("%02X ", actionData.Data()[i]);
		}
		DEBUG.printf("\n\n\n");
	#endif

	byte aId = cmd[4];
	if (!V2_CheckActionId(aId)) {
		V2_SendSingleByteResult(cmd, 0x01);
		return;	
	}
	uint16_t pId = (cmd[5] << 8) | cmd[6];
	uint16_t pOffset = 0;
	if (!actionData.IsPoseReady(pId, pOffset)) {
		V2_SendSingleByteResult(cmd, 0x02);
		return;	
	}

	#ifdef UBT_DUMP
		DEBUG.printf("\n\nV2_GetAdPose - B - Serial dump - first 60:\n");
		for (int i = 0; i < 60; i++) {
			DEBUG.printf("%02X ", actionData.Data()[i]);
		}
		DEBUG.printf("\n\n\n");
	#endif

	byte result[AD_POSE_SIZE];
	byte * fromPos = actionData.Data();

	fromPos += pOffset;
	memcpy(result, fromPos, AD_POSE_SIZE);
	result[2] = AD_POSE_SIZE - 4;
	result[3] = cmd[3];

	DEBUG.printf("Source Data at 0,0: \n");
	for (int i = 0; i < AD_POSE_SIZE; i++) {
		DEBUG.printf("%02X ", actionData.Data()[i]);
	}
	DEBUG.println();

	DEBUG.printf("V2_GetAdPose - Return Data: \n");
	for (int i = 0; i < AD_POSE_SIZE; i++) {
		DEBUG.printf("%02X ", result[i]);
	}
	DEBUG.println();

	V2_SendResult(result);
}

void V2_UpdateAdHeader(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_UpdateAdHeader]"));
	byte aId = cmd[4];	
	
	// Length should be header size - 4
	if (cmd[2] != (AD_HEADER_SIZE - 4)) {
		if (debug) DEBUG.printf("Invalid length: %d\n", cmd[2]);
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_SIZE);
		return;
	}

	// Action ID must be matched.  
	// i.e. must GET before UPDATE (Why?  user can load action from file, remove this checking)
	// Should always initilize before updating action header, it should rewrite the whole action.
	//
	// if (cmd[4] != actionData.Header()[4]) {
		if (debug) DEBUG.printf("InitialObject(%d)\n", cmd[4]);
		actionData.InitObject(cmd[4]);
	// }
	for (int i = 0; i < AD_HEADER_SIZE; i++) {
		actionData.Header()[i] = cmd[i];
	}
	
	byte result = actionData.WriteHeader();

	if (debug) DEBUG.printf("Action %d header updated - %d\n", aId, result);
	V2_SendSingleByteResult(cmd, result);
	
}


void V2_UpdateAdPose(byte *cmd) {
	
	if (debug) DEBUG.println(F("[V2_UpdateAdPose]"));
	
	// Length should be {len} {actionId} {poseId} {data} => pose datasize + 3
	if (cmd[2] != (AD_POSE_SIZE - 4)) {
		if (debug) DEBUG.printf("Invalid length: %d\n", cmd[2]);
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_SIZE);
		return;
	}	
	
	byte aId = cmd[4];
	uint16_t pId = (cmd[AD_POFFSET_SEQ_HIGH] << 8) | cmd[AD_POFFSET_SEQ];

	// Action ID must be matched.  i.e. must GET before UPDATE
	if (aId != actionData.Header()[4]) {
		if (debug) DEBUG.printf("ID not matched: %d (current: %d)\n", aId, actionData.Header()[4]);
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_AID_NOT_MATCH);
		return;
	}

	int poseOffset = actionData.PoseOffset();
	int bufferEndPose = actionData.BufferEndPose();

	if (debug) DEBUG.printf("[V2_UpdateAdPose] - pId: %d, pCnt: %d, poffset: %d, AD_PBUFFER_COUNT %d, End: %d\n", pId, actionData.PoseCnt(), poseOffset, AD_PBUFFER_COUNT, bufferEndPose);

	if ((pId >= actionData.PoseCnt()) || (pId < poseOffset) || (pId > bufferEndPose)) {
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_PID_OUT_RANGE);
		return;
	}

	int startPos = AD_POSE_SIZE * (pId - poseOffset);
	for (int i = 0; i < AD_POSE_SIZE; i++) {
		actionData.Data()[startPos + i] = cmd[i];
	}

	byte result = RESULT::SUCCESS;

	if (debug) DEBUG.printf("[V2_UpdateAdPose] - Saved\n");


	if ((pId == (actionData.PoseCnt() - 1)) || (pId == bufferEndPose)) {
		result = actionData.WritePoseData();
	}

	V2_SendSingleByteResult(cmd, result);

	#ifdef UBT_DUMP
		DEBUG.printf("\n\nV2_UpdateAdPose - Serial dump - first 60:\n");
		for (int i = 0; i < 60; i++) {
			DEBUG.printf("%02X ", actionData.Data()[i]);
		}
		DEBUG.printf("\n\n\n");
	#endif
}



void V2_UpdateAdName(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_UpdateAdName]"));
	byte id = cmd[4];
	if (actionData.id() != id) {
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_AID_NOT_MATCH);
		return;
	}
	if (cmd[5] > AD_NAME_SIZE) {
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_AD_NAME_SIZE);
		return;
	}
	byte * startPos = actionData.Header() + AD_OFFSET_NAME;
	memset(startPos, 0, AD_NAME_SIZE);
	byte len = cmd[5];

	// byte * copyPos = cmd + 6;
	if (debug) DEBUG.print("Action Name: ");
	for (int i = 0; i < len; i++) { 
		if (debug) DEBUG.print((char) cmd[6 + i]);
		actionData.Header()[AD_OFFSET_NAME + i] = cmd[6 + i];
	}
	if (debug) DEBUG.println();
	V2_SendSingleByteResult(cmd, RESULT::SUCCESS);
}

void V2_DeleteActionFile(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_DeleteActionFile]"));
	byte id = cmd[4];
	byte result = actionData.DeleteActionFile(id);
	V2_SendSingleByteResult(cmd, result);
}

#pragma endregion


#pragma region Combo: V2_CMD_GET_COMBO / V2_CMD_UPD_COMBO

void V2_GetCombo(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_GetCombo]"));
	byte seq = cmd[4];
	if (seq >= CD_MAX_COMBO) {
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_COMBO_OUT_RANGE);
		return;
	}
	comboTable[seq].Data()[3] = cmd[3];
	V2_SendResult(comboTable[seq].Data());

}

void V2_UpdateCombo(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_UpdateCombo]"));

	V2_SendSingleByteResult(cmd, RESULT::SUCCESS);
}

#pragma endregion

#pragma region SPIFFS: V2_CMD_READSPIFFS / V2_CMD_WRITESPIFFS
/*
void V2_ReadSPIFFS(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_ReadSPIFFS]"));
	byte result = V2_UBT_ReadSPIFFS(cmd);
	V2_SendSingleByteResult(cmd, result);
}

byte V2_UBT_ReadSPIFFS(byte *cmd) {
	return 0;
}

void V2_WriteSPIFFS(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_WriteSPIFFS]"));

	#ifdef UBT_DUMP
		DEBUG.printf("\n\nV2_WriteSPIFFS - first 60:\n");
		DEBUG.printf("Action Id: %d\n", actionData.id());
		for (int i = 0; i < 60; i++) {
			DEBUG.printf("%02X ", actionData.Data()[i]);
		}
		DEBUG.printf("\n\n\n");
	#endif

	byte result = actionData.WriteSPIFFS();
	V2_SendSingleByteResult(cmd, result);
}
*/

#pragma endregion
