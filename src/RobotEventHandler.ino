#include "robot.h"

void InitEventHandler() {
	// Turn on 6050 even not set for autoStand, MPU can be used in other area
	//if (config.autoStand()) {
		MpuInit();
	//}
	
	ssbPort.begin(ssbConfig.baud);

	ssb.Begin(&ssbPort, &DEBUG);
	ssb.SetEnableTxCalback(EnableSsbTxCallBack);

	_dbg->msg("edsMpu6050.Setup(0x%02X)", EDS_MPU6050_I2CADDR);
	edsMpu6050.Setup(EDS_MPU6050_I2CADDR, config.mpuCheckFreq(), (1000 / config.positionCheckFreq()));

	_dbg->msg("edsTouch.Setup(%d, %d, %d)", EDS_TOUCH_GPIO, config.touchDetectPeriod(), config.touchReleasePeriod());
	edsTouch.Setup(EDS_TOUCH_GPIO, config.touchDetectPeriod(), config.touchReleasePeriod());

	_dbg->msg("edsPsxButton.Setup() => GPIO: %d, BAUD: %ld, BUffer: %d", ssbConfig.tx_pin, ssbConfig.baud, ssbConfig.buffer_size);
	edsPsxButton.Setup(&ssb);
	
	// TODO: add normal check ms to config object
	_dbg->msg("edsBattery.Setup(%d, %d, %d, %d)", config.minVoltage(), config.maxVoltage(), 5000,config.voltageAlarmInterval() * 1000);
	edsBattery.Setup(config.minVoltage(), config.maxVoltage(), 5000, config.voltageAlarmInterval() * 1000);

	// Use array later
	for (int i = 0; i <= ED_MAX_DEVICE; i++) eds[i] = NULL;

	eds[(byte) EventData::DEVICE::mpu] = &edsMpu6050;
	eds[(byte) EventData::DEVICE::touch] = &edsTouch;
	eds[(byte) EventData::DEVICE::psx_button] = &edsPsxButton;
	eds[(byte) EventData::DEVICE::battery] = &edsBattery;

	eIdle.LoadData(EVENT_IDEL_FILE);
	eBusy.LoadData(EVENT_BUSY_FILE);

}


unsigned long nextHandlerMs = 0;


unsigned long nextMpuCheckMs = 0;
unsigned long nextTouchCheckMs = 0;
unsigned long nextShowMpuMs = 0;

