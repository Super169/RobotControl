#include "EdsCounter.h"

EdsCounter::EdsCounter(EventData *data, MyDebugger *dbg, byte devId) {
    _Device = (uint8_t) EventData::DEVICE::counter;
    Config(data, dbg, devId);
    _reportInterval = 100;  // Counter can be reported 10 times/second
    _isEnabled = true;      // Always enabled
    _isAvailable = true;
}

EdsCounter::~EdsCounter() {
}

void EdsCounter::Setup() {
    for (uint8_t i = 0; i < ECNT_MAX; i++) {
        _counter[i] = 0;
    }
}

bool EdsCounter::GetData() {
    _thisDataReady = false;
    if (!IsReady()) return false;
    _lastDataReady = false;

    for (uint8_t i = 0; i < ECNT_MAX; i++) {
        _data->SetData(_Device, _DevId, i, _counter[i]);
    }

    _lastDataReady = true;
    _thisDataReady = true;
    return true;
}


bool EdsCounter::SetCounter(uint8_t target, int16_t value) {
    if (target >= ECNT_MAX) return false;
    _counter[target] = value;
}