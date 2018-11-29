#include "EventData.h"


EventData::EventData()
{
    Clear();
}

EventData::~EventData()
{
}

void EventData::Clear() {
    for (int i = 0; i < ED_DATA_SIZE; i++) {
        _ready[i] = false;
        _data[i] = ED_INVALID_DATA;
    }
}

bool EventData::IsValid(uint8_t device, uint8_t devId, uint8_t target) {
    if (device > ED_MAX_DEVICE) return false;
    return (target < _size[device]);
}

bool EventData::IsReady(uint8_t device, uint8_t devId, uint8_t target) {
    if (!IsValid(device, devId, target)) return false;
    return _ready[Offset(device, devId, target)];
}

bool EventData::MarkReady(uint8_t device, uint8_t devId, uint8_t target, bool ready) {
    if (!IsValid(device, devId, target)) return false;
    _ready[Offset(device, devId, target)] = ready;
    return true;
}

uint8_t EventData::Offset(uint8_t device, uint8_t devId, uint8_t target) {
    // not need to validate for private function
    return _offset[device] + target;
}

uint8_t EventData::DeviceDataSize(uint8_t device) {
    if (device > ED_MAX_DEVICE) return 0;
    return _size[device];
}


bool EventData::SetData(uint8_t device, uint8_t devId, uint8_t target, int16_t value) {
    if (!IsValid(device, devId, target)) return false;
    uint8_t offset = Offset(device, devId, target);
    _data[offset] = value;
    MarkReady(device, devId, target, true);
    return true;
}

int16_t EventData::GetData(uint8_t device, uint8_t devId, uint8_t target) {
    if (!IsReady(device, devId, target)) return ED_INVALID_DATA;
    uint8_t offset = Offset(device, devId, target);
    return _data[offset];
}

void EventData::DumpData(Stream *output) {
    // Hard to read if just dump the array.
    output->printf("\nEventData::DumpData:\n");
    output->printf("\nMPU: ");
    ShowValue(output, 0, 0);
    ShowValue(output, 1, 0);
    ShowValue(output, 2, 0);
    output->printf("\nTouch: ");
    ShowValue(output, 3, 0);
    output->printf("\nPSX: ");
    ShowValue(output, 4, 1);
    output->printf("\nBattery Reading: ");
    ShowValue(output, 5, 0);
    output->printf("\nBattery Level: ");
    ShowValue(output, 6, 0);
    output->printf("\n\n");
}

void EventData::ShowValue(Stream *output, uint8_t idx, uint8_t mode) {
    uint16_t u16;
    if (_ready[idx]) {
        switch (mode) {
            case 1: // HEX
                u16 = _data[idx];
                output->printf("%04X ", u16);
                break;
            case 2: // Binary
                output->printf("%16b ", _data[idx]);
                break;
            default: // DEC 
                output->printf("%d ", _data[idx]);
                break;
        }
    } else {
        output->printf("---- ");
    }
}