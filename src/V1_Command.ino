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
			case 'i':
				V1_CheckSD(&Serial);
				break;
			case 'I':
				V1_CheckSD(&Serial1);
				break;
			case 'P':
				V1_PlayAction();
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

#pragma region "P - Play action"

void V1_PlayAction() {
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
#pragma endregion

#pragma region "Z - Reset Connection"

void V1_ResetConnection() {
	if (debug) DEBUG.println(F("[V1_ResetConnection]"));
	// bus control is not under RobotServo, should be done separately
	robotPort.end();
	delay(100);
	robotPort.begin(busConfig.baud);
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
