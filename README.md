# CryptoGarage ESP8266 firmware #
**Smartphone garage door opener with focus on security**

TODO: Description

## Usage ##

TODO: Create Wiki
If you just wanna know how to control this thing, head over to the [Wiki](link)

## Library ##

TODO: list used libs

## Build ##

TODO: ESP8266 build instructions

* Install Arduino IDE
* In File > Preferences, add this Board Manager URL
```http://arduino.esp8266.com/stable/package_esp8266com_index.json```
* Go to Tools > Board > Boards Manager and install ESP8266 Board.
* ```https://github.com/wladimir-computin/ESP8266-CryptoGarage.git```
* Open ESP8266-CryptoGarage.ino in Arduino IDE
* In Tools, set the following settings:
```
Board:           Generic ESP8266 Module
Flash Mode:      DIO
Flash Frequency: 40 MHz
CPU Frequency:   80 MHz
Flash Size (ESP-01S): 1M (64K SPIFFS)
Debug Port:      Disabled
Debug Level:     None
Reset Method:    ck
```
* Compile via STRG + R
* Go to Arduino build directory. (in Windows %user%\AppData\Local\Temp\arduino_build_*)
* Flash .bin via Webrowser / OTA

You'd better be sure not to break the OTA update functionality, otherwise you will be stuck with USB-flashing, which is shitty if you don't have a ESP8266 USB Serial Adapter.