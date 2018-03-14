#include "robot.h"

// A - Angle : (V1_GetServoAngle, V1_GetServoAngleText)
// B - Debug
// C
// D - Download Action Data (MCU -> PC)
// E
// F - Free Servo
// G 
// H
// I
// J
// K
// L - Lock servo
// M - Move
// N
// O
// P - Play Action/Combo
// Q
// R - Read Action Data form SPIFFS
// S - Stop Action
// T - Detect Servo
// U - Upload Action Data (PC -> MCU)
// V
// W - Write Action Data to SPIFFS
// X 
// Y 
// Z - Reset Connection

bool V1_CommandSet() {
	byte cmd = cmdBuffer.read();
	switch (cmd) {
			case 'A':
				V1_GetServoAngle();
				break;
			case 'a':
				V1_GetServoAngleText();
				break;
			case 'B':
				SetDebug(true);
				break;
			case 'b':
				SetDebug(false);
				break;
			case 'D':
				break;
			case 'F':
				break;
			case 'f':
				break;
			case 'J':
				break;
			case 'L':
				break;
			case 'l':
				break;
			case 'M':
				break;
			case 'm':
				break;
			case 'R':
				break;
			case 'T':
				V1_DetectServo();
				break;
			case 't':
				V1_DetectServoText();
				break;
			case 'S':
				break;
			case 'U':
				break;
			case 'W':
				break;
			case 'Z':
				V1_ResetConnection();
				break;
	}
	return true;
}

#pragma region "Utilities"

void serialPrintByte(byte data) {
	if (data < 0x10) Serial.print("0");
	Serial.print(data, HEX);
}

int getServoId() {
	if (!Serial.available()) return -1;
	int id = (byte) Serial.read();
	if ((id >= '0') && (id <= '9')) {
		id -= '0';
	} else if ((id >= 'A') && (id <= 'G')) {
		id = 10 + id - 'A';
	} else if ((id >= 'a') && (id <= 'g')) {
		id = 10 + id - 'a';
	} else {
		return -2;
	}
	return id;
}

#pragma endregion

#pragma region "A,a - Get Servo Angle"

void V1_GetServoAngle() {
	if (debug) DEBUG.println(F("[V1_GetServoAngleHex]"));
	byte outBuffer[32];
	for (int id = 1; id <= 16; id++) {
		int pos = 2 * (id - 1);
		if (servo.exists(id)) {
			if (servo.isLocked(id)) {
				outBuffer[pos] = servo.lastAngle(id);
				outBuffer[pos+1] = (servo.isLocked(id) ? 1 : 0);
			} else {
				outBuffer[pos] = servo.getPos(id);
				outBuffer[pos+1] = 0;
			}
		} else {
			outBuffer[pos] = 0xFF;
			outBuffer[pos+1] = 0;
		}
	}
	if (debug) {
		for (int i = 0; i < 32; i++) {
			DebugPrintByte(outBuffer[i]);
			DEBUG.print(" ");
		}
		DEBUG.println();
	}
	Serial.write(outBuffer, 32);
}

void V1_GetServoAngleText() {
	if (debug) DEBUG.println(F("[V1_GetServoAngleHex]"));
	// No extra debug output for Text command
	Serial.println(F("\nServo Angle:\n"));
	for (int id = 1; id <= 16; id++) {
		Serial.print(F("Servo "));
		Serial.print(id);
		Serial.print(": ");
		if (servo.exists(id)) {
			byte angle = servo.getPos(id);
			Serial.print(angle, DEC);
			Serial.print(" [");
			serialPrintByte(angle);
			Serial.print("]  ");
			if (servo.isLocked(id)) {
				Serial.print(" Locked");
			}
			Serial.println();
		} else {
			Serial.println("Not Available");
		}
	}
}

#pragma endregion

#pragma region "T - Test/Detect Servo"

