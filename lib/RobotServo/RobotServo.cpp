#include "RobotServo.h" 

RobotServo::RobotServo() {
    _initialized = false;
}


RobotServo::~RobotServo() {
    
}

void RobotServo::showInfo() {
    _servo.showInfo();
}

bool RobotServo::init(byte maxId, byte maxRetry) {
    if (_initialized) return false;
    _maxId = maxId;
    _maxRetry = maxRetry;
    _servo.init(maxId, maxRetry);
    _initialized = true;
    return true;
}


bool RobotServo::begin(Stream *busPort, Stream *debugPort) {
    return _servo.begin(busPort, debugPort);
}

bool RobotServo::end() {
    return _servo.end();
}

bool RobotServo::reset() {
    return _servo.reset();
}