void RobotEventHandler() {

if (millis() < nextHandlerMs) return;

#ifdef NEW_EVENT_HANDLER
/*
*	New version will EventHandler object
*	- TODO: define generic data source class for better program flow
*           sensors should be inherit from this data source class
*           Generic method for data soruce:
*             - Initialization
*             - Data Colection
*             - Post Checking Control
*           with internal logic to handle frequency of checking
*
*	1) Data collection
*	2) Condition checking
*	3) Post checking control 
*	4) Action
*
*/
	bool showResult = false;

	// Data Collection
	eData.Clear();

	EventHandler *eActive;
	if (V2_ActionPlaying) {
		eActive = &eBusy;
	} else {
		eActive = &eIdle;
	}

	// This part can be changed to use a loop if all data source changed to EventDataSource type

	/*
	if (eActive->IsRequired((uint8_t) EventData::DEVICE::mpu)) {
		if (edsMpu6050.GetData()) showResult = true;
	}

	if (eActive->IsRequired((uint8_t) EventData::DEVICE::touch)) {
		if (edsTouch.GetData()) showResult = true;
	}

	if (eActive->IsRequired((uint8_t) EventData::DEVICE::psx_button)) {
		if (edsPsxButton.GetData()) showResult = true;
	}

	if (eActive->IsRequired((uint8_t) EventData::DEVICE::battery)) {
		if (edsBattery.GetData()) showResult = true;
	} 
	*/

	for (int device = 0; device <= ED_MAX_DEVICE; device++) {
		if (eds[device] != NULL) {
			if (eActive->IsRequired(device)) {
				if (eds[device]->GetData()) showResult = true;
			} 
		}
	}
	
	
	// TODO: Study if it should move MpuGetData() to EsdMpu6050
	if (millis() > nextMpuCheckMs) {

		if (mpuExists && eActive->IsRequired((uint8_t) EventData::DEVICE::mpu)) {
			// DEBUG.printf("MPU is required\n");
			if (MpuGetData()) {
				eData.SetData(EventData::DEVICE::mpu, 0, 0, ax);
				eData.SetData(EventData::DEVICE::mpu, 0, 1, ay);
				eData.SetData(EventData::DEVICE::mpu, 0, 2, az);
				if (millis() > nextShowMpuMs) {
					if (debug) DEBUG.printf("-> x: %d , y: %d , z: %d \n", ax, ay, az);
					showResult = true;
					nextShowMpuMs = millis() + 5000;
				}
			} else {
				if (debug) DEBUG.printf("Fail reading MPU data\n");
			}
		} else {
			// DEBUG.printf("MPU %sexists\n",  (mpuExists ? "" : "not "));
			// DEBUG.printf("MPU %srequired", (eActive->IsRequired((uint8_t) EventData::DEVICE::mpu) ? "" : "not "));
		}
		nextMpuCheckMs = millis() + (1000 / config.positionCheckFreq());
	}




	// Condition checking

	EventHandler::EVENT event = eActive->CheckEvents();
    EventHandler::ACTION action = event.data.action;


	// Post checking control 
	/*
	*	Need to think about how to prevent keep triggering the same event as condition may not changed
	*   May add time interval for eData once handled
	*/

	// Action	
	bool eventMatched = false;
	
	if (event.data.type) {

		if (debug) {
			_dbg->msg("##########");
			eData.DumpData(&DEBUG);
			eActive->DumpEvents(&DEBUG);
			_dbg->msg("###### Event matched: %d", event.data.type);
			_dbg->msg("##########\n\n");
		}
		
		eventMatched = true;
		switch (action.data.type) {

            case (uint8_t) EventHandler::ACTION_TYPE::na:
                break;
	
			case (uint8_t) EventHandler::ACTION_TYPE::headLed:
                if (debug) _dbg->msg("Set head led %d \n", action.data.parm_1);
				ActionSetHeadLed(action.data.parm_1);
				break;


            case (uint8_t) EventHandler::ACTION_TYPE::playAction:
                if (debug) DEBUG.printf("Play action %d \n", action.data.parm_1);
				ActionPlayAction(action.data.parm_1);
                break;

            case (uint8_t) EventHandler::ACTION_TYPE::stopAction:
                if (debug) DEBUG.printf("Stop action\n");
				ActionStopPlay();
                break;

            case (uint8_t) EventHandler::ACTION_TYPE::mp3PlayMp3:
                if (debug) DEBUG.printf("Play mp3 %d\n", action.u16data.parm_u16);
				ActionMp3PlayMp3(action.u16data.parm_u16);
                break;

            case (uint8_t) EventHandler::ACTION_TYPE::mp3PlayFile:
                if (debug) DEBUG.printf("Play mp3 at  %d : %d \n", action.data.parm_1, action.data.parm_2);
				ActionMp3PlayFile(action.data.parm_1, action.data.parm_2);
                break;

			case (uint8_t) EventHandler::ACTION_TYPE::mp3Stop:
                if (debug) DEBUG.printf("Stop play MP3\n");
				ActionMp3Stop();
				break;


            case (uint8_t) EventHandler::ACTION_TYPE::gpio:
                if (debug) DEBUG.printf("Set gpio %d to %s\n", action.data.parm_1, 
                                                    (action.data.parm_2 ? "HIGH" : "LOW"));
				digitalWrite(action.data.parm_1, action.data.parm_2);
                break;

			
            case (uint8_t) EventHandler::ACTION_TYPE::system_action:
                if (debug) DEBUG.printf("Play system action %d \n", action.data.parm_1);
				ActionPlaySystemAction(action.data.parm_1);
                break;


            default:
                if (debug) DEBUG.printf("Unknown action %d \n", action.data.type);
                break;			
		}
	} else {
		if (debug && showResult) {
			_dbg->msg("----------");
			eData.DumpData(&DEBUG);
			eActive->DumpEvents(&DEBUG);
			_dbg->msg("No Event matched");
			_dbg->msg("----------\n\n");
		}
	}


	/*
	if (eActive->IsRequired((uint8_t) EventData::DEVICE::mpu)) {
		edsMpu6050.PostHandler( eventMatched, 
		                        eActive->LastEventRelated((uint8_t) EventData::DEVICE::mpu),
							    eActive->IsPending((uint8_t) EventData::DEVICE::mpu) );
	}

	if (eActive->IsRequired((uint8_t) EventData::DEVICE::touch)) {
		edsTouch.PostHandler( eventMatched, 
		                      eActive->LastEventRelated((uint8_t) EventData::DEVICE::touch),
							  eActive->IsPending((uint8_t) EventData::DEVICE::touch) );
	}

	if (eActive->IsRequired((uint8_t) EventData::DEVICE::psx_button)) {
		edsPsxButton.PostHandler( eventMatched, 
								  eActive->LastEventRelated((uint8_t) EventData::DEVICE::psx_button),
								  eActive->IsPending((uint8_t) EventData::DEVICE::psx_button) );
	}


	if (eActive->IsRequired((uint8_t) EventData::DEVICE::battery)) {
		edsBattery.PostHandler( eventMatched, 
		                        eActive->LastEventRelated((uint8_t) EventData::DEVICE::battery),
								eActive->IsPending((uint8_t) EventData::DEVICE::battery) );
	}
	*/

	for (int device = 0; device <= ED_MAX_DEVICE; device++) {
		if (eds[device] != NULL) {
			if (eActive->IsRequired(device)) {
				eds[device]->PostHandler( eventMatched, 
							  			  eActive->LastEventRelated(device),
										  eActive->IsPending(device) );
			}
		}
	}
	

#else

	// Old version with direct setting

	#ifndef DISABLE_BATTERY_CHECK
   	CheckVoltage();
	#endif

	// Check auto-response action (should only play if not in action)
	if (V2_ActionPlaying) return;
	CheckPosition();
	CheckTouch();  // TODO: Too many messages!

#endif

nextHandlerMs = millis() + EVENT_HANDLER_ELAPSE_MS;

}


