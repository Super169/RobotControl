#include "robot.h"
#include "V2_Command.h"

/*
// Command start with A9 9A

// {len} = cmd[2] = count( {len} xx .. xx )
// {sum} = cmd[{len} + 2] = sum( {len} xx .. xx )
// Size = A9 9A [{len} ...] {sum} ED = [{len} ... ] + 4 = len + 4
// minimum : A9 9A 2 {cmd} {sum} ED = 6

// Command set:
//
//   01 - Reset (fix)    		 	: A9 9A 02 01 03 ED
//   02 - Set Debug (fix) 			: A9 9A 03 02 00 05 ED
//                                    A9 9A 03 02 01 06 ED    
//   03 - Set DevMode (fix) 		: A9 9A 03 03 00 06 ED
//									  A9 9A 03 03 01 07 ED
//   04 - Get Config (fix)          : A9 9A 02 04 06 ED
//   05 - Set Config 
//   06 - Default Config
//   07 - Enter USB-TTL mode (fix)  : A9 9A 07 03 01 01 05 ED
//   0A - Command Enable (fix)		: A9 9A 02 0A 0C ED
//                                    A9 9A 07 0A 00 01 01 01 01 15 ED
//   0B - Check battery (fix)       : A9 9A 02 0B 0D ED
//   11 - Get Angle (fix) 			: A9 9A 02 11 13 ED
//   12 - Get One Angle (fix)		: A9 9A 03 12 01 16 ED
//   13 - Get Adj Angle (fix)		: A9 9A 02 13 15 ED
//   14 - Get One Adj Angle (fix) 	: A9 9A 03 14 01 18 ED
//	 21 - Lock servo (var)			: A9 9A 06 21 01 02 03 04 31 ED
//	 22 - Unlock servo (var)		: A9 9A 05 22 01 02 03 2D ED
//	 23 - Servo move (var)			: A9 9A 05 23 00 5A A0 22 ED
//									: A9 9A 08 23 01 5A A0 02 00 A0 C8 ED 
//	 24 - Set LED (var)				: A9 9A 04 24 00 01 29 ED
//                                  : A9 9A 06 24 01 00 02 01 2E ED
//   31 - Set Head LED (fix)        : A9 9A 03 31 00 34 ED
//                                    A9 9A 03 31 01 35 ED
//	 32	- Stop MP3 (fix)			: A9 9A 02 32 34 ED
//   33 - Play File (fix)			: A9 9A 04 33 00 01 38 ED
//                                    A9 9A 04 33 FF 02 38 ED
//   34 - Play MP3 File (fix)		: A9 9A 03 34 01 38 ED
//   35 - Play Avert File (fix)		: A9 9A 03 35 01 39 ED
//   36 - MP3 Set Volume (fix)		: A9 9A 04 36 00 0F 49 ED
//                                    A9 9A 04 36 01 01 3C ED
//   37 - MP3 Command				: A9 9A 05 37 12 00 01 4F ED    { play first MP3 file }
//                                    A9 9A 05 37 0F 01 02 4E ED    { play 01\002 file }
//                                    A9 9A 05 37 16 00 00 52 ED    { stop play }
//   41 - Play Action               : A9 9A 03 41 00 43 ED
//   42 - Play Combo                : A9 9A 03 42 00 44 ED
//   4F - Stop playing              : A9 9A 02 4F 51 ED
//   61 - Read Action Header (fix) 	: A9 9A 03 61 01 65 ED
//   62 - Read Action Data (fix)	: A9 9A 03 62 01 65 ED

//   71 - Update action header      : {Refer to ActionInfo}
//   72 - Update action pose        : {refer to PoseInfo}	
//   74 - Update action name	    : A9 9A 0B 74 01 07  44 65 66 61 75 6C 74 D7 ED
//   75 - Delete action file	    : A9 9A 03 75 00 78 ED

//   81 - Check MPU					: A9 9A 02 81 83 ED
//   82 - Get MPU					: A9 9A 02 82 84 ED

//   91 - Get Event Header
//   92 - Get Event Data
//   93 - Save Event Header
//   94 - Save Event Data

//   FF - Get Version				: A9 9A 02 FF 01 ED

*/

