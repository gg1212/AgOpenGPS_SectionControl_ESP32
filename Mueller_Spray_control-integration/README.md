# Mueller Spray Control computer ESP32 Section Control
## Overview
I have connected the section control unit between the existing Mueller Spray Control boxes and the sprayer. This allows me to do section control without any modification to the existing boxes and cables.

In manual mode, AOG displays the working area based on the section status.

In automatic mode, AOG controls the relays and the sprayer does GPS automatic section control based on AOG data.

The overview of my setup:

![overview](/Mueller_Spray_control-integration/Images/global.png)

## Reading section state from reverse polarity valves
My valves are reverse polarity, so to read the state on the ESP32, I have added a PC817 8 channel board. I connected the section wire on which there is 12v when the section is open to the PC817 board.
There is also a bypass valve (reverse polarity too) on my sprayer and I connected that to the 8th channel on the PC817 to activate the main(work) switch.

![esp32](/Mueller_Spray_control-integration/Images/esp32.png)

## Relay board & sending section status to rate computer
### Relay board
Valves are reverse polarity here is the challenge, you need 2 relays per section valve. 12V goes to NC of relay 1 and NO of relay 2. GND goes to NO of relay 1 and NC of relay 2. The section valve is connected to relay 1 & 2 COM.

So for the 7 sections, I used 14 relays of a 16 relays board which I connected as in the example below.

### Rate controller section state
It is really important that the Spray Control computer knows how many sections are spraying in order to adjust rate correctly. So, the info should be coming from the output of the relays instead of the output of the switches as before. 

I have used another PC817 optocoupler board to get the 12v status from relay board and switch rate computer pin to GND. Note that it requires to remove the 3K resistor so I soldered a wire to bypass it.

![relays](/Mueller_Spray_control-integration/Images/relays.png)

## Hardware
I have designed a 3D printed box that fits between Mueller Spray Control box and the switches box, that way I can connect to the section wires and give the signal from real status(relay board+optocoupler). In order to centralize the switches, the man/auto switch has been put on this 3D printed box which fits perfectly like a 3rd Mueller box. I have also added 7 leds that shows the section state based on the relays as the original led on shows the switches state.

The box model is available in [3D](/Mueller_Spray_control-integration/3D) folder.

The connectors between Mueller boxes are Harting 48pins DIN 41612 connectors.

The connectors between Mueller box and sprayer are 30pins DIN 41622 connectors that I ordered at Beyne factory webshop : [30p male](https://erp.beyne.be/fr_BE/shop/product/cnc-30-m-prise-30-poles-m-4485) and [30p female](https://erp.beyne.be/fr_BE/shop/product/cnc-30-v-prise-30-poles-f-4483).

![3d box](/Mueller_Spray_control-integration/Images/3d.png)

## Mueller Pinout
### Spray Control S 48pins connector

|Pin|To |Info|
|---|---|----|
|Z3|Z3|12v|
|Z4|Z4|GND|
|Z6|Z6|GND|
|Z12|Z12|GND|
|B1|B1||
|B4|B4||
|B5|B5||
|B12|B12||
|B13|B13||
|D1|PC817-1|Section1|
|D2|PC817-2|Section2|
|D3|PC817-3|Section3|
|D4|PC817-4|Section4|
|D5|PC817-5|Section5|
|D6|PC817-6|Section6|
|D7|PC817-7|Section7|
|D13|D13||
|D14|D14||
|D15|D15||
|D16|D16||

### 30pin Sprayer connector
30pin DIN41622 - Input (from box)
|Pin|Info|To|
|--|--|--|		
|A1|Rate|A1 Out|
|A2|Rate|A2 Out|
|A3|Main On/Off|PC817-8 + A3 Out|
|A4|Main On/Off|A4 Out|
|A5|Sec1|PC817-1|
|A6|Sec1||
|A7|Sec2|PC817-2|
|A8|Sec2||
|A9|Sec3|PC817-3|
|A10|Sec3||
|B1|Sec4|PC817-4|
|B2|Sec4||
|B3|Sec5|PC817-5|
|B4|Sec5||
|B5|Sec6|PC817-6|
|B6|Sec6||
|B7|Sec7|PC817-7|
|B8|Sec7||
|B9|Sec8|B9 Out|
|B10|Sec8|B10 Out|
|C1|Sec9|C1 Out|
|C2|Sec9|C2 Out|
|C3|12v|C3 + 12V|
|C4|unknow|C4 Out|
|C5|Gnd|C5 + GND|
|C6|unknow|C6 Out|
|C7|unknow|C7 Out|
|C8|unknow|C8 Out|
|C9|unknow|C9 Out|
|C10|unknow|C10 Out|

30pin DIN41622 - Output (To Sprayer)
|Pin|To|
|--|--|	
|A1|A1 In|
|A2|A2 In|
|A3|A3 In|
|A4|A4 In|
|A5|Relay|
|A6|Relay|
|A7|Relay|
|A8|Relay|
|A9|Relay|
|A10|Relay|
|B1|Relay|
|B2|Relay|
|B3|Relay|
|B4|Relay|
|B5|Relay|
|B6|Relay|
|B7|Relay|
|B8|Relay|
|B9|B9 In|
|B10|B10 In|
|C1|C1 In|
|C2|C2 In|
|C3|C3 + 12V|
|C4|C4 In|
|C5|C5 + GND|
|C6|C6 In|
|C7|C7 In|
|C8|C8 In|
|C9|C9 In|
|C10|C10 In|