#include "EdsBattery.h"

EdsBattery::EdsBattery(EventData *data, MyDebugger *dbg, byte devId) {
    _Device = (uint8_t) EventData::DEVICE::battery;
    Config(data, dbg, devId);
}

EdsBattery::~EdsBattery() {
}

void EdsBattery::Setup(uint16_t minVoltage, uint16_t maxVoltage, uint16_t normalCheckMs, uint16_t alarmtIntervalMs) {
    _minVoltage = minVoltage;
    _maxVoltage = maxVoltage;
    _normalCheckMs = normalCheckMs;
    _alarmIntervalMs = alarmtIntervalMs;
}

bool EdsBattery::GetData() {
    _thisDataReady = false;
    if (!IsReady()) return false;
    _lastDataReady = false;
    
    uint16_t v = analogRead(0);
    int iPower = GetPower(v);

    /*
    _dbg->msg("Battery: %d, %d, %d", _Device, _DevId, (uint8_t) EventData::BATTERY_TARGET::reading);
    _dbg->msg("Battery reading: %d (%d - %d)", v, _minVoltage, _maxVoltage);
    _dbg->msg("Battery level: %d", iPower);
    */

    _data->SetData(_Device, _DevId, (uint8_t) EventData::BATTERY_TARGET::reading, v);
    _data->SetData(_Device, _DevId, (uint8_t) EventData::BATTERY_TARGET::level, iPower);
    _lastDataReady = true;
    _thisDataReady = true;
    _lastReportMS = millis();
    _lastReportReading = v;
    _lastReportLevel = iPower;
    return true;
}

byte EdsBattery::GetPower(uint16_t v) {
	// Power in precentage instead of voltage
	float power = ((float) (v - _minVoltage) / (_maxVoltage - _minVoltage) * 100.0f);
	int iPower = (int) (power + 0.5); // round up
	if (iPower > 100) iPower = 100;
	if (iPower < 0) iPower = 0;
	return (byte) iPower;
}

/*  
*   PostHandler
*/
void EdsBattery::PostHandler(bool eventMatched, bool isRelated) {
    if (!IsReady()) return;
    if (_thisDataReady && isRelated) {
        _nextReportMs = millis() + _alarmIntervalMs;
    } else {
        _nextReportMs = millis() + _normalCheckMs;
    }
}