bool V2_Command() {

	if (cmdBuffer.available() < 6) return false;
	byte header[3];
	cmdBuffer.peek(header, 3);
	if (header[1] != 0x9A) {
		if (_dbg->require(200)) _dbg->log(200, 0, "Invalid start code");
		return cmdSkip(true);
	}
	int len = header[2];
	if (cmdBuffer.available() < len + 4) return false;

	byte cmd[len+4];
	cmdBuffer.peek(cmd, len+4);

	if (debug) {
		for (int i = 0; i < len + 4; i++) {
			DEBUG.printf("%02X ", cmd[i]);
		}
	}

	if (cmd[len+3] != 0xED) {
		if (_dbg->require(200)) _dbg->log(200, 0, " => Invalid end code");
		return cmdSkip(true);
	}

	byte sum = CheckVarSum(cmd);
	if (debug) {
		DEBUG.printf("=> %02X\n", sum);
	}

	// bypass checksum in devMode
	if (!devMode) {
		if (cmd[len+2] != sum) {
			if (debug) 	DEBUG.println(F("Invalid checksum"));
			return cmdSkip(true);
		}
	}

	// header, checksum, endcode passed
	cmdBuffer.skip(len+4);

	// V2_CMD_ENABLE should always available
	if ((!enable_V2) && (cmd[3] != V2_CMD_ENABLE)) {
		if (debug) {
			DEBUG.print(F("V2 Command disabled:"));
			for (int i = 0; i < len + 4; i++) {
				DEBUG.printf(" %02X", cmd[i]);
			}
			DEBUG.println();
		}  
		return true;
	}

	switch (cmd[3]) {

		case V2_CMD_GET_VERSION:
			V2_GetVersion(cmd);
			return true;
			break;

		case V2_CMD_DEBUG:
			V2_SetDebug(cmd);
			return true;
			break;
		
		case V2_CMD_DEVMODE:			
			V2_SetDevMode(cmd);
			return true;
			break;

		case V2_CMD_CHECK_BATTERY:
			V2_CheckBattery(cmd);
			return true;
			break;

		case V2_CMD_MP3_STOP:
			V2_Mp3Stop(cmd);
			return true;
			break;

		case V2_CMD_MP3_SETVOLUME:
			V2_Mp3SetVolume(cmd);
			return true;
			break;

		case V2_CMD_MP3_COMMAND:
			V2_Mp3Command(cmd);
			return true;
			break;

		case V2_CMD_STOPPLAY:
			V2_ResetAction();
			if (config.mp3Enabled()) {
				mp3.begin();
				mp3.stop();
				mp3.end();
			}
			V2_SendSingleByteResult(cmd, 0);
			return true;
			break;

		// allow to disable eventhandler event playing 
		case V2_CMD_SET_EH_MODE:
			V2_SetEventHandlerMode(cmd);
			return true;
			break;

		case V2_CMD_SET_ACTIONSPEED:
			V2_SetActionSpeed(cmd);
			return true;
			break;

	}


	// No command except STOP is allowed when action playing
	// Even some command will not affect the action, but for safety, do not play any of them.
	// In fact, there shoudl have such command sending from controller when action is playing.
	// All return the generic result:
	//   A9 9A 03 {cmd} FF {sum} ED
	if (V2_ActionPlaying) {
		V2_SendSingleByteResult(cmd, 0xFF);
		return true;
	}


	switch (cmd[3]) {

		case V2_CMD_RESET:
			V2_Reset(cmd);
			break;

		case V2_CMD_ENABLE:
			V2_CommandEnable(cmd);
			break;

		case V2_CMD_GETCONFIG:
			V2_GetConfig(cmd);
			break;

		case V2_CMD_SETCONFIG:
			V2_SetConfig(cmd);
			break;

		case V2_CMD_DEFAULTCONFIG:
			V2_DefaultConfig(cmd);
			break;

		case V2_CMD_GET_EH_MODE:
			V2_GetEventHandlerMode(cmd);
			break;

		case V2_CMD_GET_NETWORK:
			V2_GetNetwork(cmd);
			break;

		case V2_CMD_GET_WIFI_CONFIG:
			V2_GetWiFiConfig(cmd);
			break;

		case V2_CMD_SET_WIFI_CONFIG:
			V2_SetWiFiConfig(cmd);
			break;

		case V2_CMD_PARTIAL_CONFIG:
			V2_SetPartialWiFiConfig(cmd);
			break;

		case V2_CMD_SERVOTYPE:
			V2_GetServoType(cmd);
			break;

		case V2_CMD_SERVOANGLE:
			V2_GetServoAngle(cmd);
			break;

		case V2_CMD_ONEANGLE:
			V2_GetOneAngle(cmd);
			break;

		case V2_CMD_SERVOADJANGLE:
			V2_GetServoAdjAngle(cmd);
			break;

		case V2_CMD_ONEADJANGLE:
			V2_GetOneAdjAngle(cmd);
			break;

		case V2_CMD_SETADJANGLE:
			V2_SetAdjAngle(cmd);
			break;

		case V2_CMD_SERVOCMD:
			V2_ServoCommand(cmd);
			break;

		case V2_CMD_SETANGLE:
			V2_SetAngle(cmd)			;
			break;

		case V2_CMD_LOCKSERVO:
			V2_LockServo(cmd, true);
			break;

		case V2_CMD_UNLOCKSERVO:
			V2_LockServo(cmd, false);
			break;

		case V2_CMD_SERVOMOVE:
			V2_ServoMove(cmd);
			break;

		case V2_CMD_LED:
			V2_SetLED(cmd);
			break;

		case V2_CMD_SET_HEADLED:
			V2_SetHeadLED(cmd);
			break;

		case V2_CMD_MP3_PLAYFILE:
			V2_Mp3PlayFile(cmd);
			break;

		case V2_CMD_MP3_PLAYMP3:
			V2_Mp3PlayMp3(cmd);
			break;

		case V2_CMD_MP3_PLAYADVERT:
			V2_Mp3PlayAdvert(cmd);
			break;

		case V2_CMD_PLAYACTION:
			V2_PlayAction(cmd);
			break;

		case V2_CMD_PLAYCOMBO:
			V2_PlayCombo(cmd);
			break;

		case V2_CMD_GET_ADLIST:
			V2_GetAdList(cmd);
			break;

		case V2_CMD_GET_ADHEADER:
			V2_GetAdHeader(cmd);
			break;

		case V2_CMD_GET_ADPOSE:
			V2_GetAdPose(cmd);
			break;
			
		case V2_CMD_GET_COMBO:
			V2_GetCombo(cmd);
			break;

		case V2_CMD_UPD_COMBO:
			V2_UpdateCombo(cmd);
			break;

		case V2_CMD_UPD_ADHEADER:
			V2_UpdateAdHeader(cmd);
			break;

		case V2_CMD_UPD_ADPOSE:
			V2_UpdateAdPose(cmd);
			break;

		case V2_CMD_UPD_ADNAME:
			V2_UpdateAdName(cmd);
			break;

		case V2_CMD_DEL_ACTION:
			V2_DeleteActionFile(cmd);
			break;

		case V2_CMD_CHK_MPU:
			V2_CheckMPU(cmd);
			break;

		case V2_CMD_GET_MPU_DATA:
			V2_GetMPUData(cmd);
			break;

		case V2_CMD_GET_EVENT_HEADER:
			V2_GetEventHeader(cmd);
			break;
		case V2_CMD_GET_EVENT_DATA:
			V2_GetEventData(cmd);
			break;
		case V2_CMD_SAVE_EVENT_HEADER:
			V2_SaveEventHeader(cmd);
			break;
		case V2_CMD_SAVE_EVENT_DATA:
			V2_SaveEventData(cmd);
			break;

		case V2_CMD_USB_TTL:
			V2_USB_TTL(cmd);
			break;

		default:
			if (debug) {
				DEBUG.printf("Undefined command: %02X\n", cmd[3]);
			}
	}

	return true;
}

void V2_SendResult(byte *result) {
	uint16_t len = result[2];
	uint16_t size = len + 4;
	result[0] = 0xA9;
	result[1] = 0x9A;
	result[len+2] = CheckVarSum(result);
	result[len+3] = 0xED;

	if (SWFM.wifiClientConnected()) {
		SWFM.write(result, size);
	} else {
		Serial.write(result, size);
	}	
}

void V2_SendSingleByteResult(byte *cmd, byte data) {
	byte result[7];
	result[2] = 3;
	result[3] = cmd[3];
	result[4] = data;
	if (_dbg->require(100)) _dbg->log(100, 0, "SendSingleByteResult: %d\n", data);
	V2_SendResult(result);
}

void V2_GetVersion(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_GetVersion]");
	byte result[10];
	result[2] = 6;
	result[3] = cmd[3];
	result[4] = VERSION_MAJOR;
	result[5] = VERSION_MINOR;
	result[6] = VERSION_SUB;
	result[7] = VERSION_FIX;
	V2_SendResult(result);	
}

#pragma region V2_CMD_RESET / V2_CMD_DEBUG / V2_CMD_DEVMODE / V2_CMD_GETCONFIG / V2_CMD_SETCONFIG

void V2_Reset(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_Reset]");
	robotPort.end();
	delay(200);
	robotPort.begin(busConfig.baud);
	byte showAngle = 0;
	if (cmd[2] > 2) showAngle = cmd[4];
	if ((showAngle) && (showAngle != '0')) {
		V2_GetServoAngle(cmd);
	} else {
		V2_SendSingleByteResult(cmd, RESULT::SUCCESS);
	}
}

void V2_SetDebug(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_SetDebug]");
	byte status = (cmd[4] ? 1 : 0);
	if (debug && !status) DEBUG.printf("Disable debug mode\n");
	SetDebug(status);
	config.setDebug(status);
	config.writeConfig();
	if (debug) DEBUG.printf("Debug mode %s\n", (status ? "enabled" : "disabled"));
	V2_SendSingleByteResult(cmd, RESULT::SUCCESS);
}

