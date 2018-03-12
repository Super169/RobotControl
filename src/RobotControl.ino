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
- FB BF : Get USB Type - not implemented
- F4 4F : Get USB Type - not implemented
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
	delay(5000);
	servo.setDebug(false);  // Disable servo debug first, enable it later if needed
	retBuffer = servo.retBuffer();

	// start both serial by default, serial for command input, serial1 for debug console
	// Note: due to ESP-12 hw setting, serial1 cannot be used for input
	Serial.begin(115200);
	Serial1.begin(115200);

	DEBUG.println(F("\nUBTech Robot Control v2.0\n"));
	unsigned long actionSzie = sizeof(actionTable);


DEBUG.println("All servos ready");

	servo.begin();
	ReadSPIFFS(false);

	DEBUG.println(F("Control board ready"));
}

void loop() {
	fx_remoteControl();
}

void fx_remoteControl() {
	while (!Serial.available());
	cmd = Serial.read();
	delay(1);
	switch (cmd) {

		case 0xFA:
		case 0xFC:
			cmd_UBTCommand(cmd);
			break;
	}
	clearInputBuffer();
}

void clearInputBuffer() {
	while (Serial.available()) {
		Serial.read();
		delay(1);
	}
}