void V1_DetectServo() {
	if (debug) DEBUG.println(F("[V1_DetectServo]"));
	byte showAngle = 0;
	if (cmdBuffer.available()) showAngle = cmdBuffer.read();
	servo.detectServo(1,16);
	if ((showAngle) && (showAngle != '0')) V1_GetServoAngle();
}

void V1_DetectServoText() {
	if (debug) DEBUG.println(F("[V1_DetectServoText]"));
	byte showAngle = 0;
	if (cmdBuffer.available()) showAngle = cmdBuffer.read();
	servo.detectServo(1,16);
	if ((showAngle) && (showAngle != '0')) V1_GetServoAngleText();
}

#pragma endregion

#pragma region "Z - Reset Connection"

void V1_ResetConnection() {
	if (debug) DEBUG.println(F("[V1_ResetConnection]"));
	servo.end();
	delay(200);
	servo.begin();
	byte showAngle = 0;
	if (cmdBuffer.available()) showAngle = cmdBuffer.read();
	if ((showAngle) && (showAngle != '0')) V1_GetServoAngle();
}

#pragma endregion














void cmd_GetServoAdjAngleHex() {
	byte outBuffer[32];
	for (int id = 1; id <= 16; id++) {
		int pos = 2 * (id - 1);
		if (servo.exists(id)) {
			uint16  adjAngle = servo.getAdjAngle(id);
			outBuffer[pos] = adjAngle / 256;
			outBuffer[pos+1] = adjAngle % 256;
		} else {
			outBuffer[pos] = 0x7F;
			outBuffer[pos+1] = 0x7F;
		}
	}
	Serial.write(outBuffer, 32);
}


#pragma region Lock/Free Servo

// Return: 
//   0 - L/U
//   1 - log count
//   (2n)   - servo id
//   (2n+1) - angle / 0xFF for error
void cmd_LockServoHex(bool goLock) {
	byte result[40];
	byte cnt = 0;
	char mode = (goLock ? 'L' : 'U');
	result[0] = mode;
	while (Serial.available()) {
		byte id = Serial.read();
		if ((cnt == 0) && (id == 0)) {
			// Only 1 paramter 0x00 for lock/unlock all
			if (!Serial.available()) {
				for (int servoId = 1; servoId <= 16; servoId++)  {
					result[2*servoId] = servoId;
					if (servo.exists(servoId)) {
						result[2*servoId+1] = servo.getPos(servoId, goLock);
					} else {
						result[2*servoId+1] = 0xFF;
					}
				}
				cnt = 16;
			} 
			break;
		}
		cnt++;
		result[2*cnt] = id;
		if ((id >= 1) && (id <= 16) && servo.exists(id)) {
			result[2*cnt+1] = servo.getPos(id, goLock);
		} else {
			result[2*cnt+1] = 0xFF;
		}
		// For safety, not reasonable to have more than MAC servo
		if (cnt >= 16) break;
	}
	result[1] = cnt;
	cnt = 2 * (cnt + 1);
	Serial.write(result, cnt);
}

void cmd_LockServo(bool goLock) {
	int id = getServoId();
	char mode = (goLock ? 'l' : 'u');
	switch (id) {
		case -1:
			Serial.print(mode);
			Serial.write(0x01);
			break;
		case -2:
			Serial.print(mode);
			Serial.write(0x02);
			break;
		case 0:
			Serial.print(mode);
			Serial.write(0x00);
			Serial.write(0x00);
			for (int servoId = 1; servoId <= 16; servoId++)  {
				if (servo.exists(servoId)) servo.getPos(servoId, goLock);
			}
			break;
		default:
			Serial.print(mode);
			Serial.write(0x00);
			Serial.write(id);
			byte angle;
			if ((id >= 1) && (id <= 16) && servo.exists(id)) {
				angle = servo.getPos(id, goLock);
			} else {
				angle = 0xFF;
			}
			Serial.write(angle);
			break;
	}
}

#pragma endregion