void V2_SetDevMode(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_SetDevMode]");
	devMode = (cmd[4] ? 1 : 0);
	if (debug) DEBUG.printf("Developer mode %s\n", (devMode ? "enabled" : "disabled"));
	V2_SendSingleByteResult(cmd, RESULT::SUCCESS);
}

void V2_CommandEnable(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_CommandEnable]");
	byte result[12];
	// Should have only 2 options:
	//   cmd[2] = 8 : set all 6 flags
	//   cmd[2] = 2 : enquiry  (just assume all non-7 length is for enquiry)
	if (cmd[2] == 8) {
		enable_V1 = cmd[4];
		enable_V2 = cmd[5];
		enable_UBTBT = cmd[6];
		enable_UBTCB = cmd[7];
		enable_UBTSV = cmd[8];
		enable_HAILZD = cmd[9];
	} 
	result[2] = 8;
	result[3] = cmd[3];
	result[4] = (enable_V1 ? 1 : 0);
	result[5] = (enable_V2 ? 1 : 0);
	result[6] = (enable_UBTBT ? 1 : 0);
	result[7] = (enable_UBTCB ? 1 : 0);
	result[8] = (enable_UBTSV ? 1 : 0);
	result[9] = (enable_HAILZD ? 1 : 0);

	V2_SendResult(result);
}

void V2_GetConfig(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_GetConfig]");
	config.Data()[3] = cmd[3];

	V2_SendResult((byte *) config.Data());

}

void V2_SetConfig(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_SetConfig]");
	if (cmd[2] != RC_CONFIG_DATA_SIZE) {
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_SIZE);
		return;
	}

	memcpy((byte *) config.Data(), (byte *) cmd, RC_RECORD_SIZE);
	byte result = config.writeConfig();
	V2_SendSingleByteResult(cmd, result);
}

void V2_DefaultConfig(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_DefaultConfig]");
	config.initConfig();
	byte result = config.writeConfig();
	if (result == RESULT::SUCCESS) {
		if (!SWFM.resetDefault()) {
			result = RESULT::ERR::UPDATE_CONDIG;
		}
	}
	V2_SendSingleByteResult(cmd, result);
}

#pragma endregion


void V2_CheckBattery(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_CheckBattery]");
	if (cmd[2] != 2) {
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_SIZE);
		return;
	} 


	byte result[9];
	result[2] = 5;
	result[3] = cmd[3];
	uint16_t v = analogRead(0);
	byte iPower = edsBattery[0]->GetPower(v);
	result[4] = iPower;
	result[5] = v >> 8;
	result[6] = v & 0xFF;

	if (debug) DEBUG.printf("[V2_CheckBattery] - %d -> %d%%\n", v, iPower);

	V2_SendResult(result);
}

void V2_GetNetwork(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_GetNetwork]");
	if (cmd[2] != 2) {
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_SIZE);
		return;
	} 

	byte result[60];
	memset(result, 0, 60);
	result[2] = 56;
	result[3] = cmd[3];
	result[4] = NetworkMode;
	String tmp;
	char buf[20];
	
	switch (NetworkMode) {
		case NETWORK_ROUTER:
			memset(buf, 0, 20);
			tmp = WiFi.SSID();
			tmp.toCharArray(buf, 20);
			memcpy((byte *)(result + 5), buf, 20);
			tmp = WiFi.localIP().toString();
			tmp.toCharArray(buf, 20);
			memcpy((byte *)(result + 25), buf, 20);
			break;
		case NETWORK_AP:
			memcpy((byte *)(result + 5), AP_Name, strlen(AP_Name));
			/*
			for (size_t i = 0; i < strlen(AP_Name); i++ ) {
				result[5+i] = AP_Name[i];
			}
			*/
			tmp = WiFi.softAPIP().toString();
			tmp.toCharArray(buf, 20);
			memcpy((byte *)(result + 25), buf, 20);
			break;
	}
	
	result[45] = port >> 8;
	result[46] = port & 0xFF;
	V2_SendResult(result);
}

void V2_GetWiFiConfig(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_GetWiFiConfig]");
	if (cmd[2] != 2) {
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_SIZE);
		return;
	} 

	byte result[SWFM_CONFIG_FILE_SIZE];

	memset(result, 0, 60);
	memcpy(result, SWFM.getConfig(), SWFM_CONFIG_FILE_SIZE);
	result[2] = SWFM_CONFIG_FILE_SIZE - 4;
	result[3] = cmd[3];
	V2_SendResult(result);
}


void V2_SetWiFiConfig(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_SetWiFiConfig]");
	uint8_t result = RESULT::SUCCESS;

	if (cmd[2] != (SWFM_CONFIG_FILE_SIZE - 4)) {
		result = RESULT::ERR::PARM_SIZE;
	} else if (!SWFM.setConfig((uint8_t *) cmd)) {
		result = RESULT::ERR::UPDATE_CONDIG;
	} 
	V2_SendSingleByteResult(cmd, result);
	return;
}

void V2_SetPartialWiFiConfig(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_SetPartialWiFiConfig]");
	uint8_t result = RESULT::SUCCESS;

	// data to be overwrite should has len from 1 to SWFM_CONFIG_FILE_SIZE
	// added 3 extra bytes of {len} {cmd} {offset}, command's data length should be 4 to SWFM_CONFIG_FILE_SIZE + 3
	// also, {offset} + {overwrite len} should <= SWFM_CONFIG_FILE_SIZE
	if ((cmd[2] < 4) || (cmd[2] > SWFM_CONFIG_FILE_SIZE + 3)) {
		result = RESULT::ERR::PARM_SIZE;
	} else if (cmd[2] + cmd[4] - 3 > SWFM_CONFIG_FILE_SIZE) {
		result = RESULT::ERR::PARM_SIZE;
	} else {
		byte buffer[SWFM_CONFIG_FILE_SIZE];
		memset(buffer, 0, 60);
		memcpy(buffer, SWFM.getConfig(), SWFM_CONFIG_FILE_SIZE);
		uint8_t *target = buffer + cmd[4];
		uint8_t *source = cmd + 5;
		size_t size = cmd[2] - 3;
		memcpy(target, source, size);
		if (!SWFM.setConfig((uint8_t *) buffer)) {
			result = RESULT::ERR::UPDATE_CONDIG;
		}
	} 
	V2_SendSingleByteResult(cmd, result);
	return;
}


#pragma region V2_CMD_SERVOTYPE / V2_CMD_SERVOANGLE / V2_CMD_ONEANGLE

void V2_GetServoType(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_GetServoType]");
	byte servoType = rs.servoType();
	V2_SendSingleByteResult(cmd, servoType);
}

void V2_GetServoAngle(byte *cmd) {
	
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_GetServoAngle]");
	uint16_t len = 2 * config.maxServo() + 2;
	byte result[len+4];
	
	result[2] = len;
	result[3] = cmd[3];
	byte *ptr = result + 4;
	
	UBT_GetServoAngle(ptr);
	V2_SendResult(result);
	
}

