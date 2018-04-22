/*
UBTech Alpha 1S Control Board (ESP8266 version) - Version 2.x
By Super169

Command set for V2

UBTech Servo Command
- FA AF {id} {cmd} xx xx xx xx {sum} ED
- FC CF {id} {cmd} xx xx xx xx {sum} ED

UBTech Controller Command
- F1 1F 00 00 00 00 00 00 {sum} ED : Get version
- EF FE 00 00 00 00 00 00 {sum} ED : Servo status
- F2 2F 00 00 00 00 00 00 {sum} ED : Stop play
- F5 5F 01 00 00 00 00 00 {sum} ED : Firmware version
- FB BF - not implemented : will be handled as BT command
- F4 4F : fine tuning - not implemented
- F9 9F : Get USB Type - not implemented
- F3 3F {len} xx - xx {sum} ED : Play action by name

UBTech Alpha 1 BT Protocol
- FB BF {len} {cmd} xx ... xx {sum} ED

V2 Control Board Command
- A9 9A {len} {cmd} xx ... xx {sum} ED

** All V1 Control Board Command will be obsolete
- {cmd} xx ... xx

Major change on 2.x:
- New command set for V2 with start / end cod and checksum for better communization control
- Use major loop for action playback, so there has no block in playing action, can be stopped anytime
- Default reset action with all servo locked, and LED on
- Use single file to store individual action
- Define name for action (to be used in PC UI only), and a maximum of 255 actions allowed
- Support up to 255 steps for each action
- Support servo LED control
- Support Robot Head Light control


PIN Assignment:
GPIO-12 : One-wire software serial - servo contorl
GPIO-2  : Serial1 - debug console

GPIO-14, GPIO-13 : software serial Rx,Tx connected to MP3 module
GPIO-15 : Head LED

*/

#include "robot.h"



// Start a TCP Server on port 6169
uint16_t port = 6169;
WiFiServer server(port);
WiFiClient client;

// #define PIN_SETUP 		13		// for L's PCB


void setup() {

	pinMode(HEAD_LED_GPIO, OUTPUT);
	SetHeadLed(false);
	// digitalWrite(HEAD_LED_GPIO, LOW);

	// Start OLED ASAP to display the welcome message
	myOLED.begin();  
	myOLED.clr();
	myOLED.show();
	
	myOLED.setFont(OLED_font6x8);
	myOLED.printFlashMsg(0,0, msg_welcomeHeader);
	myOLED.printFlashMsg(62,1, msg_author);
	myOLED.printFlashMsg(0,4, msg_welcomeMsg);	
	myOLED.show();

	// Delay 2s to wait for all servo started
	delay(2000);

	// start both serial by default, serial for command input, serial1 for debug console
	// Note: due to ESP-12 hw setting, serial1 cannot be used for input
	Serial.begin(115200);
	Serial1.begin(115200);
	delay(100);
	DEBUG.println("\n\n");

	config.readConfig();

	// SetDebug(false);  // Disable servo debug first, enable it later if needed
	SetDebug(true);

	retBuffer = servo.retBuffer();


	DEBUG.println(F("\nUBTech Robot Control v2.0\n"));
	unsigned long actionSzie = sizeof(actionTable);

	char *AP_Name = (char *) "Alpha 1S";
	char *AP_Password = (char *) "12345678";

	char buf[20];
	bool isConnected = false;
	
	/*
	if (digitalRead(PIN_SETUP) == LOW) {
		myOLED.clr(2);
		myOLED.print(0,2,"Connecting to router");
		myOLED.show();
		wifiManager.setDebugOutput(false);
		wifiManager.setAPCallback(configModeCallback);
		wifiManager.setConfigPortalTimeout(60);
		wifiManager.setConfigPortalTimeout(60);
		wifiManager.setTimeout(60);
		DEBUG.printf("Try to connect router\n");
		isConnected = wifiManager.autoConnect(AP_Name, AP_Password);
	}
	*/

	String ip;

	if (isConnected) {
		ip = WiFi.localIP().toString();
		myOLED.clr();
		memset(buf, 0, 20);
		String ssid = WiFi.SSID();
		ssid.toCharArray(buf, 20);
		myOLED.print(0,0, buf);
	} else {
		DEBUG.println(F("Start using softAP"));
		DEBUG.printf("Please connect to %s\n\n", AP_Name);
		WiFi.softAP(AP_Name, AP_Password);
		IPAddress myIP = WiFi.softAPIP();
		ip = myIP.toString();
		myOLED.clr();
		myOLED.print(0,0, "AP: ");
		myOLED.print(AP_Name);
	}

	memset(buf, 0, 20);
	ip.toCharArray(buf, 20);
	myOLED.print(0,1, buf);
	myOLED.print(":");
	myOLED.print(port);
	myOLED.show();

	DEBUG.print("Robot IP: ");
	DEBUG.println(ip);
	DEBUG.printf("Port: %d\n\n", port);
	//	RobotMaintenanceMode();
	server.begin();


	DEBUG.println("Starting robot servo: ");
	
	// clean up software serial
	servo.begin();
	delay(100);
	servo.end();
	delay(100);
	//

	servo.begin();
	servo.lockAll();
	// TODO: change to V2 when ready
	V1_UBT_ReadSPIFFS(0);

	DEBUG.println(F("Control board ready\n\n"));
	SetHeadLed(true);
	// digitalWrite(HEAD_LED_GPIO, HIGH);

	// Play MP3 for testing only
	mp3.begin();
	mp3.stop();
	delay(10);
	mp3.setVol(config.mp3Volume());
	delay(10);
	mp3_Vol = mp3.getVol();
	
	mp3.playMp3File(1);
	delay(10);

	myOLED.print(0,4,"MP3 Vol: ");
	myOLED.print(mp3_Vol);
	myOLED.show();

	servo.setLED(0, 1);

/*
	// Testing on ActionData
	actionData.InitObject(1);
	actionData.Header()[48] = 1;
	byte wCnt = actionData.WriteSPIFFS();
	Serial.printf("\nBytes write: %d\n", wCnt);
	bool success = actionData.ReadSPIFFS(1);
	Serial.printf("\nRead action %d : %s\n", 1, (success ? "Success" : "Failed"));
*/
}

