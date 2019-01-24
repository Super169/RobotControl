#include "EdsMpu6050.h"



EdsMpu6050::EdsMpu6050(EventData *data, MyDebugger *dbg, byte devId) {
    _Device = (uint8_t) EventData::DEVICE::mpu;
    Config(data, dbg, devId);
    _isAvailable = false;
}

EdsMpu6050::~EdsMpu6050() {
}

void EdsMpu6050::Setup(uint8_t i2cAddr, uint16_t threadhold, uint16_t elapseMs) {
    if (_dbg->require(110)) _dbg->log(110, 0, "EdsMpu6050::Setup(0x%02X, threadhold: %d ms, elapse: %d ms)", i2cAddr, threadhold, elapseMs);
    _i2cAddr = i2cAddr;
    _threadhold = threadhold;
    _continueCheckMs  = elapseMs;
    _delayCheckMs = EMPU_DELAY_CHECK_MS;

    _data->SetThreadhold(_Device, threadhold);

    Wire.begin();
    // Check if MPU available
    Wire.beginTransmission(_i2cAddr);
    uint8_t i2cError = Wire.endTransmission();
    _isAvailable = (i2cError == 0);
    if (_isAvailable) {
        Wire.beginTransmission(_i2cAddr);
        Wire.write(0x6B);  // PWR_MGMT_1 register
        Wire.write(0);     // set to zero (wakes up the MPU-6050)
        Wire.endTransmission(true);
        if (_dbg->require(10)) _dbg->log(10, 0, "MPU6050 initialized.");
    } else {
        if (_dbg->require(10)) _dbg->log(10, 0, "MPU6050 is not available.");
    }
}


bool EdsMpu6050::GetData() {
    _thisDataReady = false;
    _thisDataError = false;
    if (!IsReady())  return false;

    if (!GetMpuData()) {
        _thisDataError = true;
        return false;
    }

    _data->SetData(_Device, _DevId, 0, _ax);
    _data->SetData(_Device, _DevId, 1, _ay);
    _data->SetData(_Device, _DevId, 2, _az);
    if (_dbg->require(100)) _dbg->log(100,0,"MPU 6050: ( %d , %d , %d )", _ax, _ay, _az);

    _thisDataReady = true;

    return true;
}

/*
void EdsMpu6050::PostHandler(bool eventMatched, bool isRelated, bool pending) {
    _nextReportMs = millis() + _elapseMs;    
}
*/

// public function for using outside EDS
bool EdsMpu6050::GetMpuData() {
    if (!IsAvailable()) return false;
    _ax = _ay = _az = _tmp = _gx = _gy = _gz = 0;
    Wire.beginTransmission(_i2cAddr);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(_i2cAddr, (size_t) EMPU_DATA_SIZE,true);  // request a total of 14 registers
    memset(_mpuBuffer, 0, EMPU_DATA_SIZE);
    for (int i = 0; i < EMPU_DATA_SIZE; i++) {
        if (Wire.available()) {
            _mpuBuffer[i] = Wire.read();
        } else {
            _dbg->printf("Incomplete MPU data, only %d bytes returned\n", i);
            memset(_mpuBuffer, 0, EMPU_DATA_SIZE);
            return false;
        }
    }
    _ax = _mpuBuffer[0] << 8 | _mpuBuffer[1];
    _ay = _mpuBuffer[2] << 8 | _mpuBuffer[3];
    _az = _mpuBuffer[4] << 8 | _mpuBuffer[5];
    _gx = _mpuBuffer[8] << 8 | _mpuBuffer[9];
    _gy = _mpuBuffer[10] << 8 | _mpuBuffer[11];
    _gz = _mpuBuffer[12] << 8 | _mpuBuffer[13];

    return true;
}