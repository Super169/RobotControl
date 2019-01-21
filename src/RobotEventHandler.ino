#include "robot.h"

void InitEventHandler() {
	
	eventHandlerSuspended = false;
	ssbPort.begin(ssbConfig.baud);

	ssb.Begin(&ssbPort, &DEBUG);
	ssb.SetEnableTxCalback(EnableSsbTxCallBack);

	edsMpu6050.SetEnabled(config.mpuEnabled());
	if (config.mpuEnabled()) {
		_dbg->log(10,0,"edsMpu6050.Setup(0x%02X, %d, %d)", EDS_MPU6050_I2CADDR, (1000 / config.mpuPositionCheckFreq()), config.mpuCheckFreq());
		edsMpu6050.Setup(EDS_MPU6050_I2CADDR, (1000 / config.mpuPositionCheckFreq()), config.mpuCheckFreq());
	} else {
		_dbg->log(10,0,"MPU6050 disabled");
	}

	edsTouch.SetEnabled(config.touchEnabled());
	if (config.touchEnabled()) {
		_dbg->log(10,0,"edsTouch.Setup(%d, %d, %d)", EDS_TOUCH_GPIO, config.touchDetectPeriod(), config.touchReleasePeriod());
		edsTouch.Setup(EDS_TOUCH_GPIO, config.touchDetectPeriod(), config.touchReleasePeriod());
	} else {
		_dbg->log(10,0,"Touch disabled");
	}

	_dbg->log(10,0,"Sub-system board: GPIO: %d, BAUD: %ld, BUffer: %d", ssbConfig.tx_pin, ssbConfig.baud, ssbConfig.buffer_size);

	edsPsxButton.SetEnabled(config.psxEnabled());
	if (config.psxEnabled()) {
		_dbg->log(10,0,"edsPsxButton.Setup(&ssb, %d, %d, %d)", config.psxCheckMs(), config.psxNoEventMs(), config.psxIgnoreRepeatMs());
		edsPsxButton.Setup(&ssb, config.psxCheckMs(), config.psxNoEventMs(), config.psxIgnoreRepeatMs());
	} else {
		_dbg->log(10,0,"PSX disabled");
	}
	
	edsSonic.SetEnabled(config.sonicEnabled());
	if (config.sonicEnabled()) {
		_dbg->log(10,0,"edsSonic.Setup(&ssb)");
		edsSonic.Setup(&ssb);
	} else {
		_dbg->log(10,0,"Sonic sensor disabled");
	}


	// TODO: add normal check ms to config object
	// _dbg->log(10,0,"edsBattery.Setup(%d, %d, %d, %d)", config.batteryMinValue(), config.batteryMaxValue(), 5000,config.voltageAlarmInterval() * 1000);
	// edsBattery.Setup(config.batteryMinValue(), config.batteryMaxValue(), 5000, config.voltageAlarmInterval() * 1000);
	_dbg->log(10,0,"edsBattery.Setup(%d, %d, %d, %d)", config.batteryMinValue(), config.batteryMaxValue(), config.batteryNormalSec() * 1000, config.batteryAlarmSec() * 1000);
	edsBattery.Setup(config.batteryMinValue(), config.batteryMaxValue(), config.batteryNormalSec() * 1000, config.batteryAlarmSec() * 1000);

	// Use array later
	for (int i = 0; i <= ED_MAX_DEVICE; i++) eds[i] = NULL;

	eds[(byte) EventData::DEVICE::mpu] = &edsMpu6050;
	eds[(byte) EventData::DEVICE::touch] = &edsTouch;
	eds[(byte) EventData::DEVICE::psx_button] = &edsPsxButton;
	eds[(byte) EventData::DEVICE::battery] = &edsBattery;
	eds[(byte) EventData::DEVICE::sonic] = &edsSonic;

	eIdle.LoadData(EVENT_IDEL_FILE);
	eBusy.LoadData(EVENT_BUSY_FILE);

}

