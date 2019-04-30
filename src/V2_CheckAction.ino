#include "robot.h"
#include <math.h>
#include "V2_Command.h"


void V2_ResetAction() {
	V2_ActionPlaying = false;
	V2_ActionCombo = 0;
	V2_NextAction = 0;
	V2_NextPose = 0;
	V2_GlobalTimeMs = 0;
	V2_NextPlayMs = 0;
	V2_ActionPlayCount = 0;
}

bool V2_IsEndPose() {
	return (V2_NextPose >= actionData.PoseCnt());
}

void V2_CheckAction() {
	
	if (!V2_ActionPlaying) return;
	if (millis() < V2_NextPlayMs) return;

	if (debug) DEBUG.printf("%08ld V2_CheckAction: %d - %d - %d\n", millis(), V2_ActionCombo, V2_NextAction, V2_NextPose);


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
	if (V2_IsEndPose())	 {
		if (V2_ActionPlayCount < 2) {
			if (debug) DEBUG.printf("Action Completed\n");
			// TODO: go next action in Combo when Combo is ready
			V2_ResetAction();
			return;
		}
		// Set count to 0xFF for endless loop
		if (V2_ActionPlayCount != 0xFF) V2_ActionPlayCount--;
		V2_NextPose = 0;		
		if (debug) DEBUG.printf("Action finish, continue with last %d times\n", V2_ActionPlayCount);
	}


	uint16_t offset = 0;
	if (!actionData.IsPoseReady(V2_NextPose, offset)) {
		// Fail loading pose data
		if (debug) DEBUG.printf("Fail loading pose data: %d - %d\n", actionData.id(), V2_NextAction);
		V2_ResetAction();
		return;
	}


	byte *pose = actionData.Data();
	// pose = pose + AD_POSE_SIZE * V2_NextPose;
	pose = pose + offset;

	if (debug && deepDebug) {
		DEBUG.println("\nPOSE Data: ");
		for (int i = 0; i < AD_POSE_SIZE; i++) DEBUG.printf("%02X ", pose[i]);
		DEBUG.println("\n");
	}


	// safetu check: running corrupted data may damage the servo
	if ((pose[0] != 0xA9) || (pose[1] != 0x9A) || (pose[AD_POSE_SIZE - 1] != 0xED)) {
		if (debug) DEBUG.printf("Data file corrupted, action STOP!\n");
		V2_ResetAction();
		return;
	}
	    

	// Play current pose here
	// Should use float for rounding here? // or just let it truncated.
	float fServoTimeMs = (pose[AD_POFFSET_STIME] << 8) | pose[AD_POFFSET_STIME+1];
	// int iServoTime = round(fServoTimeMs / V2_ServoTimeRatio);
	// byte servoTime = (byte) iServoTime;
	int iServoTime = fServoTimeMs;  // For new servo library, use MS as servo time when calling move command
	uint16_t servoTime = (uint16_t) iServoTime;
	servoTime = CalAdjMs(servoTime);

	if (debug && deepDebug) DEBUG.printf("Servo Time: %f (%f) -> %d\n", fServoTimeMs, actionTimeFactor, servoTime);

	byte ledChange = 0;
	for (int i = 0; i < 8; i++) {
		ledChange |= pose[AD_POFFSET_LED + i];
	}

	if (debug) DEBUG.printf("%08ld -- Start servo command\n", millis());

	if (debug && deepDebug) DEBUG.printf("LED changed: %s\n", (ledChange ? "YES" : "NO"));


	// Move servo only if servo time > 0
	if ((servoTime > 0) || (ledChange)) {
		for (int id = 1; id <= config.maxServo(); id++) {
			if (rs.exists(id)) {
				if (ledChange) {
					int h = (id - 1) / 4;
					int l = 2 * ( 3 - ((id -1) % 4));
					byte led = (pose[AD_POFFSET_LED + h] >> l) & 0b11;
					if ((led == 0b10) || (led == 0b11)) {
						byte newMode = (led & 1);
						rs.setLED(id, !newMode);
					}
				}
				if (servoTime > 0) {
					byte angle = pose[AD_POFFSET_ANGLE + id - 1];
					// Check for dummy action to reduce commands
					if ((angle <= 0xF0) && ((V2_NextPose) || (rs.lastAngle(id) != angle))) {
						rs.goAngle(id, angle, servoTime);
						delay(1);
					}
				}
			}
		}
	}

	if (debug) DEBUG.printf("%08ld -- End servo command\n", millis());

	// Check HeadLED, follow servo status 0 - on, 1 - off
	byte headLight = pose[AD_POFFSET_HEAD];
	if (headLight == 0b10) {
		if (debug && deepDebug) DEBUG.printf("HeadLED: %d -> %d\n", headLight, 1);
		if (headLed != 1) SetHeadLed(1);
	} else if (headLight == 0b11) {
		if (debug && deepDebug) DEBUG.printf("HeadLED: %d -> %d\n", headLight, 0);
		if (headLed != 0) SetHeadLed(0);
	}

	// Chagne MP3
    byte mp3Folder = pose[AD_POFFSET_MP3_FOLDER];
    byte mp3File = pose[AD_POFFSET_MP3_FILE];
    byte mp3Vol = pose[AD_POFFSET_MP3_VOL];

	if (debug && deepDebug) DEBUG.printf("MP3: %d %d %d\n", mp3Folder, mp3File, mp3Vol);

	if (mp3Vol == AD_MP3_STOP_VOL) {
		mp3.begin();
		mp3.stop();
		mp3.end();
	} else {
		if (mp3File != 0xFF) {
			mp3.begin();
			if (mp3Folder == 0xff) {
				mp3.playFile(mp3File);
			} else {
				mp3.playFolderFile(mp3Folder, mp3File);
			}
			mp3.end();		
		}
	}

	uint16_t waitTimeMs = (pose[AD_POFFSET_WTIME] << 8) | pose[AD_POFFSET_WTIME+1];
	waitTimeMs = CalAdjMs(waitTimeMs);
	
	if (V2_UseGlobalTime) {
		V2_GlobalTimeMs += waitTimeMs;
		V2_NextPlayMs = V2_GlobalTimeMs;			
	} else {
		V2_NextPlayMs = millis() + waitTimeMs;
	}


	if (debug) DEBUG.printf("Wait Time: %d -> %ld\n", waitTimeMs, V2_NextPlayMs);

	V2_NextPose++;

	if (V2_IsEndPose())	 {
		if (V2_ActionPlayCount < 2) {
			// TODO: go next action in Combo when Combo is ready
			if (debug) DEBUG.printf("Action Completed\n");
			V2_ResetAction();
			return;
		}
		// Set count to 0xFF for endless loop
		if (V2_ActionPlayCount != 0xFF) V2_ActionPlayCount--;
		V2_NextPose = 0;		
		if (debug) DEBUG.printf("Action finished, continue with last %d times\n", V2_ActionPlayCount);
	}

	// Try to preLoad next pose by checking is
	actionData.IsPoseReady(V2_NextPose);

}


uint16_t CalAdjMs(uint16_t ms) {
	// DEBUG.printf("CalAdjMs %d Factor: %f \n", ms, actionTimeFactor);
	if ((ms == 0) || (actionTimeFactor == 1.0f)) return ms;
	uint32_t adjMs = (uint32_t) (actionTimeFactor * ms);
	if (adjMs > 0xFFFF) return 0xFFFF;
	// DEBUG.printf("CalAdjMs %d Factor: %f => %d \n", ms, actionTimeFactor, adjMs);
	return (uint16_t) adjMs;
}