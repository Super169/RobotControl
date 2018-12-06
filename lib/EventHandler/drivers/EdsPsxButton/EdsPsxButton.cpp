#include "EdsPsxButton.h"

EdsPsxButton::EdsPsxButton(EventData *data, MyDebugger *dbg, byte devId) {
    _Device = (uint8_t) EventData::DEVICE::psx_button;
    Config(data, dbg, devId);
}

EdsPsxButton::~EdsPsxButton() {
}

/*
void EdsPsxButton::Initialize(EventData *data) {
    _data = data;
}
*/

void EdsPsxButton::Setup(SSBoard *ssb) {
     if (_dbg->require(110)) _dbg->log(110, 0, "EdsPsxButton::Setup(*ssb)");
    _ssb = ssb;
    // Check if device available

    byte cmd[] = {0xA8, 0x8A, 0x02, 0x01, 0x03, 0xED};
    _isAvailable = _ssb->SendCommand((byte *) cmd, true);
    if (_isAvailable) { 
        if (_dbg->require(10)) _dbg->log(10, 0, "PSX Conbroller detected");
    } else {
        if (_dbg->require(10)) _dbg->log(10, 0, "PSX Conbroller not available");
    }
}

bool EdsPsxButton::GetData() {
    _thisDataReady = false;
    if (!IsReady()) return false;

    bool _prevDataRady = _lastDataReady;
    _lastDataReady = false;

    byte cmd[] = {0xA8, 0x8A, 0x02, 0x01, 0x03, 0xED};
    unsigned long startMs = millis();
    if (!_ssb->SendCommand((byte *) cmd, true)) return false;
    
    unsigned long diff = millis() - startMs;
    //_dbg->msg("It taks %d ms to read PSX", diff); 

    Buffer *result = _ssb->ReturnBuffer();
    // Data returned: A8 8A 0B 01 ?? ?? ?? {1} {2} ....
    uint16_t data;
    byte *button = (byte *) &data;
    button[0] = result->peek(8);
    button[1] = result->peek(7);

    // Due to the behavious of PSX control board, it will repeat the value within 1s
    // Need to skip repeated value if handled
    if (_prevDataRady && (_lastReportValue == data) && (_lastValueHandled) && ((millis() - _lastReportMS) < EPB_IGNORE_REPEAT_TIME)) {
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
        _nextReportMs = millis() + 100;
    } else {
        _nextReportMs = millis() + EDS_CONTINUE_CHECK_MS;
    }
}

void EdsPsxButton::Shock() {
    if (!IsAvailable()) return;
    byte cmd[] = {0xA8, 0x8A, 0x02, 0x02, 0x04, 0xED};
    _ssb->SendCommand((byte *) cmd, false); 
}