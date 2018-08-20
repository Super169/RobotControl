#include "SimpleWiFiManager.h"

size_t SimpleWiFiManager::write(byte data) {
    if (!_wifiClientConnected) return 0;
    if (!_wifiClient.connected()) {
        _wifiClientConnected = false;
        return 0;
    }
    return _wifiClient.write(data);
}

size_t SimpleWiFiManager::write(byte *data, size_t cnt) {
    if (!_wifiClientConnected) return 0;
    if (!_wifiClient.connected()) {
        _wifiClientConnected = false;
        return 0;
    }
    return _wifiClient.write(data, cnt);
}


size_t SimpleWiFiManager::udpSendPacket(const char* target, const char *data) {
    _udpClient.beginPacket(target, udpTxPort());
    size_t cnt = _udpClient.print(data);
	_udpClient.endPacket();
    return cnt;
}

uint8_t SimpleWiFiManager::checkData() {
    if (_wifiServerRunning) {
        bool newConnection = true;
        if (_wifiClientConnected) {
            if (_wifiClient.connected()) {
                newConnection = false;
            }else {
                _wifiClientConnected = false;
                _dbg.msg("Wifi client disconnected.");
            }
        }
        if (!_wifiClientConnected) {
            _wifiClient = _wifiServer->available();
        }
        if (_wifiClient) {
            if (_wifiClient.connected()) {
                _wifiClientConnected = true;
                if (newConnection) {
                    _dbg.msg("Wifi client connected.");
                }
                while (_wifiClient.available()) {
                    _buffer.write(_wifiClient.read());
                    // Add 1ms delay to make sure data transmission is completed.
                    if (!_wifiClient.available()) delay(1);
                }
            } 
        }
    }

    if (_udpClientRunning) {
        int packetSize = _udpClient.parsePacket();
        if (packetSize) {
            byte packet[packetSize];
            int udpRxCnt = _udpClient.read(packet, packetSize);
            for (int i =  0; i < udpRxCnt; i++) _buffer.write(packet[i]);
        }
    }
    
    return (_buffer.available());
}