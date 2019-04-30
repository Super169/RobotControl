#include "robot.h"

void InitEventHandler() {

	eventHandlerSuspended = false;
	ssbPort.begin(ssbConfig.baud);

	ssb.Begin(&ssbPort, &DEBUG);
	ssb.SetEnableTxCalback(EnableSsbTxCallBack);

	eds = (EventDataSource**) malloc(sizeof(EventDataSource*) * (eData.DevCount()));
	for (int i = 0; i < eData.DevCount(); i++) eds[i] = NULL;

	for (int id = 0; id < ED_COUNT_MPU; id++) {
		edsMpu6050[id] = new EdsMpu6050(&eData, _dbg, id);
		edsMpu6050[id]->SetEnabled(config.mpuEnabled());
		if (config.mpuEnabled()) {
			_dbg->log(10,0,"edsMpu6050.Setup(0x%02X, %d, %d)", EDS_MPU6050_I2CADDR, (1000 / config.mpuPositionCheckFreq()), config.mpuCheckFreq());
			edsMpu6050[id]->Setup(EDS_MPU6050_I2CADDR, (1000 / config.mpuPositionCheckFreq()), config.mpuCheckFreq());
		} else {
			_dbg->log(10,0,"MPU6050 disabled");
		}
		eds[eData.DevOffset((byte) EventData::DEVICE::mpu,id)] = edsMpu6050[id];
	}

	for (int id = 0; id < ED_COUNT_TOUCH; id++) {
		edsTouch[id] = new EdsTouch(&eData, _dbg, 0);
		edsTouch[id]->SetEnabled(config.touchEnabled());
		if (config.touchEnabled()) {
			_dbg->log(10,0,"edsTouch.Setup(%d, %d, %d)", EDS_TOUCH_GPIO, config.touchDetectPeriod(), config.touchReleasePeriod());
			edsTouch[id]->Setup(EDS_TOUCH_GPIO, config.touchDetectPeriod(), config.touchReleasePeriod());
		} else {
			_dbg->log(10,0,"Touch disabled");
		}
		eds[eData.DevOffset((byte) EventData::DEVICE::touch,id)] = edsTouch[id];
	}
	_dbg->log(10,0,"Sub-system board: GPIO: %d, BAUD: %ld, BUffer: %d", ssbConfig.tx_pin, ssbConfig.baud, ssbConfig.buffer_size);

	for (int id = 0; id < ED_COUNT_PSXBUTTON; id++) {
		edsPsxButton[id] = new EdsPsxButton(&eData, _dbg, id);
		edsPsxButton[id]->SetEnabled(config.psxEnabled());
		if (config.psxEnabled()) {
			_dbg->log(10,0,"edsPsxButton.Setup(&ssb, %d, %d, %d)", config.psxCheckMs(), config.psxNoEventMs(), config.psxIgnoreRepeatMs());
			edsPsxButton[id]->Setup(&ssb, config.psxCheckMs(), config.psxNoEventMs(), config.psxIgnoreRepeatMs());
		} else {
			_dbg->log(10,0,"PSX disabled");
		}
		eds[eData.DevOffset((byte) EventData::DEVICE::psx_button,id)] = edsPsxButton[id];
	}

	// TODO: add normal check ms to config object
	for (int id = 0; id < ED_COUNT_BATTERY; id++) {
		edsBattery[id] = new EdsBattery(&eData, _dbg, id);
		_dbg->log(10,0,"edsBattery.Setup(%d, %d, %d, %d)", config.batteryMinValue(), config.batteryMaxValue(), config.batteryNormalSec() * 1000, config.batteryAlarmSec() * 1000);
		edsBattery[id]->Setup(config.batteryMinValue(), config.batteryMaxValue(), config.batteryNormalSec() * 1000, config.batteryAlarmSec() * 1000);
		eds[eData.DevOffset((byte) EventData::DEVICE::battery,id)] = edsBattery[id];
	}

	
	for (int id = 0; id < ED_COUNT_SONIC; id++) {
		edsSonic[id] = new EdsSonic(&eData, _dbg, id);
		edsSonic[id]->SetEnabled(config.sonicEnabled());
		if (config.sonicEnabled()) {
			_dbg->log(10,0,"edsSonic[%d].Setup(&ssb, %d, %d)", id, 1000 / config.sonicCheckFreq(), config.sonicDelaySec() * 1000);
			edsSonic[id]->Setup(&ssb, 1000 / config.sonicCheckFreq(), config.sonicDelaySec() * 1000);
		} else {
			_dbg->log(10,0,"Sonic sensor disabled");
		}
		// By default, suspend sonic related devices
		edsSonic[id]->Suspend(true);
		eds[eData.DevOffset((byte) EventData::DEVICE::sonic,id)] = edsSonic[id];
	}

	for (int id = 0; id < ED_COUNT_MAZE; id++) {
		edsMaze[id] = new EdsMaze(&eData, _dbg, id);
		edsMaze[id]->SetEnabled(config.sonicEnabled());
		if (config.sonicEnabled()) {
			_dbg->log(10,0,"edsMaze[%d].Setup(&ssb, &rs, %d, %d, %d, %d, %d, %d, %d)", id,  config.mazeWallDistance(), 
								config.mazeServo(), config.mazeServoDirection() , config.mazeServoMoveMs(), config.mazeServoWaitMs(),
								1000 / config.sonicCheckFreq(), config.sonicDelaySec() * 1000);
			edsMaze[id]->Setup(&ssb, &rs, config.mazeWallDistance(), 
								config.mazeServo(), config.mazeServoDirection() , config.mazeServoMoveMs(), config.mazeServoWaitMs(),
								1000 / config.sonicCheckFreq(), config.sonicDelaySec() * 1000);
		} else {
			_dbg->log(10,0,"Maze feature disabled");
		}
		// By default, suspend sonic related devices
		edsMaze[id]->Suspend(true);
		eds[eData.DevOffset((byte) EventData::DEVICE::maze,id)] = edsMaze[id];
	}


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

	for (int device = 0; device < eData.DevCount(); device++) {		
		if (eds[device] != NULL) {
			if (eActive->IsRequired(eds[device]->Device(), eds[device]->DevId())) {
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

	for (int devIdx = 0; devIdx < eData.DevCount(); devIdx++) {
		if (eds[devIdx] != NULL) {
			int device = eds[devIdx]->Device();
			int devId = eds[devIdx]->DevId();
			if (eActive->IsRequired(device, devId)) {
				eds[devIdx]->PostHandler( (bool) (event.data.type), 
							  			  eActive->LastEventRelated(device, devId),
										  eActive->IsPending(device, devId) );
				
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

			case (uint8_t) EventHandler::ACTION_TYPE::sonic:
                if (_dbg->require(110)) _dbg->log(110,0,"Turn sonic %s", (action.data.parm_1 ==  1 ? "On" : "Off"));
				ActionSonic(action.data.parm_1);
				break;

            default:
                if (_dbg->require(110)) _dbg->log(110,0,"Unknown action %d \n", action.data.type);
				validAction = false;
                break;			
		}

		// TODO: check relattion based on devId
		if (config.psxShock()) {
			for (int id = 0; id < ED_COUNT_PSXBUTTON; id++) {
				if ((validAction) && eActive->LastEventRelated((uint8_t) EventData::DEVICE::psx_button, id))  {
					edsPsxButton[id]->Shock();
				}
			}
		}

	} else {
		if (_dbg->require(250) && showResult) {
			DEBUG.printf("\n----------\n");
			eData.DumpData(&DEBUG);
			eActive->DumpEvents(&DEBUG);
			DEBUG.printf("No Event matched\n----------\n\n");
		}
	}

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