void showServoInfoHex() {
	servoCnt = 0;
	byte outBuffer[16];
	for (int id = 1; id <= 16; id++) {
		if (servo.exists(id)) {
			outBuffer[id-1] = servo.getPos(id);
		} else {
			outBuffer[id-1] = 0xFF;
		}
	}
	Serial.write(outBuffer, 16);
}

void showServoInfo() {
	Serial.println(F("\n\nAvailable servo:"));
	servoCnt = 0;
	for (int id = 1; id <= 16; id++) {
		if (id < 10) Serial.print('0');
		Serial.print(id);
		Serial.print(F(": "));
		if (servo.exists(id)) {
			byte angle = servo.getPos(id);
			for (int i = 4; i <8; i++) {
				Serial.print(retBuffer[i] < 0x10 ? " 0" : " ");
				Serial.print(retBuffer[i], HEX);
			}
			Serial.print("  [");
			Serial.print(angle, DEC);
			Serial.println("]");
			servoCnt++;
		} else {
			Serial.println("Missing");
		}

	}
	Serial.println();
	if (servoCnt > 0) {
		Serial.print(servoCnt);
	} else {
		Serial.print(F("No "));
	}
	Serial.print(F(" servo detected."));
}

void fx_playAction() {
	byte actionCode = 0;  // action 0, 'A' is standby action
	if (Serial.available()) {
		byte ch = Serial.read();
		if ((ch >= 'A') && (ch <= 'Z')) {
			actionCode = ch - 'A';
		} else {
			return; // Return for invalid action
		}
	}		
	playAction(actionCode);
}

void playAction(byte actionCode) {
	Serial.println("Play Action");
	servo.setDebug(true);
	for (int po = 0; po < MAX_POSES; po++) {
		int waitTime = actionTable[actionCode][po][WAIT_TIME_HIGH] * 256 + actionTable[actionCode][po][WAIT_TIME_LOW];
		byte time = actionTable[actionCode][po][EXECUTE_TIME];
		// End with all zero, so wait time will be 0x00, 0x00, and time will be 0x00 also
		if ((waitTime == 0) && (time == 0)) break;
		if (time > 0) {
			for (int id = 1; id <= 16; id++) {
				byte angle = actionTable[actionCode][po][ID_OFFSET + id];
				// max 240 degree, no action required if angle not changed, except first action
				if ((angle <= 0xf0) && 
					((po == 0) || (angle != actionTable[actionCode][po-1][ID_OFFSET + id]))) {
					servo.move(id, angle, time);
				}
			}
		}
		delay(waitTime);
	}
	servo.setDebug(false);
}



#pragma region Read/Write SPIFFS

void cmd_ReadSPIFFS() {
	ReadSPIFFS(true);
}

void ReadSPIFFS(bool sendResult) {
	byte result[2];
	result[0] = 'R';
	SPIFFS.begin();
	if (SPIFFS.exists(actionDataFile)) {
		File f = SPIFFS.open(actionDataFile, "r");
		if (!f) {
			result[1] = READ_ERR_OPEN_FILE;
		} else {
			if (f.size() != sizeof(actionTable)) {
				result[1] = READ_ERR_FILE_SIZE;
			} else {
				memset(actionTable, 0, sizeof(actionTable));
				size_t wCnt = f.readBytes((char *)actionTable, sizeof(actionTable));
				f.close();
				if (wCnt == sizeof(actionTable)) {
					result[1] = READ_OK;
				} else {
					result[1] = READ_ERR_READ_FILE;
				}
			}
		}
	} else {
		result[1] = READ_ERR_NOT_FOUND;
	}
	SPIFFS.end();	
	if (sendResult) Serial.write(result, 2);
}

bool cmd_WriteSPIFFS() {
	byte result[2];
	result[0] = 'W';
	SPIFFS.begin();
	File f = SPIFFS.open(actionDataFile, "w");
	if (!f) {
		result[1] = WRITE_ERR_OPEN_FILE;
	} else {
		size_t wCnt = f.write((byte *)actionTable, sizeof(actionTable));
		f.close();
		if (wCnt == sizeof(actionTable)) {
			result[1] = WRITE_OK;
		} else {
			result[1] = WRITE_ERR_WRITE_FILE;
		}
	}
	SPIFFS.end();	
	Serial.write(result, 2);
}

