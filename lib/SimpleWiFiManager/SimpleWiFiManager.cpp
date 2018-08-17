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

	if (!_wifi.enableServer.getBool()) return true;

	_httpServer.reset(new WiFiServer((uint16_t) _wifi.serverPort.getInt()));
	_httpServer->begin((uint16_t) _wifi.serverPort.getInt());
	_dbg.msg("Server running at port: %d", (uint16_t) _wifi.serverPort.getInt());
	_serverRunning = true;

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
	_dbg.println("\n----------");
	for (int idx = 0; idx < SWFM_CONFIG_FILE_SIZE; idx++) {
		_dbg.printf(" %02x", _file.buffer[idx]);
	}
	_dbg.println("\n----------");

	for (int idx = 0; idx < _mdCnt; idx++) {
		_md[idx]->toBuffer(_file.buffer);
	}
	_dbg.println("\n----------");
	for (int idx = 0; idx < SWFM_CONFIG_FILE_SIZE; idx++) {
		_dbg.printf(" %02x", _file.buffer[idx]);
	}
	_dbg.println("\n----------");

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


bool SimpleWiFiManager::getHttpParm(String s, MyData &data) {
	bool valChanged = false;
	String value = myUtil::getHtmlParmValue(s, data.key());
	_dbg.printf("Parm %s = %s\n", data.key().c_str(), value.c_str());
	String newValue;
	bool b_value;
	bool old_value;
	int64_t i64_value;

	switch (data.type()) {

		case MD_BOOL:
			b_value = false;
			old_value = data.getBool();
			if (value.length() > 0) b_value = (value.compareTo("enable") == 0);
			if (b_value != old_value) {
				data.setBool(b_value);
				newValue =  data.getPrintable();
				valChanged = true;
			}
			break;

		case MD_INTEGER:
			i64_value = atoll(value.c_str());
			if ((i64_value > 0) && (i64_value != data.getInt())) {
				data.setInt(i64_value);
				newValue = data.getPrintable();
				valChanged = true;
			}
			break;

		case MD_STRING:
			if (data.getString().compareTo(value) != 0) {
				data.setString(value);
				newValue = data.getPrintable();
				valChanged = true;
			}

	}
	if (valChanged) {
		_dbg.printf("%s set to \"%s\"\n", data.label().c_str(), newValue.c_str());
	}
	return valChanged;
}


void SimpleWiFiManager::httpServerHandler() {
	if (!isReady()) return;
	if (!isServerRunning()) return;

	WiFiClient client = _httpServer->available();
    if (client) {
        _dbg.msg("HTTP client connected");
        // an http request ends with a blank line
        while (client.connected()) {
            String data = "";
            if (client.available()) {
                delay(1);
                data = client.readString();
                _dbg.println(data);
                _dbg.printf("Data length: %d\n", data.length());
            }

			int pos;
			String parmList = "";
			pos = data.indexOf("GET /?");
			
			if (pos >= 0) {
				unsigned int endPos = pos + 6;
				while (endPos < data.length()) {
					char ch = data.charAt(endPos);
					if ((ch == '\r') || (ch == '\n' )) {
						break;
					}
					endPos++;
				}
				parmList = data.substring(pos, endPos - 1);
				_dbg.printf("\n-- Parm string: %s\n\n\n", parmList.c_str());
			}
			
			if (parmList.length() > 0) {

				data = parmList;

				bool b_value = false;

				String value;
				value = myUtil::getHtmlParmValue(data, "key");
				if (value.length() > 0) {
					uint64_t chipId = myUtil::getDeviceId();
					_dbg.printf("Chip Key : %s\n", myUtil::getInt64String(chipId).c_str());
					uint64_t key = atoll(value.c_str());
					_dbg.printf("HTML Key : %s\n",myUtil::getInt64String(key).c_str());
					if (chipId == key) {

						bool changeSetting = false;

						changeSetting |= getHttpParm(data, _wifi.enableRouter);
						changeSetting |= getHttpParm(data, _wifi.ssid);
						changeSetting |= getHttpParm(data, _wifi.password);
						changeSetting |= getHttpParm(data, _wifi.routerTimeout);
						changeSetting |= getHttpParm(data, _wifi.enableAP);
						changeSetting |= getHttpParm(data, _wifi.apName);
						changeSetting |= getHttpParm(data, _wifi.apKey);
						changeSetting |= getHttpParm(data, _wifi.enableServer);
						changeSetting |= getHttpParm(data, _wifi.serverPort);
						changeSetting |= getHttpParm(data, _wifi.enableUDP);
						changeSetting |= getHttpParm(data, _wifi.udpRxPort);
						changeSetting |= getHttpParm(data, _wifi.udpTxPort);

						if (changeSetting) {
							if (saveConfig()) {
								_dbg.printf("WiFi Setting updated.\n");
							}
						}
					} else {
						_dbg.println("Key not matched");
					}
				}
			}


			String s = "";
			s.concat("HTTP/1.1 200 OK\n");
			s.concat("Content-Type: text/html\n");
			s.concat("Connection: close\n");
			s.concat("\n");
            s.concat("<!DOCTYPE HTML><html><head>\n");
            s.concat("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>\n");
			s.concat("<h1>myRobot - WiFi Settings</h1>\n");
			s.concat("<p>Server running at ");
			s.concat(ip());
			s.concat("</p>\n");
            s.concat("<form>");
            s.concat("<table>");
            s.concat("<tbody>");

            s.concat("<tr><td colspan=\"2\"><b>Station Mode&nbsp;&nbsp;</b></td></tr>");

			httpTrCheckbox(&s, _wifi.enableRouter, 2);
			httpTrField(&s, _wifi.ssid);
			httpTrField(&s, _wifi.password);
			httpTrField(&s, _wifi.routerTimeout);

            s.concat("<tr><td colspan=\"2\">&nbsp;</td></tr>");

            s.concat("<tr><td colspan=\"2\"><b>AP Mode&nbsp;&nbsp;</b></td></tr>");
			
			httpTrCheckbox(&s, _wifi.enableAP, 2);
			httpTrField(&s, _wifi.apName);
			httpTrField(&s, _wifi.apKey);
			
			s.concat("<tr><td colspan=\"2\">&nbsp;</td></tr>");

            s.concat("<tr><td colspan=\"2\"><b>Settings Web Server&nbsp;&nbsp;</b></td></tr>");

			httpTrCheckbox(&s, _wifi.enableServer, 2);
			httpTrField(&s, _wifi.serverPort);

            s.concat("<tr><td colspan=\"2\">&nbsp;</td></tr>");

            s.concat("<tr><td colspan=\"2\"><b>Settings Web Server&nbsp;&nbsp;</b></td></tr>");

			httpTrCheckbox(&s, _wifi.enableUDP, 2);
			httpTrField(&s, _wifi.udpRxPort);
			httpTrField(&s, _wifi.udpTxPort);

            s.concat("<tr><td colspan=\"2\">&nbsp;</td></tr>");

            s.concat("</tbody>\n");
            s.concat("</table>\n");
            s.concat("<input type=\"hidden\" name=\"key\" value=\"");
            s.concat(myUtil::getDeviceId());
            s.concat("\">\n");
			s.concat("<input type=\"submit\" value=\"Update\">\n");
            s.concat("</form>\n");
            
            s.concat("</html>\n");
            // HTTP response ends with another blank line, for safety double blank line
			s.concat("\n\n");

			client.print(s);

            break;        
        }
        // give the web browser time to receive the data
        delay(1);

        // close the connection:
        client.stop();
        _dbg.msg("HTTP client disconnected");
    }
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