#include "robot.h"

void RobotMaintenanceMode() {
	unsigned long nextMs;
	bool keepGoing = true;
	while (keepGoing) {
		ClearBuffer();
		nextMs = 0;
		while (!Serial.available()) {
			if (millis() > nextMs) {
				Serial.println("(L)ist, (D)elete, (G)enerate, E(x)it");
				nextMs = millis() + 5000;
			}
		}
		ch = Serial.read();
		switch (ch) {
			case 'L':
			case 'l':
				RM_ListFile();
				break;
			case 'D':
			case 'd':
				RM_DelFile();
				break;
			case 'G':
			case 'g':
				RM_GenFile();
				break;
			case 'X':
			case 'x':
				keepGoing = false;
				break;
		}
	}
}

void ClearBuffer() {
	while (Serial.available()) {
		Serial.read();
		delay(1);
	}
}

void WaitForInput() {
	ClearBuffer();
	while (!Serial.available());
}

void GetTypeId(char *action, int *id) {
	if (Serial.available()) {
		*action = Serial.read();
	} else {
		*action = 0;
	}
	int aId = -1;
	while (Serial.available()) {
		char ch = Serial.read();
		if ((ch >= '0') && (ch <= '9')) {
			if (aId == -1) {
				aId = ch - '0';
			} else {
				aId = aId * 10 + ch - '0';
			}
		} else {
			break;
		}
	}
	*id = aId;
}

void RM_ListFile() {
	char ch = 0;
	if (Serial.available()) ch = Serial.read();
	Serial.print("\nList files in SPIFFS:\n");
	SPIFFS.begin();
	Dir dir;
	switch (ch) {
		case 'A':
		case 'a':
			Serial.println("\nList of action files: \n");
			dir = SPIFFS.openDir("/alpha/action/");
			break;
		default:
			Serial.println("\nFiles of all files in SPIFFS:\n");
			dir = SPIFFS.openDir("");
			break;
	}
	while (dir.next()) {
		Serial.print(dir.fileName());

		File f = dir.openFile("r");
		Serial.printf("   %d\n", f.size());
		f.close();
	}
	Serial.println();
	SPIFFS.end();	
}

void RM_DelFile() {
	char ch = 0;
	if (Serial.available()) ch = Serial.read();
	switch (ch) {
		case 'A':
		case 'a':
			RM_DelAction();
			break;

		default:
			Serial.print("Invalid file type\n\n");
			break;
	}
}

void RM_DelAction() {
	int aId = -1;
	while (Serial.available()) {
		char ch = Serial.read();
		if ((ch >= '0') && (ch <= '9')) {
			if (aId == -1) {
				aId = ch - '0';
			} else {
				aId = aId * 10 + ch - '0';
			}
		} else {
			break;
		}
	}
	byte result = actionData.DeleteActionFile((byte) aId);

	char fileName[25];
	memset(fileName, 0, 25);
	sprintf(fileName, ACTION_FILE, aId);
	switch (result) {
		case 0:
			Serial.printf("Action file delted: %s\n\n", fileName);
			break;
		case 1:
			Serial.printf("Action file not found: %s\n\n", fileName);
			break;
		case 2:
			Serial.printf("Fail to delete file: %s\n\n", fileName);
			break;
		default:
			Serial.printf("Fail to delete file: %s\n\n", fileName);
			break;
	}
	SPIFFS.end();
}


void RM_GenFile() {
	char ch = 0;
	if (Serial.available()) ch = Serial.read();
	switch (ch) {
		case 'A':
		case 'a':
//			RM_GenAction();
			break;

		default:
			Serial.print("Invalid file type\n\n");
			break;
	}
}
/*
// standby pose
// ID:   01  02  03  04  05  06  07  08  09  10  11  12  13  14  15  16
// Dec:  90  90  90  90  90  90  90  60  76 110  90  90 120 104  70  90
// Hex:  5A  5A  5A  5A  5A  5A  5A  3C  4C  6E  5A  4A  78  68  46  5A
// Servo Time: 1000ms => 0x03E8
// Wait Time:  1000ms => 0x03E8
// No LED changed
byte POSE_STANDBY[32] = { 0x00, 0x01, 0x03, 0xE8, 0x03, 0xE8,
                          0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A,
						  0x5A, 0x3C, 0x4C, 0x6E, 0x5A,  
						  0x4A, 0x78, 0x68, 0x46, 0x5A,
						  0x00, 0x00, 0x00, 0x00,
						  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


void RM_GenAction() {
	int aId = -1;
	while (Serial.available()) {
		char ch = Serial.read();
		if ((ch >= '0') && (ch <= '9')) {
			if (aId == -1) {
				aId = ch - '0';
			} else {
				aId = aId * 10 + ch - '0';
			}
		} else {
			break;
		}
	}
	if ((aId < 0) || (aId > 255)) {
		Serial.print("Invalid action ID\n\n");
		return;
	}
	char fileName[25];
	memset(fileName, 0, 25);
	sprintf(fileName, ACTION_FILE, aId);
	SPIFFS.begin();
	if (SPIFFS.exists(fileName)) {
		if (!SPIFFS.remove(fileName)) {
			Serial.printf("Fail to delete file: %s\n\n", fileName);
			return;
		}
	}
	actionData.InitObject(aId);
	actionData.SetActionName("Standby");
	actionData.UpdatePose(aId, 0, POSE_STANDBY);
	actionData.RefreshActionInfo();
	actionData.WriteSPIFFS();
	Serial.printf("Action %d Pose %d saved to SPIFFS\n", aId, 0);
	SPIFFS.end();
}
*/