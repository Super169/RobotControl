#include "ComboData.h"

ComboData::ComboData() {
	
}

ComboData::~ComboData() {
}


void ComboData::InitCombo(byte seq) {
#ifdef DEBUG_ComboData
	Serial1.printf("ComboData::InitObject(%d)\n", seq);
#endif	

	// To avoid duplicate memory required when saving record and simplify the action
	// use the same structe in memory & file
	memset(_data, 0, CD_COMBO_DATA_SIZE);
	_data[0] = 0xA9; // File identifier A9 9A
	_data[1] = 0x9A;
	_data[2] = CD_COMBO_DATA_SIZE - 4;
	_data[4] = seq;
	_data[CD_COMBO_DATA_SIZE - 1] = 0xED;
}