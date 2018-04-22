#include "ActionData.h"

ActionData::ActionData() {
	_name = (char *) (_header + AD_OFFSET_NAME);
}

ActionData::~ActionData() {
	_name = NULL;
}

void ActionData::InitObject(byte actionId) {
	// To avoid duplicate memory required when saving record and simplify the action
	// use the same structe in memory & file
	memset(_header, 0, AD_HEADER_SIZE);
	memset(_data, 0, AD_DATA_SIZE);
	_header[0] = 0xA9; // File identifier A9 9A
	_header[1] = 0x9A;
	_header[2] = AD_HEADER_SIZE - 4;
	_id = actionId;
	SetActionName(NULL, 0);
	_header[AD_HEADER_SIZE] = 0xED;
}

bool ActionData::ReadSPIFFS(byte actionId) {
	bool success = false;
	SPIFFS.begin();
	success = ReadActionFile(actionId);
	SPIFFS.end();
	return success;
}

bool ActionData::ReadActionFile(int actionId) {
	char fileName[25];
	memset(fileName, 0, 25);
	sprintf(fileName, ACTION_FILE, actionId);
// Serial.printf("File name: %s\n", fileName);
	if (!SPIFFS.exists(fileName)) {
		// Start a new action table 
// Serial.printf("Initialize object\n");
		InitObject(actionId);
		return true;
	}
	File f = SPIFFS.open(fileName, "r");
	if (!f) return false;

	if (f.size() < AD_HEADER_SIZE) {
// Serial.printf("File size %d, less than %d bytes.\n", f.size(), AD_HEADER_SIZE);
		return false;
	}
	int dataSize = f.size() - AD_HEADER_SIZE;
	if ((dataSize % AD_POSE_SIZE) != 0) {
		return false;
	} 
	int poseCnt = dataSize / AD_POSE_SIZE;
	
// Serial.printf("\nFile size: %d, with %d poses\n", f.size(), poseCnt);

	byte *buffer;
	buffer = (byte *) malloc(AD_HEADER_SIZE);
	size_t bCnt = f.readBytes((char *) buffer, AD_HEADER_SIZE);
	bool valid = (bCnt = AD_HEADER_SIZE);
	if (valid) {
		// Check start / end code / pose cnt
		valid = ((buffer[0] = 0xA9) && (buffer[1] = 0x9A) && 
		         (buffer[AD_HEADER_SIZE - 1] == 0xED) && 
				 (buffer[AD_OFFSET_POSECNT] == poseCnt)
				);
	}
	if (valid) {
		// Check checksum
		byte sum = 0;
		byte sumPos = AD_HEADER_SIZE - 2;
		for (int i = 2; i < sumPos; i++) {
			sum += buffer[i];
		}
		valid = (sum == buffer[sumPos]);
	}
	if (valid) {
		// valid means:
		//   1) file size = AD_HEADER_SIZE + n * AD_POSE_SIZE 
		//   2) header has start/end code, header, pose count and checksum matched
		InitObject(actionId);
		memcpy(_header, buffer, AD_HEADER_SIZE);
		// To allow swap action file faster, no checking on ID, and force assign ID here
		_header[AD_OFFSET_ID] = actionId;
		if (dataSize) bCnt = f.readBytes((char *)_data, dataSize);
		// As _header already overwrite, no checking on data
		// Or for safety, it should InitObject again if any error in data.
	}
	free(buffer);
	f.close();

	return valid;
}

byte ActionData::WriteSPIFFS() {
	byte result = 0;
	_header[0] = 0xA9;
	_header[1] = 0x9A;
	_header[AD_OFFSET_LEN] = AD_HEADER_SIZE - 4;
	_header[AD_OFFSET_ID] = _id;
	byte sum = 0;
	byte sumPos = AD_HEADER_SIZE -2;
	for (int i = 2; i < sumPos; i++) sum += _header[i];
	_header[sumPos] = sum;
	_header[AD_HEADER_SIZE - 1] = 0xED;

	size_t dataSize = _header[AD_OFFSET_POSECNT] * AD_POSE_SIZE;
	char fileName[25];
	memset(fileName, 0, 25);
	sprintf(fileName, ACTION_FILE, _id);

	SPIFFS.begin();
	File f = SPIFFS.open(fileName, "w");
	if (!f) {
		result = 1;
	} else {
		size_t hCnt = f.write((byte *) _header, AD_HEADER_SIZE);
		size_t dCnt = f.write((byte *) _data, dataSize);
		f.close();

		if ((hCnt == AD_HEADER_SIZE) && (dCnt == dataSize)) {
			result = 0;
		} else {
			result = 2;
		}
	}
	SPIFFS.end();	
	return result;
}

