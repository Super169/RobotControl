#include "SimpleWiFiManager.h"
SimpleWiFiManager SWFM;


#define DEBUG_PORT  Serial1

String ip;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(1);
    Serial1.begin(115200);
    while (!Serial1) delay(1);
    DEBUG_PORT.println("\n\n\nLet's GO!\n\n");

	SWFM.setWiFiServer(6169);
	SWFM.setDebug(&DEBUG_PORT);
	SWFM.begin();
	ip = SWFM.ip();
}

void loop() {
    SWFM.httpServerHandler();
	uint8_t cnt = SWFM.checkData();
	if (cnt) {
		DEBUG_PORT.printf("\nWiFi Data received: %d bytes\n", cnt);
		while (SWFM.available()) {
			DEBUG_PORT.printf("%02X ", SWFM.read());
		}
		DEBUG_PORT.println();
		SWFM.resetBuffer();
	}
}