void V2_GetOneAngle(byte *cmd) {

	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_GetOneAngle]");
	byte result[9];
	result[2] = 5;
	result[3] = cmd[3];

	if (cmd[2] == 3) {
		byte id = cmd[4];
		result [4] = id;
		if ((id) && (id <= config.maxServo()) && (rs.exists(id))) {
				if (rs.isLocked(id)) {
					result[5] = rs.lastAngle(id);
					result[6] = 1;
				} else {
					result[5] = rs.getAngle(id);
					result[6] = 0;
				}
		} else {
			result[5] = 0xff;
			result[6] = 0xff;
		}
	} else {
		result[4] = 0x00;
		result[5] = 0xff;
		result[6] = 0xff;
	}
	V2_SendResult(result);

}


#pragma endregion

#pragma region V2_CMD_SERVOADJANGLE / V2_CMD_ONEADJANGLE

void V2_GetServoAdjAngle(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_GetServoAdjAngle]");
	// uint16_t len = 34;
	uint16_t len = 2 * config.maxServo() + 2;
	byte result[len+4];
	result[2] = len;
	result[3] = cmd[3];
	byte *ptr = result + 4;
	UBT_GetServoAdjAngle(ptr);
	V2_SendResult(result);

}

// A9 9A 04 {cmd} {id} {high} {low} {sum} ED

void V2_GetOneAdjAngle(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_GetOneAdjAngle]");
	byte result[9];
	result[2] = 5;
	result[3] = cmd[3];

	if (cmd[2] == 3) {
		byte id = cmd[4];
		result [4] = id;
		if (cmd[2] > 2) id = cmd[4];
		if ((id) && (id <= config.maxServo()) && (rs.exists(id))) {
			uint16_t  adjAngle = rs.getAdjAngle(id);
			result[5] = adjAngle / 256;
			result[6] = adjAngle % 256;
		} else {
			result[5] = 0x7F;
			result[6] = 0x7F;
		}
	} else {
		result[4] = 0x00;
		result[5] = 0x7F;
		result[6] = 0x7F;
	}
	V2_SendResult(result);
}

void V2_SetAdjAngle(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_SetAdjAngle]");
	byte id = cmd[4];
	if (cmd[2] == 5) {
		byte id = cmd[4];
		uint16_t adjSet = (cmd[5] << 8) | cmd[6];
		if (debug) DEBUG.printf("Set angle: %d\n", adjSet);
		uint16_t adjResult = rs.setAdjAngle(id, adjSet);
		if (debug) DEBUG.printf("Result angle: %d\n", adjResult);
		V2_SendSingleByteResult(cmd, (adjSet == adjResult ? 0 : 2));
	} else {
		V2_SendSingleByteResult(cmd, 1);
	}
}

void V2_ServoCommand(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_ServoCommand]");
	byte result = 0;
	result = rs.servoCommand(cmd);
	V2_SendSingleByteResult(cmd, result);
}

void V2_SetAngle(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_SetAngle]");
	byte result = 0;
	result = rs.setAngle(cmd[4], cmd[5], cmd[6]);
	V2_SendSingleByteResult(cmd, result);
}


#pragma endregion

#pragma region V2_CMD_LOCKSERVO / V2_CMD_LOCKSERVO

void V2_LockServo(byte *cmd, bool goLock) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_LockServo - %s]\n", (goLock ? "Lock" : "Unlock"));

	byte result[40];  // max: A9 9A len cmd n {2 * n} {sum} ED - n <= 16 => max: 39
	result[3] = cmd[3];
	byte cnt = 0;
	if ((cmd[2] == 2) || ((cmd[2] == 3) && (cmd[4] == 0))) {
		// All servo
		for (int id = 1; id <= config.maxServo(); id++)  {
			byte pos = 5 + 2 * cnt;
			if (rs.exists(id)) {
				result[pos] = id;
				if (rs.isLocked(id) && goLock) {
					// prevent lock servo if already locked, and request to lock
					result[pos + 1] = rs.lastAngle(id);
				} else  {
					result[pos + 1] = rs.getAngle(id, goLock);	
				}
				cnt++;
			}
		}
		result[4] = cnt;
		if (debug) DEBUG.printf("All servo: %d\n",cnt);
	} else {
		int reqCnt = cmd[2] - 2;
		for (int i = 0; i < reqCnt; i++) {
			byte id = cmd[4 + i];
			if (id) {
				// check if already exists
				for (int j = 0; j < i; j++) {
					if (cmd[4 + j] == id) {
						if (debug) {
							DEBUG.printf("Duplicate ID: %d \n", id);
						}
						id = 0; // clear the id if already exists
						break;
					}
				}
			}
			if (id) {
				byte pos = 5 + 2 * cnt;
				if (rs.exists(id)) {
					result[pos] = id;
					if (rs.isLocked(id) && goLock) {
						// prevent lock servo if already locked, and request to lock
						result[pos + 1] = rs.lastAngle(id);
					} else  {
						result[pos + 1] = rs.getAngle(id, goLock);	
					}
					cnt++;
				}
			}
		}
		result[4] = cnt;
		if (debug) DEBUG.printf("Selected %d servos : %d\n", reqCnt, cnt);
	}
	result[2] = 3 +  result[4] * 2;
	V2_SendResult(result);
}

#pragma endregion

#pragma region  V2_CMD_SERVOMOVE

// A9 9A {len} 23 {id,angle,time_h, time_L}
void V2_ServoMove(byte *cmd) {
	byte moveAngle;
	uint16_t moveTime;
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_ServoMove]");
	int cnt = (cmd[2] - 2) / 4;
	byte moveParm[3 * config.maxServo()];
	memset(moveParm, 0xFF, 3 * config.maxServo());
	int pos;

	if ((cnt == 1) && (cmd[4] == 0)) {
		moveAngle = cmd[5];
		moveTime = (cmd[6] << 8) | cmd[7];
		for (byte id = 1; id <= config.maxServo(); id++ ) {
			if (rs.exists(id)) {
				pos = 3 * (id - 1);
				moveParm[pos] = moveAngle;
				moveParm[pos+1] = moveTime >> 8;
				moveParm[pos+2] = moveTime & 0xFF;
			}
		}
	} else {
		byte id;
		for (int i = 0; i < cnt; i++) {
			pos = 4 + 4 * i;
			id = cmd[pos];
			if ((id != 0) && rs.exists(id) && (moveParm[3*(id-1)] == 0xFF)) {
				moveAngle = cmd[pos+1];
				moveTime = (cmd[pos+2] << 8) | cmd[pos+3];
				pos = 3 * (id - 1);
				moveParm[pos] = moveAngle;
				moveParm[pos+1] = moveTime >> 8;
				moveParm[pos+2] = moveTime & 0xFF;
			} else {
				if (debug) {
					if ((id == 0) || !rs.exists(id)) {
						DEBUG.printf("Servo ID: %02d is invalid\n", id);
					} else if (moveParm[3*(id-1)] != 0xFF) {
						DEBUG.printf("Servo ID: %02d is duplicated\n", id);
					} else {
						DEBUG.printf("Unknwon error: ID: %d , %d, %02X\n", id, rs.exists(id), moveParm[3*(id-1)] );
					}
				}
			}
		}
	}
	
	if (debug) {
		DEBUG.println("move parameters:");
		for (int i=0; i < 3 * config.maxServo(); i++) {
			DEBUG.printf("%02X ", moveParm[i]);
		}
		DEBUG.println();
	}

	int moveCnt = 0;
	// byte result[55]; // max: A9 9A {len} {cmd} {cnt} (4 * 16) {sum} ED = 64 + 7 = 71
	int arraySize = 4 * config.maxServo() + 7;
	byte result[arraySize]; // max: A9 9A {len} {cmd} {cnt} (4 * 16) {sum} ED = 64 + 7 = 71
	for (int id = 1; id <= config.maxServo(); id++) {
		pos = 3 * (id - 1);
		if (moveParm[pos] != 0xFF) {
			int resultPos = 5 + 4 * moveCnt;
			result[resultPos] = id;
			moveAngle = moveParm[pos];
			uint16_t moveTime = (moveParm[pos+1] << 8) | moveParm[pos+2];
			result[resultPos+1] = moveAngle;
			result[resultPos+2] = moveTime >> 8;
			result[resultPos+3] = moveTime & 0xFF;
			if (debug) DEBUG.printf("Move servo %02d to %d [%02X], time: %d [%02X]\n", id, moveAngle, moveAngle, moveTime, moveTime);
			rs.goAngle(id, moveAngle, moveTime);
			moveCnt++;
		}
	}
	result[2] = 3 + 4 * moveCnt;
	result[3] = cmd[3];
	result[4] = moveCnt;
	V2_SendResult(result);
	
}

