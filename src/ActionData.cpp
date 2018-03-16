#include "ActionData.h"

ActionData::ActionData() {
	_header = (byte *) malloc(AD_HEADER_SIZE);
	_data = (byte *) malloc(AD_DATA_SIZE + 6);
	_name = (char *) (_header + AD_OFFSET_NAME);
}


ActionData::~ActionData() {
	free(_header);
	free(_data);
	_header = NULL;
	_data = NULL;
	_name = NULL;
}

void ActionData::InitObject(byte actionId) {
	memset(_header, 0, AD_HEADER_SIZE);
	memset(_data, 0, AD_DATA_SIZE);
	_header[0] = 0xA9; // File identifier A9 9A
	_header[1] = 0x9A;
	_header[2] = 124;
	_header[127] = 0xED;
	_id = actionId;
	_header[AD_OFFSET_ID] = _id;
}


bool ActionData::ReadSPIFFS(byte actionId) {
	SPIFFS.begin();
	SPIFFS.end();
	return true;
}

byte ActionData::WriteSPIFFS() {
	byte result = 0;
	_header[0] = 0xA9;
	_header[1] = 0x9A;
	_header[AD_OFFSET_LEN] = 124;
	_header[AD_OFFSET_ID] = _id;
	byte sum = 0;
	for (int i = 2; i < 126; i++) sum += _header[i];
	_header[126] = sum;
	_header[127] = 0xED;

	size_t dataSize = _header[48] * AD_POSE_SIZE;
	char fileName[25];
	memset(fileName, 0, 25);
	sprintf(fileName, "/alpha/action/%03d.dat", _id);
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