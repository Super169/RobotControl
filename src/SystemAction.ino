#include "robot.h"

void ActionPlaySystemAction(byte systemActionId) {
	_dbg->msg("ActionPlaySystemAction: %d", systemActionId);
	switch (systemActionId) {
		case 1:
			EyeLedHandle();
			break;	
		case 11:
			SystemAction_011();
			break;	
		case 12:
			SystemAction_012();
			break;	
		case 13:
			SystemAction_013();
			break;	
		case 14:
			SystemAction_014();
			break;	
		case 200:
			SystemAction_200();
			break;	
		case 201:
			SystemAction_201();
			break;	
		case 210:
			SystemAction_210();
			break;	
		case 250:
			if (_dbg->require(10))	_dbg->log(10,0, "USB-TTL for Robot bus");
			USB_TTL(&robotPort);
			break;	
		case 251:
			if (_dbg->require(10))	_dbg->log(10,0, "USB-TTL for Sub-system bus");
			USB_TTL(&ssbPort);
			break;	
		case 252:
			if (_dbg->require(10))	_dbg->log(10,0, "USB-TTL for MP3 bus");
			USB_TTL(&mp3_ss);
			break;	
		case 253:
			if (_dbg->require(10))	_dbg->log(10,0, "USER-TTL for Robot bus");
			USER_TTL(&robotPort);
			break;	
		case 254:
			if (_dbg->require(10))	_dbg->log(10,0, "USER-TTL for Sub-system bus");
			USER_TTL(&ssbPort);
			break;	
	}
}

void SystemAction_011() {
	if (_dbg->require(110)) _dbg->log(110,0, "SystemAction_011");
	GoMoveServo(5,5,80,100);
}

void SystemAction_012() {
	if (_dbg->require(110)) _dbg->log(110,0, "SystemAction_012");
	GoMoveServo(5,5,100,100);
}

void SystemAction_013() {
	if (_dbg->require(110)) _dbg->log(110,0, "SystemAction_013");
	GoMoveServo(5,5,120,100);
}

void SystemAction_014() {
	int id = 2;
	int elapseMs = 50;
	int step = 10;
	int time = 0;
	byte angle = 0;
	byte cmd[] = {0xFA, 0xAF,0x00,0x01,0x00, 0x00, 0x00, 0x00, 0x00, 0xED};
	cmd[2] = id;
	cmd[5] = (time / 20);
	cmd[7] = (time / 20);

	// Wait 2s to reset in angle 0 before start
	byte sum = 0;
	for (int i = 2; i < 8; i++) {
		sum += cmd[i];
	}
	cmd[8] = sum;
	robotPort.write(cmd, 10);
	delay(2000);

	while (angle <= 240) {
		cmd[4] = (angle & 0xFF);

		robotPort.enableTx(true);
		delayMicroseconds(10);

		byte sum = 0;
		for (int i = 2; i < 8; i++) {
			sum += cmd[i];
		}
		cmd[8] = sum;
		robotPort.write(cmd, 10);

		robotPort.enableTx(false);
		delayMicroseconds(10);
		angle += step;

		delay(elapseMs);
	}

}
void SystemAction_200() {
	DEBUG.printf("\n\nSystem Action 200 (System Information):\n\n");
	DEBUG.printf("Chip ID: %d\n", system_get_chip_id());
	DEBUG.printf("CPU Frequency: %d\n", system_get_cpu_freq());
	DEBUG.printf("Boot Version: %d\n", system_get_boot_version());
	DEBUG.printf("SDK Version: %s\n", system_get_sdk_version());
	DEBUG.printf("Free Heap Size: %d\n", system_get_free_heap_size());
	DEBUG.printf("VDD33: %d\n", system_get_vdd33());
	
	DEBUG.printf("\n\n");
}

void SystemAction_201() {
	// Force to read and dump all data
	DEBUG.printf("\n\nSystem Action 201 (dump SPIFFS):\n\n");
	if (!SPIFFS.begin()) {
		DEBUG.printf("Fail to start SPIFFS\n\n");
		return;
	}
	Dir dir;
	dir = SPIFFS.openDir("");
	unsigned long fcnt, fsize;
	fcnt = 0;
	fsize = 0;
	while (dir.next()) {
		DEBUG.printf("%-40s", dir.fileName().c_str());
		File f = dir.openFile("r");
		DEBUG.printf("   %8d\n", f.size());
		fcnt++;
		fsize += f.size();
		f.close();
	}
	DEBUG.printf("\n%12ld File(s)    %10ld bytes\n\n", fcnt, fsize);
	SPIFFS.end();	


}

void SystemAction_210() {
	// Force to read and dump all data
	DEBUG.printf("\n\nSystem Action 210 (check data soruce):\n\n");
	eData.Clear();
	for (int devIdx = 0; devIdx < eData.DevCount(); devIdx++) {
		if (eds[devIdx] != NULL) {
			if (eds[devIdx]->IsAvailable()) {
				eds[devIdx]->ForceGetData();
			} else {
				DEBUG.printf("eds[%d] (Device: %d, DevId: %d) is not available\n", 
							devIdx, eds[devIdx]->Device(), eds[devIdx]->DevId());
			}
		}
	}
	eData.DumpData(&DEBUG);
}