#pragma endregion


#pragma region Move Servo

// Input id, angle, time
// Output - action, status, # servo, id
void cmd_MoveServoHex() {
	byte result[20];
	int moveCount = goMoveServoHex(result);
	Serial.write(result, moveCount + 3);
}

int goMoveServoHex(byte *result) {
	result[0] = 'M';
	result[1] = 0x00;
	result[2] = 0x00;
	byte inBuffer[51];  // Max 16 servo + end mark: 17 * 3 = 51
	byte inCount = 0;
	byte moveData[16][3];
	byte moveCnt = 0;
	while (Serial.available()) {
		if (inCount >= 51) {
			result[1] = MOVE_ERR_PARM_CNT;
			return 0;	
		}
		inBuffer[inCount++] = (byte) Serial.read();
	}
	return moveMultiServo(inCount, inBuffer, result);
}


void cmd_moveServo() {
	byte result[20];
	int moveCount = goMoveServo(result);
	Serial.print("\nMove - ");
	if (result[1]) {
		Serial.print(" Error : ");
		serialPrintByte(result[1]);
		Serial.println();
		return;
	}
	Serial.print(moveCount);
	Serial.print(" servo moved: ");
	for (int i = 0; i < moveCount; i++) {
		Serial.print(" ");
		Serial.print(result[ 3 + i]);
	}
	Serial.println();
}

int SerialParseInt() {
	if (!Serial.available()) return -1;
	ch = Serial.peek();
	if ((ch < '0') || (ch > '9')) return -2;
	int data = Serial.parseInt();
	return data;
}

int goMoveServo(byte *result)
{
	result[0] = 'M';
	result[1] = 0x00;
	result[2] = 0x00;
	byte inBuffer[51];  // Max 16 servo + end mark: 17 * 3 = 51
	byte inCount = 0;
	byte moveData[16][3];
	byte moveCnt = 0;
	while (Serial.available()) {
		if (inCount >= 51) {
			result[1] = MOVE_ERR_PARM_CNT;
			return 0;	
		}
		int value = SerialParseInt();
		if ((value < 0) || (value > 255)) {
			result[1] = MOVE_ERR_PARM_VALUE;
			return 0;	
		}
		inBuffer[inCount++] = (byte) value;

		if (Serial.available()) {
			if ((Serial.peek() == 0x0A) || (Serial.peek() == 0x0D)) {
				break;
			}
			if (Serial.peek() != ',') {
				result[1] = MOVE_ERR_PARM_CONTENT;
				return 0;	
			}
			// Read the ',' separator
			Serial.read();  
		} 
	}
	moveMultiServo(inCount, inBuffer, result);
}

