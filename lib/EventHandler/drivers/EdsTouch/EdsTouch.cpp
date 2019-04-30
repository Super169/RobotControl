#include "EdsTouch.h"


EdsTouch::EdsTouch(EventData *data, MyDebugger *dbg, byte devId) {
    _Device = (uint8_t) EventData::DEVICE::touch;
    Config(data, dbg, devId);
    _isAvailable = true;
}

EdsTouch::~EdsTouch() {
}

void EdsTouch::Setup(uint8_t gpioPin, unsigned long touchDetectPeriod, unsigned long touchReleasePeriod) {
    if (_dbg->require(110)) _dbg->log(110, 0, "EdsTouch::Setup(GPIO: %d, Detect: %ld ms, Release: %ld ms)", gpioPin, touchDetectPeriod, touchReleasePeriod);
    _gpioPin = gpioPin;
    _touchDetectPeriod = touchDetectPeriod;
    _touchReleasePeriod = touchReleasePeriod;
    pinMode(_gpioPin, INPUT);
}

void EdsTouch::ResetTouchAction() {
    if (_dbg->require(210)) _dbg->log(210,0,"EdsTouch::ResetTouchAction\n");
    _touchStartMs = 0;
    _touchReleaseMs = 0;
    _touchCount = 0;
    _waitRelease = false;
}

bool EdsTouch::IsTouchPressed() {
    pinMode(_gpioPin,INPUT);
    return  digitalRead(_gpioPin);
}

uint8_t EdsTouch::CheckTouchAction() {

    uint8_t retCode = ETU_TOUCH_NONE;
    if (!IsReady()) return retCode;
    if (_dbg->require(210)) _dbg->log(210,0,"EdsTouch::CheckTouchAction()");

    unsigned long currMs = millis();    // for safety, use same time in ms for all checking later
    bool currStatus = IsTouchPressed();

    if (currStatus != _lastStatus) {
        if (currStatus) {
            _touchCount++;
            _touchReleaseMs = 0;
            if (!_touchStartMs) _touchStartMs = millis();
        } else {
            _touchReleaseMs = currMs;
        }
        _lastStatus = currStatus;
    }
    
    if (_touchStartMs) {
        // Touch detected
        if ((millis() - _touchStartMs) > _touchDetectPeriod) {
            if (!_waitRelease) {
                _waitRelease = true;
                // End of detection
                if (_touchCount == 1) {
                    if (currStatus) {
                        // Long hold
                        if (_dbg->require(200)) _dbg->log(200,0,"EdsTouch: Long hold");
                        retCode = ETU_TOUCH_LONG;
                    } else {
                        // Single click
                        if (_dbg->require(200)) _dbg->log(200,0,"EdsTouch: Single click");
                        retCode = ETU_TOUCH_SINGLE;
                    }
                } else if (_touchCount == 2) {
                    // double click
                    if (_dbg->require(200)) _dbg->log(200,0,"EdsTouch: Double click");
                    retCode = ETU_TOUCH_DOUBLE;
                } else {
                    // triple click
                    if (_dbg->require(200)) _dbg->log(200,0,"EdsTouch: Triple click");
                    retCode = ETU_TOUCH_TRIPLE;
                }
            } else {
                // must release for reasonable time to stop action
                if ((!currStatus) && ((currMs - _touchReleaseMs) > _touchReleasePeriod)) {
                    ResetTouchAction();
                }
            }
        }
    } 

    return retCode;
}


bool EdsTouch::GetData() {
    _thisDataReady = false;
    if (!IsReady()) return false;
    _lastDataReady = false;
    uint8_t touchAction = CheckTouchAction();
    _data->SetData(_Device, _DevId, 0, touchAction);
    _lastDataReady = true;
    _thisDataReady = true;
    _lastReportValue = touchAction;
    _lastReportMS = millis();
    if ((touchAction != 0) && _dbg->require(100)) _dbg->log(100,0,"EdsTouch::GetData() => %d", touchAction);
    return (touchAction != 0);
}

void EdsTouch::PostHandler(bool eventMatched, bool isRelated, bool pending) {
    if (!IsReady()) return;
    if (_dbg->require(210)) _dbg->log(210,0,"EdsTouch::PostHandler(%d,%d,%d)",eventMatched, isRelated, pending);
    // Cannot use generic routine as it need to check the value != 0 to confirm value is not handled
    if ((_thisDataReady) && (_lastReportValue != 0) && ((!eventMatched) || (isRelated))) {
        _nextReportMs = millis() + _delayCheckMs;
    } else {
        _nextReportMs = millis() + _continueCheckMs;
    }
}

