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

bool V1_Command() {
	byte cmd = cmdBuffer.read();

	// a little bit danger as subsequent data may trrigger other commands
	// but unforunately, V1 command does not have start/sum/end structure, 
	// it need to identify the command one by one until parameters read.
	// For to simplify the logic, skip the command directly here.
	// Unless the data part meet the start/sum/end of other command, it will not be triggered.
	if (!enable_V1) {
		if (debug)  DEBUG.printf("V1 Command disabled: %c [0x%02X]\n", cmd, cmd);
		return true;
	}

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
				V1_DownloadActionData();
				break;

			case 'F':
				V1_LockServo(false);
				break;

			case 'f':
				V1_LockServoText(false);
				break;

			case 'i':
				V1_CheckSD(&Serial);
				break;
			case 'I':
				V1_CheckSD(&Serial1);
				break;

			case 'J':
				V1_GetServoAdjAngle();
				break;

			case 'L':
				V1_LockServo(true);
				break;

			case 'l':
				V1_LockServoText(true);
				break;

			case 'M':
				V1_MoveServo();
				break;

			case 'm':
				V1_MoveServoText();
				break;

			case 'P':
				V1_PlayAction();
				break;
				
			case 'R':
				V1_ReadSPIFFS();
				break;
				
			case 'T':
				V1_DetectServo();
				break;

			case 't':
				V1_DetectServoText();
				break;

			case 'U':
				V1_UploadActionData();
				break;

			case 'W':
				V1_WriteSPIFFS();
				break;

			case 'Z':
				V1_ResetConnection();
				break;
	}
	return true;
}

#pragma region "Utilities"

int getServoId() {
	if (!cmdBuffer.available()) return -1;
	int id = (byte) cmdBuffer.read();
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
	if (debug) DEBUG.println(F("[V1_GetServoAngle]"));
	byte outBuffer[32];
	UBT_GetServoAngle(outBuffer);
	Serial.write(outBuffer, 32);
}

void V1_GetServoAngleText() {
	if (debug) DEBUG.println(F("[V1_GetServoAngleText]"));
	byte outBuffer[32];
	UBT_GetServoAngle(outBuffer);
	Serial.println(F("\nServo Angle:\n"));
	for (int id = 1; id <= 16; id++) {
		int pos = 2 *  (id - 1);
		Serial.printf("Servo %02d: ", id);
		if (outBuffer[pos] == 0xFF) {
			Serial.println("---");
		} else {
			Serial.printf("%3d [0x%02X]  %s\n", outBuffer[pos], outBuffer[pos], (outBuffer[pos+1] ? "Locked": ""));
		}
	}
}

#pragma endregion

#pragma region "D - Download Action Data"

void V1_DownloadActionData() {
	if (debug) DEBUG.println(F("[V1_DownloadActionData]"));
	byte *ptr = (byte *)actionTable;
	long size = MAX_POSES * MAX_POSES_SIZE;
	for(int action = 0; action < MAX_ACTION; action ++) {
		Serial.write(ptr, size);
		ptr += size;
		delay(1);  // add 1 ms delay for processing the data
	}
}

#pragma endregion

#pragma region "F, f, L, l - Lock/Free Servo"

// Return: 
//   0 - L/U
//   1 - log count
//   (2n)   - servo id
//   (2n+1) - angle / 0xFF for error

