#include "ActionData.h"

// #define DEBUG_ActionData

ActionData::ActionData() {
	_name = (char *) (_header + AD_OFFSET_NAME);
}

ActionData::~ActionData() {
	_name = NULL;
}

void ActionData::InitObject(byte actionId) {
#ifdef DEBUG_ActionData
	Serial1.printf("ActionData::InitObject(%d)\n", actionId);
#endif	

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

byte ActionData::ReadSPIFFS(byte actionId) {
#ifdef DEBUG_ActionData
	Serial1.printf("ActionData::ReadSPIFFS(%d)\n", actionId);
#endif		
	return ReadActionFile(actionId);
}

byte ActionData::ReadActionFile(int actionId) {
#ifdef DEBUG_ActionData
	Serial1.printf("ActionData::ReadActionFile(%d)\n", actionId);
#endif		

	if (!SPIFFS.begin()) return false;
	bool success = SpiffsReadActionFile(actionId);
	SPIFFS.end();
	return success;
}

byte ActionData::SpiffsReadActionFile(int actionId) {
#ifdef DEBUG_ActionData
	Serial1.printf("ActionData::SpiffsReadActionFile(%d)\n", actionId);
#endif		

	byte result;
	char fileName[25];
	memset(fileName, 0, 25);
	sprintf(fileName, ACTION_FILE, actionId);
	if (!SPIFFS.exists(fileName)) {
		InitObject(actionId);
		return true;
	}
	result = ReadActionHeader(actionId);
	if (result == RESULT::SUCCESS) result = ReadActionPose();
	return result;

}

byte ActionData::ReadActionHeader(int actionId) {
#ifdef DEBUG_ActionData
	Serial1.printf("ActionData::ReadActionHeader(%d)\n", actionId);
#endif		
	if (!SPIFFS.begin()) return false;
	byte result = SpiffsReadActionHeader(actionId);
	SPIFFS.end();
#ifdef DEBUG_ActionData
	Serial1.printf("ActionData::ReadActionHeader -> %d\n", result);
#endif		
	return result;
}

byte ActionData::SpiffsReadActionHeader(int actionId) {
#ifdef DEBUG_ActionData
	Serial1.printf("ActionData::SpiffsReadActionHeader(%d)\n", actionId);
#endif		
	char fileName[25];
	memset(fileName, 0, 25);
	sprintf(fileName, ACTION_FILE, actionId);
	if (!SPIFFS.exists(fileName)) {
		return RESULT::ERR::FILE_NOT_FOUND;		// File must checked before calling the function
	}
	File f = SPIFFS.open(fileName, "r");
	if (!f) return RESULT::ERR::FILE_OPEN_READ;

	if (f.size() < AD_HEADER_SIZE) {
		return RESULT::ERR::FILE_SIZE;
	}
	int dataSize = f.size() - AD_HEADER_SIZE;
	if ((dataSize % AD_POSE_SIZE) != 0) {
		return RESULT::ERR::FILE_SIZE;
	} 
	uint16_t poseCnt = dataSize / AD_POSE_SIZE;
	byte poseCntHigh = (poseCnt >> 8);
	byte poseCntLow = (poseCnt & 0xFF);
	
	byte *buffer;
	buffer = (byte *) malloc(AD_HEADER_SIZE);
	size_t bCnt = f.readBytes((char *) buffer, AD_HEADER_SIZE);
	bool valid = (bCnt = AD_HEADER_SIZE);
	byte result = (valid ? RESULT::SUCCESS : RESULT::ERR::FILE_READ_COUNT);
	if (valid) {
		// Check start / end code / pose cnt
		valid = ((buffer[0] = 0xA9) && (buffer[1] = 0x9A) && 
		         (buffer[AD_HEADER_SIZE - 1] == 0xED) && 
				 (buffer[AD_OFFSET_POSECNT_LOW] == poseCntLow) &&
				 (buffer[AD_OFFSET_POSECNT_HIGH] == poseCntHigh)
				);
		if (!valid) {
			result = RESULT::ERR::AD_HEADER_CONTENT;
		
#ifdef DEBUG_ActionData
	Serial1.printf("\n*** Header content not matched:\n");
	for (int i = 0; i < AD_HEADER_SIZE; i++) {
		Serial1.printf("%02X ", buffer[i]);
	}
	Serial1.println();
	Serial1.printf("poseCnt: %d - H: %2X, L: %02X => %02X %2X\n", poseCnt, poseCntHigh, poseCntLow, buffer[AD_OFFSET_POSECNT_HIGH], buffer[AD_OFFSET_POSECNT_LOW]);
#endif		
			
		} 
	}
	if (valid) {
		// Check checksum
		byte sum = 0;
		byte sumPos = AD_HEADER_SIZE - 2;
		for (int i = 2; i < sumPos; i++) {
			sum += buffer[i];
		}
		valid = (sum == buffer[sumPos]);
		if (!valid) result = RESULT::ERR::AD_HEADER_CHECKSUM;
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

	return result;
}

byte ActionData::ReadActionPose() {	
#ifdef DEBUG_ActionData
	Serial1.printf("ActionData::ReadActionPose - aId: %d, _poseOffset: %d\n", _id, _poseOffset);
#endif		
	if (!SPIFFS.begin()) return false;
	byte result = SpiffsReadActionPose();
	SPIFFS.end();
#ifdef DEBUG_ActionData
	Serial1.printf("ActionData::ReadActionPose - SpiffsReadActionPose returnd: %d\n", result);
#endif
	return result;
}


byte ActionData::SpiffsReadActionPose() {	
#ifdef DEBUG_ActionData
	Serial1.printf("ActionData::SpiffsReadActionPose - aId: %d, _poseOffset: %d\n", _id, _poseOffset);
#endif		
	char fileName[25];
	memset(fileName, 0, 25);
	sprintf(fileName, ACTION_FILE, _id);
	if (!SPIFFS.exists(fileName)) {
		return RESULT::ERR::FILE_NOT_FOUND;		// File must checked before calling the function
	}
	File f = SPIFFS.open(fileName, "r");
	if (!f) return RESULT::ERR::FILE_OPEN_READ;
	// no more change on size here, should be confirmed in ReadActionHeader before

	uint32_t fileOffset = AD_HEADER_SIZE + _poseOffset * AD_POSE_SIZE;

	bool success = f.seek(fileOffset, SeekSet);
	if (!success) return RESULT::ERR::FILE_SEEK;


	uint16_t readPoseCnt = PoseCnt() - _poseOffset;
	if (readPoseCnt > AD_PBUFFER_COUNT) readPoseCnt = AD_PBUFFER_COUNT;

	memset(_data, 0, AD_PBUFFER_SIZE);
	size_t readPoseSize = readPoseCnt * AD_POSE_SIZE;
	size_t bCnt = f.readBytes((char *)_data, readPoseSize);

	f.close();
	return (bCnt == readPoseSize ? RESULT::SUCCESS : RESULT::ERR::FILE_READ_COUNT);

}

byte ActionData::WriteHeader() {
#ifdef DEBUG_ActionData
	Serial1.printf("ActionData::WriteHeader\n");
#endif		
	byte result = RESULT::SUCCESS;
	_header[0] = 0xA9;
	_header[1] = 0x9A;
	_header[AD_OFFSET_LEN] = AD_HEADER_SIZE - 4;
	_header[AD_OFFSET_COMMAND] = AD_COMMAND;
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

#ifdef DEBUG_ActionData
	Serial1.printf("Save to %s\n", fileName);
#endif	

	if (!SPIFFS.begin()) return RESULT::ERR::FILE_SPIFFS;
	File f = SPIFFS.open(fileName, "w");
	if (!f) {
		result = RESULT::ERR::FILE_OPEN_WRITE;
	} else {
		size_t hCnt = f.write((byte *) _header, AD_HEADER_SIZE);
		f.close();

#ifdef DEBUG_ActionData
		Serial1.printf("Byte saved: %d (expected : %d)\n", hCnt, AD_HEADER_SIZE);
#endif	

		result = (hCnt == AD_HEADER_SIZE ? 0 : RESULT::ERR::FILE_WRITE_COUNT);
	}
	SPIFFS.end();	
	return result;
	
}

// Result:
//   0 - OK
//   1 - SPIFFS error
//   1 - File not found
//   2 - File size not matched
//   3 - Fail opening file

byte ActionData::WritePoseData() {
#ifdef DEBUG_ActionData
	Serial1.printf("ActionData::WritePoseData: aId: %d ; pCnt: %d ; pOffset: %d\n", _id, PoseCnt(), _poseOffset);
#endif		
	if (!SPIFFS.begin()) return RESULT::ERR::FILE_SPIFFS;
	byte result = SpiffsWritePoseData();
	SPIFFS.end();
#ifdef DEBUG_ActionData
	Serial1.printf("ActionData::WritePoseData - return %d\n", result);
#endif		
	return result;
}

byte ActionData::SpiffsWritePoseData() {
#ifdef DEBUG_ActionData
	Serial1.printf("ActionData::SpiffsWritePoseData: aId: %d ; pCnt: %d ; pOffset: %d\n", _id, PoseCnt(), _poseOffset);
#endif		
	// Check if all pose ready
	int writePoseCnt = PoseCnt() - _poseOffset;
	if (writePoseCnt <= 0) return RESULT::SUCCESS;	
	if (writePoseCnt > AD_PBUFFER_COUNT) writePoseCnt = AD_PBUFFER_COUNT;

	// check if related pose is ready
	bool poseEmpty = false;
	for (int i = 0; i < writePoseCnt; i++) {
		int offset = i * AD_POSE_SIZE;
		if ((poseEmpty = (_data[offset] != 0xA9) || (_data[offset+1] != 0x9A) || (_data[offset+59] != 0xED))) break;
	}
	if (poseEmpty) return RESULT::ERR::AD_POSE_NOT_READY;

	char fileName[25];
	memset(fileName, 0, 25);
	sprintf(fileName, ACTION_FILE, _id);

	if (!SPIFFS.exists(fileName)) return RESULT::ERR::FILE_NOT_FOUND;

	File f = SPIFFS.open(fileName, "r");
	if (!f) return RESULT::ERR::FILE_OPEN_READ;
	int size = f.size();
	f.close();
	int expSize = AD_HEADER_SIZE + _poseOffset * AD_POSE_SIZE;
	if (size != expSize) return RESULT::ERR::FILE_SIZE;

	f = SPIFFS.open(fileName, "a");
	if (!f) return RESULT::ERR::FILE_OPEN_APPEND;

	int dataSize = writePoseCnt * AD_POSE_SIZE;
	int wCnt = f.write((byte *) _data, dataSize);

	f.close();

	// don't worrry even _poseOffset > PoseCnt, it just force to read poseData when use as required pose is not in range
	_poseOffset += AD_PBUFFER_COUNT;

	return (wCnt == dataSize ? RESULT::SUCCESS : RESULT::ERR::FILE_WRITE_COUNT);
}

byte ActionData::WriteSPIFFS() {

#ifdef DEBUG_ActionData
	Serial1.printf("ActionData.WriteSPIFFS: %d\n", _id);
#endif	

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

// Pose ready means: a valid pose, and pose already in memory, if not load it
//   - Detect for in memory: related batch in memory, i.e. current _poseOffet is first one in batch
// Optional: check if pose ready loaded - not implemented
bool ActionData::IsPoseReady(uint16_t poseId, uint16_t &offset) {
#ifdef DEBUG_ActionData
	Serial1.printf("ActionData.IsPoseReady(%d, --) - %d\n", poseId, _poseOffset);
#endif	

	if (poseId >= PoseCnt()) return false;
	uint16_t batch = (poseId / AD_PBUFFER_COUNT);
	uint16_t expOffset = batch * AD_PBUFFER_COUNT;
	if (_poseOffset != expOffset) {
		_poseOffset = expOffset;
		#ifdef DEBUG_ActionData
			Serial1.printf("ActionData.IsPoseReady - _poseOffset changed to %d\n", _poseOffset);
		#endif	

		if (ReadActionPose()) return false;
	}
	// Just for safety, should be in range
	if (poseId < _poseOffset) return false;
	if (poseId >= (_poseOffset + AD_PBUFFER_COUNT)) return false;

	offset = (poseId - _poseOffset) * AD_POSE_SIZE;
	#ifdef DEBUG_ActionData
		Serial1.printf("ActionData.IsPoseReady(%d, --) - %d\n", poseId, _poseOffset);
	#endif	
	
	return true;
}

bool ActionData::IsPoseReady(uint16_t poseId) {
	uint16_t dummy;
	return IsPoseReady(poseId, dummy);
}

byte ActionData::DeleteActionFile(byte actionId) {
#ifdef DEBUG_ActionData
	Serial1.printf("ActionData.DeleteActionFile(%d)\n", actionId);
#endif		
	char fileName[25];
	memset(fileName, 0, 25);
	sprintf(fileName, ACTION_FILE, actionId);
	byte result = 0xFF;
	SPIFFS.begin();
	if (SPIFFS.exists(fileName)) {
		if (SPIFFS.remove(fileName)) {
			result = RESULT::SUCCESS;
		} else {
			result = RESULT::ERR::FILE_REMOVE;
		}
	} else {
		result = RESULT::ERR::FILE_NOT_FOUND;
	}
	SPIFFS.end();
	if ( (result == RESULT::SUCCESS) && (_id == actionId)) {
		InitObject(0);
	}

#ifdef DEBUG_ActionData
	Serial1.printf("ActionData.DeleteActionFile(%d) -> %d\n", actionId, result);
#endif	
	return result;	
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