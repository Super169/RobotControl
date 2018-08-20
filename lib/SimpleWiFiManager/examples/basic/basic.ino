#include "SimpleWiFiManager.h"
SimpleWiFiManager SWFM;

#include <WiFiUDP.h>
WiFiUDP udpClient;

#define DEBUG_PORT  Serial1

bool isConnected = false;
String ip;
uint8_t networkMode;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(1);
    Serial1.begin(115200);
    while (!Serial1) delay(1);
    DEBUG_PORT.println("\n\n\nLet's GO!\n\n");

	SWFM.setDebug(&DEBUG_PORT);
	SWFM.begin();
	if (SWFM.mode() == SWFM_MODE_ROUTER) {
		networkMode = SWFM_MODE_ROUTER;
		if (SWFM.enableUDP()) {
			udpClient.begin(SWFM.udpRxPort());
		}
	}
	isConnected = true;
	ip = SWFM.ip();
}

void loop() {
    SWFM.httpServerHandler();
}