#pragma endregion


#pragma region V2_CMD_LED

void V2_SetLED(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_SetLED]");
	byte id, mode;
	if ((cmd[2] == 4) && (cmd[4] == 0)) {
		mode = (cmd[5] ? 1 : 0);
		for (int id = 1; id <= config.maxServo(); id++) {
			if (rs.exists(id)) {
				if (debug) DEBUG.printf("Turn servo %02d LED %s\n", id, (mode ? "OFF" : "ON"));
				rs.setLED(id, (!mode));
			} 
		}
	} else {
		int cnt = (cmd[2] - 2) / 2;
		for (int i = 0; i < cnt; i++) {
			int pos = 4 + 2 * i;
			id = cmd[pos];
			if (rs.exists(id)) {
				mode = (cmd[pos+1] ? 1 : 0);
				if (debug) DEBUG.printf("Turn servo %02d LED %s\n", id, (mode ? "OFF" : "ON"));
				rs.setLED(id, (!mode));
			}
		}
	}
	V2_SendSingleByteResult(cmd, 0);
}

#pragma endregion


#pragma region Set Head LED

void V2_SetHeadLED(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_SetHeadLED]");
	bool status = (cmd[4] == 0x01);
	SetHeadLed(status);
	if (debug) DEBUG.printf("Turn Head LED %s\n", (status ? "ON" : "OFF"));
	V2_SendSingleByteResult(cmd, 0);
}

#pragma endregion

#pragma region Event EVENT_HANDLER

void V2_GetEventHandlerMode(byte *cmd)	 {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_GetEventHandlerMode]");
	V2_SendSingleByteResult(cmd, (eventHandlerSuspended ? 0 : 1));
}

void V2_SetEventHandlerMode(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_SetEventHandlerMode(%d)]", cmd[4]);
	eventHandlerSuspended = (cmd[4] == 0);
	V2_SendSingleByteResult(cmd, 0);
}

#pragma endregion


#pragma region MP3 Player

void V2_Mp3Command(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_Mp3Command - %02X %02X %02X]", cmd[4], cmd[5], cmd[6]);
	mp3.begin();
	mp3.sendCommand(cmd[4],cmd[5],cmd[6]);
	mp3.end();
}

void V2_Mp3Stop(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_Mp3Stop]");
	if (config.mp3Enabled()) {
		mp3.begin();
		mp3.stop();
		mp3.end();
	}
	V2_SendSingleByteResult(cmd, 0);
}

void V2_Mp3PlayFile(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_Mp3PlayFile]");
	if (!config.mp3Enabled()) return;
	byte folderSeq = cmd[4];
	byte fileSeq = cmd[5];
	ActionMp3PlayFile(folderSeq, fileSeq);
	/*
	mp3.begin();
	if (folderSeq == 0xff) {
		mp3.playFile(fileSeq);
	} else {
		mp3.playFolderFile(folderSeq, fileSeq);
	}
	mp3.end();
	*/
	V2_SendSingleByteResult(cmd, 0);
}

void V2_Mp3PlayMp3(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_Mp3PlayMp3]");
	if (!config.mp3Enabled()) return;
	byte fileSeq = cmd[4];
	ActionMp3PlayMp3((uint16_t) fileSeq);
	/*
	mp3.begin();
	mp3.playMp3File(fileSeq);
	mp3.end();
	*/
	V2_SendSingleByteResult(cmd, 0);
}

void V2_Mp3PlayAdvert(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_Mp3PlayAdvert]");
	if (!config.mp3Enabled()) return;
	byte fileSeq = cmd[4];
	mp3.begin();
	mp3.playAdFile(fileSeq);
	mp3.end();
	V2_SendSingleByteResult(cmd, 0);
}

void V2_Mp3SetVolume(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_Mp3SetVolume]");
	if (!config.mp3Enabled()) return;
	byte mode = cmd[4];
	byte value = cmd[5];	
	int newVol = mp3_Vol;
	switch (mode) {
		case 1:
			newVol += value;
			break;
		case 2:
			newVol -= value;
			break;
		default:
			newVol = value;
			break;
	}
	if (newVol < 0) {
		newVol = 0;
	} else if (newVol > 30) {
		newVol = 30;
	} 
	mp3.begin();
	mp3.setVol((uint8_t) newVol);
	delay(1);
	mp3_Vol = mp3.getVol();
	mp3.end();
	V2_SendSingleByteResult(cmd, mp3_Vol);
}

#pragma endregion



#pragma region Play Action / Combo 

void V2_PlayAction(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_PlayAction]");
	V2_ResetAction();
	if ((cmd[2] != 3) && (cmd[2] != 4)) {
		V2_SendSingleByteResult(cmd, 1);
		return;
	}
	byte actionId = cmd[4];
	byte playCount = 1;
	if (cmd[2] == 4) {
		playCount = cmd[5];
	}
	V2_GoAction(actionId, playCount, true, cmd);
}


// Function for both V1 and V2 and it need to keep V1 Play command
void V2_GoAction(byte actionId, bool v2, byte *cmd) {
	// Default play only once
	if (debug) DEBUG.printf("[V2_GoAction(%d, %s, [])]\n", actionId, (v2 ? "true" : "false"));
	V2_GoAction(actionId, 1, v2, cmd);
}