void configModeCallback (WiFiManager *myWiFiManager) {
	DEBUG.println("Fail connecting to router");
	DEBUG.print("WiFi Manager AP Enabled, please connect to ");
	DEBUG.println(myWiFiManager->getConfigPortalSSID());
	DEBUG.print("Alpha 1S Host IP: ");
	DEBUG.println(WiFi.softAPIP().toString());
	DEBUG.println(F("Will be switched to SoftAP mode if no connection within 60 seconds....."));

	// Try to display here if OLED is ready
	// Now flash LED to alert user
	for (int i = 0; i < 10; i++) {
		digitalWrite(HEAD_LED_GPIO, LOW);
		delay(200);
		digitalWrite(HEAD_LED_GPIO, HIGH);
		delay(200);
	}
	digitalWrite(HEAD_LED_GPIO, LOW);
	delay(200);
}

unsigned long noPrompt = 0;

void loop() {
	// For Wifi checking, once connected, it should use the client object until disconnect, and check within client.connected()
	client = server.available();
	if (client){
		myOLED.print(122, 1, '*');
		myOLED.show();
		while (client.connected()) {
			if (millis() > noPrompt) {
				DEBUG.println("Client connected");
				noPrompt = millis() + 1000;
			}
			while (client.available()) {
				cmdBuffer.write(client.read());
			}
			// Keep running RobotCommander within the connection.
			RobotCommander();
		}
		myOLED.print(122, 1, ' ');
		myOLED.show();
	}	
	if (millis() > noPrompt) {
		// DEBUG.println("No Client connected, or connection lost");
		noPrompt = millis() + 1000;
	}
	
	// Execute even no wifi connection, for USB & Bluetooth connection
	// Don'e put it into else case, there has no harm executing more than once.
	// Just for safety if client exists but not connected, RobotCommander can still be executed.
	RobotCommander();

}

void RobotCommander() {
	CheckSerialInput();
	remoteControl();  
	V2_CheckAction();
}


// move data from Serial buffer to command buffer
void CheckSerialInput() {
	if (Serial.available()) {
		delay(1); // make sure one command is completed
		while (Serial.available()) {
			cmdBuffer.write(Serial.read());
		}
	}
}

void remoteControl() {
	bool goNext = true;
	int preCount;
	while (cmdBuffer.available()) {
		preCount = cmdBuffer.available();
		cmd = cmdBuffer.peek();
		switch (cmd) {

			case 0xFB:
				goNext = UBTBT_Command();
				break;

			case 0xF1:
			case 0xF2:
			case 0xF3:
			case 0xF4:
			case 0xF5:
			case 0xF9:
			case 0xEF:
				goNext = UBTCB_Command();
				break;

			case 0xFA:
			case 0xFC:
				goNext = UBTSV_Command();
				break;

			case 0xA9:
				goNext = V2_Command();
				break;


			case 'A':
			case 'a':
			case 'B':
			case 'b':
			case 'D':
			case 'F':
			case 'f':
			case 'J':
			case 'L':
			case 'l':
			case 'M':
			case 'm':
			case 'P':
			case 'R':
			case 'T':
			case 't':
			case 'S':
			case 'U':
			case 'W':
			case 'Z':
				goNext = V1_Command();
				break;

			default:
				cmdSkip(true);
				break;

		}
		// for some situation, command data not yet competed.
		// need to study if it should wait for full data inside the handler
		if (goNext) {
			if (preCount == cmdBuffer.available()) {
				// Program bug, no data handled, but not ask for wait
				if (debug) {
					DEBUG.print(F("preCount: "));
					DEBUG.print(preCount);
					DEBUG.print(F(" => "));
					DEBUG.println(cmdBuffer.available());
					DEBUG.print(F("****** Missing handler: "));
					DebugShowSkipByte();
				}
				cmdBuffer.skip();
			}
			lastCmdMs = 0;
		} else {
			if ((lastCmdMs)	&& (millis() - lastCmdMs > MAX_WAIT_CMD)) {
				// Exceed max wait time for a command
				if (debug) {
					DEBUG.printf("Command timeout (%d received): ", cmdBuffer.available());
					DebugShowSkipByte();
				}
				cmdBuffer.skip();
				lastCmdMs = 0;
			} else {
				if (!lastCmdMs) lastCmdMs = millis();
				break;
			}
		}
	}
}


