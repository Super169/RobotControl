// UBT command utility

void UBT_GetServoAngle(byte *result) {
	// result must have at least 32 bytes

	for (int id = 1; id <= 16; id++) {
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
		for (int i = 0; i < 32; i++) {
			DEBUG.printf("%02X ", result[i]);
		}
		DEBUG.println();
	}
}



void UBT_ReadSPIFFS(byte cmdCode) {
	byte result[2];
	SPIFFS.begin();
	if (SPIFFS.exists(actionDataFile)) {
		File f = SPIFFS.open(actionDataFile, "r");
		if (!f) {
			if (debug) DEBUG.println(F("Fail to open SPIFFS file"));
			result[1] = READ_ERR_OPEN_FILE;
		} else {
			if (f.size() != sizeof(actionTable)) {
				if (debug) {
					DEBUG.printf("Invalid File size: %d\n", f.size());
				}
				result[1] = READ_ERR_FILE_SIZE;
			} else {
				memset(actionTable, 0, sizeof(actionTable));
				size_t wCnt = f.readBytes((char *)actionTable, sizeof(actionTable));
				f.close();
				if (wCnt == sizeof(actionTable)) {
					result[1] = READ_OK;
				} else {
					if (debug) DEBUG.println(F("Errro reading SPIFFS file"));
					result[1] = READ_ERR_READ_FILE;
				}
			}
		}
	} else {
		if (debug) {
			DEBUG.print(F("SPIFFS file not found: "));
			DEBUG.println(actionDataFile);
		}
		result[1] = READ_ERR_NOT_FOUND;
	}
	SPIFFS.end();	
	if (cmdCode) {
		result[0] = cmdCode;
		Serial.write(result, 2);
	}
}

void UBT_WriteSPIFFS(byte cmdCode) {
	byte result[2];
	SPIFFS.begin();
	File f = SPIFFS.open(actionDataFile, "w");
	if (!f) {
		result[1] = WRITE_ERR_OPEN_FILE;
	} else {
		size_t wCnt = f.write((byte *)actionTable, sizeof(actionTable));
		f.close();
		if (wCnt == sizeof(actionTable)) {
			result[1] = WRITE_OK;
		} else {
			result[1] = WRITE_ERR_WRITE_FILE;
		}
	}
	SPIFFS.end();	
	if (cmdCode) {
		result[0] = cmdCode;
		Serial.write(result, 2);
	}
}