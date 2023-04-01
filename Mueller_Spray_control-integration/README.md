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

![overview](/Mueller_Spray_control-integration/Images/esp32.png)

## Relay board & sending section status to rate computer
### Relay board
Valves are reverse polarity here is the challenge, you need 2 relays per section valve. 12V goes to NC of relay 1 and NO of relay 2. GND goes to NO of relay 1 and NC of relay 2. The section valve is connected to relay 1 & 2 COM.

So for the 7 sections, I used 14 relays of a 16 relays board which I connected as in the example below.

### Rate controller section state
It is really important that the Spray Control computer knows how many sections are spraying in order to adjust rate correctly. So, the info should be coming from the output of the relays instead of the output of the switches as before. 

I have used another PC817 optocoupler board to get the 12v status from relay board and switch rate computer pin to GND. Note that it requires to remove the 3K resistor so I soldered a wire to bypass it.

![overview](/Mueller_Spray_control-integration/Images/relays.png)

## Hardware
I have designed a 3D printed box that fits between Mueller Spray Control box and the switches box, that way I can connect to the section wires and give the signal from real status(relay board+optocoupler). In order to centralize the switches, the man/auto switch has been put on this 3D printed box which fits perfectly like a 3rd Mueller box. I have also added 7 leds that shows the section state based on the relays as the original led on shows the switches state.

The box model is available in [3D](/Mueller_Spray_control-integration/3D) folder.

The connectors between Mueller boxes are Harting 48pins DIN 41612 connectors.

The connectors between Mueller box and sprayer are 30pins DIN 41622 connectors that I ordered at Beyne factory webshop : [30p male](https://erp.beyne.be/fr_BE/shop/product/cnc-30-m-prise-30-poles-m-4485) and [30p female](https://erp.beyne.be/fr_BE/shop/product/cnc-30-v-prise-30-poles-f-4483).