#include "robot.h"

// Command start with A9 9A

// {len} = cmd[2] = count( {len} xx .. xx )
// {sum} = cmd[{len} + 2] = sum( {len} xx .. xx )
// Size = A9 9A [{len} ...] {sum} ED = [{len} ... ] + 4 = len + 4
// minimum : A9 9A 2 {cmd} {sum} ED = 6

// Command set:
//
//   01 - Reset (fix length)     	: A9 9A 03 01 00 04 ED
//   02 - Set Debug (fix length) 	: A9 9A 03 02 00 05 ED
//   03 - Set DevMode (fix length) 	: A9 9A 03 03 01 07 ED
//   11 - Get Angle (fix length) 	: A9 9A 02 11 13 ED
//   21 - Read SPIFFS (fix length) 	: A9 9A 02 21 23 ED
//   22 - Write SPIFFS (fix length) : A9 9A 02 22 24 ED


#define V2_CMD_RESET		0x01
#define V2_CMD_DEBUG		0x02
#define V2_CMD_DEVMODE      0x03
#define V2_CMD_SERVOANGLE	0x11
#define V2_CMD_READSPIFFS   0x21
#define V2_CMD_WRITESPIFFS  0x22


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

		case V2_CMD_READSPIFFS:
			V2_ReadSPIFFS(cmd);
			break;

		case V2_CMD_WRITESPIFFS:
			V2_WriteSPIFFS(cmd);
			break;

		default:
			if (debug) {
				DEBUG.print(F("Undefined command: "));
				DebugPrintByte(cmd[3]);
				DEBUG.println();
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

#pragma region 11 - GetServoAngle

void V2_GetServoAngle() {
	if (debug) DEBUG.println(F("[V2_GetServoAngle]"));
	uint16_t len = 34;
	byte outBuffer[len+4];
	outBuffer[2] = len;
	outBuffer[3] = V2_CMD_SERVOANGLE;

	for (int id = 1; id <= 16; id++) {
		int pos = 4 + 2 * (id - 1);
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
	V2_SendResult(outBuffer);
}

#pragma endregion

#pragma region SPIFFS: V2_CMD_READSPIFFS / V2_CMD_WRITESPIFFS

void V2_ReadSPIFFS(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_ReadSPIFFS]"));
	GoReadSPIFFS(true);
}

void GoReadSPIFFS(bool sendResult) {
	byte result[2];
	result[0] = V2_CMD_READSPIFFS;
	SPIFFS.begin();
	if (SPIFFS.exists(actionDataFile)) {
		File f = SPIFFS.open(actionDataFile, "r");
		if (!f) {
			if (debug) DEBUG.println(F("Fail to open SPIFFS file"));
			result[1] = READ_ERR_OPEN_FILE;
		} else {
			if (f.size() != sizeof(actionTable)) {
				if (debug) {
					DEBUG.print(F("Invalid File size: "));
					DEBUG.println(f.size());
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
	if (sendResult) Serial.write(result, 2);
}

void V2_WriteSPIFFS(byte *cmd) {
	if (debug) DEBUG.println(F("[V2_WriteSPIFFS]"));
	byte result[2];
	result[0] = V2_CMD_WRITESPIFFS;
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