#include "SimpleWiFiManager.h"

SimpleWiFiManager::SimpleWiFiManager() {
	_configReady = false;
	_mode = SWFM_MODE_NONE;
	_serverRunning = false;
	_mdCnt = 0;
    _md[_mdCnt++] = &_wifi.varMajor;
    _md[_mdCnt++] = &_wifi.varMinor;
    _md[_mdCnt++] = &_wifi.enableRouter;
    _md[_mdCnt++] = &_wifi.ssid;
    _md[_mdCnt++] = &_wifi.password;
    _md[_mdCnt++] = &_wifi.routerTimeout;
    _md[_mdCnt++] = &_wifi.enableAP;
    _md[_mdCnt++] = &_wifi.apName;
    _md[_mdCnt++] = &_wifi.apKey;
    _md[_mdCnt++] = &_wifi.enableServer;
    _md[_mdCnt++] = &_wifi.serverPort;
    _md[_mdCnt++] = &_wifi.enableUDP;
    _md[_mdCnt++] = &_wifi.udpRxPort;
    _md[_mdCnt++] = &_wifi.udpTxPort;
	_chipId = myUtil::getInt64String(myUtil::getDeviceId());
	_buffer.init(256);

	_wifiServerEnabled = false;
	_wifiServerPort = 0;
	_wifiServerRunning = false;
	_wifiClientConnected = false;

	_udpClientRunning = false;
}

SimpleWiFiManager::~SimpleWiFiManager() {
	
}

void SimpleWiFiManager::setDebug(Stream *dbg, bool enableDebug) {
	_dbg.setOutput(dbg, enableDebug);
}

void SimpleWiFiManager::enableDebug(bool value) {
	_dbg.enableDebug(value);
}

void SimpleWiFiManager::dumpSettings() {
	if (!_dbg.isEnabled()) return;

#ifdef _ENABLE_TRACE_
	_dbg.println("SWFM->dumpSetting");
#endif

	_dbg.println("Config setting: ");
    for (int idx = 0; idx < _mdCnt; idx++) {
        _dbg.printf("%s (%s) : %s\n", _md[idx]->label().c_str(), 
		                              _md[idx]->key().c_str(),
		                              _md[idx]->getPrintable().c_str());
    }

}

uint8_t *SimpleWiFiManager::getConfig() {
	mdToArray();
	return _file.buffer;
}

bool SimpleWiFiManager::setConfig(uint8_t *data) {
	memcpy(_file.buffer, data, SWFM_CONFIG_FILE_SIZE);
	mdFromArray();
	return saveConfig();
}

bool SimpleWiFiManager::resetDefault() {
	setDefaultConfig();
	return saveConfig();
}

void SimpleWiFiManager::setWiFiServer(uint16_t port) {
	_wifiServerPort = port;
	_wifiServerEnabled = true;
}

bool SimpleWiFiManager::begin(int preferMode) {
#ifdef _ENABLE_TRACE_
	_dbg.println("SWFM->begin");
#endif

	if (!_configReady) {
		readConfig();
		if (!_configReady) setDefaultConfig();
	}

	dumpConfig();

	if (isReady()) return true;

	if ((preferMode == SWFM_MODE_NONE) || (preferMode == SWFM_MODE_ROUTER))  {

		if (_wifi.enableRouter.getBool()) {

			if (!myUtil::isEmpty(_wifi.ssid.getString())) {

				WiFi.begin(_wifi.ssid.getString().c_str(), (myUtil::isEmpty(_wifi.password.getString()) ? NULL : _wifi.password.getString().c_str()));
				_dbg.msg("Connecting to %s", _wifi.ssid.getString().c_str());
				unsigned long endMs = millis() + _wifi.routerTimeout.getInt() * 1000;
				while ((millis() < endMs) && (WiFi.status() != WL_CONNECTED)) {
					delay(5000);
					_dbg.write('.');
				}
				_dbg.println();
				if (WiFi.status() == WL_CONNECTED) {
					_mode = SWFM_MODE_ROUTER;
					_dbg.msg("Connected to %s with IP %s", _wifi.ssid.getString().c_str(), ip().c_str());
				}
			}

		}

	}

	if (!isReady()) {
		if (((preferMode == SWFM_MODE_NONE) || (preferMode == SWFM_MODE_AP)) && (_wifi.enableAP.getBool()))  {
			if (myUtil::isEmpty(_wifi.apName.getString())) {
				_wifi.apName.getString() = defaultAPName();
			}
			// becareful, empty string is different from NULL for AP Key
			// You cannot enter empty string for connection, so it has to set NULL 
			if (WiFi.softAP(_wifi.apName.getString().c_str(), (myUtil::isEmpty(_wifi.apKey.getString()) ? NULL : _wifi.apKey.getString().c_str()))) {
				_mode = SWFM_MODE_AP;
				_dbg.msg("SoftAP [%s:%s] started with IP %s", 
				         _wifi.apName.getString().c_str(), (myUtil::isEmpty(_wifi.apKey.getString()) ? "<<NULL>>" : _wifi.apKey.getString().c_str()),  ip().c_str());
			}
		}
	}

	if (!isReady()) return false;

	if (_wifi.enableServer.getBool()) {
		if (MDNS.begin(_wifi.apName.getString().c_str())) {
			_dbg.msg("MDNS responder started : %s", _wifi.apName.getString().c_str());
		}

		_httpServer.reset(new ESP8266WebServer((int) _wifi.serverPort.getInt()));

		_httpServer->on(String(F("/")), std::bind(&SimpleWiFiManager::handleRoot, this));
		_httpServer->onNotFound (std::bind(&SimpleWiFiManager::handleNotFound, this));

		_httpServer->begin();

		_dbg.msg("HTTP Server running at port: %d", (uint16_t) _wifi.serverPort.getInt());
		_serverRunning = true;
	}

	if (_wifiServerEnabled) {
		_wifiServer.reset(new WiFiServer(_wifiServerPort));
		_wifiServer->begin();
		_dbg.msg("WiFi Server running at port: %d", _wifiServerPort);
		_wifiServerRunning = true;
	}

	if (_wifi.enableUDP.getBool()) {
		uint16_t udpRxPort = (uint16_t) _wifi.udpRxPort.getInt();
		if (udpRxPort) {
			_udpClient.begin(udpRxPort);
			_dbg.msg("UDP Client running at port: %d", udpRxPort);
			_udpClientRunning = true;
		}
	}

	return true;
}

