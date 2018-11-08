#ifndef _OTA_H_
#define _OTA_H_

// #define ENABLE_OTA

#ifdef ENABLE_OTA
//OTA Setting
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
const char* ssid = "wuhulanren";          
const char* password = "wuhulanren";
#define EN_OTA true

// OTA.ino
void ArduinoOTASetup();
void ArduinoOTAHandle();

#endif

#endif