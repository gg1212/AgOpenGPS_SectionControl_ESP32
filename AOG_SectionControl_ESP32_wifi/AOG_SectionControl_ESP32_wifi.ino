// Section Control code for AgOpenGPS V5.7.2 for ESP32
// Wifi UDP only
// runs a wifi client and connect to mobile hotspot in the same network as AOG computer

// forked from MTZ8302 ESP32 SectionControl GitHub https://github.com/mtz8302
//
//
// Man/Auto pin switch to GND
// Section pin switch to GND
// Main pin switch to 3.3v
//
//

//libraries -------------------------------
#include "EEPROM.h"
#include "Update.h"
#include <WiFi.h>
#include <WiFiUdp.h>


byte vers_nr = 1;

struct set {
  //User config: ***********************************************************************************

  uint16_t BaudRate = 38400;              // Baudrate = speed of serial port or USB or Bluetooth. AOG uses 38400 for UART

  //the following lines should be configured by the user to fit the programm to the sprayer/ESP32
  //GPIOs of the ESP32 (current setting is for the layout shown as example WIKI)

  uint8_t SectNum = 7;					// number of sections
  uint8_t SectRelaysInst = 1;				// relays for SC output are equiped (0=no output, only documentation)
  uint8_t SectRelaysON = 0;				// relays spray on 1 or 0 (high or low)
  uint8_t Relay_PIN[16] = {15, 2, 4, 16, 17, 18, 19, 21, 255, 255, 255, 255, 255, 255, 255}; //GPIOs of ESP32 OUT to sections of sprayer HIGH/3.3V = ON
  uint8_t Relais_MainValve_PIN = 255;		// PIN for Main fluid valve 255 = unused
  uint8_t SectSWInst = 1;					// 1 if section input switches are equiped, else: 0
  uint8_t SectSWAutoOrOn = 0;				// Section switches spray/auto on 1 = high = used with pullup, 0 = low = pulldown
  uint8_t SectSW_PIN[16] = {13, 12, 14, 27, 26, 25, 33, 255, 255, 255, 255, 255, 255, 255}; //section switches to GPIOs of ESP32
  uint8_t	SectMainSWType = 1;				// 0 = not equiped 1 = (ON)-OFF-(ON) toggle switch or push buttons 2 = connected to hitch level sensor 3 = inverted hitch level sensor
  uint16_t	HitchLevelVal = 2000;		// Value for hitch level: switch AOG section control to Auto if lower than... ESP:2000 nano 500
  uint8_t	SectMainSW_PIN = 35;			// ESP32 to AOG Main auto toggle switch open=nothing/AOG button GND=OFF +3,3=AOG Auto on	OR connected to hitch level sensor
  uint8_t	SectAutoManSW_PIN = 32;			// Main Auto/Manual switch 39:!!no internal pullup!!

  uint8_t DocOnly = 0;					// 0: use as section control, 1: Documentation only = AOG writes the state of the input switches


  bool debugmode = false;
  bool debugmodeRelay = false;
  bool debugmodeSwitches = false;
  bool debugmodeDataFromAOG = false;
  bool debugmodeDataToAOG = false;

  // END of user config ****************************************************************************
}; set Set;

//Fill in your WiFi network info SSID and Password
const char* ssid = "";
const char* password = "";


bool EEPROM_clear = false;

unsigned long previousMillis = 0;
unsigned long interval = 5000;

WiFiUDP WiFiUDPFromAOG = WiFiUDP();
WiFiUDP WiFiUDPToAOG = WiFiUDP();

IPAddress WiFi_ipDestination, WiFi_Myip;
byte WiFi_ipDest_ending = 255;  

