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
	_header[AD_OFFSET_ID] = _id;
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
Serial1.printf("File name: %s\n", fileName);
	if (!SPIFFS.exists(fileName)) {
		// Start a new action table 
Serial1.printf("Initialize object\n");
		InitObject(actionId);
		return true;
	}
	File f = SPIFFS.open(fileName, "r");
	if (!f) return false;

	if (f.size() <= AD_HEADER_SIZE) return false;
	int dataSize = f.size() - AD_HEADER_SIZE;
	if ((dataSize % AD_POSE_SIZE) != 0) return false;
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
		bCnt = f.readBytes((char *)_data, dataSize);
		// As _header already overwrite, no checking on data
		// Or for safety, it should InitObject again if any error in data.
	}
	f.close();
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
	Serial.println("Go print file name:\n");
	memset(fileName, 0, 25);
	sprintf(fileName, ACTION_FILE, _id);
	Serial.println(fileName);

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

void ActionData::RefreshPoseCnt() {
	int pCnt = -1;
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
	}
	// MAX_POSE is 255
	if (pCnt == -1) pCnt = AD_MAX_POSE;
    _header[AD_OFFSET_POSECNT] = pCnt;
}