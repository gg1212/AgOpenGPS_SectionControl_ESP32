void assignGPIOs() {
//#if HardwarePlatform == 0
	for (int i = 0; i < Set.SectNum; i++)
	{
		if (Set.SectSWInst) {
			if (Set.SectSW_PIN[i] < 255) { pinMode(Set.SectSW_PIN[i], INPUT_PULLUP); }
			else { Serial.print("no PIN set for section switch #"); Serial.println(i+1); }
			if (Set.Relay_PIN[i] < 255) { pinMode(Set.Relay_PIN[i], OUTPUT); }
			else { Serial.print("no PIN set for section relay #"); Serial.println(i+1); }
		}
	}
	if (Set.SectSWInst) {
		if (Set.SectAutoManSW_PIN < 255) { pinMode(Set.SectAutoManSW_PIN, INPUT_PULLUP); Serial.print("PIN ");Serial.print(Set.SectAutoManSW_PIN);Serial.print(" set for Auto/Manual switch"); }
		else { Serial.println("no PIN set for Auto/Manual toggle switch / buttons"); }
	}
	if (Set.SectMainSWType != 0) {
		if (Set.SectMainSW_PIN < 255) { pinMode(Set.SectMainSW_PIN, INPUT_PULLUP); Serial.print("PIN ");Serial.print(Set.SectMainSW_PIN);Serial.print(" set for Main switch"); }
		else { Serial.println("no PIN set for Main ON/OFF toggle switch / buttons"); }
	}

	if (Set.SectMainSW_PIN < 255) { pinMode(Set.Relais_MainValve_PIN, OUTPUT); }

	RelayOUT[0] = 0;
	RelayOUT[1] = 0;
	SetRelays();


//#endif


	//PWM rate settings Adjust to desired PWM Rate
	//TCCR1B = TCCR1B & B11111000 | B00000010;    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz
	//TCCR1B = TCCR1B & B11111000 | B00000011;    // set timer 1 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)

}
