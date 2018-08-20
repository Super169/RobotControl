#include "SimpleWiFiManager.h"


void SimpleWiFiManager::handleNotFound() {
	_dbg.msg("Start handleNotFound");
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += _httpServer->uri();
	message += "\nMethod: ";
	message += (_httpServer->method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += _httpServer->args();
	message += "\n";
	for (uint8_t i = 0; i < _httpServer->args(); i++) {
		message += " " + _httpServer->argName(i) + ": " + _httpServer->arg(i) + "\n";
	}
	_httpServer->send(404, "text/plain", message);
	_dbg.msg("End handleNotFound");	
}

bool SimpleWiFiManager::checkHttpArg(MyData *data) {
    bool valChanged = false;
    String value = "";
    if (_httpServer->hasArg(data->key().c_str())) {
        value = _httpServer->arg(data->key().c_str());
    }
    _dbg.printf("Parm %s = %s\n", data->key().c_str(), value.c_str());
	bool b_value;
	bool old_value;
	int64_t i64_value;

	switch (data->type()) {

		case MD_BOOL:
			b_value = false;
			old_value = data->getBool();
			if (value.length() > 0) b_value = (value.compareTo("enable") == 0);
			if (b_value != old_value) {
				data->setBool(b_value);
				valChanged = true;
			}
			break;

		case MD_INTEGER:
			i64_value = atoll(value.c_str());
			if ((i64_value > 0) && (i64_value != data->getInt())) {
				data->setInt(i64_value);
				valChanged = true;
			}
			break;

		case MD_STRING:
			if (data->getString().compareTo(value) != 0) {
				data->setString(value);
				valChanged = true;
			}
            break;

	}
	if (valChanged) {
		_dbg.printf("%s set to \"%s\"\n", data->label().c_str(), data->getPrintable().c_str());
	}
	return valChanged;

}


void SimpleWiFiManager::handleRoot() {
	_dbg.msg("Start handleRoot");

    // check for arguments, at leastt 2 arguments (key + any)
    if ( (_httpServer->args() > 1) && 
         (_httpServer->hasArg("key")) && (_chipId == _httpServer->arg("key")) ) {
        bool changeSetting = false;
        for (int idx = 0; idx < _mdCnt; idx++) {
            if (checkHttpArg(_md[idx])) changeSetting = true;
        }
		if (changeSetting) {
			if (saveConfig()) {
				_dbg.printf("WiFi Setting updated.\n");
			}
		}
    }


	String s = "";
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

	_httpServer->send(200,"text/html", s);

	_dbg.msg("HTML page sent");


/*
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

			_dbg.msg("HTML page sent");

            break;        
        }
        // give the web browser time to receive the data
        delay(1);

        // close the connection:
        client.stop();
        _dbg.msg("HTTP client disconnected");
    }
*/		
	_dbg.msg("End handleRoot");
}