void SimpleWiFiManager::stopServer() {
	if (_serverRunning) _httpServer->stop();
	_serverRunning = false;
}


bool SimpleWiFiManager::isReady() {
	return ((_mode == SWFM_MODE_ROUTER) || (_mode == SWFM_MODE_AP));
}

String SimpleWiFiManager::ip() {
	switch (_mode) {
		case SWFM_MODE_ROUTER:
			return WiFi.localIP().toString();
		case SWFM_MODE_AP:
			return WiFi.softAPIP().toString();
	}
	return "";
}

bool SimpleWiFiManager::readConfig() {
#ifdef _ENABLE_TRACE_
	_dbg.println("SWFM->readConfig");
#endif

	WIFI_SETTING fileData;

	setDefaultConfig();
    if (!myUtil::readSPIFFS(SWFM_CONFIG_FILE, (char *) fileData.buffer, SWFM_CONFIG_FILE_SIZE)) return false;
	if ( (fileData.buffer[SWFM_OFFSET_A9] == 0xA9) && 
	     (fileData.buffer[SWFM_OFFSET_9A] == 0x9A) &&
         (fileData.buffer[SWFM_OFFSET_END_BYTE] == 0xED) &&
		 (fileData.buffer[SWFM_OFFSET_VER_MAJOR] == (uint8_t) _wifi.varMajor.getInt())) {

		memcpy(_file.buffer, fileData.buffer, SWFM_CONFIG_FILE_SIZE);
		mdFromArray();
		_configReady = true;
	} else {
		_dbg.println("\nConfig file not matched.");
		_dbg.printf("A9 9A - %02X %02X \n",fileData.buffer[SWFM_OFFSET_A9], fileData.buffer[SWFM_OFFSET_9A]);
		_dbg.printf("ED - %02X \n",fileData.buffer[SWFM_OFFSET_END_BYTE]);
		_dbg.printf("Major Version - %d (%s) \n",
		             fileData.buffer[SWFM_OFFSET_VER_MAJOR],
					 _wifi.varMajor.getPrintable().c_str());

	}
    return true;	
}

void SimpleWiFiManager::mdFromArray() {
	for (int idx = 0; idx < _mdCnt; idx++) {
		_md[idx]->fromBuffer(_file.buffer);
	}
}

void SimpleWiFiManager::mdToArray() {
	memset(_file.buffer, 0, SWFM_FILE_BUFFER_SIZE);

	if (_dbg.isEnabled()) {
		_dbg.msg("\nSimpleWiFiManager::mdToArray");
		_dbg.println("\n--Before--");
		for (int idx = 0; idx < SWFM_CONFIG_FILE_SIZE; idx++) {
			_dbg.printf("%02x ", _file.buffer[idx]);
		}
		_dbg.println("\n----------");

		for (int idx = 0; idx < _mdCnt; idx++) {
			_md[idx]->toBuffer(_file.buffer);
		}
		_dbg.println("\n--After---");
		for (int idx = 0; idx < SWFM_CONFIG_FILE_SIZE; idx++) {
			_dbg.printf("%02x ", _file.buffer[idx]);
		}
		_dbg.println("\n----------");
	}

	_file.buffer[SWFM_OFFSET_A9] = 0xA9;
	_file.buffer[SWFM_OFFSET_9A] = 0x9A;
	_file.buffer[SWFM_OFFSET_RECORD_SIZE] = SWFM_CONFIG_FILE_SIZE - 4;
	_file.buffer[SWFM_OFFSET_COMMAND] = 0x0C;
	_file.buffer[SWFM_OFFSET_END_BYTE] = 0xED;
}

