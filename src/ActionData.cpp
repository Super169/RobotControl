#include "ActionData.h"

ActionData::ActionData() {
	_name = (char *) (_header + AD_OFFSET_NAME);
}

ActionData::~ActionData() {
	_name = NULL;
}

void ActionData::InitObject(byte actionId) {

Serial1.printf("InitObject(%d)\n\n", actionId);

	// To avoid duplicate memory required when saving record and simplify the action
	// use the same structe in memory & file
	memset(_header, 0, AD_HEADER_SIZE);
	memset(_data, 0, AD_PBUFFER_SIZE);
	_header[0] = 0xA9; // File identifier A9 9A
	_header[1] = 0x9A;
	_header[2] = AD_HEADER_SIZE - 4;
	_id = actionId;
	SetActionName(NULL, 0);
	_header[AD_HEADER_SIZE - 1] = 0xED;
	_header[AD_OFFSET_POSECNT_LOW] = 0;
	_header[AD_OFFSET_POSECNT_HIGH] = 0;
	_poseOffset = 0;
}

bool ActionData::ReadSPIFFS(byte actionId) {
	return ReadActionFile(actionId);
}

/*
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
*/

bool ActionData::ReadActionFile(int actionId) {

	if (!SPIFFS.begin()) return false;
	bool success = SpiffsReadActionFile(actionId);
	SPIFFS.end();
	return success;
}

bool ActionData::SpiffsReadActionFile(int actionId) {

	bool success;
	char fileName[25];
	memset(fileName, 0, 25);
	sprintf(fileName, ACTION_FILE, actionId);
	if (!SPIFFS.exists(fileName)) {
		InitObject(actionId);
		return true;
	}
	success = ReadActionHeader(actionId);
	if (success) success = ReadActionHeader(actionId);
	return ReadActionPose();

}

bool ActionData::ReadActionHeader(int actionId) {

	if (!SPIFFS.begin()) return false;
	bool success = SpiffsReadActionHeader(actionId);
	SPIFFS.end();
	return success;
}

bool ActionData::SpiffsReadActionHeader(int actionId) {
	char fileName[25];
	memset(fileName, 0, 25);
	sprintf(fileName, ACTION_FILE, actionId);
	if (!SPIFFS.exists(fileName)) {
		return false;		// File must checked before calling the function
	}
	File f = SPIFFS.open(fileName, "r");
	if (!f) return false;

	if (f.size() < AD_HEADER_SIZE) {
		return false;
	}
	int dataSize = f.size() - AD_HEADER_SIZE;
	if ((dataSize % AD_POSE_SIZE) != 0) {
		return false;
	} 
	uint16_t poseCnt = dataSize / AD_POSE_SIZE;
	byte poseCntHigh = (poseCnt >> 8);
	byte poseCntLow = (poseCnt & 0xFF);
	
	byte *buffer;
	buffer = (byte *) malloc(AD_HEADER_SIZE);
	size_t bCnt = f.readBytes((char *) buffer, AD_HEADER_SIZE);
	bool valid = (bCnt = AD_HEADER_SIZE);
	if (valid) {
		// Check start / end code / pose cnt
		valid = ((buffer[0] = 0xA9) && (buffer[1] = 0x9A) && 
		         (buffer[AD_HEADER_SIZE - 1] == 0xED) && 
				 (buffer[AD_OFFSET_POSECNT_LOW] == poseCntLow) &&
				 (buffer[AD_OFFSET_POSECNT_HIGH] == poseCntHigh)
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
		
		// May read first batch of pose here, but for consistence, close the file and read using ReadActionPose.

	}
	free(buffer);
	f.close();

	return valid;
}

bool ActionData::ReadActionPose() {	
	if (!SPIFFS.begin()) return false;
	bool success = SpiffsReadActionPose();
	SPIFFS.end();
	return success;
}


bool ActionData::SpiffsReadActionPose() {	
	char fileName[25];
	memset(fileName, 0, 25);
	sprintf(fileName, ACTION_FILE, _id);
	if (!SPIFFS.exists(fileName)) {
		return false;		// File must checked before calling the function
	}
	File f = SPIFFS.open(fileName, "r");
	if (!f) return false;
	// no more change on size here, should be confirmed in ReadActionHeader before

	uint32_t fileOffset = AD_HEADER_SIZE + _poseOffset * AD_POSE_SIZE;

	bool success = f.seek(fileOffset, SeekSet);
	if (!success) return false;



	uint16_t readPoseCnt = PoseCnt() - _poseOffset;
	if (readPoseCnt > AD_PBUFFER_COUNT) readPoseCnt = AD_PBUFFER_COUNT;

	memset(_data, 0, AD_PBUFFER_SIZE);
	size_t readPoseSize = readPoseCnt * AD_POSE_SIZE;
	size_t bCnt = f.readBytes((char *)_data, readPoseSize);

	f.close();
	return (bCnt == readPoseSize);

}



byte ActionData::WriteSPIFFS() {

Serial1.printf("ActionData.WriteSPIFFS: %d\n", _id);

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

	size_t dataSize = _header[AD_OFFSET_POSECNT_LOW] * AD_POSE_SIZE;
	char fileName[25];
	memset(fileName, 0, 25);
	sprintf(fileName, ACTION_FILE, _id);

Serial1.printf("Save to %s\n", fileName);

	SPIFFS.begin();
	File f = SPIFFS.open(fileName, "w");
	if (!f) {
		result = 1;
	} else {
		size_t hCnt = f.write((byte *) _header, AD_HEADER_SIZE);
		size_t dCnt = f.write((byte *) _data, dataSize);
		f.close();

		Serial1.printf("Byte saved: %d : %d : %d\n", hCnt, dCnt, dataSize);

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

	return 0;
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
	_header[AD_OFFSET_POSECNT_HIGH] = pCnt >> 8;
	_header[AD_OFFSET_POSECNT_LOW] = pCnt & 0xFF;
	_header[AD_OFFSET_AFFECTSERVO] = (byte) (servos / 256);
	_header[AD_OFFSET_AFFECTSERVO + 1] = (byte) (servos % 256);
}

// Pose ready means: a valid pose, and pose already in memory, if not load it
//   - Detect for in memory: related batch in memory, i.e. current _poseOffet is first one in batch
// Optional: check if pose ready loaded - not implemented
bool ActionData::IsPoseReady(uint16_t poseId, uint16_t &offset) {

	if (poseId >= PoseCnt()) return false;
	uint16_t batch = (poseId / AD_PBUFFER_COUNT);
	uint16_t expOffset = batch * AD_PBUFFER_COUNT;
	if (_poseOffset != expOffset) {
		_poseOffset = expOffset;
		if (!ReadActionPose()) return false;
	}
	// Just for safety, should be in range
	if (poseId < _poseOffset) return false;
	if (poseId >= (_poseOffset + AD_PBUFFER_COUNT)) return false;

	offset = (poseId - _poseOffset) * AD_POSE_SIZE;
	return true;
}


/*

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
					  0xFF, 0xFF, 0xFF,
					  0x00, 0x00, 0x00, 0x00, 0x00
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


*/