void RobotEventHandler() {

	if (eventHandlerSuspended) return;

	if (millis() < nextHandlerMs) return;

/*
*	New version with EventHandler object
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

	// Part 1: Data Collection
	eData.Clear();

	EventHandler *eActive;
	if (V2_ActionPlaying) {
		eActive = &eBusy;
		if (lastPlaying) {
			// status changed, need to reset everything if possble
			eIdle.ResetEventLastMs();
		}
	} else {
		eActive = &eIdle;
		if (lastPlaying) {
			// status changed, need to reset everything if possble
			eBusy.ResetEventLastMs();
		}
	}
	lastPlaying = V2_ActionPlaying;

	for (int device = 0; device <= ED_MAX_DEVICE; device++) {
		if (eds[device] != NULL) {
			if (eActive->IsRequired(device)) {
				if (eds[device]->GetData()) showResult = true;
			} 
		}
	}
	
	// Part 2: Condition checking

	EventHandler::EVENT event = eActive->CheckEvents();
    EventHandler::ACTION action = event.data.action;


	// Part 3: Post checking control 
	/*
	*	Need to think about how to prevent keep triggering the same event as condition may not changed
	*   May add time interval for eData once handled
	*/
	for (int device = 0; device <= ED_MAX_DEVICE; device++) {
		if (eds[device] != NULL) {
			if (eActive->IsRequired(device)) {
				eds[device]->PostHandler( (bool) (event.data.type), 
							  			  eActive->LastEventRelated(device),
										  eActive->IsPending(device) );
			}
		}
	}
	
	// Part 4: Action	
	
	if (event.data.type) {

		if (_dbg->require(250)) {
			DEBUG.printf("\n##########\n");
			eData.DumpData(&DEBUG);
			eActive->DumpEvents(&DEBUG);
			DEBUG.printf("###### Event matched: %d\n##########\n\n", event.data.type);
		}
		
		bool validAction = true;
		switch (action.data.type) {

            case (uint8_t) EventHandler::ACTION_TYPE::na:
				validAction = false;
                break;
	
			case (uint8_t) EventHandler::ACTION_TYPE::headLed:
                if (_dbg->require(110)) _dbg->log(110,0,"Set head led %d \n", action.data.parm_1);
				ActionSetHeadLed(action.data.parm_1);
				break;


            case (uint8_t) EventHandler::ACTION_TYPE::playAction:
                if (_dbg->require(110)) _dbg->log(110,0,"Play action %d \n", action.data.parm_1);
				ActionPlayAction(action.data.parm_1);
                break;

            case (uint8_t) EventHandler::ACTION_TYPE::stopAction:
                if (_dbg->require(110)) _dbg->log(110,0,"Stop action\n");
				ActionStopPlay();
                break;

            case (uint8_t) EventHandler::ACTION_TYPE::mp3PlayMp3:
                if (_dbg->require(110)) _dbg->log(110,0,"Play mp3 %d\n", action.u16data.parm_u16);
				ActionMp3PlayMp3(action.u16data.parm_u16);
                break;

            case (uint8_t) EventHandler::ACTION_TYPE::mp3PlayFile:
                if (_dbg->require(110)) _dbg->log(110,0,"Play mp3 at  %d : %d \n", action.data.parm_1, action.data.parm_2);
				ActionMp3PlayFile(action.data.parm_1, action.data.parm_2);
                break;

			case (uint8_t) EventHandler::ACTION_TYPE::mp3Stop:
                if (_dbg->require(110)) _dbg->log(110,0,"Stop play MP3\n");
				ActionMp3Stop();
				break;


            case (uint8_t) EventHandler::ACTION_TYPE::gpio:
                if (_dbg->require(110)) _dbg->log(110,0,"Set gpio %d to %s\n", action.data.parm_1, 
                                                    	(action.data.parm_2 ? "HIGH" : "LOW"));
				digitalWrite(action.data.parm_1, action.data.parm_2);
                break;

			
            case (uint8_t) EventHandler::ACTION_TYPE::system_action:
                if (_dbg->require(110)) _dbg->log(110,0,"Play system action %d \n", action.data.parm_1);
				ActionPlaySystemAction(action.data.parm_1);
                break;

            case (uint8_t) EventHandler::ACTION_TYPE::servo:
                if (_dbg->require(110)) _dbg->log(110,0,"Move servo %d (%d : %d ms)", action.data.parm_1, (int8_t) action.data.parm_2, action.data.parm_3);
				ActionServo(action.data.parm_1, (int8_t) action.data.parm_2, action.data.parm_3);
                break;


            default:
                if (_dbg->require(110)) _dbg->log(110,0,"Unknown action %d \n", action.data.type);
				validAction = false;
                break;			
		}

		if ((validAction) && eActive->LastEventRelated((uint8_t) EventData::DEVICE::psx_button))  {
			if (config.psxShock()) edsPsxButton.Shock();
		}

	} else {
		if (_dbg->require(250) && showResult) {
			DEBUG.printf("\n----------\n");
			eData.DumpData(&DEBUG);
			eActive->DumpEvents(&DEBUG);
			DEBUG.printf("No Event matched\n----------\n\n");
		}
	}

	/*
	if (showResult && (millis() > nextShowMs )) {
		if (V2_ActionPlaying)
		eData.DumpData(&Serial1);
		nextShowMs = millis() + 1000;
	}
	*/

	nextHandlerMs = millis() + EVENT_HANDLER_ELAPSE_MS;

}


void EnableSsbTxCallBack(bool send) {
    if (send) {
        ssbPort.enableTx(true); 
		delayMicroseconds(10);
    } else {
        ssbPort.enableTx(false);
    }
}