String SimpleWiFiManager::defaultAPName() {
	return (_wifi.apName.getStringDefault() + String((uint32_t) myUtil::getDeviceId()));
}

void SimpleWiFiManager::setDefaultConfig() {
#ifdef _ENABLE_TRACE_
	_dbg.println("SWFM->setDefaultConfig");
#endif

	for (int idx = 0; idx < _mdCnt; idx++) {
		_md[idx]->reset();
	}
	_wifi.apName.setString(defaultAPName());
	_configReady = true;
}


bool SimpleWiFiManager::saveConfig() {
#ifdef _ENABLE_TRACE_
	_dbg.println("SWFM->saveConfig");
#endif
	mdToArray();
    return myUtil::writeSPIFFS(SWFM_CONFIG_FILE, _file.buffer, SWFM_CONFIG_FILE_SIZE);	
}

void SimpleWiFiManager::dumpConfig() {
	_dbg.println("\n\nSimple WiFi Manager Settings:\n");
	for (int idx = 0; idx < _mdCnt; idx++) {
		_dbg.printf("%s (%s) : %s\n", 
		             _md[idx]->label().c_str(),
					 _md[idx]->key().c_str(),
					 _md[idx]->getPrintable().c_str());
	}
}

void SimpleWiFiManager::httpServerHandler() {
	if (!isReady()) return;
	if (!isServerRunning()) return;

    // _dnsServer->processNextRequest();
	_httpServer->handleClient();
}

void SimpleWiFiManager::httpTrField(String *s, MyData data) {
	char buf[256];
	String maxLen = "";
	if (data.type() == MD_INTEGER) {
		maxLen = "maxlength=\"";
		switch (data.size()) {
			case 1:
				maxLen += "3\"";
				break;
			case 2:
				maxLen += "5\"";
				break;
			case 4:
				maxLen += "10\"";
				break;
			case 8:
			default:
				// no need to set for such large numbers
				maxLen = "";
				break;
		}
	} else if (data.type() == MD_STRING) {
		maxLen = "maxLength=\"" + String(data.size()) + "\" ";
	}
	sprintf(buf, "<tr><td>%s</td><td><input type=\"text\" name=\"%s\" value=\"%s\" %s></td></tr>", 
	        data.label().c_str(), data.key().c_str(), data.getPrintable().c_str(), maxLen.c_str());
	s->concat(buf);
}


void SimpleWiFiManager::httpTrCheckbox(String *s, MyData data, int colSpan) {
	char buf[256];
	sprintf(buf, "<tr><td colspan=\"%d\"><input type=\"checkbox\" name=\"%s\" value=\"enable\" %s>%s</td>",
	        colSpan, data.key().c_str(), (data.getBool() ? "checked" : ""), data.label().c_str());
	s->concat(buf);
}

bool SimpleWiFiManager::updateRouter(bool enableRouter, String ssid, String password, uint32_t long routerTimeout, bool updateConfig) {
	_wifi.enableRouter.setBool(enableRouter);
	_wifi.ssid.setString(ssid);
	_wifi.password.setString(password);
	_wifi.routerTimeout.setInt(routerTimeout);
	return (updateConfig ? saveConfig() : true);
}

bool SimpleWiFiManager::updateAP(bool enableAP, String apName, String apKey, bool updateConfig) {
	_wifi.enableAP.setBool(enableAP);
	_wifi.apName.setString(apName);
	_wifi.apKey.setString(apKey);
	return (updateConfig ? saveConfig() : true);
}

bool SimpleWiFiManager::updateServer(bool enableServer, uint16_t serverPort, bool updateConfig) {
	_wifi.enableServer.setBool(enableServer);
	_wifi.serverPort.setInt(serverPort);
	return (updateConfig ? saveConfig() : true);
}

bool SimpleWiFiManager::updateUDP(bool enableUDP, uint16_t udpRxPort, uint16_t udpTxPort, bool updateConfig) {
	_wifi.enableUDP.setBool(enableUDP);
	_wifi.udpRxPort.setInt(udpRxPort);
	_wifi.udpTxPort.setInt(udpTxPort);
	return (updateConfig ? saveConfig() : true);
}

bool SimpleWiFiManager::updateSettings(bool enableRouter, String ssid, String password, uint32_t long routerTimeout,
					bool enableAP, String APName, String APKey,
					bool enableServer, uint16_t serverPort,
					bool enableUDP, uint16_t udpRxPort, uint16_t udpTxPort) {
	updateRouter(enableRouter, ssid, password, routerTimeout, false);
	updateAP(enableAP, APName, APKey, false);
	updateServer(enableServer, serverPort, false);
	updateUDP(enableUDP, udpRxPort, udpTxPort, false);
	return saveConfig();							
}


