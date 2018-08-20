#ifndef _MY_UTIL_H_
#define _MY_UTIL_H_

#ifdef ESP32
	#include <ESP.h>
	#include <FS.h>
	#include <SPIFFS.h>
#else
	#include <ESP8266WiFi.h>
	#include <FS.h>
	#define FILE_READ	"r"
	#define FILE_WRITE	"w"
#endif

class myUtil {
	public:
		static uint32_t getDeviceId();
		static bool readSPIFFS(const char *filename, String &data);
		static bool readSPIFFS(const char *fileName, char *buffer, uint16_t size);
		static bool writeSPIFFS(const char *filename, const char *data);
		static bool writeSPIFFS(const char *fileName, uint8_t *buffer, uint16_t size);
		static bool isEmpty(String data);
		inline static void clearSerialBuffer() { return clearStreamBuffer(&Serial); }
		static void clearStreamBuffer(Stream *stream);
		static String getInt64String(int64_t data);
	private:

};

#endif
