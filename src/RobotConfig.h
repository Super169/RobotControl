#ifndef _ROBOT_CONFIG_H_
#define _ROBOT_CONFIG_H_

#include <ESP8266WiFi.h>
#include <FS.h>
#include "RESULT.h"

#define CURRENT_VERSION         1

#define DEFAULT_ENABLE_DEBUG    true
#define DEFAULT_CONNECT_ROUTER  true
#define DEFAULT_ENABLE_OLED     true
#define DEFAULT_ENABLE_TOUCH    false
#define DEFAULT_REF_VOLTAGE     1100        // aroud 3.2v for A0 of D1 mini, 1.0v for ADC of ESP-12
#define DEFAULT_MIN_VOLTAGE     600
#define DEFAULT_MAX_VOLTAGE     840

// #define DEFAULT_ALARM_VOLTAGE   650
// #define DEFAULT_ALARM_MP3       0
// #define DEFAULT_ALARM_INTERVAL  30
#define DEFAULT_BATTERY_NORMAL_SEC  5
#define DEFAULT_BATTERY_ALARM_SEC   30
#define MIN_BATTERY_CHECK_FREQ      1       // At least one per second
#define MAX_BATTERY_CHECK_FREQ      50      // Don't make it too busy, 50 is more than enough

#define DEFAULT_MAX_SERVO       16

#define DEFAULT_MAX_DETECT_RETRY        2
#define DEFAULT_MAX_COMMAND_WAIT_MS     2
#define DEFAULT_MAX_COMMAND_RETRY       10

#define DEFAULT_MP3_ENABLED     true
#define DEFAULT_MP3_VOLUME      20
#define DEFAULT_MP3_STARTUP     1

#define DEFAULT_ENABLE_MPU6050  false
#define DEFAULT_MPU_CHECK_FREQ      10
#define DEFAULT_POSITION_CHECK_FREQ 20
/*
#define DEFAULT_AUTO_STAND      false
#define DEFAULT_AUTO_FACE_UP    0
#define DEFAULT_AUTO_FACE_DOWN  0
*/

#define DEFAULT_TOUCH_NO_ACTION 0
#define DEFAULT_TOUCH_DETECT_PERIOD  1500
#define DEFAULT_TOUCH_RELEASE_PERIOD 1000
#define MIN_TOUCH_PERIOD             100

#define DEFAULT_ENABLE_PSX_BUTTON       false
#define DEFAULT_PSX_CHECK_MS            20
#define DEFAULT_PSX_IGNORE_REPEAT_MS    200


#define RC_RECORD_SIZE          60
#define RC_CONFIG_DATA_SIZE     56
#define RC_VERSION              4
#define RC_ENABLE_DEBUG         5
#define RC_CONNECT_ROUTER       6
#define RC_ENABLE_OLED          7
// #define RC_ENABLE_TOUCH         8   // V0: ????
#define RC_REF_VOLTAGE          10
#define RC_MIN_VOLTAGE          12
#define RC_MAX_VOLTAGE          14
#define RC_ALARM_VOLTAGE        16  // V0: Obsolete
#define RC_ALARM_MP3            18  // V0: Obsolete
#define RC_BATTERY_NORAML_SEC   18  // V1: New
#define RC_ALARM_INTERVAL       19  // V0: Rename
#define RC_BATTERY_ALARM_SEC    19  // V1: Rename
#define RC_MAX_SERVO            21
#define RC_MAX_DETECT_RETRY     22
#define RC_MAX_COMMAND_WAIT_MS  23
#define RC_MAX_COMMAND_RETRY    24
#define RC_MP3_ENABLED          31
#define RC_MP3_VOLUME           32
#define RC_MP3_STARTUP          33
#define RC_ENABLE_PSX_BUTTON    35  // V1: New
#define RC_PSX_CHECK_MS         36
#define RC_PSX_IGNORE_REPEAT_MS 38

#define RC_AUTO_STAND           41  // V0: Rename
#define RC_ENABLE_MPU6050       41  // V1: Rename

#define RC_AUTO_FACE_UP         42  // V0: Obsolete
#define RC_MPU_CHECK_FREQ       42  // V1: Moved from 52

#define RC_AUTO_FACE_DOWN       43  // V0: Obsolete
#define RC_POSITION_CHECK_FREQ  43  // V1: Moved from 53

#define V0_TOUCH_ACTION			44  // V0: Obsolete
#define RC_ENABLE_TOUCH         44  // V1: New

#define RC_TOUCH_DETECT_PERIOD  48   
#define RC_TOUCH_RELEASE_PERIOD 50   
#define V0_MPU_CHECK_FREQ       52  // Moved to 42
#define V0_POSITION_CHECK_FREQ  53  // Moved to 43

#define V0_TOUCH_ACTION_CNT		4   // V0: Obsolete




class RobotConfig {

    const char* _configFileName = "/robot/config.dat";

    public:
        RobotConfig();
        RobotConfig(HardwareSerial *hsDebug);
        void initConfig();
        bool readConfig();
        byte writeConfig();
		void dumpConfig();

        uint8_t * Data() { return (uint8_t *) _data; }

        bool setDebug(bool debug);
        bool setRouter(bool value);
        bool setOLED(bool value);
		bool setTouch(bool value);

        void setRefVoltage(uint16_t refVoltage); 
        void setMinVoltage(uint16_t minVoltage); 
        void setMaxVoltage(uint16_t maxVoltage); 
        void setBatteryNormalSec(uint8_t value);
        void setBatteryAlarmSec(uint8_t value);
        void setBattery(uint16_t refVoltage, uint16_t minVoltage, uint16_t maxVoltage, uint8_t normalSec, uint8_t alarmSec);

