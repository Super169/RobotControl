#include "EventData.h"


EventData::EventData()
{
    // DevOffset - location of starting point of device, flag has 1 byte only
    // offset - location of starting point of data, dat can have multiple bytes
    _devCount = 0;
    _dataSize = 0;
    for (int i = 0; i <= ED_MAX_DEVICE; i++) {
        _offset[i] = _dataSize;
        _devOffset[i] = _devCount;
        _dataSize += _size[i] * _idCount[i];
        _devCount += _idCount[i];
    }
    _ready = (bool *) malloc(_dataSize);
    _threadhold = (uint16_t *) malloc(_devCount * sizeof(uint16_t));
    Clear();
}

EventData::~EventData()
{
}

void EventData::Clear() {
    for (int i = 0; i < ED_DATA_SIZE; i++) {
        _data[i] = ED_INVALID_DATA;
    }
    // for safety, do not use memset as data type may changed to multiple bytes later
    for (int i = 0; i < _dataSize; i++) {
        _ready[i] = false;
    }
    for (int i = 0; i < _devCount; i++) {
        _threadhold[i] = 0;
    }
}

bool EventData::IsValid(uint8_t device, uint8_t devId, uint8_t target) {
    if (device > ED_MAX_DEVICE) return false;
    if (devId > MaxId(device)) return false;
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
    // no need to validate for private function
    return _offset[device] + devId * _size[device] + target;
}

uint8_t EventData::DevOffset(uint8_t device, uint8_t devId) {
    // no need to validate for private function
    return _devOffset[device] + devId;
}

uint8_t EventData::IdCount(uint8_t device) {
    if (device > ED_MAX_DEVICE) return 0;
    return _idCount[device];
}

uint8_t EventData::MaxId(uint8_t device) {
    if (device > ED_MAX_DEVICE) return 0;
    return _idCount[device] - 1;
}

bool EventData::SetThreadhold(uint8_t device, uint8_t devId, uint16_t threadhold) {
    if (!IsValid(device, devId)) return false;
    _threadhold[DevOffset(device, devId)] = threadhold;
    return true;
}

uint16_t EventData::Threadhold(uint8_t device, uint8_t devId) {
    if (!IsValid(device, devId)) return 0;
    return _threadhold[DevOffset(device, devId)];
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
    uint8_t device, devCount;

    output->printf("Max Device: %d\n", ED_MAX_DEVICE);
    for (int i = 0; i <= ED_MAX_DEVICE; i++) {
        output->printf("%d: %d, %d, [ %d , %d ], [ %d , %d ]\n", 
                       i, _size[i], _idCount[i], 
                       _offset[i], Offset(i,0,0),
                       _devOffset[i], DevOffset(i, 0));
    }
    output->printf("\n");

    device = (uint8_t) DEVICE::mpu;
    devCount = _idCount[device];
    output->printf("\nMPU (device: %d x %d): \n", device, devCount);
    for (int id = 0; id < devCount; id++) {
        output->printf("-- Id: %d => ", id);
        ShowValue(output, Offset(device, id, 0), 0);
        ShowValue(output, Offset(device, id, 1), 0);
        ShowValue(output, Offset(device, id, 2), 0);
        output->printf("\n");
    }

    device = (uint8_t) DEVICE::touch;
    devCount = _idCount[device];
    output->printf("\nTouch (device: %d x %d): \n", device, devCount);
    for (int id = 0; id < devCount; id++) {
        output->printf("-- Id: %d => ", id);
        ShowValue(output, Offset(device, id, 0), 0);
        output->printf("\n");
    }

    device = (uint8_t) DEVICE::psx_button;
    devCount = _idCount[device];
    output->printf("\nPSX (device: %d x %d): \n", device, devCount);
    for (int id = 0; id < devCount; id++) {
        output->printf("-- Id: %d => ", id);
        ShowValue(output, Offset(device, id, 0), 2);
        output->printf("\n");
    }

    device = (uint8_t) DEVICE::battery;
    devCount = _idCount[device];
    output->printf("\nBattery (device: %d x %d): \n", device, devCount);
    for (int id = 0; id < devCount; id++) {
        output->printf("-- Id: %d => ", id);
        ShowValue(output, Offset(device, id, 0), 0);
        ShowValue(output, Offset(device, id, 1), 0);
        output->printf("\n");
    }

    device = (uint8_t) DEVICE::sonic;
    devCount = _idCount[device];
    output->printf("\nSonic (device: %d x %d): \n", device, devCount);
    for (int id = 0; id < devCount; id++) {
        output->printf("-- Id: %d => ", id);
        ShowValue(output, Offset(device, id, 0), 0);
        output->printf("\n");
    }

    output->printf("\n\n");
}

void EventData::DumpData(Stream *output, uint8_t device) {
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
