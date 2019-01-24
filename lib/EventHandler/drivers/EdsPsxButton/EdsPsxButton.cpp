#include "EdsPsxButton.h"

EdsPsxButton::EdsPsxButton(EventData *data, MyDebugger *dbg, byte devId) {
    _Device = (uint8_t) EventData::DEVICE::psx_button;
    Config(data, dbg, devId);
    _isAvailable = false;
}

EdsPsxButton::~EdsPsxButton() {
}

/*
void EdsPsxButton::Initialize(EventData *data) {
    _data = data;
}
*/

void EdsPsxButton::Setup(SSBoard *ssb, uint8_t normalCheckMs, uint8_t noEventMs, uint16_t ignoreRepeatMs) {
     if (_dbg->require(110)) _dbg->log(110, 0, "EdsPsxButton::Setup(*ssb)");
    _ssb = ssb;
    _normalCheckMs = normalCheckMs;
    _noEventMs = noEventMs;
    _ignoreRepeatMs = ignoreRepeatMs;

    // Check if device available

    Ping();
    if (_isAvailable) { 
        if (_dbg->require(10)) _dbg->log(10, 0, "PSX Conbroller detected");
    } else {
        if (_dbg->require(10)) _dbg->log(10, 0, "PSX Conbroller not available");
    }
}

bool EdsPsxButton::Ping() {
    byte cmd[] = {0xA8, 0x8A, 0x04, 0x01, 0x00, 0x01, 0x06, 0xED};
    cmd[4] = _DevId;

    byte tryCnt = 0;
    while (tryCnt++ < 5) {
        _isAvailable = _ssb->SendCommand((byte *) cmd, true);
        if (_isAvailable) break;
    }
    return (_isAvailable);
}


bool EdsPsxButton::GetData() {
    _thisDataReady = false;
    _thisDataError = false;
    if (!IsReady()) return false;

    bool _prevDataRady = _lastDataReady;
    _lastDataReady = false;

    
    // byte cmd[] = {0xA8, 0x8A, 0x02, 0x01, 0x03, 0xED};
    // byte cmd[] = { 0xA8, 0x8A, 0x07, 0x01, 0x00, 0x03, 0x2E, 0x01, 0x01, 0x3C, 0xED };
    byte cmd[] = { 0xA8, 0x8A, 0x06, 0x01, 0x00, 0x02, 0x28, 0x02, 0x33, 0xED };
    cmd[4] = _DevId;

    unsigned long startMs = millis();
    if (!_ssb->SendCommand((byte *) cmd, true)) {
        _thisDataError = true;
        return false;
    }
    
    unsigned long diff = millis() - startMs;
    //_dbg->msg("It taks %d ms to read PSX", diff); 

    Buffer *result = _ssb->ReturnBuffer();
    // Data returned: A8 8A 0B 01 ?? ?? ?? {1} {2} ....
    // New return:  A8 8A 08 01 00 02 28 02  [ EF 00 ] 24 ED 
    uint16_t data;
    byte *button = (byte *) &data;
    button[0] = result->peek(8);
    button[1] = result->peek(9);

    // Due to the behavious of PSX control board, it will repeat the value within 1s
    // Need to skip repeated value if handled
    if (_prevDataRady && (_lastReportValue == data) && (_lastValueHandled) && ((millis() - _lastReportMS) < _ignoreRepeatMs)) {
        // ignore this value as it has just been handled
        // _dbg->msg("PSX Button already handled: %d, %04X : %04X, %d, %ld", _prevDataRady, _lastReportValue, data, _lastValueHandled, millis() - _lastReportMS);
        _lastDataReady = true;
        return false;
    } else {
        // _dbg->msg("PSX Button NOT handled: %d, %04X : %04X, %d, %ld", _prevDataRady, _lastReportValue, data, _lastValueHandled, millis() - _lastReportMS);
    }
    _data->SetData(_Device, _DevId, 0, data);
    _lastDataReady = true;
    _thisDataReady = true;
    _lastReportMS = millis();
    _lastReportValue = data;
    _lastValueHandled = false;
    if ((data != 0xFFFF) && (_dbg->require(100))) _dbg->log(100,0,"PSX Button: [%d,%d,%d] %02X %02X => %04X", _Device, _DevId, 0, button[0], button[1], data);
    return (data != 0xFFFF);
}


/*  
*   PostHandler
*       Can wait longer if data reported and handled
*/
void EdsPsxButton::PostHandler(bool eventMatched, bool isRelated, bool pending) {
    if (!IsReady()) return;
    if (_dbg->require(210)) _dbg->log(210,0,"EdsPsxButton::PostHandler(%d,%d,%d)",eventMatched, isRelated, pending);
    if (_thisDataReady) _lastValueHandled = isRelated;
    // wait longer if 
    //   - button pressed, and no event required or handled: i.e. !eventMatched
    if ((_lastDataReady) && (_lastReportValue != 0xFFFF) && ((!eventMatched) || (isRelated))) {
        _nextReportMs = millis() + _noEventMs;
    } else {
        _nextReportMs = millis() + _normalCheckMs;
    }
}

void EdsPsxButton::Shock() {
    if (!IsAvailable()) return;
    // byte cmd[] = {0xA8, 0x8A, 0x02, 0x02, 0x04, 0xED};
    byte cmd[] = {0xA8, 0x8A, 0x07, 0x01, 0x00, 0x03, 0x2F, 0x01, 0x01, 0x3C, 0xED};
    _ssb->SendCommand((byte *) cmd, true); 
}