void V2_GoAction(byte actionId, byte playCount, bool v2, byte *cmd) {
	if (debug) DEBUG.printf("[V2_GoAction(%d, %d, %s, [])]\n", actionId, playCount,  (v2 ? "true" : "false"));
	if (actionData.id() != actionId) {
		// need to load actionData
		if (debug) DEBUG.printf("Read action %d from SPIFFS\n", actionId);
		if (int error = actionData.ReadSPIFFS(actionId)) {
			if (debug) DEBUG.printf("Fail to get Id matched - %d\n", error);
			if (v2) V2_SendSingleByteResult(cmd, 2);
			return;
		}
	}

	// Load starting with pId = 0
	uint16_t pId = 0;
	if (!actionData.IsPoseReady(pId)) {
		if (v2) V2_SendSingleByteResult(cmd, 3);
		return;	
	}
	

	// Just for safety, check the poseCnt again.  
	// May skip this step if those information is always updated.
	//
	// -- In new version, it cannot be checked as only partial action is loaded in memory
	//
	// if (debug) DEBUG.printf("Refresh action %d from SPIFFS\n", actionId);
	// actionData.RefreshActionInfo();

	if (actionData.PoseCnt() > 0) {
		V2_ActionCombo = 0;
		V2_NextAction = actionId;
		V2_ActionPlayCount = playCount;
		V2_NextPose = 0;
		V2_GlobalTimeMs = millis();
		V2_NextPlayMs = V2_GlobalTimeMs;
		V2_ActionPlaying = true;
	}
	if (v2) V2_SendSingleByteResult(cmd, 0);
	if (debug) DEBUG.printf("Ready to play action %d with %d steps, %d time(s)\n", actionId, actionData.PoseCnt(), V2_ActionPlayCount);

}

void V2_PlayCombo(byte *cmd) {
	
}

void V2_SetActionSpeed(byte *cmd) {
	if (debug) DEBUG.printf("[V2_SetActionSpeed]\n");

	if (cmd[4] == 0) {
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_INVALID);
		return;
	}

	if (debug) DEBUG.printf("V2_SetActionSpeed - Set speed to %d\n", cmd[4]);
	actionTimeFactor = 100.0f / cmd[4];
	V2_SendSingleByteResult(cmd, 0);
}


#pragma endregion


#pragma region ActionTable

bool V2_CheckActionId(byte actionId) {

	if (debug) DEBUG.printf("V2_CheckActionId - Current Id: %d (%d); requested Id: %d\n", actionData.id(), actionData.Header()[4], actionId);
	if (actionId != actionData.id() ) {
		if (int error = actionData.ReadSPIFFS(actionId)) {
			// anything to do if still fail to read
			if (debug) DEBUG.printf("Fail to get Id matched - %d\n", error);
			return false;
		}
		if (debug) DEBUG.printf("V2_CheckActionId - Current Id changed to %d (%d)\n", actionData.id(), actionData.Header()[4]);
	} 
	return true;
}

void V2_GetAdList(byte *cmd) {
	if (debug) DEBUG.println("[V2_GetAdList]");
	int dataSize = 32;  // 8 * 32 = 256 bit-wise flag
	byte result[dataSize + 6];
	memset(result, 0, dataSize + 6);
	result[2] = dataSize + 2;
	result[3] = cmd[3];

	SPIFFS.begin();
	Dir dir;
	dir = SPIFFS.openDir(ACTION_PATH);

	while (dir.next()) {
		int id = getActionId(dir);
		if (id >= 0) {
			int h, l;
			h = (id / 8);
			l = 7 - (id % 8);
			result[4 + h] |= (1 << l);
		}
	}
	V2_SendResult(result);
}

int getActionId(Dir dir) {

	String fName = dir.fileName();


	if (!fName.endsWith(".dat"))  return -1;

	int seq = 0;
	for (int i = 0; i < 3; i++) {
		byte c = fName[ACTION_POS + i];
		if ((c < '0') || (c > '9')) {
			seq = -1;
			break;
		}
		seq = seq * 10 + (c - '0');
	}
	if ((seq < 0) || (seq > 255)) return -1;
		
	File f= dir.openFile("r");
	int size = f.size();
	f.close();

	if (size < AD_HEADER_SIZE) return -1;

	size -= AD_HEADER_SIZE;
	int tail = (size % AD_POSE_SIZE);
	if (tail) return -1;

	return seq;
}

void V2_GetAdHeader(byte *cmd) {
	if (debug) DEBUG.println("[V2_GetAdHeader]");
	byte aId = cmd[4];	
	if (!V2_CheckActionId(aId)) {
		V2_SendSingleByteResult(cmd, 0x01);
		return;	
	}
	actionData.Header()[3] = cmd[3];
	V2_SendResult((byte *) actionData.Header());
}


void V2_GetAdPose(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_GetAdPose]");

	#ifdef UBT_DUMP
		DEBUG.printf("\n\nV2_GetAdPose - A - Serial dump - first 60:\n");
		for (int i = 0; i < 60; i++) {
			DEBUG.printf("%02X ", actionData.Data()[i]);
		}
		DEBUG.printf("\n\n\n");
	#endif

	byte aId = cmd[4];
	if (!V2_CheckActionId(aId)) {
		V2_SendSingleByteResult(cmd, 0x01);
		return;	
	}
	uint16_t pId = (cmd[5] << 8) | cmd[6];
	uint16_t pOffset = 0;
	if (!actionData.IsPoseReady(pId, pOffset)) {
		V2_SendSingleByteResult(cmd, 0x02);
		return;	
	}

	#ifdef UBT_DUMP
		DEBUG.printf("\n\nV2_GetAdPose - B - Serial dump - first 60:\n");
		for (int i = 0; i < 60; i++) {
			DEBUG.printf("%02X ", actionData.Data()[i]);
		}
		DEBUG.printf("\n\n\n");
	#endif

	byte result[AD_POSE_SIZE];
	byte * fromPos = actionData.Data();

	fromPos += pOffset;
	memcpy(result, fromPos, AD_POSE_SIZE);
	result[2] = AD_POSE_SIZE - 4;
	result[3] = cmd[3];

	if (debug) {
		DEBUG.printf("Source Data at 0,0: \n");
		for (int i = 0; i < AD_POSE_SIZE; i++) {
			DEBUG.printf("%02X ", actionData.Data()[i]);
		}
		DEBUG.println();

		DEBUG.printf("V2_GetAdPose - Return Data: \n");
		for (int i = 0; i < AD_POSE_SIZE; i++) {
			DEBUG.printf("%02X ", result[i]);
		}
		DEBUG.println();
	}

	V2_SendResult(result);
}

void V2_UpdateAdHeader(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_UpdateAdHeader]");
	byte aId = cmd[4];	
	
	// Length should be header size - 4
	if (cmd[2] != (AD_HEADER_SIZE - 4)) {
		if (debug) DEBUG.printf("Invalid length: %d\n", cmd[2]);
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_SIZE);
		return;
	}

	// Action ID must be matched.  
	// i.e. must GET before UPDATE (Why?  user can load action from file, remove this checking)
	// Should always initilize before updating action header, it should rewrite the whole action.
	//
	// if (cmd[4] != actionData.Header()[4]) {
		if (debug) DEBUG.printf("InitialObject(%d)\n", cmd[4]);
		actionData.InitObject(cmd[4]);
	// }
	for (int i = 0; i < AD_HEADER_SIZE; i++) {
		actionData.Header()[i] = cmd[i];
	}
	
	byte result = actionData.WriteHeader();

	if (debug) DEBUG.printf("Action %d header updated - %d\n", aId, result);
	V2_SendSingleByteResult(cmd, result);
	
}


