#include "robot.h"

#define GENERIC_COMMAND_WAIT_TIME	200

bool SendGenericCommand(byte *cmd, byte sendCnt) {

	// Clear robotPort buffer
	while (robotPort.available()) {
		char ch = robotPort.read();
		if (!robotPort.available()) delay(1);
	}

	robotPort.enableTx(true); 
	delayMicroseconds(10);
	robotPort.write(cmd, sendCnt);
	robotPort.enableTx(false); 

    unsigned long endMs = millis() + GENERIC_COMMAND_WAIT_TIME;
    while ( (millis() < endMs) && (!robotPort.available()) ) delay(1);
    if (!robotPort.available()) {
        if (debug) DEBUG.printf("No return detected\n");
		return false;
	}

	byte cnt = 0;
	byte buffer[64];
	while (robotPort.available()) {
		buffer[cnt++] = robotPort.read();
		if (cnt >= 64) break;
		// wait 1 ms for serial data to make sure all related result received
		if (!robotPort.available()) delay(1);
	}
	// Clear buffer
	while (robotPort.available()) {
		robotPort.read();
		if (!robotPort.available()) delay(1);
	}
	
	String sender;
	if (cnt > 0) {
		if (SWFM.wifiClientConnected()) {
			SWFM.write(buffer, cnt);
			sender = "Network";
		} else {
			Serial.write(buffer, cnt);
            sender = "Serial";
		}
		if (debug) DEBUG.printf("Send result via %s\n", sender.c_str());
	}

    return true;
}