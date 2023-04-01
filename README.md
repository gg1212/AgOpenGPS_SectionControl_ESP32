# AgOpenGPS Section Control ESP32 WiFi
This code runs AOG section control on ESP32. 
The ESP32 is connected to my mobile hotspot using WiFi, my laptop running AOG is also connected to the same WiFi hotspot. The AgIO UDP scanner will find the module once you configure the network address in AgIO. Note that you cannot fix the subnet of an Android hotspot so you may have to reconfigure subnet in AgIO if it switched on the phone.

You need to edit the code and add your WiFi SSID and password before uploading the code to ESP32.

That's a wiring diagram for 7sections, main(work) switch and manual/auto switch:
![wiring diagram](/Images/esp32_wiring_diagram.png)


This code is based on [mtz8302/AOG_SectionControl_ESP32](https://github.com/mtz8302/AOG_SectionControl_ESP32) for which I removed all the rate control and web interface related stuff. I have also updated the communication code to make it ready for v5.7.2