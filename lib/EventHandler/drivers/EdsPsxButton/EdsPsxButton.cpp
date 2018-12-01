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
    _ssb = ssb;
}

bool EdsPsxButton::GetData() {
    _lastDataReady = false;
    if (!IsReady()) return false;

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
    _data->SetData(_Device, _DevId, 0, data);
    if (data != 0xFFFF) _dbg->msg("PSX Button: [%d,%d,%d] %02X %02X => %04X", _Device, _DevId, 0, button[0], button[1], data);
    _lastDataReady = true;
    _lastReportMS = millis();
    _lastReportValue = data;
    return (data != 0xFFFF);
}


/*  
*   PostHandler
*       Can wait longer if data reported and handled
*/
void EdsPsxButton::PostHandler(bool eventMatched, bool isRelated) {
    if (!IsReady()) return;
    // wait longer if 
    //   - button pressed, and no event required or handled: i.e. !eventMatched
    if ((_lastDataReady) && (_lastReportValue != 0xFFFF) && ((!eventMatched) || (isRelated))) {
        _nextReportMs = millis() + 100;
    } else {
        _nextReportMs = millis() + EDS_CONTINUE_CHECK_MS;
    }
}
