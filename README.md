# RobotControl

Arduino ESP8266 project for UBTech Alpha 1S Controller.

Initial version 1.0 comes from the sample in my UBTech library.

Versionn 2.0 will define a new set of communization protocol.
In this version, it will support both V1 & V2 commands.


## Major changes in V2

- New command set for V2 with start / end cod and checksum for better communization control
- Use major loop for action playback, so there has no block in playing action, can be stopped anytime
- Default reset action with all servo locked, and LED on
- Use single file to store individual action
- Define name for action (to be used in PC UI only), and a maximum of 255 actions allowed
- Support up to 255 steps for each action
- Support servo LED control
- Support Robot Head Light control
- Support remote control by
  - USB direct connect
  - Bluetooth 
  - WiFi with router 
  - WiFi with softAP

## Dependency

- [Wifi Manager]
- [UBTech] Library 
- Latest [espsoftwareserial] 
  - After the merge on [Helf duplex serial communization]

Save all above libraries in lib folder.

## Installation

This is a Visual Studio Code project with PlatformIO IDE.

- Install [Visual Studio Code]
- Install the PlatformIO IDE extension
- Open the folder in Visual Studio Code

## Hardware

- ESP8266 board
- HC-05 for Bluetooth connection

## Pin Assignment for ESP8266 Board

| Pin | Usage |
| ------ | ------ |
| GPIO-12 | Connect to signal pin of Servo |
| Rx | Connect to Tx of HC-05 |
| Tx | Connect to Rx of HC-05 |
| GOPI-2 | Connect to Rx of serial console for debug output 


[Wifi Manager]: <https://github.com/tzapu/WiFiManager.git>
[UBTech]: <https://github.com/Super169/UBTech.git>
[espsoftwareserial]: <https://github.com/plerup/espsoftwareserial.git>
[Visual Studio Code]: <https://code.visualstudio.com/>
[Helf duplex serial communization]: <https://github.com/plerup/espsoftwareserial/commit/12664f2355be24b49138a83a76de96803e3040d7>