int moveMultiServo(int inCount, byte* inBuffer, byte *result) {
	if ((inCount < 6) || (inCount % 3 != 0)) {
		result[1] = MOVE_ERR_PARM_CNT;
		return 0;
	}
	if ((inBuffer[inCount-1] != 0x00) || (inBuffer[inCount-2] != 0x00) || (inBuffer[inCount-3] != 0x00)) {
		result[1] = MOVE_ERR_PARM_END;
		return 0;
	}

	byte moveData[16][3];
	byte moveCnt = 0;
	if (inBuffer[0] == 0) {
		if (inCount != 6) {
			result[1] = MOVE_ERR_PARM_ALL_CNT;
			return 0;
		}
		if (inBuffer[1] > 240) {
			result[1] = MOVE_ERR_PARM_ALL_ANGLE;
			return 0;
		}
		moveCnt = 0;
		for (int id = 1; id <= 16; id++) {
			if (servo.exists(id)) {
				moveData[moveCnt][0] = id;
				moveData[moveCnt][1] = inBuffer[1];
				moveData[moveCnt][2] = inBuffer[2];
				moveCnt++;
			}
		}
	} else {
		moveCnt = 0;
		for (int i = 0; i < inCount - 3; i += 3) {
			int id = inBuffer[i];
			if ((id == 0) || (id > 16)) {
				result[1] = MOVE_ERR_PARM_ONE_ID;
				return 0;
			}
			if (inBuffer[i + 1] > 240) {
				result[1] = MOVE_ERR_PARM_ONE_ANGLE;
				return false;
			}
			bool servoFound = false;
			for (int j = 0; j < moveCnt; j++) {
				if (moveData[j][0] == id) {
					result[1] = MOVE_ERR_PARM_DUP_ID;
					return false;
				}
			}
			if (servo.exists(id)) {
				moveData[moveCnt][0] = id;
				moveData[moveCnt][1] = inBuffer[i + 1];
				moveData[moveCnt][2] = inBuffer[i + 2];
				moveCnt++;
			}
		}
	}
	result[1] = 0x00;
	result[2] = moveCnt;
	if (!moveCnt) return 0;

	for (int i = 0; i < moveCnt; i++) {
		servo.move(moveData[i][0], moveData[i][1], moveData[i][2]);
		result[i+3] = moveData[i][0];
	}
	return moveCnt;
}

#pragma endregion

#pragma region Download / Upload Action Data

void cmd_DownloadActionDataHex() {
	byte *ptr = (byte *)actionTable;
	long size = MAX_POSES * MAX_POSES_SIZE;
	for(int action = 0; action < MAX_ACTION; action ++) {
		Serial.write(ptr, size);
		ptr += size;
		delay(1);  // add 1 ms delay for processing the data
	}
}

//  'W' actionId 0xFF				=> clear action
//  'W' actionId poseId 20xData 	=> upload pose data

void cmd_UploadActionDataHex() {
	byte inBuffer[MAX_POSES_SIZE];
	byte result[6];
	memset(result,0xfe,4);
	result[0] = 'W';
	int poseCnt = 0;
	byte *ptr;
	if (Serial.available()) {
		result[1] = Serial.read();
		if (Serial.available()) {
			result[2] = Serial.read();
			poseCnt = 0;
			while (Serial.available()) {
				inBuffer[poseCnt++] = Serial.read();
				if (poseCnt >= MAX_POSES_SIZE) break;
				delay(1);  // for safety, add 1ms delay here,otherwise it may cause missing data.   It won't cause much delay, as only 20 bytes is sent
			}
		}
	}

	if (result[1] > MAX_ACTION) {
		result[3] = UPLOAD_ERR_ACTION;
		Serial.write(result, 4);
		return;
	}

	if (result[2] == 0xFF) {
		if (poseCnt > 0) {
			result[3] = UPLOAD_ERR_CLEAR_POSE;
			Serial.write(result, 4);
			return;
		}
		ptr = &actionTable[result[1]][0][0];
		long size = MAX_POSES * MAX_POSES_SIZE;
		memset(ptr, 0, size);
		result[3] = UPLOAD_CLEAR_OK;
		Serial.write(result, 4);
		return;
	}
	
	if (result[2] > MAX_POSES_SIZE) {
		result[3] = UPLOAD_ERR_POSE;
		Serial.write(result, 4);
		return;
	}
	
	if (poseCnt != MAX_POSES_SIZE) {
		result[3] = UPLOAD_ERR_POSE_DATA;
		result[4] = poseCnt;
		Serial.write(result, 5);
		Serial.write(inBuffer, poseCnt);
		return;
	}
			
	ptr = & actionTable[result[1]][result[2]][0];
	long size = MAX_POSES * MAX_POSES_SIZE;
	memcpy(ptr, inBuffer, MAX_POSES_SIZE);
	result[3] = UPLOAD_OK;
	Serial.write(result, 4);
}

#pragma endregion