void GoMoveServo(byte servoId, int step, int execTime, int waitTime) {
	if (_dbg->require(110)) _dbg->log(110,0, "GoMoveServo(step: %d, exec: %d, wait: %d)", step, execTime, waitTime);
	rs.setLED(servoId,true);
	rs.goAngle(servoId, 0, 2000);
	unsigned long startMs = millis();
	while (millis() - startMs < 2000) {
		SetHeadLed(true);
		delay(200);
		SetHeadLed(false);
		delay(200);
	}
	int angle = 0;
	while (angle <= 240) {
		int16_t currAngle = (int16_t) rs.getAngle(servoId);
		int16_t newAngle = currAngle + step;
		if (newAngle < 0) newAngle = 0;
		if (newAngle > rs.maxAngle()) newAngle = rs.maxAngle();
		if (currAngle == newAngle) return;	// no action required
		if (_dbg->require(100)) _dbg->log(100,0,"Servo Id %d move to %d in %d ms", servoId, newAngle, execTime);
		while (!rs.goAngle(servoId, newAngle, execTime));
		delay(waitTime);
		angle += step;
	}
	rs.setLED(servoId, false);
}

#define TTL_EXIT_PIN	13

void USB_TTL(SoftwareSerial *ttl) {
	for (int i = 0; i < 5; i++) {
		SetHeadLed(true);
		delay(100);
		SetHeadLed(false);
		delay(100);
	}
	SetHeadLed(true);
	byte buffer[128];
	int cnt = 0;
	while (1) {
		if (digitalRead(TTL_EXIT_PIN)) {
			break;
		}
		if (Serial.available()) {
			cnt = 0;
			while (Serial.available()) {
				buffer[cnt++] = Serial.read();
				if (cnt >= 128) break;
				if (!Serial.available()) delayMicroseconds(100);
			}
			ttl->flush();
			ttl->enableTx(true);
			delayMicroseconds(5);			
			ttl->write(buffer, cnt);
			ttl->enableTx(false);
			// if (_dbg->require(100))	_dbg->log(100,0, "USB -> TTL : %d bytes", cnt);
		}
		if (ttl->available()) {
			cnt = 0;
			while (ttl->available()) {
				Serial.write(ttl->read());
				cnt++;
				if (!Serial.available()) delayMicroseconds(10);
			}
			// if (_dbg->require(100))	_dbg->log(100,0, "USB <<< TTL : %d bytes", cnt);
		}
	}
	if (_dbg->require(10))	_dbg->log(10,0, "Exit USB-TTL");
	SetHeadLed(false);
}

#define EXIT_CODE_SIZE	5
const byte exitCode[] = {0xA9, 0x9A, 0x01, 0x06, 0x09};

void USER_TTL(SoftwareSerial *ttl) {
	for (int i = 0; i < 5; i++) {
		SetHeadLed(true);
		delay(100);
		SetHeadLed(false);
		delay(100);
	}
	SetHeadLed(true);
	byte buffer[128];
	byte lastCode[EXIT_CODE_SIZE];
	byte lastCnt = 0;
	int cnt = 0;
	while (1) {
		if (Serial.available()) {
			cnt = 0;
			while (Serial.available()) {
				buffer[cnt++] = Serial.read();
				if (cnt >= 128) break;
				// It need to wait 100us to make sure continue code, but it waste time
				// use lastCode buffer can be better than wait 100us
				// Unforunately, when communicate with UBT servo, it does not allow delay within the 10 byte codes.
				// So keep the delay and let them go in same batch
				if (!Serial.available()) delayMicroseconds(100);
			}

			if (cnt == EXIT_CODE_SIZE) {
				bool goExit = true;
				for (int i = 0; i < EXIT_CODE_SIZE; i++) {
					if (buffer[i] != exitCode[i]) {
						goExit = false;
						break;
					}
				}
				if (goExit) break;
			}

			/*
			// no need to check if cnt > 5, over the code size
			if (cnt <= EXIT_CODE_SIZE) {
				if (cnt == EXIT_CODE_SIZE) {
					// fill last 5 from buffer
					for (int idx = 0; idx < EXIT_CODE_SIZE; idx++) {
						lastCode[idx] = buffer[idx];
					}
					lastCnt = EXIT_CODE_SIZE;
				} else {
					if (lastCnt + cnt > EXIT_CODE_SIZE) {
						byte removeCnt = lastCnt + cnt - EXIT_CODE_SIZE;
						for (int idx = 0; idx < (lastCnt - removeCnt); idx++) {
							lastCode[idx] = lastCode[idx + removeCnt];
						}
						lastCnt -= removeCnt;
					}
					// fill all from buffer
					for (int idx = 0; idx < cnt; idx ++) {
						lastCode[lastCnt + idx] = buffer[idx];
					}
					lastCnt += cnt;
				}
				if (lastCnt == EXIT_CODE_SIZE) {
					bool goExit = true;
					for (int i = 0; i < EXIT_CODE_SIZE; i++) {
						if (lastCode[i] != exitCode[i]) {
							goExit = false;
							break;
						}
					}
					if (goExit) break;
				}

			}
			*/

			ttl->flush();
			ttl->enableTx(true);
			delayMicroseconds(5);			
			ttl->write(buffer, cnt);
			ttl->enableTx(false);
			// if (_dbg->require(100))	_dbg->log(100,0, "USB -> TTL : %d bytes", cnt);
		}
		if (ttl->available()) {
			cnt = 0;
			while (ttl->available()) {
				Serial.write(ttl->read());
				cnt++;
			}
			// if (_dbg->require(100))	_dbg->log(100,0, "USB <<< TTL : %d bytes", cnt);
		}
	}
	if (_dbg->require(10))	_dbg->log(10,0, "Exit USER-TTL");
	SetHeadLed(false);
}