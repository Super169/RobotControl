#ifndef _SIMPLE_WIFI_MANAGER_H_
#define _SIMPLE_WIFI_MANAGER_H_

#define _ENABLE_TRACE_

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiUDP.h>

#include <FS.h>
#define FILE_READ	"r"
#define FILE_WRITE	"w"	

#include "myUtil.h"
#include "MyDebugger.h"
#include "MyData.h"
#include "Buffer.h"

#define SWFM_OFFSET_A9            	0
#define SWFM_OFFSET_9A              1
#define SWFM_OFFSET_RECORD_SIZE     2
#define SWFM_OFFSET_COMMAND         3
#define SWFM_OFFSET_VER_MAJOR       4
#define SWFM_OFFSET_VER_MINOR       5
#define SWFM_OFFSET_ENABLE_ROUTER   8
#define SWFM_OFFSET_SSID            10
#define SWFM_OFFSET_PASSWORD        40
#define SWFM_OFFSET_ROUTER_TIMEOUT  60
#define SWFM_OFFSET_ENABLE_AP       62
#define SWFM_OFFSET_AP_NAME         64
#define SWFM_OFFSET_AP_KEY          84
#define SWFM_OFFSET_ENABLE_SERVER   104
#define SWFM_OFFSET_SERVER_PORT     106
#define SWFM_OFFSET_ENABLE_UDP      108
#define SWFM_OFFSET_UDP_RX_PORT     110
#define SWFM_OFFSET_UDP_TX_PORT     112
#define SWFM_OFFSET_CHECKSUM        118
#define SWFM_OFFSET_END_BYTE        119

#define SWFM_CONFIG_FILE_SIZE		120
#define SWFM_FILE_BUFFER_SIZE		120
#define SWFM_MD_SIZE 20 

#define SWFM_MODE_NONE       0
#define SWFM_MODE_ROUTER     1
#define SWFM_MODE_AP         2

#define SWFM_CONFIG_FILE	 "/system/wifi.cfg"

struct __field {
	int    type;
	String label;
	String key;
	int64_t defInt;
	String defString;
	int64_t vInt;
	String vString;
};

class SimpleWiFiManager {

	public:
		SimpleWiFiManager();
		~SimpleWiFiManager();

		void setDebug(Stream *dbg, bool enableDebug = true);
		void enableDebug(bool value);

		void setWiFiServer(uint16_t port);
		bool begin(int preferMode = SWFM_MODE_NONE);
		void httpServerHandler();
		void stopServer();
		
		void dumpSettings();

		uint8_t *getConfig();
		bool setConfig(uint8_t *data);
		bool resetDefault();

		bool updateRouter(bool enableRouter, String ssid, String password, uint32_t long routerTimeout, bool updateConfig = true);
		bool updateAP(bool enableAP, String APName, String APKey, bool updateConfig = true);
		bool updateServer(bool enableServer, uint16_t serverPort, bool updateConfig = true);
		bool updateUDP(bool enableUDP, uint16_t udpRxPort, uint16_t udpTxPort, bool updateConfig = true);
		bool updateSettings(bool enableRouter, String ssid, String password, uint32_t long routerTimeout,
							bool enableAP, String APName, String APKey,
							bool enableServer, uint16_t serverPort,
							bool enableUDP, uint16_t udpRxPort, uint16_t udpTxPort);


		bool isReady();
		bool isServerRunning() { return _serverRunning; }
		uint8_t mode() { return _mode; }
		String ip();

		bool enableRouter() { return _wifi.enableRouter.getBool(); }
		String ssid() { return _wifi.ssid.getString(); }
		String password() { return _wifi.password.getString(); }
		uint32_t routerTimeout() { return (uint32_t) _wifi.routerTimeout.getInt(); }
		bool enableAP() { return _wifi.enableAP.getBool(); }
		String apName() { return _wifi.apName.getString(); }
		String apKey() { return _wifi.apKey.getString(); }
		bool enableServer() { return _wifi.enableServer.getBool(); }
		uint16_t serverPort() { return (uint16_t) _wifi.serverPort.getInt(); }
		bool enableUDP() { return _wifi.enableUDP.getBool(); }
		uint16_t udpRxPort() { return (uint16_t) _wifi.udpRxPort.getInt(); }
		uint16_t udpTxPort() { return (uint16_t) _wifi.udpTxPort.getInt(); }

		bool wifiClientConnected() { return _wifiClientConnected; }
		size_t write(byte data);
		size_t write(byte *data, size_t cnt);
		size_t udpSendPacket(const char* target, const char *data);


		uint8_t checkData();
		Buffer buffer() { return _buffer; }
		void resetBuffer() { _buffer.reset(); }
		uint16_t available() { return _buffer.available(); }
		byte peek() { return _buffer.peek(); }
		bool peek(byte *storage, uint16_t count) { return _buffer.peek(storage, count); }
		byte read() { return _buffer.read(); }
		bool read(byte *storage, uint16_t count) { return _buffer.read(storage, count); }
		bool skip(uint16_t count = 1) { return _buffer.skip(count); }

