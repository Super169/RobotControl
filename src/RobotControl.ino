/*
UBTech Alpha 1S Control Board (ESP8266 version) - Version 2.x
By Super169

Command set for V2

UBTech Servo Command
- FA AF {id} {cmd} xx xx xx xx {sum} ED
- FC CF {id} {cmd} xx xx xx xx {sum} ED

UBTech Controller Command
- F1 1F 00 00 00 00 00 00 {sum} ED : Get version
- EF FE 00 00 00 00 00 00 {sum} ED : Servo status
- F2 2F 00 00 00 00 00 00 {sum} ED : Stop play
- F5 5F 01 00 00 00 00 00 {sum} ED : Firmware version
- FB BF - not implemented : will be handled as BT command
- F4 4F : fine tuning - not implemented
- F9 9F : Get USB Type - not implemented
- F3 3F {len} xx - xx {sum} ED : Play action by name

UBTech Alpha 1 BT Protocol
- FB BF {len} {cmd} xx ... xx {sum} ED

V2 Control Board Command
- A9 9A {len} {cmd} xx ... xx {sum} ED

** All V1 Control Board Command will be obsolete
- {cmd} xx ... xx

Major change on 2.x:
- New command set for V2 with start / end cod and checksum for better communization control
- Use major loop for action playback, so there has no block in playing action, can be stopped anytime
- Re-design the object model for servo and control board
- Default reset action with all servo logged, and LED on
- Use single file to store separate action, action with name, a maximum of 99 action allowed
- Support unlimited step in action
- Support servo LED control
- Support Robot Head LED control


PIN Assignment:
GPIO-12 : One-wire software serial - servo contorl
GPIO-2  : Serial1 - debug console

*/

#include "robot.h"

void setup() {
	// Delay 2s to wait for all servo started
	delay(2000);

	// SetDebug(false);  // Disable servo debug first, enable it later if needed
	SetDebug(true);

	retBuffer = servo.retBuffer();

	// start both serial by default, serial for command input, serial1 for debug console
	// Note: due to ESP-12 hw setting, serial1 cannot be used for input
	Serial.begin(115200);
	Serial1.begin(115200);

	DEBUG.println(F("\nUBTech Robot Control v2.0\n"));
	unsigned long actionSzie = sizeof(actionTable);

	servo.begin();
	// TODO: change to V2 when ready
	V1_goReadSPIFFS(false);

	DEBUG.println(F("Control board ready\n\n"));
}

void loop() {
	CheckSerialInput();
	playAction();
	CheckSerialInput();
	remoteControl();
}

// move data from Serial buffer to command buffer
void CheckSerialInput() {
	if (Serial.available()) {
		delay(1); // make sure one command is completed
		while (Serial.available()) {
			cmdBuffer.write(Serial.read());
		}
	}
}

void playAction() {

}

void remoteControl() {
	bool goNext = true;
	int preCount;
	while (cmdBuffer.available()) {
		preCount = cmdBuffer.available();
		cmd = cmdBuffer.peek();
		switch (cmd) {

			case 0xFB:
				goNext = UBT_BTCommand();
				break;

			case 0xF1:
			case 0xF2:
			case 0xF3:
			case 0xF4:
			case 0xF5:
			case 0xF9:
			case 0xEF:
				goNext = UBT_ControlBoard();
				break;

			case 0xFA:
			case 0xFC:
				goNext = UBT_ServoCommand();
				break;

			case 0xA9:
				goNext = V2_CommandSet();
				break;


			case 'A':
			case 'a':
			case 'B':
			case 'b':
			case 'D':
			case 'F':
			case 'f':
			case 'J':
			case 'L':
			case 'l':
			case 'M':
			case 'm':
			case 'P':
			case 'R':
			case 'T':
			case 't':
			case 'S':
			case 'U':
			case 'W':
			case 'Z':
				goNext = V1_CommandSet();
				break;

			default:
				cmdSkip(true);
				break;

		}
		// for some situation, command data not yet competed.
		// need to study if it should wait for full data inside the handler
		if (goNext) {
			if (preCount == cmdBuffer.available()) {
				// Program bug, no data handled, but not ask for wait
				if (debug) {
					DEBUG.print(F("preCount: "));
					DEBUG.print(preCount);
					DEBUG.print(F(" => "));
					DEBUG.println(cmdBuffer.available());
					DEBUG.print(F("****** Missing handler: "));
					DebugShowSkipByte();
				}
				cmdBuffer.skip();
			}
			lastCmdMs = 0;
		} else {
			if ((lastCmdMs)	&& (millis() - lastCmdMs > MAX_WAIT_CMD)) {
				// Exceed max wait time for a command
				if (debug) {
					DEBUG.print(F("Command timeout: "));
					DebugShowSkipByte();
				}
				cmdBuffer.skip();
				lastCmdMs = 0;
			} else {
				if (!lastCmdMs) lastCmdMs = millis();
				break;
			}
		}
	}
}


