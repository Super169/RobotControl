/*
	UBT Servo Coomunization Command

*/
#include "robot.h"

#pragma region "UBTech Command"
bool UBTSV_Command() {	

	if (debug) DEBUG.print(F("UBT Servo Command detected\n"));

	// if (cmdBuffer.available() < 10) return false;
	// It's not logicaly that 10 byte cannot be received within 1 ms, so consider as invalid input directly
	if (cmdBuffer.available() < 10) {
		if (debug) DEBUG.print(F("Invalid length\n"));
		return cmdSkip(true);
	}

	byte cmd[10];
	byte result[10];

	cmdBuffer.peek(cmd, 10);  // Don't read command, just check the data first
	if (cmd[9] != 0xED) {
		if (debug) DEBUG.print(F("Invalid end code\n"));
		return cmdSkip(true);
	}

	if (((cmd[0] == 0xFA) && (cmd[1] != 0xAF)) || ((cmd[0] == 0xFC) && (cmd[1] != 0xCF))) {
		if (debug) DEBUG.print(F("Invalid start code\n"));
		return cmdSkip(true);
	}

	byte sum = CheckSum(cmd);
	if (cmd[8] != sum) {
		if (debug) {
			if (debug) DEBUG.print(F("Invalid checksum\n"));
		}
		return cmdSkip(true);
	}
	// Now a complete command received, clear data from buffer
	cmdBuffer.skip(10);

	if (!enable_UBTSV) return true;

	if (debug) DEBUG.print(F("UBT Servo Command sent\n"));
	robotPort.enableTx(true); 
	robotPort.write(cmd, 10);
	robotPort.enableTx(false); 

    unsigned long endMs = millis() + UBT_COMMAND_WAIT_TIME;
    while ( (millis() < endMs) && (!robotPort.available()) ) delay(1);
    if (!robotPort.available()) {
		return true;
	}

	byte cnt = 0;
	byte buffer[20];
	while (robotPort.available()) {
		buffer[cnt++] = robotPort.read();
		if (cnt >= 20) break;
		// wait 1 ms for serial data to make sure all related result received
		if (!robotPort.available()) delay(1);
	}
	// Clear buffer
	while (robotPort.available()) {
		robotPort.read();
		if (!robotPort.available()) delay(1);
	}
	
	String sender = "Serial";
	if (cnt > 0) {
		if (SWFM.wifiClientConnected()) {
			SWFM.write(buffer, cnt);
			sender = "Network";
		} else {
			Serial.write(buffer, cnt);
		}
		if (debug) DEBUG.printf("UBT Servo Command result returned via %s\n", sender.c_str());
	}

	return true;

}
#pragma endregion