unsigned nextPositionCheckMs = 0;

void CheckPosition() {
  if (!config.autoStand()) return;
  if (millis() > nextPositionCheckMs) {
    // DEBUG.println("No Client connected, or connection lost");
      // if (EN_MPU6050) MpuGetActionHandle();
    // if (debug) DEBUG.println("MpuGetActionHandle"); // too many message will cause debug port hang
    MpuGetActionHandle();

    // TODO, use config for frequency
    nextPositionCheckMs = millis() + (1000 / config.positionCheckFreq());
  }
}

void CheckTouch() {
	if (!config.enableTouch()) return;
	//touch handle
	
	// uint8_t touchMotion = DetectTouchMotion();
	uint8_t touchMotion = CheckTouchAction();
	//if(touchMotion == TOUCH_LONG) ReserveEyeBreath();
	//if(touchMotion == TOUCH_DOUBLE) ReserveEyeBlink();
	EyeLedHandle();
}

// ADC_MODE(ADC_VCC);  -- only use if checking input voltage ESP.getVcc() is required.
unsigned long nextVoltageMs = 0;
unsigned long nextVoltageAlarmMs = 0;

void CheckVoltage() {
	if (millis() > nextVoltageMs) {

		uint16_t v = analogRead(0);
		// if (debug) DEBUG.printf("analgeRead(0): %d\n", v);
		int iPower = GetPower(v);
		if (config.enableOLED()) {

			// Code form James
			myOLED.printNum(104,0,iPower, 10, 3, false);
			myOLED.print(122,0,"%");
			myOLED.show();

			// Code from L, TODO: use config to get reference voltage for conversion
			myOLED.print(0,5,"Volt: ");
			myOLED.printFloat((float)(analogRead(A0)/1024.0*11.0));
			// myOLED.printNum(0,10,analogRead(A0));
			myOLED.show();
		}

		if (v < config.alarmVoltage())	{
			if (millis() > nextVoltageAlarmMs) {
				nextVoltageAlarmMs += (config.voltageAlarmInterval() * 1000);
				if (debug) DEBUG.printf("Battery low: %d (%d) \n", v, config.alarmVoltage());
				if (config.voltageAlarmMp3()) {
					mp3.begin();
					mp3.stop();
					mp3.playMp3File(config.voltageAlarmMp3());
					mp3.end();
				}
			}
		} 

		nextVoltageMs = millis() + 1000;
		// DEBUG.printf("v: %d, min: %d, max: %d, alarm: %d, power: %d%%\n\n", v, config.minVoltage(), config.maxVoltage(), config.alarmVoltage(), iPower);
	}

}

// For consistence, build common function to convert A0 value to Power rate
byte GetPower(uint16_t v) {
	// Power in precentage instead of voltage
	float power = ((float) (v - config.minVoltage()) / (config.maxVoltage() - config.minVoltage()) * 100.0f);
	int iPower = (int) (power + 0.5); // round up
	if (iPower > 100) iPower = 100;
	if (iPower < 0) iPower = 0;
	return (byte) iPower;
}


void EnableSsbTxCallBack(bool send) {
    if (send) {
        ssbPort.enableTx(true); 
		delayMicroseconds(10);
    } else {
        ssbPort.enableTx(false);
    }
}
