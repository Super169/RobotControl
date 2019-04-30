#include "EdsMaze.h"

EdsMaze::EdsMaze(EventData *data, MyDebugger *dbg, byte devId) {
    _Device = (uint8_t) EventData::DEVICE::maze;
    Config(data, dbg, devId);
    _isAvailable = false;
}

EdsMaze::~EdsMaze() {
}

/*
void EdsMaze::Initialize(EventData *data) {
    _data = data;
}
*/

/*
*       Ping:
*          Send:    A8 8A 04 02 00 01 07 ED
*          Return:  A8 8A 05 02 00 01 00 08 ED
*/

void EdsMaze::Setup(SSBoard *ssb, RobotServo *rs, uint8_t wallDistance, 
                    uint8_t servoId, bool servoReversed, uint16_t servoMoveMs, uint16_t servoWaitMs,
                    unsigned long continueCheckms, unsigned long delayCheckMs) {
     if (_dbg->require(110)) _dbg->log(110, 0, "EdsMaze::Setup(*ssb)");
    _ssb = ssb;
    _rs = rs;
    _servoId = servoId;
    _wallDistance = wallDistance;

    _servoReversed = servoReversed;
    _servoMoveMs = servoMoveMs;
    _servoWaitMs = servoWaitMs;

    // Check if device available



    Ping();
    if (_isAvailable) {
        if (_dbg->require(10)) _dbg->log(10, 0, "Maze Solver activated");
    } else {
        if (_dbg->require(10)) _dbg->log(10, 0, "Maze Solver not available");
    }
    _data->SetThreadhold(_Device, 0);
    _pendingCheckMs = 0;
    _continueCheckMs = continueCheckms;
    _delayCheckMs = delayCheckMs;
}

bool EdsMaze::Ping() {
    _isAvailable = false;
    if (!Ping(EDS_MAZE_FRONT_ID)) return false;
    // use 3 sensor if no servo defined
    if (_servoId) {
        if (!_rs->exists(_servoId)) return false;
    } else {
        if (!Ping(EDS_MAZE_LEFT_ID)) return false;
        if (!Ping(EDS_MAZE_RIGHT_ID)) return false;
    }
    _isAvailable = true;
    return true;
}

bool EdsMaze::Ping(byte id) {
    byte cmd[] = {0xA8, 0x8A, 0x04, 0x02, 0x00, 0x01, 0x07, 0xED};
    cmd[4] = id;
    bool ready = false;
    byte tryCnt = 0;
    while (tryCnt++ < 5) {
        ready = _ssb->SendCommand((byte *) cmd, true);
        if (ready) break;
    }
    return (ready);
}

/*
*       Get Data:
*          Send:    A8 8A 06 02 00 02 28 02 34 ED	
*          Return:  A8 8A 08 02 00 02 28 02 00 B4 EA ED （B4，180cm）
*/

bool EdsMaze::GetData() {
    _thisDataReady = false;
    _thisDataError = false;

    if (!IsReady()) return false;

    uint16 disFront, disLeft, disRight;

    if (_servoId) {
        if (!GetOneSensorData(&disFront, &disLeft, &disRight)) return false;
    } else {
        if (!GetThreeSensorData(&disFront, &disLeft, &disRight)) return false;
    }

    uint16_t action;

    if (disLeft >= _wallDistance) {
        action = EDS_MAZE_GO_LEFT;
    } else if (disFront >= _wallDistance) {
        action = EDS_MAZE_GO_FRONT;
    } else if (disRight >= _wallDistance) {
        action = EDS_MAZE_GO_RIGHT;
    } else {
        action = EDS_MAZE_TURN_BACK;
    }

    _data->SetData(_Device, _DevId, 0, action);
    if ((_dbg->require(100))) _dbg->log(100,0,"Maze: [%d,%d] : (%d, %d, %d) => %d", 
                                        _Device, _DevId, 
                                        disFront, disLeft, disRight, action);
    _thisDataReady = true;

    return true;
}


bool EdsMaze::GetThreeSensorData(uint16 *disFront, uint16 *disLeft, uint16 *disRight) {
    bool showMsg = false;
    if (millis() - _lastMsgMs > 1000) {
        _dbg->log(10,0,"EdsMaze::GetThreeSensorData()");
        showMsg = true;
        _lastMsgMs = millis();
    }
    if (!ReadSensor(EDS_MAZE_FRONT_ID, disFront)) return false;
    if (!ReadSensor(EDS_MAZE_LEFT_ID, disLeft)) return false;
    if (!ReadSensor(EDS_MAZE_RIGHT_ID, disRight)) return false;    
    if (showMsg) {
        _dbg->log(10, 0, "=> %d, %d, %d", *disLeft, *disFront, *disRight);
    }
    return true;
}

bool EdsMaze::GetOneSensorData(uint16 *disFront, uint16 *disLeft, uint16 *disRight) {
    bool showMsg = false;
    if (millis() - _lastMsgMs > 1000) {
        _dbg->log(10,0,"EdsMaze::GetOneSensorData()");
        showMsg = true;
        _lastMsgMs = millis();
    }
    if (!_rs->exists(_servoId)) return false;
    //   0 - left
    //  90 - center
    // 180 - right
    // checking left, right, center
    _rs->goAngle(_servoId, (_servoReversed ? 180 : 0) , _servoMoveMs);
    delay(_servoWaitMs); // 1s is good enough; maybe add parameter to control later
    if (!ReadSensor(EDS_MAZE_FRONT_ID, disLeft)) return false;

    _rs->goAngle(_servoId, (_servoReversed ? 0 : 180) , _servoMoveMs);
    delay(_servoWaitMs); // 1s is good enough; maybe add parameter to control later
    if (!ReadSensor(EDS_MAZE_FRONT_ID, disRight)) return false;

    _rs->goAngle(_servoId, 90, _servoMoveMs); 
    delay(_servoWaitMs); // 1s is good enough; maybe add parameter to control later
    if (!ReadSensor(EDS_MAZE_FRONT_ID, disFront)) return false;

    if (showMsg) {
        _dbg->log(10, 0, "GetOneSensorData (%d) => %d, %d, %d", _servoReversed,  *disLeft,  *disFront, *disRight);
    }

    return true;
}

bool EdsMaze::ReadSensor(byte id, uint16 *data) {
    byte cmd[] = { 0xA8, 0x8A, 0x06, 0x02, 0x00, 0x02, 0x28, 0x02, 0x34, 0xED };
    cmd[4] = id;

    unsigned long startMs = millis();
    if (!_ssb->SendCommand((byte *) cmd, true)) {
        _thisDataError = true;
        _dbg->log(10, 0, "Fail getting sonic sensor data");
        return false;
    }

    unsigned long diff = millis() - startMs;
    //_dbg->msg("It taks %d ms to read PSX", diff); 

    Buffer *result = _ssb->ReturnBuffer();
    // Data returned: A8 8A 08 02 00 02 28 02 00 {*} EA ED

    *data = result->peek(8) << 8 | result->peek(9);
    return true;
}