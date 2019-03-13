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
GPIO-2  : Serial1 - debug console
GPIO-12 : One-wire software serial - servo contorl
GPIO-14 : One-wire software serial - sub-system board
GPIO-13 : Touch sensor
GPIO-15 : Head LED
GPIO-16 : software serial Tx connected to MP3 module's Rx

*/

#include "robot.h"


// #define PIN_SETUP 		13		// for L's PCB

void setup() {
	
	String ip;

	pinMode(HEAD_LED_GPIO, OUTPUT);
	SetHeadLed(false);
	// digitalWrite(HEAD_LED_GPIO, LOW);

	// Delay 2s to wait for all servo started
	delay(2000);

	// start both serial by default, serial for command input, serial1 for debug console
	// Note: due to ESP-12 hw setting, serial1 cannot be used for input
	Serial.begin(115200);
	Serial1.begin(115200);
	delay(100);

	debugger.setOutput(&DEBUG);
	_dbg->setLogLevel(DEFAULT_LOG_LEVEL);

	DEBUG.println("\n\n");

	// TODO: must remove
	//robotPort.begin(busConfig.baud);
	//_dbg->log(0,0,"Hard code enter USB-TTL to robot bus");
	//USB_TTL(&robotPort);

	//ssbPort.begin(ssbConfig.baud);
	//_dbg->log(0,0,"Hard code enter USB-TTL to Sub-system board");
	//USB_TTL(&ssbPort);


	config.readConfig();
	config.dumpConfig();

	// Reset by program in case config crashed
	/*
	config.initConfig();
	config.writeConfig();
	config.dumpConfig();
	*/

	// config.setMaxServo(20);

	SetDebug(config.enableDebug());

	if (config.enableOLED()) {

		// Start OLED ASAP to display the welcome message
		myOLED.begin();  
		myOLED.clr();
		myOLED.show();
		
		myOLED.setFont(OLED_font6x8);
		myOLED.printFlashMsg(0,0, msg_welcomeHeader);
		myOLED.printFlashMsg(62,1, msg_author);
		myOLED.printFlashMsg(0,4, msg_welcomeMsg);	
		myOLED.show();

	}

	
	// retBuffer = servo.retBuffer();

	if (debug) DEBUG.println(F("\nUBTech Robot Control v2.0\n"));

	char buf[20];

	// connect router setting is defined inside Simple WiFi Manager
	SWFM.setWiFiServer(port);
	SWFM.setDebug(&Serial1);
	SWFM.begin();
	
	if (SWFM.mode() == SWFM_MODE_ROUTER) {
		NetworkMode = NETWORK_ROUTER;
		/*
		if (SWFM.enableUDP()) {
			udpClient.begin(SWFM.udpRxPort());
		}
		*/
	} else {
		NetworkMode = NETWORK_AP;
	}

	isConnected = true;
	ip = SWFM.ip();

	localSegment = "";
	localSegmentStart = 0;
	if (ip.length()) {
		unsigned int idx = 0;
		unsigned int dotCnt = 0;
		// no need to check first byte as it won't be dot
		while (++idx < ip.length()) {
			if (ip.charAt(idx) == '.') {
				if (++dotCnt == 2) {
					localSegment = ip.substring(0,idx+1);
					localSegmentStart = localSegment.c_str()[0];
					break;
				}
			}
		}

	}
	DEBUG.printf("Local segment: %s\n", localSegment.c_str());

	memset(buf, 0, 20);
	ip.toCharArray(buf, 20);
	if (config.enableOLED()) {
		myOLED.print(0,1, buf);
		myOLED.print(":");
		myOLED.print(port);
		myOLED.show();
	}

	DEBUG.print("Robot IP: ");
	DEBUG.println(ip);
	DEBUG.printf("Port: %d\n\n", port);

	DEBUG.println("Starting robot servo: ");
	
	// Initialization for RobotServo

	robotPort.begin(busConfig.baud);
    rs.setEnableTxCalback(enableTxCallback);
    rs.begin(&robotPort, &DEBUG);
    rs.init(config.maxServo(),config.maxCommandRetry());
    rs.detectServo();
	rs.lock();


	DEBUG.printf("%08ld Control board ready\n\n", millis());
	SetHeadLed(true);
	// digitalWrite(HEAD_LED_GPIO, HIGH);
	if (config.mp3Enabled()) {
		// Play MP3 for testing only
		mp3.begin();
		mp3.stop();
		delay(10);
		mp3.setVol(config.mp3Volume());
		delay(10);
		mp3_Vol = mp3.getVol();
		
		// software serial is not stable in 9600bps, for safety, disable mp3 connection when not use
		mp3.end(); 

	}
	
	if (config.enableOLED()) {
		myOLED.print(0,4,"MP3 Vol: ");
		myOLED.print(mp3_Vol);
		myOLED.show();
	}

	// Centralize initilaize process here
	InitEventHandler();

	rs.setLED(false);

	// Load default action
	for (byte seq = 0; seq < CD_MAX_COMBO; seq++) comboTable[seq].ReadSPIFFS(seq);
	actionData.ReadSPIFFS(0);
	if (config.enableOLED()) myOLED.show();

	showNetwork();
  
  	// localip = WiFi.localIP().toString();
  	// Serial.println(localip);

	if (config.mp3Enabled() && config.mp3Startup()) {
		_dbg->log(10, 0, "Play startup MP3: %d", config.mp3Startup());
		mp3.begin();
		mp3.playMp3File(config.mp3Startup());
		delay(10);
		mp3.end();
	}

	if (config.startupAction()) {
		_dbg->log(10, 0, "Play startup action %d", config.startupAction());
		ActionPlayAction(config.startupAction());
	}


}

