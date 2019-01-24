#include "EdsSonic.h"

EdsSonic::EdsSonic(EventData *data, MyDebugger *dbg, byte devId) {
    _Device = (uint8_t) EventData::DEVICE::sonic;
    Config(data, dbg, devId);
    _isAvailable = false;
}

EdsSonic::~EdsSonic() {
}

/*
void EdsSonic::Initialize(EventData *data) {
    _data = data;
}
*/

/*
*       Ping:
*          Send:    A8 8A 04 02 00 01 07 ED
*          Return:  A8 8A 05 02 00 01 00 08 ED
*/

void EdsSonic::Setup(SSBoard *ssb, unsigned long continueCheckms, unsigned long delayCheckMs ) {
     if (_dbg->require(110)) _dbg->log(110, 0, "EdsSonic::Setup(*ssb)");
    _ssb = ssb;
    // Check if device available

    Ping();
    if (_isAvailable) {
        if (_dbg->require(10)) _dbg->log(10, 0, "Sonic Sensor detected");
    } else {
        if (_dbg->require(10)) _dbg->log(10, 0, "Sonic Sensor not available");
    }
    _data->SetThreadhold(_Device, 0);
    _pendingCheckMs = 0;
    _continueCheckMs = continueCheckms;
    _delayCheckMs = delayCheckMs;
}

bool EdsSonic::Ping() {
    byte cmd[] = {0xA8, 0x8A, 0x04, 0x02, 0x00, 0x01, 0x07, 0xED};
    cmd[4] = _DevId;

    byte tryCnt = 0;
    while (tryCnt++ < 5) {
        _isAvailable = _ssb->SendCommand((byte *) cmd, true);
        if (_isAvailable) break;
    }
    return (_isAvailable);
}

/*
*       Get Data:
*          Send:    A8 8A 06 02 00 02 28 02 34 ED	
*          Return:  A8 8A 08 02 00 02 28 02 00 B4 EA ED （B4，180cm）
*/

bool EdsSonic::GetData() {
    _thisDataReady = false;
    _thisDataError = false;

    if (!IsReady()) return false;

    byte cmd[] = { 0xA8, 0x8A, 0x06, 0x02, 0x00, 0x02, 0x28, 0x02, 0x34, 0xED };
    cmd[4] = _DevId;

    unsigned long startMs = millis();
    if (!_ssb->SendCommand((byte *) cmd, true)) {
        _thisDataError = true;
        return false;
    }

    
    unsigned long diff = millis() - startMs;
    //_dbg->msg("It taks %d ms to read PSX", diff); 

    Buffer *result = _ssb->ReturnBuffer();
    // Data returned: A8 8A 08 02 00 02 28 02 00 {*} EA ED

    uint16 data = result->peek(8) << 8 | result->peek(9);
    _data->SetData(_Device, _DevId, 0, data);
    if ((_dbg->require(100))) _dbg->log(100,0,"Sonic: [%d,%d,%d] => %d", _Device, _DevId, 0, data);
    _thisDataReady = true;

    return true;
}

