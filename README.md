# RobotControl

Arduino ESP8266 project for UBTech Alpha 1S Controller.

Initial version 1.0 comes from the sample in my UBTech library.

Versionn 2.0 will define a new set of communization protocol.
In this version, it will support both V1 & V2 commands.


## Major changes in V2, V2.2, V2.3

- New V2.0 communication protocol to avoid misinterpretation
  (some V1 commands are kept for bluetooth control)
- Support control from USB, Bluetooth and Wifi (connect to router or AP mode)
- Action setup by reading position from robot
- Convert action from UBT action file in aesx and hts format
- Play MP3 with action
- Use MPU 6050 to detect current situation
- Define the action to stand up after fall down for both face up & face down cases
- Touch sensor and response for single, double and triple click, and long touch.
- Define events via Blockly UI in MyAlphaRobot v2.2+
- Add Sub-System Bus for other device, e.g. Ultrasonic, PSX Control
- Add USB-TTL mode to work as testing board for serial servos.
- Option to set action speed
- ......


## Dependency

- [OLED12864] library

Already included in lib_deps.

## Installation

This is a Visual Studio Code project with PlatformIO IDE.

- Install [Visual Studio Code]
- Install the PlatformIO IDE extension
- Open the folder in Visual Studio Code

## Hardware

- ESP8266 board
- HC-05 Bluetooth module (optional)
- MP3-TF-16P MP3 player (optional)
- MPU6050 module (optional) 
- TTP223 touch sensor (optional)
- SSD1306 OLED module (optional)
- Sub-system boards (optional)

## Pin Assignment for ESP8266 Board

| Pin | Usage |
| ------ | ------ |
| GPIO-2 | Connect to Rx of serial console for debug output |
| GPIO-4 | Connect to SDA of I2C bus |
| GPIO-5 | Connect to SCL of I2C bus |
| GPIO-12 | Connect to signal pin of Servo |
| GPIO-13 | Connect to Touch sensor |
| GPIO-14 | Connect to Sub-System Bus (for PSX Control, Ultrasonic, ...) |
| GPIO-15 | Connect to head led |
| GPIO-16 | Connect to Rx of MP3 module |
| Rx | Connect to Tx of HC-05 |
| Tx | Connect to Rx of HC-05 |
| I2C | for MPU6050 and OLED display |

The PC setup program MyAlphaRobot can be found in https://github.com/Super169/MyAlphaRobotV2/releases/tag/MyAlphaRobot_2.3
![MyAlphaRobot](https://raw.githubusercontent.com/Super169/images/master/MyAlphaRobot/2.3/main-02-serial.png)

The new version support event setting by Blockly UI.

![MyAlphaRobot](https://raw.githubusercontent.com/Super169/images/master/MyAlphaRobot/2.3/event_handler.png)

And also online update firmware up to 921600bps, it takes less than 10 seconds to flash the firmware.

![MyAlphaRobot](https://raw.githubusercontent.com/Super169/images/master/MyAlphaRobot/2.3/burn-01.png)

You need a ESP8266 control board in order to use this firmware.

You can make a simple PCB like this,
![PCBLayout](https://raw.githubusercontent.com/Super169/images/master/RobotControlV2.0/PCB_v2_7.png)

I just make one as below, I have removed the HC-05 as it can already controlled by smartphone via WiFi.
![MyPCB](https://raw.githubusercontent.com/Super169/images/master/RobotControlV2.0/MyPCB_3.png)


and this is my testing environment for program development, including PSX Control and Ultrasonic Sensor.

![Robot](https://raw.githubusercontent.com/Super169/images/master/RobotControlV2.0/MyPCB_4.png)



Or if you cannot make the PCB yourself, you can buy a ready to use control board from Taobo.
https://item.taobao.com/item.htm?id=571368655206

![PCBLayout](https://raw.githubusercontent.com/Super169/images/master/RobotControlV2.0/TB-03.png)





ENJOY!

[UBTech]: <https://github.com/Super169/UBTech/releases/tag/RobotControl_2.0>
[OLED12864]: <https://github.com/Super169/OLED12864/releases/tag/RobotControl_2.0>
[espsoftwareserial]: <https://github.com/plerup/espsoftwareserial.git>
[Visual Studio Code]: <https://code.visualstudio.com/>
