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


bool EventData::SetThreadhold(uint8_t device, uint16_t threadhold) {
    if (device > ED_MAX_DEVICE) return false;
    _threadhold[device] = threadhold;
    return true;
}

uint16_t EventData::Threadhold(uint8_t device) {
    if (device > ED_MAX_DEVICE) return 0;
    return _threadhold[device];
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
    ShowValue(output, 4, 2);
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
                u16 = _data[idx];
                // char buffer[33];
                // itoa(u16, buffer, 2);
                // output->printf("%04X [%s] : ", u16, buffer);
                // Split to 4 digits group for easy reading
                output->printf("%04X [", u16);
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        int pos = 15 - (i * 4 + j);
                        uint16_t mask = 1 << pos;
                        output->printf(((u16 & mask) == 0 ? "0" : "1"));
                    }
                    if (i < 3) output->printf(" ");
                }
                output->printf("]");
                break;
            default: // DEC 
                output->printf("%d ", _data[idx]);
                break;
        }
    } else {
        output->printf("---- ");
    }
}