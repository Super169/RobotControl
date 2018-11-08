#include "EventHandler.h"

EventHandler::EventHandler(EventData *data)
{
    _data = data;
    _evtCount = 0;
    _events = 0;
}

EventHandler::~EventHandler()
{
    delete _events;
}

bool EventHandler::LoadData(String filename) {
    if (_evtCount > 0) {
        _evtCount = 0;
        free (_events);
    }
    size_t size = FileSize(filename.c_str());
    if (size == 0) return true;
    uint8_t buffer[size];
    if (!ReadFile(filename.c_str(), (uint8_t *) buffer, size)) return false;
    _evtCount = size / sizeof(EVENT);
    _events =  (EVENT *) malloc(size);
    memcpy((void *) _events, (void *) buffer, size);
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


bool EventHandler::DeleteFile(const char *fileName) {
	if (!SPIFFS.begin()) return false;
    bool success = false;
    if (SPIFFS.exists(fileName)) {
        if (SPIFFS.remove(fileName)) {
            Serial.printf("File %s successfully removed\n", fileName);
            success = true;
        } else {
            Serial.printf("Fail removing file %s \n", fileName);
        }
    } else {
        Serial.printf("File %s not found\n", fileName);
    }
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

bool EventHandler::IsRequired(uint8_t device) {
    if (device <= ED_MAX_DEVICE) return _reqDevice[device];
    return false;
}

void EventHandler::CheckEventsRequirement() {
    
    for (int i = 0; i < ED_MAX_DEVICE + 1; i++) _reqDevice[i] = false;

    for (uint16_t i = 0; i < _evtCount; i++) {
        if (_data->IsValid(_events[i].data.condition.data.device,
                           _events[i].data.condition.data.devId, 
                           _events[i].data.condition.data.target)) {
            _reqDevice[_events[i].data.condition.data.device] = true;
        }
    }
}

EventHandler::EVENT EventHandler::CheckEvents() {
    EVENT event;
    memset((void *) event.buffer, 0, sizeof(EVENT));

    bool skipEvent = false;
    for (uint16_t i = 0; i < _evtCount; i++) {
        if (skipEvent) {
            // Skip all related events if pre-cond failed
            skipEvent = (_events[i].data.type != (uint8_t) EVENT_TYPE::handler);
        } else {
            if (MatchCondition(_events[i].data.condition)) {
                if (_events[i].data.type == (uint8_t) EVENT_TYPE::handler) {
                    // just for safety, make a copy of event object for return
                    // Or it can just return _events[i];
                    memcpy((void *) event.buffer, (void *) _events[i].buffer, sizeof(EVENT));
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

bool EventHandler::MatchCondition(CONDITION cond) {
    if (!_data->IsValid(cond.data.device, cond.data.devId, cond.data.target)) return false;
    int16_t value = _data->GetData(cond.data.device, cond.data.devId, cond.data.target);
    switch (cond.data.checkMode) {
        case (uint8_t) CHECK_MODE::match:
            if (value == ED_INVALID_DATA) return false;
            return value == cond.data.value;
        case (uint8_t) CHECK_MODE::greater:
            if (value == ED_INVALID_DATA) return false;
            return value > cond.data.value;
        case (uint8_t) CHECK_MODE::less:
            if (value == ED_INVALID_DATA) return false;
            return value < cond.data.value;
        case (uint8_t) CHECK_MODE::button:
            uint16_t status = (value & cond.data.value);
            return (status == 0);
    }
    return false;
}

size_t EventHandler::FileSize(const char *fileName) {
	if (!SPIFFS.begin()) return 0;
    size_t size = 0;
	File f = SPIFFS.open(fileName, FILE_READ);
    if (f) size = f.size();
    f.close();
    return size;
}
