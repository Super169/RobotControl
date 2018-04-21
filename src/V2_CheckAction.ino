#include "robot.h"
#include <math.h>
#include "V2_Command.h"


void V2_ResetAction() {
	V2_ActionPlaying = false;
	V2_ActionCombo = 0;
	V2_NextAction = 0;
	V2_NextPose = 0;
	V2_NextPlayMs = 0;
}

bool V2_IsEndPose() {
	return (V2_NextPose >= actionData.Header()[AD_OFFSET_POSECNT]);
}

void V2_CheckAction() {
	
	if (!V2_ActionPlaying) return;
	if (millis() < V2_NextPlayMs) return;

	if (debug) DEBUG.printf("V2_PlayAction: %d - %d - %d\n", V2_ActionCombo, V2_NextAction, V2_NextPose);

	if (actionData.id() != V2_NextAction) {
		if (V2_NextPose) {
			// For new action, should always be started at 0
			// TODO: should quit or just reset to 0 ????
			if (debug) DEBUG.printf("Invalid poseId %d for new action.  Action stopped!\n", V2_NextPose);
			V2_ResetAction();
			return;
		}
		if (debug) DEBUG.printf("Load Action: %d -> %d\n", actionData.id(), V2_NextAction);
		actionData.ReadSPIFFS(V2_NextAction);
	}
	// Should already checked in previous pose, just check again for safety
	if (V2_IsEndPose()) {
		// TODO: go next action in Combo when Combo is ready
		if (debug) DEBUG.printf("Action Completed\n");
		V2_ResetAction();
		return;
	}

	byte *pose = actionData.Data();
	pose = pose + AD_POSE_SIZE * V2_NextPose;

	// Play current pose here
	// Should use float for rounding here? // or just let it truncated.
	float fServoTimeMs = (pose[AD_POFFSET_STIME] << 8) || pose[AD_POFFSET_STIME+1];
	int iServoTime = round(fServoTimeMs / V2_ServoTimeRatio);
	byte servoTime = (byte) iServoTime;

	// Move servo only if servo time > 0
	if (servoTime > 0) {
		for (int id = 1; id <= 16; id++) {
			byte angle = pose[AD_POFFSET_ANGLE + id - 1];
			// Check for dummy action to reduce commands
			if ((angle <= 0xF0) && (servo.lastAngle(id) != angle)) {
				servo.move(id, angle, servoTime);
			}
		}
	}

	// Check LED change
	uint16_t ledChange = (pose[AD_POFFSET_LED] << 8) || pose[AD_POFFSET_LED+1];
	if (ledChange) {
		uint16_t ledFlag = (pose[AD_POFFSET_LED+2] << 8) || pose[AD_POFFSET_LED+3];
		for (int i = 0; i < 16; i++) {
			if (ledChange && (1 << i)) {
				byte status = (byte) (ledFlag && (1 << i));
				byte id = i + 1;
				byte mode = (status ? 0 : 1);
				servo.setLED(id, mode);
			}
		}
	}

	byte headLight = pose[AD_POFFSET_HEAD];
	if (headLight == 0x10) {
		// Trun off the light
	} else if (headLight = 0x11) {
		// Turn on the light
	}

	uint16_t waitTimeMs = (pose[AD_POFFSET_WTIME] << 8) || pose[AD_POFFSET_WTIME+1];
	V2_NextPlayMs = millis() + waitTimeMs;
	V2_NextPose++;

	if (V2_IsEndPose())	 {
		if (debug) DEBUG.printf("Action Completed\n");
		V2_ResetAction();
		return;
	}
}

void V2_goPlayAction(byte actionCode) {
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