void V1_LockServo(bool goLock) {
	if (debug) {
		DEBUG.print(F("V1_LockServo("));
		DEBUG.print(goLock ? "TRUE" : "FALSE");
		DEBUG.println(F(")"));
	}
	byte result[40];
	byte cnt = 0;
	char mode = (goLock ? 'L' : 'F');
	result[0] = mode;
	while (cmdBuffer.available()) {
		byte id = cmdBuffer.read();
		if ((cnt == 0) && (id == 0)) {
			// Only 1 paramter 0x00 for lock/unlock all
			if (!cmdBuffer.available()) {
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

void V1_LockServoText(bool goLock) {
	if (debug) {
		DEBUG.print(F("V1_LockServoText("));
		DEBUG.print(goLock ? "TRUE" : "FALSE");
		DEBUG.println(F(")"));
	}
	int id = getServoId();
	char mode = (goLock ? 'L' : 'F');
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

#pragma region "J - Get Servo Adjust Angle"

void V1_GetServoAdjAngle() {
	if (debug) DEBUG.println(F("[V1_GetServoAdjAngle]"));
	byte outBuffer[32];

	UBT_GetServoAdjAngle(outBuffer);
	Serial.write(outBuffer, 32);
}

#pragma endregion

#pragma region "M,m - Move Servo"

// Input id, angle, time
// Output - action, status, # servo, id
void V1_MoveServo() {
	if (debug) DEBUG.println(F("[V1_MoveServo]"));
	byte result[20];
	int moveCount = V1_goMoveServo(result);
	Serial.write(result, moveCount + 3);
}

int V1_goMoveServo(byte *result) {
	result[0] = 'M';
	result[1] = 0x00;
	result[2] = 0x00;
	byte inBuffer[51];  // Max 16 servo + end mark: 17 * 3 = 51
	byte inCount = 0;
	// byte moveData[16][3];
	// byte moveCnt = 0;
	while (cmdBuffer.available()) {
		if (inCount >= 51) {
			result[1] = MOVE_ERR_PARM_CNT;
			return 0;	
		}
		inBuffer[inCount++] = (byte) cmdBuffer.read();
	}
	return V1_moveMultiServo(inCount, inBuffer, result);
}


void V1_MoveServoText() {
	if (debug) DEBUG.println(F("[V1_MoveServoText]"));
	byte result[20];
	int moveCount = V1_goMoveServoText(result);
	Serial.print("\nMove - ");
	if (result[1]) {
		Serial.printf(" Error: %02X\n", result[1]);
		return;
	}
	Serial.printf("%d servo moved: ", moveCount);
	for (int i = 0; i < moveCount; i++) {
		Serial.printf(" %d", result[ 3 + i]);
	}
	Serial.println();
}

int V1_BufferParseInt() {
	if (!cmdBuffer.available()) return -1;
	ch = cmdBuffer.peek();
	if ((ch < '0') || (ch > '9')) return -2;
	int data = 0;
	while ((ch >= '0') && (ch <= '9')) {
		ch = cmdBuffer.read() - '0';
		data = data * 10 + ch;
		ch = cmdBuffer.peek();
	}
	return data;
}

int V1_goMoveServoText(byte *result)
{
	result[0] = 'M';
	result[1] = 0x00;
	result[2] = 0x00;
	byte inBuffer[51];  // Max 16 servo + end mark: 17 * 3 = 51
	byte inCount = 0;
	byte moveData[16][3];
	byte moveCnt = 0;
	while (cmdBuffer.available()) {
		if (inCount >= 51) {
			result[1] = MOVE_ERR_PARM_CNT;
			return 0;	
		}
		int value = V1_BufferParseInt();
		if ((value < 0) || (value > 255)) {
			result[1] = MOVE_ERR_PARM_VALUE;
			return 0;	
		}
		inBuffer[inCount++] = (byte) value;

		if (cmdBuffer.available()) {
			if ((cmdBuffer.peek() == 0x0A) || (cmdBuffer.peek() == 0x0D)) {
				break;
			}
			if (cmdBuffer.peek() != ',') {
				result[1] = MOVE_ERR_PARM_CONTENT;
				return 0;	
			}
			// Read the ',' separator
			cmdBuffer.read();  
		} 
	}
	return V1_moveMultiServo(inCount, inBuffer, result);
}

int V1_moveMultiServo(int inCount, byte* inBuffer, byte *result) {
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

#pragma region "P - Play action"

void V1_PlayAction() {
	/*
	if (debug) DEBUG.println(F("[V1_PlayAction]"));
	byte actionCode = 0;  // action 0, 'A' is standby action
	if (cmdBuffer.available()) {
		byte ch = cmdBuffer.read();
		if ((ch >= 'A') && (ch <= 'Z')) {
			actionCode = ch - 'A';
		} else {
			if (debug) DEBUG.printf("Invalid action: %02X\n", ch);
			return; // Return for invalid action
		}
	}		
	V1_goPlayAction(actionCode);
	*/
	if (debug) DEBUG.println(F("[V1_PlayAction]"));
	byte actionId = 0; 
	if (cmdBuffer.available()) {
		byte ch = cmdBuffer.read();
		if ((ch >= 'A') && (ch <= 'Z')) {
			actionId = ch - 'A';
		} else {
			if (debug) DEBUG.printf("Invalid action: %02X\n", ch);
			return; // Return for invalid action
		}

		// Convert V1 Play Action to V2 Play Action
		byte cmd[] = {0x9A, 0x9A, 0x03, 0x41, actionId , 0x00 ,0xED};

		V2_GoAction(actionId, false, cmd);
	}		


}

void V1_goPlayAction(byte actionCode) {
	if (debug) DEBUG.printf("Start playing action %c\n", (actionCode + 'A'));
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
	if (debug) DEBUG.printf("Action %c completed\n", (actionCode + 'A'));
}

#pragma endregion

#pragma region "R,W - Read/Write SPIFFS"

void V1_ReadSPIFFS() {
	if (debug) DEBUG.println(F("[V1_ReadSPIFFS]"));
	V1_UBT_ReadSPIFFS('R');
}

void V1_UBT_ReadSPIFFS(byte cmdCode) {
	byte result[2];
	SPIFFS.begin();
	if (SPIFFS.exists(actionDataFile)) {
		File f = SPIFFS.open(actionDataFile, "r");
		if (!f) {
			if (debug) DEBUG.println(F("Fail to open SPIFFS file"));
			result[1] = READ_ERR_OPEN_FILE;
		} else {
			if (f.size() != sizeof(actionTable)) {
				if (debug) {
					DEBUG.printf("Invalid File size: %d\n", f.size());
				}
				result[1] = READ_ERR_FILE_SIZE;
			} else {
				memset(actionTable, 0, sizeof(actionTable));
				size_t wCnt = f.readBytes((char *)actionTable, sizeof(actionTable));
				f.close();
				if (wCnt == sizeof(actionTable)) {
					result[1] = READ_OK;
				} else {
					if (debug) DEBUG.println(F("Errro reading SPIFFS file"));
					result[1] = READ_ERR_READ_FILE;
				}
			}
		}
	} else {
		if (debug) {
			DEBUG.print(F("SPIFFS file not found: "));
			DEBUG.println(actionDataFile);
		}
		result[1] = READ_ERR_NOT_FOUND;
	}
	SPIFFS.end();	
	if (cmdCode) {
		result[0] = cmdCode;
		Serial.write(result, 2);
	}
}

void V1_WriteSPIFFS() {
	if (debug) DEBUG.println(F("[V1_WriteSPIFFS]"));
	V1_UBT_WriteSPIFFS('W');
}

void V1_UBT_WriteSPIFFS(byte cmdCode) {
	byte result[2];
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
	if (cmdCode) {
		result[0] = cmdCode;
		Serial.write(result, 2);
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

#pragma region "U - Upload Action Data"

//  'W' actionId 0xFF				=> clear action
//  'W' actionId poseId 20xData 	=> upload pose data

// conversion for cmdBuffer
// - This part is not as simple as before, as data go to serial buffer not cmdBuffer directly
// - For each action, around 30 * 60 = 600 bytes, so better wait all data available in cmdBuffer first
// - In general, it will read one post each time, 
// -   0 - 'W' 
// -   1 - Action Code
// -   2 - Pose Code
// -   3~22 - Pose Data
// -   i.e. 22 data after command is read

void V1_UploadActionData() {
	if (debug) DEBUG.println(F("[V1_UploadActionData]"));

	byte inBuffer[MAX_POSES_SIZE];
	byte result[6];
	memset(result,0xfe,4);
	result[0] = 'W';
	int poseCnt = 0;
	byte *ptr;

	if (cmdBuffer.available() < 2) {
		// Invalid case, as 1ms delay in command reading, which should be able to read 10 continue bytes.
		if (debug) DEBUG.println(F("Incomplete Header, skip command and clear buffer"));
		cmdBuffer.reset();
		result[1] = 0xFF;
		result[2] = 0xFF;
		result[3] = UPLOAD_ERR_HEADER;
		result[4] = (byte) cmdBuffer.available();
		Serial.write(result, 5);
		return;
	}

	result[1] = cmdBuffer.read();
	result[2] = cmdBuffer.read();

	// Since only around 12 data / ms, would be better to wait 2 ms to make sure data completed
	if ((result[2] != 0xFF) && (cmdBuffer.available() < MAX_POSES_SIZE)) {
		delay(1);
		clearInputBuffer();
	}

	// read data from buffer if completed
	poseCnt = cmdBuffer.available();
	if ((result[2] != 0xFF) && (poseCnt >= MAX_POSES_SIZE)) {
		cmdBuffer.read(inBuffer, MAX_POSES_SIZE);
	}

	// Check data content
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
	memcpy(ptr, inBuffer, size);
	result[3] = UPLOAD_OK;
	Serial.write(result, 4);
}

#pragma endregion

#pragma region "Z - Reset Connection"

void V1_ResetConnection() {
	if (debug) DEBUG.println(F("[V1_ResetConnection]"));
	servo.end();
	delay(100);
	servo.begin();
	delay(100);
	byte showAngle = 0;
	if (cmdBuffer.available()) showAngle = cmdBuffer.read();
	if ((showAngle) && (showAngle != '0')) V1_GetServoAngle();
}

#pragma endregion

void V1_CheckSD(HardwareSerial *ss) {
	if (debug) DEBUG.println(F("[V1_CheckSD]"));

	ss->println("**** SPIFFS List");
	SPIFFS.begin();
	Dir dir;
	ss->println("\nFiles in root: ");
	dir = SPIFFS.openDir("");
	while (dir.next()) {
		ss->print(dir.fileName());
		ss->print("  ");
		File f = dir.openFile("r");
		ss->println(f.size());
	}

	SPIFFS.end();;
}