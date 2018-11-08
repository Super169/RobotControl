#include "robot.h"

void InitEventHandler() {
	// Turn on 6050 even not set for autoStand, MPU can be used in other area
	//if (config.autoStand()) {
		MpuInit();
	//}
	eIdle.LoadData(EVENT_IDEL_FILE);
	eBusy.LoadData(EVENT_BUSY_FILE);

}

void RobotEventHandler() {

   	CheckVoltage();

	// Check auto-response action (should only play if not in action)
	if (V2_ActionPlaying) return;
	CheckPosition();
	CheckTouch();  // TODO: Too many messages!

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
