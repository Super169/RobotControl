#include "EventHandler.h"

void EventHandler::LoadDummyData() {
    _evtCount = 10;
    uint8_t evt[10][12]  =   {
                                {0x00, 0x02, 0x04, 0x00, 0x00, 0x03, 0x90, 0x01, 0x00, 0x00, 0x00, 0x00},
                                {0x00, 0x01, 0x01, 0x00, 0x02, 0x03, 0xC0, 0xC7, 0x01, 0x0F, 0x00, 0x00},
                                {0x00, 0x01, 0x01, 0x00, 0x02, 0x03, 0xC0, 0xC7, 0x01, 0x05, 0x00, 0x00},
                                {0x00, 0x01, 0x01, 0x00, 0x02, 0x02, 0x40, 0x38, 0x01, 0x06, 0x00, 0x00},
                                {0x00, 0x01, 0x02, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x0B, 0x00, 0x00},
                                {0x00, 0x01, 0x02, 0x00, 0x00, 0x01, 0x02, 0x00, 0x01, 0x0C, 0x00, 0x00},
                                {0x00, 0x01, 0x02, 0x00, 0x00, 0x01, 0x03, 0x00, 0x01, 0x0D, 0x00, 0x00},
                                {0x00, 0x01, 0x02, 0x00, 0x00, 0x01, 0x04, 0x00, 0x01, 0x0E, 0x00, 0x00},
                                {0x00, 0x01, 0x05, 0x00, 0x00, 0x03, 0x0A, 0x00, 0x04, 0x00, 0x02, 0x00},
                                {0x00, 0x01, 0x05, 0x00, 0x00, 0x03, 0x1E, 0x00, 0x04, 0x00, 0x03, 0x00}
                            };

    size_t size = _evtCount * sizeof(EVENT);
    _events = (EVENT *) malloc(size);
    memcpy((void *) _events, (void *) evt, size);

    Serial.printf("Data dump of _events[%d]:\n", _evtCount);
    for (int i = 0; i < _evtCount; i++) {
        for (int j = 0; j < 12; j++) {
            Serial.printf("%02X ", _events[i].buffer[j]);
        }
        Serial.printf("\n");
    }

    CheckEventsRequirement();

}


/*
void EventHandler::LoadDummyData() {

    _evtCount = 5;
    size_t size = _evtCount * sizeof(EVENT);
    _events = (EVENT *) malloc(size);
    memset((void *) _events, 0, size);

    _events[0].data.seq = 0;
    _events[0].data.type = 1;
    _events[0].data.condition.data.device = (uint8_t) DEVICE_TYPE::mpu6050;
    _events[0].data.condition.data.devId = 0;
    _events[0].data.condition.data.target = (uint8_t) MPU6050_TARGET::z;
    _events[0].data.condition.data.checkMode = (uint8_t) CHECK_MODE::less;
    _events[0].data.condition.data.value = -14400;
    _events[0].data.action.data.type = (uint8_t) ACTION_TYPE::playAction;
    _events[0].data.action.data.parm_1 = 5;

    _events[1].data.seq = 1;
    _events[1].data.type = 1;
    _events[1].data.condition.data.device = (uint8_t) DEVICE_TYPE::mpu6050;
    _events[1].data.condition.data.devId = 0;
    _events[1].data.condition.data.target = (uint8_t) MPU6050_TARGET::z;
    _events[1].data.condition.data.checkMode = (uint8_t) CHECK_MODE::greater;
    _events[1].data.condition.data.value = 14400;
    _events[1].data.action.data.type = (uint8_t) ACTION_TYPE::playAction;
    _events[1].data.action.data.parm_1 = 6;

    _events[2].data.seq = 2;
    _events[2].data.type = 1;
    _events[2].data.condition.data.device = (uint8_t) DEVICE_TYPE::batteryLevel;
    _events[2].data.condition.data.devId = 0;
    _events[2].data.condition.data.target = 0;
    _events[2].data.condition.data.checkMode = (uint8_t) CHECK_MODE::less;
    _events[2].data.condition.data.value = 30;
    _events[2].data.action.data.type = (uint8_t) ACTION_TYPE::mp3PlayMp3;
    _events[2].data.action.u16data.parm_u16 = 1234;
    
    _events[3].data.seq = 3;
    _events[3].data.type = 1;
    _events[3].data.condition.data.device = (uint8_t) DEVICE_TYPE::touch;
    _events[3].data.condition.data.devId = 0;
    _events[3].data.condition.data.target = 0;
    _events[3].data.condition.data.checkMode = (uint8_t) CHECK_MODE::match;
    _events[3].data.condition.data.value = 1;
    _events[3].data.action.data.type = (uint8_t) ACTION_TYPE::playAction;
    _events[3].data.action.data.parm_1 = 11;

    _events[4].data.seq = 4;
    _events[4].data.type = 1;
    _events[4].data.condition.data.device = (uint8_t) DEVICE_TYPE::mpu6050;
    _events[4].data.condition.data.devId = 0;
    _events[4].data.condition.data.target = (uint8_t) MPU6050_TARGET::x;
    _events[4].data.condition.data.checkMode = (uint8_t) CHECK_MODE::less;
    _events[4].data.condition.data.value = 0;
    _events[4].data.action.data.type = (uint8_t) ACTION_TYPE::stopAction;
    _events[4].data.action.data.parm_1 = 0;

    Serial.printf("sizeof(EVENT) = %d\n", sizeof(EVENT));
    Serial.printf("sizeof(_events) = %d\n", sizeof(_events));
    Serial.printf("(_events) = %08X\n", (unsigned int) _events);

    Serial.printf("Data dump of _events:\n");
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 12; j++) {
            Serial.printf("%02X ", _events[i].buffer[j]);
        }
        Serial.printf("\n");
    }

    CheckEventsRequirement();
}
*/