        /*        
        void setAlarmVoltage(uint16_t alarmVoltage); 
        void setVoltageAlarmMp3(uint8_t mp3);
        void setVoltageAlarmInterval(uint8_t interval);
        void setVoltage(uint16_t refVoltage, uint16_t minVoltage, uint16_t maxVoltage, uint16_t alarmVoltage, uint8_t mp3, uint8_t interval);
        */




        void setMaxServo(uint8_t maxServo);
        void setMaxCommandWaitMs(uint8_t maxCommandWaitMs);
        void setMaxCommandRetry(uint8_t maxCommandRetry);
        void setMaxDetectRetry(uint8_t maxDetectRetry);
        void setMp3Enabled(bool enabled);
        void setMp3Volume(uint8_t volume);
        void setMp3Startup(uint8_t mp3);
        void setMpu6050(bool enabled);
        /*       
        void setAutoStand(bool autoStand);
        void setAutoFaceUp(uint8_t faceUp);
        void setAutoFaceDown(uint8_t faceDown);
        inline void setAutoStand(bool autoStand, uint8_t faceUp, uint8_t faceDown) { setAutoStand(autoStand); setAutoFaceUp(faceUp); setAutoFaceDown(faceDown);}
        */
        /*
		void setTouchAction(uint8_t id, uint8_t value);
		inline void setTouch(uint8_t action0, uint8_t action1, uint8_t action2, uint8_t action3) {
			// setTouch((action0 || action1 || action2 || action3));
			setTouchAction(0, action0);
			setTouchAction(1, action1);
			setTouchAction(2, action2);
			setTouchAction(3, action3);
		}
        */
		void setTouchDetectPeriod(uint16_t detectPeriod);
        void setTouchReleasePeriod(uint16_t releasePeriod);
        void setMpuCheckFreq(uint8_t checkFreq);
        void setPositionCheckFreq(uint8_t checkFreq);

        void setPsxButton(bool enabled);
        void setPsxCheckMs(uint16_t value);
        void setPsxIgnoreRepeatMs(uint16_t value);
        inline void setPsx(bool enablePsxButton, uint16_t psxCheckMs, uint16_t psxIgnoreRepeatMs) {
            setPsxButton(enablePsxButton);
            setPsxCheckMs(psxCheckMs);
            setPsxIgnoreRepeatMs(psxIgnoreRepeatMs);
        }


        bool enableDebug() { return _data[RC_ENABLE_DEBUG]; }
        bool connectRouter() { return _data[RC_CONNECT_ROUTER]; }
        bool enableOLED() { return _data[RC_ENABLE_OLED]; }
        bool enableTouch();
        uint16_t refVoltage() { return getUint16_t(RC_REF_VOLTAGE); }
        uint16_t minVoltage() { return getUint16_t(RC_MIN_VOLTAGE); }
        uint16_t maxVoltage() { return getUint16_t(RC_MAX_VOLTAGE); }
        uint8_t batteryNormalSec() { return _data[RC_BATTERY_NORAML_SEC]; }
        uint8_t batteryAlarmSec() { return _data[RC_BATTERY_ALARM_SEC]; }
        // uint16_t alarmVoltage() { return getUint16_t(RC_ALARM_VOLTAGE); }
        // uint8_t voltageAlarmMp3() { return _data[RC_ALARM_MP3]; }
        // uint8_t voltageAlarmInterval() { return _data[RC_ALARM_INTERVAL]; }

        uint8_t maxServo() { return _data[RC_MAX_SERVO]; }
        uint8_t maxDetectRetry() { return _data[RC_MAX_DETECT_RETRY]; }
        uint8_t maxCommandWaitMs() { return _data[RC_MAX_COMMAND_WAIT_MS]; }
        uint8_t maxCommandRetry() { return _data[RC_MAX_COMMAND_RETRY]; }

        bool mp3Enabled() { return _data[RC_MP3_ENABLED]; }
        uint8_t mp3Volume() { return _data[RC_MP3_VOLUME]; }
        uint8_t mp3Startup() { return _data[RC_MP3_STARTUP]; }

        /*
        bool autoStand() { return _data[RC_AUTO_STAND]; }
        uint8_t faceUpAction() { return _data[RC_AUTO_FACE_UP]; }
        uint8_t faceDownAction() { return _data[RC_AUTO_FACE_DOWN]; }
        */
        bool enableMpu() { return _data[RC_ENABLE_MPU6050]; }
        uint8_t mpuCheckFreq() { return _data[RC_MPU_CHECK_FREQ]; }
        uint8_t positionCheckFreq() { return _data[RC_POSITION_CHECK_FREQ]; }

		uint8_t touchAction(uint8_t id);
        uint16_t touchDetectPeriod() { return getUint16_t(RC_TOUCH_DETECT_PERIOD); }
        uint16_t touchReleasePeriod() { return getUint16_t(RC_TOUCH_RELEASE_PERIOD); }
        
        bool enablePsxButton() { return _data[RC_ENABLE_PSX_BUTTON]; }
        uint16_t psxCheckMs() { return getUint16_t(RC_PSX_CHECK_MS); }
        uint16_t psxIgnoreRepeatMs() { return getUint16_t(RC_PSX_IGNORE_REPEAT_MS); }

    private:
        void initObject(HardwareSerial *hsDebug);
        void checkConversion();
        void checkConfig();
        void setUint16_t(uint8_t offset, uint16_t value);
        uint16_t getUint16_t(uint8_t offset);

        HardwareSerial *_dbg;
        
        uint8_t _data[RC_RECORD_SIZE];

};


#endif