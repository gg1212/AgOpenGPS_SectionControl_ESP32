
//-------------------------------------------------------------------------------------------------
//parseData 7. Maerz 2021
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  WiFi_Myip = WiFi.localIP();
  //Set IP destination to broadcast
  WiFi_ipDestination = WiFi.localIP();
  WiFi_ipDestination[3] = WiFi_ipDest_ending;
  WiFiUDPToAOG.begin(5555);
  WiFiUDPFromAOG.begin(8888);
  
}

void parseDataFromAOG() {
  bool isSteerDataFound = false, isHelloAOGFound = false, isScanFound=false;
	for (int i = 0; i < incommingDataLength[incommingBytesArrayNrToParse]; i++) {
		//sentence comming? V4.6: 80 81 7F PGN  incomSentenceDigit is set to 2 instead of 0 for new sentence
		if (incomSentenceDigit < 3) {
			if (incommingBytes[incommingBytesArrayNrToParse][i] == FromAOGSentenceHeader[incomSentenceDigit]) {
				//Serial.println("first 3 Bytes fit: sentence");
				SentenceFromAOG[incomSentenceDigit] = incommingBytes[incommingBytesArrayNrToParse][i];
				incomSentenceDigit++;
			}
			else {
				incomSentenceDigit = 0;
			}
		}//<3
		else {
			//write incoming Data to sentence array if it fits in
			if (incomSentenceDigit <= SentenceFromAOGMaxLength) {
				SentenceFromAOG[incomSentenceDigit] = incommingBytes[incommingBytesArrayNrToParse][i];
			}

			if (incomSentenceDigit == 3) {
				incomSentenceDigit++;
				//which sentence comming? PGN
				switch (incommingBytes[incommingBytesArrayNrToParse][i]) {
				case SteerDataFromAOGHeader:
					isSteerDataFound = true;//SC uses steer data in V5
					break;
        case SCHelloFromAOG:
          isHelloAOGFound=true;
          break;
        case 0xCA:
          isScanFound=true;
          break;
				default:
					//Serial.println("no matching PGN");
					incomSentenceDigit = 0;
					break;
				}//switch
			}//==3
			else {// >3
				if (incomSentenceDigit == 4) {//lenght
					SentenceFromAOGLength = incommingBytes[incommingBytesArrayNrToParse][i];
					incomSentenceDigit++;
				}//==4
				else
				{//>4	
					if (incomSentenceDigit == (SentenceFromAOGLength + 5)) { //sentence complete Length: + 4 byte header + 1 length + 1 CRC - 1 (starting at 0) 
						//sentence complete
           //CHECKSUM
						int CRCDataFromAOG = 0;
						for (byte i = 2; i < sizeof(SentenceFromAOG) - 1; i++)
						{
							CRCDataFromAOG = (CRCDataFromAOG + SentenceFromAOG[i]);
						}
						if (byte(CRCDataFromAOG) == incommingBytes[incommingBytesArrayNrToParse][i]) {
							//Serial.println("Checksum OK");
						}
						else
						{//checksum error
							if (Set.debugmodeDataFromAOG) { Serial.println("Checksum error"); }
							isSteerDataFound = false;
							incomSentenceDigit = 255;
						}
						/*
						if (Set.debugmodeDataFromAOG) {
							for (byte b = 0; b <= (SentenceFromAOGLength + 5); b++) {
								Serial.print(SentenceFromAOG[b]); Serial.print(" ");
							}
							Serial.println();
						}
*/ 
            if (isHelloAOGFound){
              //V5.7 UDP
              uint8_t helloFromMachine[] = { 128, 129, 123, 123, 5, 0, 0, 0, 0, 0, 71 };
              //Create response to AOG
              helloFromMachine[5] = RelayOUT[0];
              helloFromMachine[6] = RelayOUT[1];
              //Send to AOG
              WiFiUDPToAOG.beginPacket(WiFi_ipDestination, 9999);
              WiFiUDPToAOG.write(helloFromMachine, sizeof(helloFromMachine));
              WiFiUDPToAOG.endPacket();
              
              isHelloAOGFound=false;
              incomSentenceDigit = 255;
              
            }
            else if (isScanFound){
              //V5.7 scanner
              //Create scan reply with IP subnet
              uint8_t scanReply[] = { 128, 129, 123, 203, 7, WiFi_Myip[0], WiFi_Myip[1], WiFi_Myip[2], WiFi_Myip[3], WiFi_Myip[0], WiFi_Myip[1], WiFi_Myip[2], 23};
              //checksum
              
              int16_t CK_A = 0;
              for (uint8_t i = 2; i < sizeof(scanReply) - 1; i++)
              {
                  CK_A = (CK_A + scanReply[i]);
              }
              scanReply[sizeof(scanReply)-1] = CK_A;
              //Send to AOG
              WiFiUDPToAOG.beginPacket(WiFi_ipDestination, 9999);
              WiFiUDPToAOG.write(scanReply, sizeof(scanReply));
              WiFiUDPToAOG.endPacket();

              isScanFound=false;
              incomSentenceDigit = 255;

            }
						else if (isSteerDataFound) {
              //Serial.println("Steer from AOG found. ");

							SectGrFromAOG[0] = SentenceFromAOG[11];   // read Section control from AgOpenGPS 
							SectGrFromAOG[1] = SentenceFromAOG[12];   // read Section control from AgOpenGPS 
              //Serial.print(SentenceFromAOG[11]); 
              //Serial.print(" ");
              //Serial.print(SentenceFromAOG[12]);
              //Serial.print(" ");
							gpsSpeed = ((float)(SentenceFromAOG[6] << 8 | SentenceFromAOG[5])) * 0.1;

							isSteerDataFound = false;
							incomSentenceDigit = 255;
							DataFromAOGTime = millis();
							if (Set.debugmodeDataFromAOG) {
								Serial.print("speed: "); Serial.print(gpsSpeed);
								Serial.print(" SectGr[0]: "); Serial.print(SectGrFromAOG[0]);
								Serial.print(" SectGr[1]: "); Serial.println(SectGrFromAOG[1]);
							}
						}
					}//sentence complete

					incomSentenceDigit++;

					if (incomSentenceDigit > (SentenceFromAOGLength + 6)) {
						incomSentenceDigit = 0;
						//Serial.println("sentence too long");
					}
				}//>4
			}//==3
		}//<3
	}//for packetLength
	incommingDataLength[incommingBytesArrayNrToParse] = 0;
	incommingBytesArrayNrToParse = (incommingBytesArrayNrToParse + 1) % incommingDataArraySize;
}

void getDataFromAOGWiFi(void* pvParameters)
{
  byte nextincommingBytesArrayNr;
  unsigned int packetLength;


  if (Set.debugmode) { Serial.println("started Task get Data via WiFi"); }
  for (;;) {    
    //get Data
    packetLength = WiFiUDPFromAOG.parsePacket();
    if (packetLength > 0) {
      nextincommingBytesArrayNr = (incommingBytesArrayNr + 1) % incommingDataArraySize;
      WiFiUDPFromAOG.read(incommingBytes[nextincommingBytesArrayNr], packetLength);
      incommingBytesArrayNr = nextincommingBytesArrayNr;
      incommingDataLength[incommingBytesArrayNr] = packetLength;
    }
    else { vTaskDelay(10); }
  }
}

void AOGDataSend()
{
  WiFiUDPToAOG.beginPacket(WiFi_ipDestination, 9999);
  WiFiUDPToAOG.write(SCToAOG, DataToAOGLength);
  WiFiUDPToAOG.endPacket();
  /*Serial.print("Data to AOG ");
  for (byte n = 0; n < DataToAOGLength; n++) {
    Serial.print(SCToAOG[n]); Serial.print(" ");
  }
  Serial.println();
  */
}
