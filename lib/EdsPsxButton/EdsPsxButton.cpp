#include "EdsPsxButton.h"

EdsPsxButton::EdsPsxButton(Stream *busPort, Stream *debugPort) {
    _bus = busPort;
    _dbg.setOutput(debugPort);
    _Device = (uint8_t) EventData::DEVICE::psx_button;
    _DevId = 0;
}


void EdsPsxButton::Initialize(EventData *data) {
    _data = data;
}

void EdsPsxButton::GetData() {

}

void EdsPsxButton::PostHandler() {

}