void V2_UpdateAdPose(byte *cmd) {
	
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_UpdateAdPose]");
	
	// Length should be {len} {actionId} {poseId} {data} => pose datasize + 3
	if (cmd[2] != (AD_POSE_SIZE - 4)) {
		if (debug) DEBUG.printf("Invalid length: %d\n", cmd[2]);
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_SIZE);
		return;
	}	
	
	byte aId = cmd[4];
	uint16_t pId = (cmd[AD_POFFSET_SEQ_HIGH] << 8) | cmd[AD_POFFSET_SEQ];

	// Action ID must be matched.  i.e. must GET before UPDATE
	if (aId != actionData.Header()[4]) {
		if (debug) DEBUG.printf("ID not matched: %d (current: %d)\n", aId, actionData.Header()[4]);
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_AID_NOT_MATCH);
		return;
	}

	int poseOffset = actionData.PoseOffset();
	int bufferEndPose = actionData.BufferEndPose();

	if (debug) DEBUG.printf("[V2_UpdateAdPose] - pId: %d, pCnt: %d, poffset: %d, AD_PBUFFER_COUNT %d, End: %d\n", pId, actionData.PoseCnt(), poseOffset, AD_PBUFFER_COUNT, bufferEndPose);

	if ((pId >= actionData.PoseCnt()) || (pId < poseOffset) || (pId > bufferEndPose)) {
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_PID_OUT_RANGE);
		return;
	}

	int startPos = AD_POSE_SIZE * (pId - poseOffset);
	for (int i = 0; i < AD_POSE_SIZE; i++) {
		actionData.Data()[startPos + i] = cmd[i];
	}

	byte result = RESULT::SUCCESS;

	if (debug) DEBUG.printf("[V2_UpdateAdPose] - Saved\n");


	if ((pId == (actionData.PoseCnt() - 1)) || (pId == bufferEndPose)) {
		result = actionData.WritePoseData();
	}

	V2_SendSingleByteResult(cmd, result);

	#ifdef UBT_DUMP
		DEBUG.printf("\n\nV2_UpdateAdPose - Serial dump - first 60:\n");
		for (int i = 0; i < 60; i++) {
			DEBUG.printf("%02X ", actionData.Data()[i]);
		}
		DEBUG.printf("\n\n\n");
	#endif
}



void V2_UpdateAdName(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_UpdateAdName]");
	byte id = cmd[4];
	if (actionData.id() != id) {
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_AID_NOT_MATCH);
		return;
	}
	if (cmd[5] > AD_NAME_SIZE) {
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_AD_NAME_SIZE);
		return;
	}
	byte * startPos = actionData.Header() + AD_OFFSET_NAME;
	memset(startPos, 0, AD_NAME_SIZE);
	byte len = cmd[5];

	// byte * copyPos = cmd + 6;
	if (debug) DEBUG.print("Action Name: ");
	for (int i = 0; i < len; i++) { 
		if (debug) DEBUG.print((char) cmd[6 + i]);
		actionData.Header()[AD_OFFSET_NAME + i] = cmd[6 + i];
	}
	if (debug) DEBUG.println();
	V2_SendSingleByteResult(cmd, RESULT::SUCCESS);
}

void V2_DeleteActionFile(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_DeleteActionFile]");
	byte id = cmd[4];
	byte result = actionData.DeleteActionFile(id);
	V2_SendSingleByteResult(cmd, result);
}

#pragma endregion


#pragma region Combo: V2_CMD_GET_COMBO / V2_CMD_UPD_COMBO

void V2_GetCombo(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_GetCombo]");
	byte seq = cmd[4];
	if (seq >= CD_MAX_COMBO) {
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_COMBO_OUT_RANGE);
		return;
	}
	comboTable[seq].Data()[3] = cmd[3];
	V2_SendResult(comboTable[seq].Data());

}

void V2_UpdateCombo(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_UpdateCombo]");

	V2_SendSingleByteResult(cmd, RESULT::SUCCESS);
}

#pragma endregion

#pragma region Combo: V2_CMD_CHK_MPU / V2_CMD_GET_MPU

void V2_CheckMPU(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_CheckMPU]");
	V2_SendSingleByteResult(cmd, (edsMpu6050[0]->IsAvailable() ? RESULT::SUCCESS : RESULT::ERR::NOT_FOUND));
}

void V2_GetMPUData(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_GetMPU]");
	byte result[EMPU_RESULT_SIZE];
	result[2] = EMPU_RESULT_SIZE - 4;
	result[3] = cmd[3];
	if (edsMpu6050[0]->GetMpuData()) {
		memcpy((byte *)(result + 4), edsMpu6050[0]->MpuBuffer(), EMPU_DATA_SIZE);
	}
	V2_SendResult(result);
}

#pragma endregion


#pragma region Event: V2_CMD_GET_EVENT_HEADER

void V2_GetEventHeader(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_GetEventHeader]");
	byte result[EVENT_HEADER_RESULT_SIZE];
	memset(result, 0, EVENT_HEADER_RESULT_SIZE);
	result[2] = EVENT_HEADER_RESULT_SIZE - 4;
	result[3] = cmd[3];
	result[EH_OFFSET_VERSION] = EVENT_HANDLER_VERSION;
	EventHandler *eh;
	if (cmd[EH_OFFSET_MODE]) {
		eh = &eBusy;
		result[EH_OFFSET_MODE] = 1;
	} else {
		eh = &eIdle;
		result[EH_OFFSET_MODE] = 0;
	}
	uint16_t count = eh->Count();
	result[EH_OFFSET_COUNT] = (count & 0xFF);  // Only support 255 events at this version
	result[EH_OFFSET_ACTION] = 0;
	V2_SendResult(result);
}

void V2_GetEventData(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_GetEventData]");
	byte result[EVENT_DATA_RESULT_SIZE];
	memset(result, 0, EVENT_DATA_RESULT_SIZE);
	result[2] = EVENT_DATA_RESULT_SIZE - 4;
	result[3] = cmd[3];
	EventHandler *eh;
	if (cmd[ED_OFFSET_MODE]) {
		 eh = &eBusy;
		 result[ED_OFFSET_MODE] = 0x01;
	 } else {
		 eh = &eIdle;
		 result[ED_OFFSET_MODE] = 0x00;
	 }
	EventHandler::EVENT* events;
	events = eh->Events();
	byte count = eh->Count();
	byte startIdx = cmd[ED_OFFSET_STARTIDX];
	byte sendCnt = 0;
	if (count > startIdx) {
		sendCnt = count - startIdx;
		sendCnt = (sendCnt > EVENT_DATA_BATCH_SIZE ? EVENT_DATA_BATCH_SIZE : sendCnt);
		byte *dest = (byte *) (result + 16);
		byte *source = (byte *) events[startIdx].buffer;
		memcpy(dest, source, 12 * sendCnt);
	}
	result[ED_OFFSET_STARTIDX] = startIdx;
	result[ED_OFFSET_COUNT] = sendCnt;
	V2_SendResult(result);
}

