#include "EventDataSource.h"



bool EventDataSource::IsEnabled() { 
    return _isEnabled; 
}

bool EventDataSource::IsReady() { 
    return (_isEnabled && (millis() > _nextReportMs)); 
}

void EventDataSource::Config(EventData *data, MyDebugger *dbg, byte devId) {
    _data = data;
    _dbg = dbg;
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
