#include "myUtil.h"

uint32_t myUtil::getDeviceId() {
	uint32_t id;
	#ifdef ESP32
		id = (uint32_t) ESP.getEfuseMac();
	#else
		id = ESP.getChipId();
	#endif
	return id;
}

void myUtil::clearStreamBuffer(Stream *stream) {
	if (stream->available()) {
		while (stream->available()) {
			stream->read();
			if (!stream->available()) delay(1);
		}
	}
}

bool myUtil::readSPIFFS(const char *fileName, String &data) {
	if (!SPIFFS.begin()) return false;
	bool success = false;
	File f = SPIFFS.open(fileName, FILE_READ);
	if (f) {
		data = f.readString();
		success = true;
	}
	f.close();
	SPIFFS.end();
	return success;
}


bool myUtil::readSPIFFS(const char *fileName, char *buffer, uint16_t size) {
	if (!SPIFFS.begin()) return false;
	bool success = false;
	File f = SPIFFS.open(fileName, FILE_READ);
    if ((f) && (f.size() == size)) {
        f.readBytes(buffer, size);
        success = true;
    }
	f.close();
	SPIFFS.end();
	return success;
}

bool myUtil::writeSPIFFS(const char *fileName, const char *data) {
	if (!SPIFFS.begin()) return false;
	bool success = false;
	File f = SPIFFS.open(fileName, FILE_WRITE);
	if (f) {
		success = f.print(data);
	}
	f.close();
	SPIFFS.end();
	return success;
}

bool myUtil::writeSPIFFS(const char *fileName, uint8_t *buffer, uint16_t size) {
	if (!SPIFFS.begin()) return false;
	bool success = false;
	File f = SPIFFS.open(fileName, FILE_WRITE);
	if (f) {
		size_t wCnt = f.write(buffer, size);
		success = (wCnt == size);
	}
	f.close();
	SPIFFS.end();
	return success;
}

bool myUtil::isEmpty(String data) {
	// Will String be NULL?  Seems not possible
	// This dummy function shoudl have the same result as
	// data.equals("") / data == "" / data == NULL 
	// Just define a function here to centralize the checking, and change in single point if need.
	return (data.length() == 0);
}

int myUtil::getHtmlParmPos(String data, String parmName) {
	String specialChars = "&?";
    int pos = 0;
    char c;
    String searchKey = parmName + '=';
    while (true) {
        pos = data.indexOf(searchKey, pos);
        if (pos <= 0) break;
        c = data.charAt(pos - 1);
        // _dbg->printf("Parm: %s, pos: %d, pre-char: %02x\n", parmName.c_str(), pos, c);
		// 
        // if ((specialChars.indexOf(c) >= 0) || (c == '\r') || (c == '\n')) break;
		if ((specialChars.indexOf(c) >= 0)) break;
        pos += searchKey.length();
    }
    return pos;
}

String myUtil::getHtmlParmValue(String data, String parmName) {
    int slen = data.length();
    String value = "";

    int pos = getHtmlParmPos(data, parmName);
	if (pos < 0) return "";
    if (pos >= 0) {
        pos += parmName.length() + 1;  // skip: ssid= 
        bool notFinished = true;
        while (notFinished) {
            char c = data.charAt(pos++);
			// any rules for parameter values?
            if ((c == '&') || (c == '?') || (c == '\r') || (c == '\n') || (c < 0x20) || (pos >= slen)) {
                notFinished = false;
            } else {
				// TODO: better use substring after search instead of concatenation
                value += c;
            }
        }
    }
    return value;
}

String myUtil::getInt64String(int64_t data) {
    // can be converted to String directly
    if ((data <= 2147483647) && (data >= -2147483648)) {
        return String((int32_t) data);
    }
    // int64 has value from -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807, up to 19 digits and 1 sign
    char buf[21];
    uint64_t i64;
    memset(buf, 0, 21);
    i64 = (data < 0 ? -data : data);
    int64_t temp;
    int maxPos = 0;
    for (int i = 0; i < 19; i++) {
        temp = i64 % 10;
        i64 /= 10;
        buf[19-i] = temp;
        if (temp) maxPos = i;
    }

    char result[21];
    memset(result, 0, 21);
    uint8_t idx = 0;
    if (data < 0) result[idx++] = '-';

    for (int i = (19 - maxPos); i < 20; i++) {
        result[idx++] = '0' + buf[i];
    }
    return String(result);
}
