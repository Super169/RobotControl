#include "EventDataSource.h"



bool EventDataSource::IsEnabled() { 
    return _isEnabled; 
}

bool EventDataSource::IsReady() { 
    return (_isEnabled && (millis() > _nextReportMs)); 
}

void EventDataSource::Begin(bool isEnabled, Stream *debugPort, byte devId) {
    _isEnabled = isEnabled;
    _dbg.setOutput(debugPort);
    _Device = (uint8_t) EventData::DEVICE::touch;
    _DevId = devId;
}

void EventDataSource::SetNextReportTime() { 
    _nextReportMs = millis() + _reportInterval; 
}

void EventDataSource::PostHandler(bool eventMatched, bool isRelated) {
    if (!IsReady()) return;
    if (_lastDataReady) {
        if (eventMatched && !isRelated){
            // handled other events, need to execute again immediately 
            _nextReportMs = 0;
        } else  {
            // either handled or no action for it, can delay longer
            _nextReportMs = millis() + EDS_DELAY_CHECK_MS;
        }
    } else {
        _nextReportMs = millis() + EDS_CONTINUE_CHECK_MS;
    }
}