void showNetwork() {
	String ssid;
	String ip;

	DEBUG.println();
	
	DEBUG.printf("Network: %s\n",(SWFM.mode() == SWFM_MODE_ROUTER ? "Router" : (NetworkMode == NETWORK_AP ? "AP" : "NONE")));
	
	switch (NetworkMode) {

		case NETWORK_ROUTER:

			ssid = WiFi.SSID();
			ip = WiFi.localIP().toString();

			DEBUG.print("Router: ");
			DEBUG.println(ssid);
			DEBUG.print("Robot: ");
			DEBUG.print(ip);
			DEBUG.printf(":%d\n\n", port);

			break;

		case NETWORK_AP:

			DEBUG.printf("AP: %s - (%s)\n", SWFM.apName().c_str(), SWFM.apKey().c_str());

			IPAddress myIP = WiFi.softAPIP();
			ip = myIP.toString();

			DEBUG.print("Robot: ");
			DEBUG.print(ip);
			DEBUG.printf(":%d\n\n", port);

			break;
	}


}

unsigned long noPrompt = 0;
unsigned long revIpTime = 0;

void loop() {

	// Part 1 - check network input
	SWFM.httpServerHandler();
	uint8_t cnt = SWFM.checkData();
	if (cnt) {
		// Check for UDP IP request: 
		// 1) data.length >= localSegmentLength + 3 (for case of ?.?) & <= localSegmentLength + 7 (for case of ???.???)
		// 1) first byte match
		// 2) first n byte match segment
		if ((cnt >= localSegment.length() + 3) &&  (cnt <= localSegment.length() + 7) && (SWFM.peek() == localSegmentStart)) {
			char buffer[cnt+1];
			SWFM.peek((byte *) buffer, cnt);
			buffer[cnt] = 0;
			if (String(buffer).startsWith(localSegment)) {
				DEBUG.printf("ReceivedIP: %s\n", buffer);
				udpClient.beginPacket(buffer, SWFM.udpTxPort());
				udpClient.println(SWFM.apName().c_str());
				udpClient.print(WiFi.localIP().toString());
				udpClient.endPacket();
				SWFM.resetBuffer();
			}
		}

		while (SWFM.available()) {
			cmdBuffer.write(SWFM.read());
		}
	}

	// Part 2 - check serial input 
	CheckSerialInput();

	// Part 3 - processs input data
	remoteControl();  

	// Part 4 - continue current action
	V2_CheckAction();

	// Part 5 - check for robot events
	RobotEventHandler();

/*
	CheckVoltage();

	// Check auto-response action (should only play if not in action)
	if (V2_ActionPlaying) return;
	CheckPosition();
	CheckTouch();  // TODO: Too many messages!
*/

}

// move data from Serial buffer to command buffer
void CheckSerialInput() {
	if (Serial.available()) {
		int bCnt = 0;
		delay(1); // make sure one command is completed
		while (Serial.available()) {
			cmdBuffer.write(Serial.read());
			bCnt++;
			if (!Serial.available()) delay(1); // Try to receive a full comment in single batch
		}
		// DEBUG.printf("\nSerial received %d bytes\n", bCnt);
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

			case '#':
				goNext = HAILZD_Command();
				break;

			case 'A':
			case 'a':
			/*
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
			*/
			case 'P':
			/*
			case 'R':
			case 'T':
			case 't':
			case 'S':
			case 'U':
			case 'W':
			*/
			case 'i':  // special for debug
			case 'I':  // special for debug
			case 'Z':
       
				goNext = V1_Command();
				break;

			default:
				cmdSkip(true);
				break;

		}
		// for some situation, command data not yet completed.
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
			if ((lastCmdMs)	&& (millis() - lastCmdMs > config.maxCommandWaitMs())) {
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


void enableTxCallback(bool send) {
    if (send) {
        robotPort.enableTx(true); 
		delayMicroseconds(10);
    } else {
        robotPort.enableTx(false);
    }
}

