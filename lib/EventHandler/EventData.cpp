#include "EventData.h"


EventData::EventData()
{
    for (int i = 0; i < ED_DATA_SIZE; i++) _data[i] = ED_INVALID_DATA;
}

EventData::~EventData()
{
    
}

bool EventData::IsValid(uint8_t device, uint8_t devId, uint8_t target) {
    if (device > ED_MAX_DEVICE) return false;
    return (target < _size[device]);
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
    return true;
}

int16_t EventData::GetData(uint8_t device, uint8_t devId, uint8_t target) {
    if (!IsValid(device, devId, target)) return ED_INVALID_DATA;
    uint8_t offset = Offset(device, devId, target);
    return _data[offset];
}