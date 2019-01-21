#ifndef _ROBOT_CONFIG_H_
#define _ROBOT_CONFIG_H_

#include <ESP8266WiFi.h>
#include <FS.h>
#include "RESULT.h"

#define CURRENT_VERSION         1

#define DEFAULT_ENABLE_DEBUG            true
#define DEFAULT_CONNECT_ROUTER          true
#define DEFAULT_ENABLE_OLED             true


// #define DEFAULT_ALARM_VOLTAGE   650
// #define DEFAULT_ALARM_MP3       0
// #define DEFAULT_ALARM_INTERVAL  30

#define DEFAULT_BATTERY_REF_VOLTAGE     1100        // aroud 3.2v for A0 of D1 mini, 1.0v for ADC of ESP-12
#define DEFAULT_BATTERY_MIN_VALUE       600
#define DEFAULT_BATTERY_MAX_VALUE       840
#define DEFAULT_BATTERY_NORMAL_SEC      5
#define DEFAULT_BATTERY_ALARM_SEC       30
#define MIN_BATTERY_CHECK_FREQ          1       // At least one per second
#define MAX_BATTERY_CHECK_FREQ          50      // Don't make it too busy, 50 is more than enough

#define DEFAULT_MAX_SERVO               16
#define DEFAULT_MAX_DETECT_RETRY        2
#define DEFAULT_MAX_COMMAND_WAIT_MS     2
#define DEFAULT_MAX_COMMAND_RETRY       10

#define DEFAULT_MP3_ENABLED             true
#define DEFAULT_MP3_VOLUME              20
#define DEFAULT_MP3_STARTUP             1

#define DEFAULT_MPU_ENABLED             false
#define DEFAULT_MPU_CHECK_FREQ          10
#define DEFAULT_MPU_POSITION_CHECK_FREQ 20
/*
#define DEFAULT_AUTO_STAND      false
#define DEFAULT_AUTO_FACE_UP    0
#define DEFAULT_AUTO_FACE_DOWN  0
*/

#define DEFAULT_TOUCH_ENABLED           false
#define DEFAULT_TOUCH_NO_ACTION         0
#define DEFAULT_TOUCH_DETECT_PERIOD     1500
#define DEFAULT_TOUCH_RELEASE_PERIOD    1000
#define MIN_TOUCH_PERIOD                100

#define DEFAULT_PSX_ENABLED             false
#define DEFAULT_PSX_CHECK_MS            20
#define DEFAULT_PSX_NO_EVENT_MS         100
#define DEFAULT_PSX_IGNORE_REPEAT_MS    200
#define DEFAULT_PSX_SHOCK               false
#define MIN_PSX_MS                      10
#define MIN_PSX_IGNORE_REPEAT_MS        50

#define RC_RECORD_SIZE                  60
#define RC_CONFIG_DATA_SIZE             56
#define RC_VERSION                      4
#define RC_ENABLE_DEBUG                 5
#define RC_CONNECT_ROUTER               6
#define RC_ENABLE_OLED                  7

#define RC_BATTERY_REF_VOLTAGE          10
#define RC_BATTERY_MIN_VALUE            12
#define RC_BATTERY_MAX_VALUE            14
#define RC_BATTERY_NORAML_SEC           18  // V1: New
#define RC_BATTERY_ALARM_SEC            19  // V1: Rename

#define RC_MAX_SERVO                    21
#define RC_MAX_DETECT_RETRY             22
#define RC_MAX_COMMAND_WAIT_MS          23
#define RC_MAX_COMMAND_RETRY            24

#define RC_MP3_ENABLED                  31
#define RC_MP3_VOLUME                   32
#define RC_MP3_STARTUP                  33

#define RC_PSX_ENABLED                  35  // V1: New
#define RC_PSX_CHECK_MS                 36
#define RC_PSX_NO_EVENT_MS              37
#define RC_PSX_IGNORE_REPEAT_MS         38
#define RC_PSX_SHOCK                    40

#define RC_MPU_ENABLED                  41  // V1: Rename
#define RC_MPU_CHECK_FREQ               42  // V1: Moved from 52
#define RC_MPU_POSITION_CHECK_FREQ      43  // V1: Moved from 53


#define RC_TOUCH_ENABLED                47  // V1: New
#define RC_TOUCH_DETECT_PERIOD          48   
#define RC_TOUCH_RELEASE_PERIOD         50   

#define RC_SONIC_ENABLED                53


