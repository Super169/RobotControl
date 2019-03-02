#include "EventDataSource.h"


bool EventDataSource::SetEnabled(bool enabled) { 
    _isEnabled = enabled;
    return IsEnabled();
}

bool EventDataSource::IsAvailable() { 
    return (_isAvailable); 
}

bool EventDataSource::IsEnabled() { 
    return (_isAvailable && _isEnabled); 
}

bool EventDataSource::IsReady() { 
    return (_isAvailable && _isEnabled && !_isSuspended && (millis() > _nextReportMs)); 
}

void EventDataSource::Suspend(bool suspend) { 
    _isSuspended = suspend; 
}

void EventDataSource::Config(EventData *data, MyDebugger *dbg, byte devId) {
    _data = data;
    _dbg = dbg;
    _DevId = devId;
}

void EventDataSource::SetNextReportTime() { 
    _nextReportMs = millis() + _reportInterval; 
}

/*
*   PostHandling (eventMatched, isRelated, pending)
*   - eventMatched : this round has event matched
*   - isRelated : the matched event is related to this device
*   - pending : some event pending on this data within threadhold: see EventHander.cpp
*               condition require to match for continous time before confirmed (e.g. Mpu6050)
*/
void EventDataSource::PostHandler(bool eventMatched, bool isRelated, bool pending) {
    if (!IsReady()) return;
    if (_dbg->require(210)) _dbg->log(210,0,"EventDataSource[%d]::PostHandler(%d,%d,%d)",_Device, eventMatched, isRelated, pending);
    if (_thisDataReady) {
        if (isRelated) {
            // Current data has event matched
            _nextReportMs = millis() + _delayCheckMs;
        } else if (eventMatched) {
            // Maybe handled with other events, should check again in next round
            _nextReportMs = 0;
        } else {
            // Current data did not trigger any event
            _nextReportMs = millis() + _continueCheckMs;
        }
    } else if (pending) {
        _nextReportMs = millis() + _pendingCheckMs;
    } else if (_thisDataError) {
        _nextReportMs = millis() + _continueCheckMs;
    } else {
        // not ready at GetData, but ready now
        // Do nothing and let it getData again in next round
    }

}
