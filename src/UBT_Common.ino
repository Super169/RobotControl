// UBT command utility
#include "robot.h"

void UBT_GetServoAngle(byte *result) {
	// result must have at least 32 bytes

	for (int id = 1; id <= config.maxServo() ; id++) {
		int pos = 2 * (id - 1);
		if (servo.exists(id)) {
			if (servo.isLocked(id)) {
				result[pos] = servo.lastAngle(id);
				result[pos+1] = 1;
			} else {
				result[pos] = servo.getPos(id);
				result[pos+1] = 0;
			}
		} else {
			result[pos] = 0xFF;
			result[pos+1] = 0;
		}
	}

	if (debug) {
		for (int i = 0; i < 2 * config.maxServo(); i++) {
			DEBUG.printf("%02X ", result[i]);
		}
		DEBUG.println();
	}
}

void UBT_GetServoAdjAngle(byte *result) {
	for (int id = 1; id <= config.maxServo(); id++) {
		int pos =2 * (id - 1);
		if (servo.exists(id)) {
			uint16  adjAngle = servo.getAdjAngle(id);
			result[pos] = adjAngle / 256;
			result[pos+1] = adjAngle % 256;
		} else {
			result[pos] = 0x7F;
			result[pos+1] = 0x7F;
		}
	}
}

