#include "EdsBattery.h"

EdsBattery::EdsBattery(EventData *data) {
    _data = data;
}

EdsBattery::~EdsBattery() {
}

void EdsBattery::Begin(uint16_t minVoltage, uint16_t maxVoltage, uint16_t alarmtIntervalMs, Stream *debugPort, byte devId) {
    _minVoltage = minVoltage;
    _maxVoltage = maxVoltage;
    _alarmIntervalMs = alarmtIntervalMs;
    _dbg.setOutput(debugPort);
    _Device = (uint8_t) EventData::DEVICE::battery;
    _DevId = devId;
    _isEnabled = true;

    _dbg.enableDebug(false);    // Disable debug
}

void EdsBattery::GetData() {
    _lastDataReady = false;
    if (!IsReady()) return;
    
    uint16_t v = analogRead(0);
    int iPower = GetPower(v);

    _dbg.msg("Battery: %d, %d, %d", _Device, _DevId, (uint8_t) EventData::BATTERY_TARGET::reading);
    _dbg.msg("Battery reading: %d (%d - %d)", v, _minVoltage, _maxVoltage);
    _dbg.msg("Battery level: %d", iPower);

    _data->SetData(_Device, _DevId, (uint8_t) EventData::BATTERY_TARGET::reading, v);
    _data->SetData(_Device, _DevId, (uint8_t) EventData::BATTERY_TARGET::level, iPower);
    _lastDataReady = true;
    _lastReportMS = millis();
    _lastReportReading = v;
    _lastReportLevel = iPower;
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
    if (_lastDataReady && isRelated) {
        _nextReportMs = millis() + _alarmIntervalMs;
    } else {
        _nextReportMs = millis() + EDS_DELAY_CHECK_MS;
    }
}
