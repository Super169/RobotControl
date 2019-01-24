#include "EventHandler.h"

// #define EH_DEBUG

EventHandler::EventHandler(EventData *data)
{
    _data = data;
    _evtCount = 0;
    _events = NULL;
    _eventLastMs = NULL;
    _isRequired = (bool *) malloc(_data->DevCount());
    _lastEventRelated = (bool *) malloc(_data->DevCount());
    for (int i = 0; i < _data->DevCount(); i++) {
        _isRequired[i] = false;
        _lastEventRelated[i] = false;
    }
}

EventHandler::~EventHandler()
{
    delete _events;
    delete _eventLastMs;
}

/*
*  Free the memory and reset counter to 0
*/
void EventHandler::ReleaseMemory() {
    if (_evtCount > 0) {
        _evtCount = 0;
        free (_events);
        free (_eventLastMs);
    }
}


/*
*  Set event count and reserve memory
*/
void EventHandler::SetCount(uint8_t count) {
    _evtCount = count;
    size_t size = count * sizeof(EVENT);
    _events =  (EVENT *) malloc(size);
    memset(_events, 0, size);
    size = count * sizeof(unsigned long);
    _eventLastMs = (unsigned long *) malloc(size);
    memset(_eventLastMs, 0, size);
}

bool EventHandler::FillData(uint8_t startIdx, uint8_t count, byte *buffer) {
    if (startIdx > _evtCount) return false;
    if (startIdx + count > _evtCount) return false;
    void *dest = (void *) (_events + startIdx * sizeof(EVENT));
    size_t size = count * sizeof(EVENT);
    memcpy(dest, buffer, size);
    return true;
}

/*
*  Reset the handler with specified event count, this is for used when writing from PC
*/
void EventHandler::Reset(uint8_t count) {
    ReleaseMemory();
    SetCount(count);
}

/*
*/
bool EventHandler::Clone(EventHandler *source) {
    ReleaseMemory();
    SetCount(source->Count());
    size_t size = _evtCount * sizeof(EVENT);
    memcpy((void *) _events, (void *) source->Events(), size);
    CheckEventsRequirement();
    return true;
}

/*
*   Validation
*   - just check if all events are ready
*/
bool EventHandler::IsValid() {
    for (int i = 0; i < _evtCount; i++) {
        EVENT evt = _events[i];
        if (evt.data.type == 0) return false;
        if (!_data->IsValid(evt.data.condition.data.device, evt.data.condition.data.devId, evt.data.condition.data.target)) return false;
    }
    return true;
}


bool EventHandler::LoadData(String filename) {
    ReleaseMemory();
    size_t size = FileSize(filename.c_str());
    if (size == 0) return true;
    uint8_t buffer[size];
    if (!ReadFile(filename.c_str(), (uint8_t *) buffer, size)) return false;
    SetCount(size / sizeof(EVENT));
    /*
    _evtCount = size / sizeof(EVENT);
    _events =  (EVENT *) malloc(size);
    */
    memcpy((void *) _events, (void *) buffer, size);
    CheckEventsRequirement();
    return true;
}


bool EventHandler::ReadFile(const char *fileName, uint8_t *buffer, size_t size) {
	if (!SPIFFS.begin()) return false;
	bool success = false;
	File f = SPIFFS.open(fileName, FILE_READ);
    if ((f) && (f.size() == size)) {
        f.read(buffer, size);
        success = true;
    }
	f.close();
	SPIFFS.end();
	return success;
}

bool EventHandler::SaveData(String filename) {
    if (_evtCount == 0) {
        return DeleteFile(filename.c_str());
    }
    size_t size = _evtCount * sizeof(EVENT);
    return WriteFile(filename.c_str(), (uint8_t *) _events, size);
}


bool EventHandler::DeleteFile(const char *fileName, bool mustExists) {
	if (!SPIFFS.begin()) return false;
    bool success = false;
    if (SPIFFS.exists(fileName)) {
        if (SPIFFS.remove(fileName)) {
            success = true;
        }
    } else {
        if (!mustExists) success = true;
    }
    SPIFFS.end();
    return success;
}

bool EventHandler::WriteFile(const char *fileName, uint8_t *buffer, size_t size) {
	if (!SPIFFS.begin()) return false;
	bool success = false;
	File f = SPIFFS.open(fileName, FILE_WRITE);
	if (f) {
		size_t wCnt = f.write(buffer, size);
		success = (wCnt == size);
	}
	f.close();
	SPIFFS.end();
	return success;
}

bool EventHandler::IsRequired(uint8_t device, uint8_t devId) {
    if (_data->IsValid(device, devId)) return _isRequired[_data->DevOffset(device, devId)];
    return false;
}

bool EventHandler::LastEventRelated(uint8_t device, uint8_t devId) {
    if (_data->IsValid(device, devId)) return _lastEventRelated[_data->DevOffset(device, devId)];
    return false;
}

bool EventHandler::IsPending(uint8_t device, uint8_t devId) {
    for (int i = 0; i < _evtCount; i++) {
        if ((_events[i].data.condition.data.device == device) && (_events[i].data.condition.data.devId == devId))    {
            if (_eventLastMs[i]) {
                return true;
            }
        }
    }
    return false;
}

void EventHandler::CheckEventsRequirement() {
    
    for (int i = 0; i < _data->DevCount() + 1; i++) _isRequired[i] = false;

    for (uint16_t i = 0; i < _evtCount; i++) {
        if (_data->IsValid(_events[i].data.condition.data.device,
                           _events[i].data.condition.data.devId, 
                           _events[i].data.condition.data.target)) {
            _isRequired[_data->DevOffset(_events[i].data.condition.data.device,_events[i].data.condition.data.devId)] = true;
        }
    }
}