byte SCToAOG[14] = { 0x80, 0x81, 0x7B, 0xEA, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0}, SCToAOGOld[14] = { 0x80, 0x81, 0x7B, 0xEA, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// sentences to AOG V5 -------------------------------------------------------------------
const byte FromAOGSentenceHeader[3] = { 0x80, 0x81, 0x7F };
#define SteerDataFromAOGHeader  0xFE  //take section info from steer packet
#define SCHelloFromAOG  0xC8

//write incoming Data to sentence array if it fits in
#define SentenceFromAOGMaxLength 14
byte SentenceFromAOG[SentenceFromAOGMaxLength], SentenceFromAOGLength;

//global, as serial/USB may not come at once, so values must stay for next loop
byte incomSentenceDigit = 0, DataToAOGLength = 14;

#define incommingDataArraySize 5
byte incommingBytes[incommingDataArraySize][500], incommingBytesArrayNr = 0, incommingBytesArrayNrToParse = 0;
unsigned int incommingDataLength[incommingDataArraySize] = { 0, 0, 0, 0, 0 };


TaskHandle_t taskHandle_DataFromAOGWiFi;

//values to decide position of section switch
#define SWOFF 650 // analog in is lower than .. for off
#define SWON 3000 // analog in is lower than .. for auto/on


//analog value of the toggle switches (default: middle=unpressed)
int MainSWVal = (SWOFF + SWON) / 2;
int MainSWValOld = (SWOFF + SWON) / 2;

//state of the switches HIGH = ON/Auto LOW = OFF
boolean SectSWVal[16] = { HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH };
boolean AutoSWVal = HIGH;
byte debugmodeSwitchesBak = 3;  //0 = false 1 = true 3 = not used

//loop time variables in microseconds
const unsigned long LOOP_TIME = 500;//in msec; 1000 = 1 Hz
const unsigned long SectSWDelayTime = 200;// 1500;//1400; //time the arduino waits after manual Switch is used before acception command from AOG in msec
unsigned long lastTime = LOOP_TIME;
unsigned long now = LOOP_TIME;
unsigned long DataFromAOGTime = 0;
unsigned long SectAutoSWTime = LOOP_TIME;
unsigned long SectAutoSWlastTime = LOOP_TIME;
unsigned long SectMainSWlastTime = LOOP_TIME;
unsigned long SectSWcurrentTime = LOOP_TIME;

//program flow

//bit 0 on byte[0] is section 1
byte SectGrFromAOG[] = { 0, 0 }; //Sections ON / OFF Bitwise !!!
byte RelayOUT[] = { 0, 0 }; //Sections ON / OFF Bitwise !!!
byte RelayOUTOld[] = { 0, 0 };
byte SectSWOffToAOG[] = { 0, 0 };
byte SectMainToAOG = 0;
byte uTurnRelay = 0;
boolean SectMainSWpressed = false;
boolean SectSWpressed = false;
boolean SectSWpressedLoop = false;
boolean SectAuto = true;
boolean SectAutoOld = true;
boolean SectAutoSWpressed = false;


boolean SectMainOn = false;

boolean newDataToAOG = false;

float gpsSpeed = 0; //speed from AgOpenGPS



void setup()
{
  delay(100);//wait for power to stabilize
  delay(100);//wait for IO chips to get ready
  //set up communication
  Serial.begin(Set.BaudRate);
  delay(10);
  delay(50);
  //get EEPROM data
  restoreEEprom();
  delay(100);
  
  //incomSentenceDigit = 0;
  
  //set GPIOs
  assignGPIOs();
  delay(50);

  //switches -> set relais
  if ((Set.SectSWInst) || (Set.SectMainSWType != 0)) {
    SectSWRead();
  }
  SetRelays();
  initWiFi();
  //get Data
  xTaskCreate(getDataFromAOGWiFi, "DataFromAOGHandleWiFi", 5000, NULL, 1, &taskHandle_DataFromAOGWiFi);

  delay(500);

}  //end setup


//-------------------------------------------------------------------------------------------------

void loop() {
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
  
  //new data from AOG? Data comes via extra task and is written into byte array. Parsing called here
  if (incommingDataLength[incommingBytesArrayNrToParse] != 0) {
    parseDataFromAOG();
    vTaskDelay(5);
  }
  else {
    vTaskDelay(5);  //wait if no new data to give time to other tasks
  }
  //read switches/inputs
  if ((Set.SectSWInst) || (Set.SectMainSWType != 0))
  {
    SectSWRead(); //checks if section switch is toggled and sets RelayOUT

    //reset debugSwitches: has been change to true, if debugmodeDataToAOG = true for 1 loop
    if ((Set.debugmodeSwitches) && (debugmodeSwitchesBak == 0)) {
      Set.debugmodeSwitches = false;
    }

    SCToAOG[10] = SectSWOffToAOG[0]; //LowByte if bit set -> AOG section forced off
    SCToAOG[12] = SectSWOffToAOG[1]; //HiByte if bit set -> AOG section forced off
    SCToAOG[5] = SectMainToAOG; // Bits: AOG section control AUTO, Section control OFF, Rate L+R ...
    //new data?
    for (byte idx = 5; idx < DataToAOGLength; idx++) {
      if (SCToAOG[idx] != SCToAOGOld[idx]) {
        newDataToAOG = true;
        SCToAOGOld[idx] = SCToAOG[idx];
      }
    }
    
  }
  //no switches: set Relais as AOG commands
  else {
    RelayOUT[0] = SectGrFromAOG[0];
    RelayOUT[1] = SectGrFromAOG[1];
  }

  SetRelays();


  //timed loop: runs or if new data from switches to send data to AOG
  now = millis();
  if ((now > LOOP_TIME + lastTime) || (newDataToAOG)) {
    lastTime = now;
    //add the checksum
    int CRCtoAOG = 0;
    for (byte i = 2; i < sizeof(SCToAOG) - 1; i++)
    {
      CRCtoAOG = (CRCtoAOG + SCToAOG[i]);
    }
    SCToAOG[sizeof(SCToAOG) - 1] = CRCtoAOG;

    AOGDataSend(); 
    //send data to AOG in loop because using newdatatoAOG is not reliable if a UDP packet is lost
    if (newDataToAOG) {
      vTaskDelay(5);
      AOGDataSend(); //send data to AOG
      newDataToAOG = false;
    }
  }
}