// V0 setting to be removed later
#define V0_ENABLE_TOUCH                 8   // V0: ????
#define V0_ALARM_VOLTAGE                16  // V0: Obsolete
#define V0_ALARM_MP3                    18  // V0: Obsolete
#define V0_AUTO_STAND                   41  // V0: Rename
#define V0_AUTO_FACE_UP                 42  // V0: Obsolete
#define V0_AUTO_FACE_DOWN               43  // V0: Obsolete
#define V0_TOUCH_ACTION			        44  // V0: Obsolete
#define V0_MPU_CHECK_FREQ               52  // Moved to 42
#define V0_POSITION_CHECK_FREQ          53  // Moved to 43
#define V0_TOUCH_ACTION_CNT		        4   // V0: Obsolete


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

        void setBatteryRefVoltage(uint16_t refVoltage); 
        void setBatteryMinValue(uint16_t minVoltage); 
        void setBatteryMaxValue(uint16_t maxVoltage); 
        void setBatteryNormalSec(uint8_t value);
        void setBatteryAlarmSec(uint8_t value);
        void setBattery(uint16_t refVoltage, uint16_t minVoltage, uint16_t maxVoltage, uint8_t normalSec, uint8_t alarmSec);

        void setMaxServo(uint8_t maxServo);
        void setMaxCommandWaitMs(uint8_t maxCommandWaitMs);
        void setMaxCommandRetry(uint8_t maxCommandRetry);
        void setMaxDetectRetry(uint8_t maxDetectRetry);

        void setMp3Enabled(bool enabled);
        void setMp3Volume(uint8_t volume);
        void setMp3Startup(uint8_t mp3);
        inline void setMp3(bool enabled, uint8_t volume, uint8_t mp3) {
            setMp3Enabled(enabled);
            setMp3Volume(volume);
            setMp3Startup(mp3);
        }

		bool setTouchEnabled(bool value);
		void setTouchDetectPeriod(uint16_t detectPeriod);
        void setTouchReleasePeriod(uint16_t releasePeriod);
        inline void setTouch(bool enabled, uint16_t detectPeriod, uint16_t releasePeriod) {
            setTouchEnabled(enabled);
            setTouchDetectPeriod(detectPeriod);
            setTouchReleasePeriod(releasePeriod);
        }

        void setMpuEnabled(bool enabled);
        void setMpuCheckFreq(uint8_t checkFreq);
        void setMpuPositionCheckFreq(uint8_t positionCheckFreq);
        inline void setMpu(bool enabled, uint8_t checkFreq, uint8_t positionCheckFreq) {
            setMpuEnabled(enabled);
            setMpuCheckFreq(checkFreq);
            setMpuPositionCheckFreq(positionCheckFreq);
        }

        void setPsxEnabled(bool enabled);
        void setPsxCheckMs(uint8_t value);
        void setPsxNoEventMs(uint8_t value);
        void setPsxIgnoreRepeatMs(uint16_t value);
        void setPsxShock(bool enabled);
        inline void setPsx(bool enablePsxButton, uint8_t psxCheckMs, uint8_t psxNoEventMs, uint16_t psxIgnoreRepeatMs, bool psxShock) {
            setPsxEnabled(enablePsxButton);
            setPsxCheckMs(psxCheckMs);
            setPsxNoEventMs(psxNoEventMs);
            setPsxIgnoreRepeatMs(psxIgnoreRepeatMs);
            setPsxShock(psxShock);
        }

        void setSonicEnabled(bool enabled);


        bool enableDebug() { return _data[RC_ENABLE_DEBUG]; }
        bool connectRouter() { return _data[RC_CONNECT_ROUTER]; }
        bool enableOLED() { return _data[RC_ENABLE_OLED]; }
        uint16_t batteryRefVoltage() { return getUint16_t(RC_BATTERY_REF_VOLTAGE); }
        uint16_t batteryMinValue() { return getUint16_t(RC_BATTERY_MIN_VALUE); }
        uint16_t batteryMaxValue() { return getUint16_t(RC_BATTERY_MAX_VALUE); }
        uint8_t batteryNormalSec() { return _data[RC_BATTERY_NORAML_SEC]; }
        uint8_t batteryAlarmSec() { return _data[RC_BATTERY_ALARM_SEC]; }

        uint8_t maxServo() { return _data[RC_MAX_SERVO]; }
        uint8_t maxDetectRetry() { return _data[RC_MAX_DETECT_RETRY]; }
        uint8_t maxCommandWaitMs() { return _data[RC_MAX_COMMAND_WAIT_MS]; }
        uint8_t maxCommandRetry() { return _data[RC_MAX_COMMAND_RETRY]; }

        bool mp3Enabled() { return _data[RC_MP3_ENABLED]; }
        uint8_t mp3Volume() { return _data[RC_MP3_VOLUME]; }
        uint8_t mp3Startup() { return _data[RC_MP3_STARTUP]; }

        bool mpuEnabled() { return _data[RC_MPU_ENABLED]; }
        uint8_t mpuCheckFreq() { return _data[RC_MPU_CHECK_FREQ]; }
        uint8_t mpuPositionCheckFreq() { return _data[RC_MPU_POSITION_CHECK_FREQ]; }

        bool touchEnabled() { return _data[RC_TOUCH_ENABLED]; }
        uint16_t touchDetectPeriod() { return getUint16_t(RC_TOUCH_DETECT_PERIOD); }
        uint16_t touchReleasePeriod() { return getUint16_t(RC_TOUCH_RELEASE_PERIOD); }
        
        bool psxEnabled() { return _data[RC_PSX_ENABLED]; }
        uint8_t psxCheckMs() { return _data[RC_PSX_CHECK_MS]; }
        uint8_t psxNoEventMs() { return _data[RC_PSX_NO_EVENT_MS]; }
        uint16_t psxIgnoreRepeatMs() { return getUint16_t(RC_PSX_IGNORE_REPEAT_MS); }
        bool psxShock() { return _data[RC_PSX_SHOCK]; }

        bool sonicEnabled() { return _data[RC_SONIC_ENABLED]; }

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