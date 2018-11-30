#include "EdsPsxButton.h"

EdsPsxButton::EdsPsxButton(EventData *data, MyDebugger *dbg, byte devId) {
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
    _dbg->enableDebug(false);
}

void EdsPsxButton::GetData() {
    _lastDataReady = false;
    if (!IsReady()) return;
    byte cmd[] = {0xA8, 0x8A, 0x02, 0x01, 0x03, 0xED};
    if (_ssb->SendCommand((byte *) cmd, true)) {
        Buffer *result = _ssb->ReturnBuffer();
        // Data returned: A8 8A 0B 01 ?? ?? ?? {1} {2} ....
        uint16_t data;
        byte *button = (byte *) &data;
        button[0] = result->peek(8);
        button[1] = result->peek(7);
        _data->SetData(_Device, _DevId, 0, data);
        if (data != 0xFFFF) _dbg->msg("PSX Button: %02X %02X => %04X", button[0], button[1], data);
        _lastDataReady = true;
        _lastReportMS = millis();
        _lastReportValue = data;
    }
}


/*  
*   PostHandler
*       Can wait longer if data reported and handled
*/
void EdsPsxButton::PostHandler(bool eventMatched, bool isRelated) {
    if (!IsReady()) return;
    if ((_lastDataReady) && (isRelated)) {
        _nextReportMs = millis() + EPB_DELAY_CHECK_MS;
    }
}
