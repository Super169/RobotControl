#include "ComboData.h"

ComboData::ComboData() {
	
}

ComboData::~ComboData() {
}


void ComboData::InitCombo(byte seq) {
#ifdef DEBUG_ComboData
	Serial1.printf("ComboData::InitObject(%d)\n", seq);
#endif	
	_seq = seq;
	// To avoid duplicate memory required when saving record and simplify the action
	// use the same structe in memory & file
	memset(_data, 0, CD_COMBO_DATA_SIZE);
	_data[0] = 0xA9; // File identifier A9 9A
	_data[1] = 0x9A;
	_data[2] = CD_COMBO_DATA_SIZE - 4;
	_data[4] = seq;
	_data[CD_COMBO_DATA_SIZE - 1] = 0xED;
}

byte ComboData::ReadSPIFFS(byte seq) {
#ifdef DEBUG_ComboData
	Serial1.printf("ComboData::ReadSPIFFS(%d)\n", seq);
#endif		
	if (!SPIFFS.begin()) return RESULT::ERR::SPIFFS;
	byte success = SpiffsReadComboFile(seq);
	SPIFFS.end();
	return success;
}

byte ComboData::SpiffsReadComboFile(byte seq) {
#ifdef DEBUG_ComboData
	Serial1.printf("ComboData::SpiffsReadActionHeader(%d)\n", seq);
#endif		
	InitCombo(seq);

	char fileName[25];
	memset(fileName, 0, 25);
	sprintf(fileName, COMBO_FILE, seq);
	if (!SPIFFS.exists(fileName)) {
		return RESULT::ERR::FILE_NOT_FOUND;		// File must checked before calling the function
	}
	
	File f = SPIFFS.open(fileName, "r");
	if (!f) return RESULT::ERR::FILE_OPEN_READ;

	if (f.size() < CD_COMBO_DATA_SIZE) {
		return RESULT::ERR::FILE_SIZE;
	}

	byte *buffer;
	buffer = (byte *) malloc(CD_COMBO_DATA_SIZE);
	size_t bCnt = f.readBytes((char *) buffer, CD_COMBO_DATA_SIZE);
	byte result = RESULT::ERR::FILE_SIZE;

	if (bCnt == CD_COMBO_DATA_SIZE) {
		memcpy(_data, buffer, CD_COMBO_DATA_SIZE);
		result = RESULT::SUCCESS;
	}

	free(buffer);
	f.close();
	return result;
}
