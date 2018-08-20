#include "SimpleWiFiManager.h"
SimpleWiFiManager SWFM;

#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

ESP8266WebServer server(80);
#define DEBUG_PORT  Serial1

#include "MyDebugger.h"
MyDebugger dbg;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(1);
    Serial1.begin(115200);
    while (!Serial1) delay(1);
    DEBUG_PORT.println("\n\n\nLet's GO!\n\n");

	dbg.setOutput(&DEBUG_PORT);

	SWFM.setDebug(&DEBUG_PORT);
	SWFM.begin();

	if (WiFi.status() != WL_CONNECTED) {
		dbg.msg("Unexpected error, wifi not connected");
		while (WiFi.status() != WL_CONNECTED) delay(1);
	}

	if (MDNS.begin("esp8266")) {
		dbg.msg("MDNS responder started");
	}

	server.on("/", handleRoot);
	server.on("/update", handleUpdate);
	server.onNotFound(handleNotFound);

	server.begin();
	dbg.msg("HTTP Server started");
	

}

void loop() {
	server.handleClient();
}

void handleRoot() {
	dbg.msg("Start handleRoot");
  	server.send(200, "text/plain", "hello from esp8266!");
	dbg.msg("End handleRoot");
}

void handleNotFound() {
	dbg.msg("Start handleNotFound");
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i = 0; i < server.args(); i++) {
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}
	server.send(404, "text/plain", message);
	dbg.msg("End handleNotFound");	
}

void handleUpdate() {
	dbg.msg("Start handleUpdate");
	String message = "<html><body>Record update HTML page\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i = 0; i < server.args(); i++) {
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}
	message += "</body></html>";
	server.send(404, "text/plain", message);
	dbg.msg("End handleUpdate");	
}