void V2_SaveEventHeader(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_SaveEventHeader]");
	byte result = RESULT::ERR::UNKNOWN;
	byte mode = cmd[EH_OFFSET_MODE];	
	byte version = cmd[EH_OFFSET_VERSION];	
	byte count = cmd[EH_OFFSET_COUNT];
	byte action = cmd[EH_OFFSET_ACTION];
	if (version == EVENT_HANDLER_VERSION) {
		switch (action) {
			case 1:
				// before sending data
				eTemp.Reset(count);
				result = RESULT::SUCCESS;
				break;
			case 2:
				result = SaveEventHandler(cmd);
				break;
			default:
				break;
		}
	} else {
		result = RESULT::ERR::VERSION_NOT_MATCH;
	}
	V2_SendSingleByteResult(cmd, result);
}

byte SaveEventHandler(byte *cmd) {
	byte mode = cmd[EH_OFFSET_MODE];	
	byte count = cmd[EH_OFFSET_COUNT];
	byte action = cmd[EH_OFFSET_ACTION];	
	// after data sent, update SPIFFS
	if (count != eTemp.Count()) return RESULT::ERR::NOT_MATCH;
	if (!eTemp.IsValid()) {
		DEBUG.println("eTemp->Not valid");
		return RESULT::ERR::NOT_READY;
	}
	EventHandler* eTarget;
	eTarget = (mode ? &eBusy : &eIdle);

	if (eTarget->Clone(&eTemp)) {
		if (eTarget->SaveData(mode ? EVENT_BUSY_FILE : EVENT_IDEL_FILE)) {
			return RESULT::SUCCESS;
		}
		return RESULT::ERR::WRITE;
	}
	return RESULT::ERR::COPY;
}

void V2_SaveEventData(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_SaveEventData]");
	byte result = RESULT::ERR::UNKNOWN;
	byte mode = cmd[ED_OFFSET_MODE];
	byte startIdx = cmd[ED_OFFSET_STARTIDX];
	byte count = cmd[ED_OFFSET_COUNT];
	if ((startIdx < eTemp.Count()) && (startIdx + count <= eTemp.Count())) {
		size_t dataCnt = 12 * count;
		byte *source = (byte *) (cmd + 16);
		EventHandler::EVENT* events = eTemp.Events();
		byte *dest = (byte *) events[startIdx].buffer;
		memcpy(dest, source, dataCnt);

		result = RESULT::SUCCESS;
	} else {
		result = RESULT::ERR::DATA_OVERFLOW;
	}
	V2_SendSingleByteResult(cmd, result);
}

void V2_USB_TTL(byte *cmd) {
	if (_dbg->require(110)) _dbg->log(110, 0, "[V2_USB_TTL]");
	byte mode = cmd[4];
	byte bus = cmd[5];
	if ((mode > 1) || (bus > 1)) {
		V2_SendSingleByteResult(cmd, RESULT::ERR::PARM_INVALID);
		return;
	}
	V2_SendSingleByteResult(cmd, RESULT::SUCCESS);
	switch (mode) {
		case 0:
			switch (bus) {
				case 0:
					if (_dbg->require(110)) _dbg->log(110, 0, "USER-TTL for robot");
					USER_TTL(&robotPort);
					return;
				case 1:
					if (_dbg->require(110)) _dbg->log(110, 0, "USER-TTL for Sub-System");
					USER_TTL(&ssbPort);
					return;
			}
		case 1:
			switch (bus) {
				case 0:
					if (_dbg->require(110)) _dbg->log(110, 0,"USB-TTL for robot");
					USB_TTL(&robotPort);
					return;
				case 1:
					if (_dbg->require(110)) _dbg->log(110, 0, "USB-TTL for Sub-System");
					USB_TTL(&ssbPort);
					return;
			}
	}
}

/*

Sample transaction for saving event data

A9 9A 0C 93 00 01 05 01 00 00 00 00 00 00 A6 ED

A9 9A 7C 94 00 00 05 00 00 00 00 00 00 00 00 00 00 02 04 00 00 03 90 01 00 00 00 00 00 01 01 00 02 03 C0 C7 01 0F 00 00 00 01 01 00 02 03 C0 C7 01 05 00 00 00 01 01 00 02 02 40 38 01 06 00 00 00 01 02 00 00 01 01 00 01 0B 00 00 00 01 02 00 00 01 02 00 01 0C 00 00 00 01 02 00 00 01 03 00 01 0D 00 00 00 01 02 00 00 01 04 00 01 0E 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 B6 ED 

A9 9A 0C 93 00 01 05 02 00 00 00 00 00 00 A7 ED

A9 9A 0C 93 01 01 05 01 00 00 00 00 00 00 A7 ED


*/

#pragma endregion


/* Commond action shared with EventHandler


*/

void ActionSetHeadLed(byte mode) {
	bool status = (mode == 0x01);
	SetHeadLed(status);	
}

void ActionPlayAction(byte actionId) {
	byte cmd[] = {0x9A, 0x9A, 0x03, 0x41, actionId , 0x00 ,0xED};
	V2_GoAction(actionId, false, cmd);
}

void ActionStopPlay() {
	V2_ResetAction();
	ActionMp3Stop();
}

void ActionMp3PlayMp3(uint16_t fileSeq) {
	mp3.begin();
	mp3.playMp3File(fileSeq);
	mp3.end();
}

void ActionMp3PlayFile(byte folderSeq, byte fileSeq) {
	mp3.begin();
	if (folderSeq == 0xff) {
		mp3.playFile(fileSeq);
	} else {
		mp3.playFolderFile(folderSeq, fileSeq);
	}
	mp3.end();
}

void ActionMp3Stop() {
	V2_ResetAction();
	if (config.mp3Enabled()) {
		mp3.begin();
		mp3.stop();
		mp3.end();
	}	
}

void ActionServo(uint8_t servoId, int8_t moveAngle, uint8_t time) {
	if (_dbg->require(210)) _dbg->log(210,0,"ActionServo(%d, %d, %d)", servoId, moveAngle, time);
	if (!rs.exists(servoId)) {
		if (_dbg->require(100)) _dbg->log(100,0,"Servo Id %d not exists", servoId);
		return;
	}
	int16_t currAngle = (int16_t) rs.getAngle(servoId);
	int16_t newAngle = currAngle + moveAngle;
	if (newAngle < 0) newAngle = 0;
	if (newAngle > rs.maxAngle()) newAngle = rs.maxAngle();
	if (currAngle == newAngle) return;	// no action required
	if (_dbg->require(100)) _dbg->log(100,0,"Servo Id %d move to %d in %d ms", servoId, newAngle, time);
	rs.goAngle(servoId, newAngle, time);
}

void ActionSonic(uint8_t status) {
	bool suspend = (status == 0);
	for (int id = 0; id < ED_COUNT_SONIC; id++) {
		edsSonic[id]->Suspend(suspend);
	}
	for (int id = 0; id < ED_COUNT_MAZE; id++) {
		edsMaze[id]->Suspend(suspend);
	}
}