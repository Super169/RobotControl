#include "robot.h"


bool lastStatus = 0;
unsigned long touchStartMs = 0;
unsigned long touchReleaseMs = 0;
unsigned long touchCount = 0;
bool waitRelease = false;

uint8_t CheckTouchAction() {

    uint8_t retCode = TOUCH_NONE;
    if (!config.enableTouch()) return retCode;

    unsigned long currMs = millis();    // for safety, use same time in ms for all checking later
    bool currStatus = IsTouchPressed();

    if (currStatus != lastStatus) {
        if (currStatus) {
            touchCount++;
            touchReleaseMs = 0;
            if (!touchStartMs) touchStartMs = millis();
        } else {
            touchReleaseMs = currMs;
        }
        lastStatus = currStatus;
    }
    
    if (touchStartMs) {
        // Touch detected
        if ((millis() - touchStartMs) > config.touchDetectPeriod()) {
            if (!waitRelease) {
                waitRelease = true;
                // End of detection
                if (touchCount == 1) {
                    if (currStatus) {
                        // Long hold
                        if (debug) DEBUG.printf("Long hold\n");
                        if (config.enableOLED()) {
                            myOLED.clr(6);
                            myOLED.print(0, 6, "Long hold");
                            myOLED.show();
                        }
                        if (config.touchAction(0)) V2_GoAction(config.touchAction(0), false, NULL);
                        retCode = TOUCH_LONG;
                    } else {
                        // Single click
                        if (debug) DEBUG.printf("Single click\n");
                        if (config.enableOLED()) {
                            myOLED.clr(6);
                            myOLED.print(0, 6, "Single click");
                            myOLED.show();
                        }
                        if (config.touchAction(1)) V2_GoAction(config.touchAction(1), false, NULL);
                        retCode = TOUCH_SINGLE;
                    }
                } else if (touchCount == 2) {
                    // double click
                    if (debug) DEBUG.printf("Double click\n");
                    if (config.enableOLED()) {
                        myOLED.clr(6);
                        myOLED.print(0, 6, "Double click");
                        myOLED.show();
                    }
                    if (config.touchAction(2)) V2_GoAction(config.touchAction(2), false, NULL);
                    retCode = TOUCH_DOUBLE;
                } else {
                    // triple click
                    if (debug) DEBUG.printf("Triple click\n");
                    if (config.enableOLED()) {
                        myOLED.clr(6);
                        myOLED.print(0, 6, "Triple click");
                        myOLED.show();
                    }
                    if (config.touchAction(3)) V2_GoAction(config.touchAction(3), false, NULL);
                    retCode = TOUCH_TRIPLE;
                }
            } else {
                // must release for reasonable time to stop actino
                if ((!currStatus) && ((currMs - touchReleaseMs) > config.touchReleasePeriod())) {
                    ResetTouchAction();
                }
            }
        }
    } 

    return retCode;
}

void ResetTouchAction() {
    if (debug) DEBUG.printf("ResetTouchAction\n");
    touchStartMs = 0;
    touchReleaseMs = 0;
    touchCount = 0;
    waitRelease = false;
}

bool IsTouchPressed() {
    pinMode(TOUCH_GPIO,INPUT);
    return  digitalRead(TOUCH_GPIO);
}