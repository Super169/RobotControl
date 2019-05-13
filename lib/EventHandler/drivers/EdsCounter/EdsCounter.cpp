#include "EdsCounter.h"

EdsCounter::EdsCounter(EventData *data, MyDebugger *dbg, byte devId) {
    _Device = (uint8_t) EventData::DEVICE::counter;
    Config(data, dbg, devId);
    _reportInterval = 1;    // Counter always available
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
    // Counter is always ready
    // if (!IsReady()) return false;

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
    return true;
}

bool EdsCounter::AdjustCounter(uint8_t target, int8_t adjustment, uint8_t maxValue) {
    if (target >= ECNT_MAX) return false;
    int16_t newValue = _counter[target] + adjustment;
    uint8_t size = maxValue + 1;
    if (newValue < 0) {
        while (newValue < 0) newValue += size;
    } else if (newValue > maxValue) {
        while (newValue > maxValue) newValue -= size;
    }
    _counter[target] = newValue;
    return true;
}

int16_t EdsCounter::GetCounter(uint8_t target) {
    if (target >= ECNT_MAX) return 0;
    return _counter[target];
}