byte ActionData::DeleteSPIFFS(byte actionId) {
	char fileName[25];
	memset(fileName, 0, 25);
	sprintf(fileName, ACTION_FILE, actionId);
	byte result = 0xFF;
	SPIFFS.begin();
	if (SPIFFS.exists(fileName)) {
		if (SPIFFS.remove(fileName)) {
			result = 0;
		} else {
			result = 2;
		}
	} else {
		result = 1;
	}
	SPIFFS.end();
	return result;
}

bool ActionData::SetActionName(char *actionName, byte len) {
	if (len > 20) return false;
	byte *ptr = _header + AD_OFFSET_NAME;
	memset(ptr, 0, 20);

	if ((actionName == NULL) || (len == 0)) {
		// reset to default name Annn
		_header[AD_OFFSET_NAME] = 'A';
		_header[AD_OFFSET_NAME+1] = '0' + (_id / 100);
		_header[AD_OFFSET_NAME+2] = '0' + ((_id % 100) / 10);
		_header[AD_OFFSET_NAME+3] = '0' + (_id % 10);
		return true;
	}
	memcpy(ptr, actionName, len);
	return true;
}

bool ActionData::SetActionName(String actionName) {
	byte *ptr = _header + AD_OFFSET_NAME;
	memset(ptr, 0, 20);

	if (actionName == NULL) {
		// reset to default name Annn
		_header[AD_OFFSET_NAME] = 'A';
		_header[AD_OFFSET_NAME+1] = '0' + (_id / 100);
		_header[AD_OFFSET_NAME+2] = '0' + ((_id % 100) / 10);
		_header[AD_OFFSET_NAME+3] = '0' + (_id % 10);
		return true;
	}
	actionName.toCharArray((char *) ptr, 20);
	return true;
}


byte ActionData::UpdatePose(byte actionId, byte poseId, byte *data) {
	if (actionId != _id) return 1;
	byte *ptr = _data + poseId * AD_POSE_SIZE;
	memcpy(ptr, data, AD_POSE_SIZE);
	*ptr = poseId;
}

void ActionData::RefreshActionInfo() {
	int pCnt = -1;
	uint16_t servos;
	for (int i = 0; i < AD_MAX_POSE; i++) {
		int pos = i * AD_POSE_SIZE;
		// Empty criteria:
		//   - Execution time = 0
		//   - Wait time = 0
		if ((_data[pos + AD_POFFSET_STIME] == 0) && (_data[pos + AD_POFFSET_STIME + 1] == 0) && 
			(_data[pos + AD_POFFSET_WTIME] == 0) && (_data[pos + AD_POFFSET_WTIME + 1] == 0))
		{
			pCnt = i;
			break;	
		}
		// no need to check if all servo used
		if (servos != 0xFFFF) {
			int aPos = pos + AD_POFFSET_ANGLE;
			for (int i = 0; i < 16; i++) {
				if (_data[aPos] < 0xF0) {
					servos |= 1 << i;
				}
			}
		}
	}
	// MAX_POSE is 255
	if (pCnt == -1) pCnt = AD_MAX_POSE;
    _header[AD_OFFSET_POSECNT] = pCnt;
	_header[AD_OFFSET_AFFECTSERVO] = (byte) (servos / 256);
	_header[AD_OFFSET_AFFECTSERVO + 1] = (byte) (servos % 256);
}


// -------------------------------------------------------------------------------------------------
//

byte sampleHeader[] = { 0xA9, 0x9A, 0x38, 0x61, 0x00, 0x00,
                        0x53, 0x75, 0x70, 0x65, 0x72, 0x20, 0x41, 0x6C, 0x70, 0x68, 
						0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
						0x00, 0x00,
						0x01, 0x00,
						0x00, 0x00, 0x03, 0xE8,
						0xFF, 0xFF, 0x00, 0x00,
						0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
						0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
						0x00, 0xED
					  };

byte sampleData[] = { 0xA9, 0x9A, 0x38, 0x62, 0x01, 0x00, 0x01,
                      0x03, 0xE8,
					  0x03, 0xE8,
					  0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x3C, 0x4C, 0x6E, 0x5A, 0x5A, 0x78, 0x68, 0x46, 0x5A,
					  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
					  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
					  0x00, 
					  0xFF, 0xFF,
					  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
					};

void ActionData::GenSample(byte actionId)
{
	InitObject(actionId);
	memcpy(_header, sampleHeader, AD_HEADER_SIZE);
	_header[AD_OFFSET_ID] = actionId;
	byte poseCnt = sizeof(sampleData) / AD_POSE_SIZE;
	_header[AD_OFFSET_POSECNT] = poseCnt;
	memcpy(_data, sampleData, poseCnt * AD_POSE_SIZE);
	for (int i = 0; i < poseCnt; i++) {
		int pos = i * AD_DATA_SIZE + AD_OFFSET_ID;
		_data[pos] = actionId;
	}
	_id = actionId;
}