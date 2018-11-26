#include "EdsPsxButton.h"

EdsPsxButton::EdsPsxButton(EventData *data) {
    _data = data;
}

EdsPsxButton::~EdsPsxButton() {
}

/*
void EdsPsxButton::Initialize(EventData *data) {
    _data = data;
}
*/

void EdsPsxButton::Begin(SSBoard *ssb, Stream *debugPort) {
    _ssb = ssb;
    _dbg.setOutput(debugPort);
    _Device = (uint8_t) EventData::DEVICE::psx_button;
    _DevId = 0;
}



void EdsPsxButton::GetData() {
    if (!IsReady()) return;
}

void EdsPsxButton::PostHandler() {

}