	private:

		String _chipId;

		// std::unique_ptr<Buffer>				_buffer;
		Buffer _buffer;
		std::unique_ptr<ESP8266WebServer>	_httpServer;
		std::unique_ptr<WiFiServer>			_wifiServer;

		// Status flag
		bool _configReady;
		uint8_t _mode;
		bool _serverRunning;

		// WiFi Server
		bool 		_wifiServerEnabled;
		uint16_t 	_wifiServerPort;
		bool		_wifiServerRunning;
		WiFiClient	_wifiClient;
		bool		_wifiClientConnected;

		WiFiUDP		_udpClient;
		bool		_udpClientRunning;

		MyDebugger _dbg;

		struct WIFI_STRUCT {
			MyData varMajor;
			MyData varMinor;
			MyData enableRouter;
			MyData ssid;
			MyData password;
			MyData routerTimeout;
			MyData enableAP;
			MyData apName;
			MyData apKey;
			MyData enableServer;
			MyData serverPort;
			MyData enableUDP;
			MyData udpRxPort;
			MyData udpTxPort;
		};

		WIFI_STRUCT _wifi = {
			MyData("Major Version", "major_version", (int64_t) 1, SWFM_OFFSET_VER_MAJOR, 1),
			MyData("Minor Version", "minor_version", (int64_t) 0, SWFM_OFFSET_VER_MINOR, 1),
			MyData("Enable Router", "enable_router", (bool) false, SWFM_OFFSET_ENABLE_ROUTER),
			MyData("SSID", "ssid", (String) "", SWFM_OFFSET_SSID, 30),
			MyData("PASSWORD", "password", (String) "", SWFM_OFFSET_PASSWORD, 20),
			MyData("Router Timeout", "router_timeout", (int64_t) 15, SWFM_OFFSET_ROUTER_TIMEOUT, 2),
			MyData("Enable AP", "enable_ap", (bool) true, SWFM_OFFSET_ENABLE_AP),
			MyData("AP Name", "ap_name", (String) "ESP", SWFM_OFFSET_AP_NAME, 20),
			MyData("AP Key", "ap_key", (String) "12345678", SWFM_OFFSET_AP_KEY, 20),
			MyData("Enable Server", "enable_server", (bool) true, SWFM_OFFSET_ENABLE_SERVER),
			MyData("Server Port", "server_port", (int64_t) 80, SWFM_OFFSET_SERVER_PORT, 2),
			MyData("Enable UDP", "enable_udp", (bool) true, SWFM_OFFSET_ENABLE_UDP),
			MyData("UDP Rx Port", "udp_rx_port", (int64_t) 9012, SWFM_OFFSET_UDP_RX_PORT, 2),
			MyData("UDP Tx Port", "udp_tx_port", (int64_t) 9020, SWFM_OFFSET_UDP_TX_PORT, 2)
		};

		MyData* _md[SWFM_MD_SIZE];
		uint8_t _mdCnt;

		struct WIFI_SETTING_STRUCT {
			uint8_t headerA9;		// 0
			uint8_t header9A;		// 1
			uint8_t size;			// 2  : 120 - 4 = 116 = 0x74
			uint8_t command;		// 3
			uint8_t ver_major;		// 4
			uint8_t ver_minor;		// 5
			uint8_t filler_01[2];	// 6
			bool enableRouter;		// 8
			uint8_t filler_02[1];	// 9
			char ssid[30];			// 10
			char password[20];		// 40
			uint8_t routerTimeout;	// 60
			uint8_t filler_03[1];	// 61
			bool enableAP;			// 62
			uint8_t filler_04[1];	// 63
			char apName[20];		// 64
			char apKey[20];			// 84
			bool enableServer;		// 104
			uint8_t filler_05[1];	// 105
			uint16_t serverPort;	// 106
			bool enableUDP;			// 108
			uint8_t filler_06[1];	// 109
			uint16_t udpRxPort;		// 110
			uint16_t udpTxPort;		// 112
			uint8_t filler_07[4];	// 114
			uint8_t sum;			// 118
			uint8_t endEA;			// 119
		};

		union WIFI_SETTING {
			uint8_t buffer[SWFM_FILE_BUFFER_SIZE];
			WIFI_SETTING_STRUCT data;
		}; 

		WIFI_SETTING _file;

		void mdFromArray();
		void mdToArray();

		bool readConfig();
		void setDefaultConfig();
		String defaultAPName();
		bool saveConfig();
		
		void dumpConfig();

		bool checkHttpArg(MyData *data);
		
		void httpTrField(String *s, MyData data);
		void httpTrCheckbox(String *s,  MyData data, int colSpan = 1);

		void handleNotFound();
		void handleRoot();
};
#endif
