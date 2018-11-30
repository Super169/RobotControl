#include "EdsMpu6050.h"



EdsMpu6050::EdsMpu6050(EventData *data, MyDebugger *dbg, byte devId) {
    Config(data, dbg, devId);
}

EdsMpu6050::~EdsMpu6050() {
}

void EdsMpu6050::Setup(uint8_t i2cAddr) {
    _i2cAddr = i2cAddr;

    _dbg->enableDebug(true); 
}


void EdsMpu6050::GetData() {

}

/*
void EdsMpu6050::PostHandler(bool eventMatched, bool isRelated) {
    
}
*/