EventHandler::EVENT EventHandler::CheckEvents() {
    EVENT event;
    memset((void *) event.buffer, 0, sizeof(EVENT));
    for (int i = 0; i < _data->DevCount(); i++) _lastEventRelated[i] = false;

    bool skipEvent = false;
    for (uint16_t i = 0; i < _evtCount; i++) {
        if (skipEvent) {
            // Skip all related events if pre-cond failed
            skipEvent = (_events[i].data.type != (uint8_t) EVENT_TYPE::handler);
            if (_data->Threadhold(_events[i].data.condition.data.device)) {
                // need to check continue condition, so it has to check event will be ignored
                MatchCondition(i, _events[i].data.condition);
            }
        } else {
            if (MatchCondition(i, _events[i].data.condition)) {
                if (_events[i].data.type == (uint8_t) EVENT_TYPE::handler) {
                    byte condDevice = _events[i].data.condition.data.device; 
                    byte condDevId =  _events[i].data.condition.data.devId;
                    // just for safety, make a copy of event object for return
                    // Or it can just return _events[i];
                    memcpy((void *) event.buffer, (void *) _events[i].buffer, sizeof(EVENT));
                    // Check events for related devices
                    _lastEventRelated[_data->DevOffset(condDevice, condDevId)] = true;
                    // Reste lastMs once handled
                    #ifdef EH_DEBUG
                    Serial1.printf("%08ld [%d] Release last ms for %d\n", millis(), i, condDevice);
                    #endif
                    _eventLastMs[i] = 0;
                    
                    int16_t j = i - 1;
                    while (j >= 0) {
                        if (_events[j].data.type == (uint8_t) EVENT_TYPE::preCond) {
                            condDevice = _events[j].data.condition.data.device;
                            condDevId =  _events[i].data.condition.data.devId;
                            _lastEventRelated[_data->DevOffset(condDevice, condDevId)] = true;
                            #ifdef EH_DEBUG
                            Serial1.printf("%08ld [%d] Also release last ms for %d\n", millis(), j,  condDevice);
                            #endif
                            _eventLastMs[j] = 0;
                        } else {
                            break;
                        }
                        j--;
                    }
                    
                    return event;
                }
            } else {
                if (_events[i].data.type == (uint8_t) EVENT_TYPE::preCond) {
                    skipEvent = true;
                }
            }
        }
    }

    return event;
}

void EventHandler::ResetEventLastMs() {
    size_t size = _evtCount * sizeof(unsigned long);
    memset(_eventLastMs, 0, size);   
}

void EventHandler::DumpEvents(Stream *output) {
    output->printf("\nEventHandler::DumpEvents:\n");
    output->printf("- Event Count: %d\n", _evtCount);
    for (int i = 0; i < _evtCount; i++) {
        byte *buffer = _events[i].buffer;
        // {idx}: {seq} {type} - [ {device}, {devId}, {target}] {mode} {val} {val} (val) => {type} {parm_1} {parm_2} {parm_3} ({parm_u16})
        output->printf("%03d: %02X %02X - ", i, buffer[0], buffer[1]);
        output->printf("[%02X,%02X,%02X] %02X : %02X %02X => ", buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
        output->printf("%02X %02X %02X %02X\n", buffer[8], buffer[9], buffer[10], buffer[11]);
    }
    output->printf("\n");
}


bool EventHandler::MatchCondition(uint16_t idx, CONDITION cond) {
    if (!_data->IsReady(cond.data.device, cond.data.devId, cond.data.target)) return false;
    int16_t value = _data->GetData(cond.data.device, cond.data.devId, cond.data.target);
    bool match = false;
    switch (cond.data.checkMode) {
        case (uint8_t) CHECK_MODE::match:
            match = (value == cond.data.value);
            break;

        case (uint8_t) CHECK_MODE::greater:
            match = (value > cond.data.value);
            break;
        case (uint8_t) CHECK_MODE::less:
            match = (value < cond.data.value);
            break;

        case (uint8_t) CHECK_MODE::button:
            uint16_t status = (value & ~cond.data.value);
            match = (status == 0);
    }
    if (match) {
        byte condDevice = cond.data.device;
        uint16_t threadhold = _data->Threadhold(condDevice);
        if (threadhold) {
            if (_eventLastMs[idx]) {
                match = millis() > (_eventLastMs[idx] + threadhold);
                #ifdef EH_DEBUG
                if (match) {
                    Serial1.printf("%08ld [%d] Device %d: Threadhold matched: %08ld + %d\n", 
                                   millis(), idx, condDevice, _eventLastMs[idx], threadhold);
                }
                #endif
            } else {
                match = false;
                _eventLastMs[idx] = millis();
                #ifdef EH_DEBUG
                Serial1.printf("%08ld [%d] Device %d: Pending threadthod: %08ld + %d\n", millis(), idx, condDevice, _eventLastMs[idx], threadhold);
                #endif
            }
        }
    } else {
        #ifdef EH_DEBUG
        if (_eventLastMs[idx]) {
            Serial1.printf("%08ld [%d] Device %d: Reset threadthod due to not match\n", millis(), idx, cond.data.device);
        }
        #endif
        _eventLastMs[idx] = 0;
    }
    return match;
}

size_t EventHandler::FileSize(const char *fileName) {
	if (!SPIFFS.begin()) return 0;
    size_t size = 0;
	File f = SPIFFS.open(fileName, FILE_READ);
    if (f) size = f.size();
    f.close();
    return size;
}

