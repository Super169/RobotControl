#include "EdsTouch.h"


EdsTouch::EdsTouch(EventData *data, MyDebugger *dbg, byte devId) {
    _Device = (uint8_t) EventData::DEVICE::touch;
    Config(data, dbg, devId);
}

EdsTouch::~EdsTouch() {
}

void EdsTouch::Setup(uint8_t gpioPin, unsigned long touchDetectPeriod, unsigned touchReleasePeriod) {
    _gpioPin = gpioPin;
    _touchDetectPeriod = touchDetectPeriod;
    _touchReleasePeriod = touchReleasePeriod;
    pinMode(_gpioPin, INPUT);
}

void EdsTouch::ResetTouchAction() {
    _dbg->msg("EdsTouch::ResetTouchAction\n");
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
                        _dbg->msg("EdsTouch: Long hold");
                        retCode = ETU_TOUCH_LONG;
                    } else {
                        // Single click
                        _dbg->msg("EdsTouch: Single click");
                        retCode = ETU_TOUCH_SINGLE;
                    }
                } else if (_touchCount == 2) {
                    // double click
                    _dbg->msg("EdsTouch: Double click\n");
                    retCode = ETU_TOUCH_DOUBLE;
                } else {
                    // triple click
                    _dbg->msg("EdsTouch: Triple click\n");
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
    return (touchAction != 0);
}

void EdsTouch::PostHandler(bool eventMatched, bool isRelated) {
    if (!IsReady()) return;
    // wait longer if 
    //   - button pressed, and no event required or handled: i.e. !eventMatched
    if ((_thisDataReady) && (_lastReportValue != 0) && ((!eventMatched) || (isRelated))) {
        _nextReportMs = millis() + EDS_DELAY_CHECK_MS;
    } else {
        _nextReportMs = millis() + EDS_CONTINUE_CHECK_MS;
    }
}



