Nerf Blaster, High Performance, micro bushless motors
# Wall-E ESP32-C3 with BLE Game Controller (Xbox controler)
Thie is an upgrade to the Wall-E project developed by Chillbasket

![Walle-E Chillibasket](https://github.com/KiwiPlastic/Wall-E_ESP32-C3_BLE_Gamepad_Ctrl/blob/main/Images/Chillibasket1.jpg?raw=true)

## 🚀 Upgrades and Features
  -	Web Server removed
  -	Added BLE Gamepad controller – Xbox controler 2026
  -	Added working Top buttons
  -	Added Bay Door Servo
  -	Controllable Bay LED light
  -	Electronics Platform – frees up Bay so can have a boot and plant etc
  -	RGB mood LED
  -	1.44 Oled feedback display
  -	All existing avaliable upgrade prints have been used and are included.
  - The only changed one is the top cover to accomidate pushbuttons
  
•	10 DOF:-
  -	Arms (2)
  -	Neck (2)
  -	Head Rotation (1)
  -	Eyes (2) 
  -	Eye Borrows (2)
  -	Bay Door (1)

•	Size:	H = 338		W = 291	D = 186

## 📦 Build & Installation

•	See this link for Wall-E Assembly Instructions and Videos by Chillbasket (https://wired.chillibasket.com/3d-printed-wall-e/) 

### Hardware: -
    1 x ESP32-C3
    1 x I2C PCA9685 16-Channel 12-bit PWM Servo Motor Driver Module
    1 x I2C PCF 8575 I/O Multiplexer
    1 x Audio Module - Mini MP3 Player Audio Module (DFPLayer)
    1 x mini-Speaker 8 or 4 ohm 3watt
    1 x 1.44 I2C OLED Display
    1 x 10mm RGB LED – common cathode 
    4 x Tactile Pushbutton with LED
    4 x 150R resistor
    1 x 4K7 Resistor
    3 x Logic Level Shifter 3v to 5v 
    1 x L298N Dual H Bridge DC Motor Driver
    10 x MG90S metal gear servos
    2 x 37mm motor with gearbox, offset shaft, 25kg, 150 rpm (Not cheap ones)
    1 x LR7843 Mosfet module
    1 x MP1482 min buck set to 2v
    1 x 1watt or 3watt LED (torch head light LED)
    2 x Buck 5amp Set to 5v and 6v
    1 x 10amp auto reset fuse
    1 x XT60 Male Connector
    16 x Bearings 10 x 15 x 4
    Misc. M3 bolts, nuts, heat inserts, heat shrink

![circuit](https://github.com/KiwiPlastic/Wall-E_ESP32-C3_BLE_Gamepad_Ctrl/blob/main/Images/Wall-E%20CCT.png?raw=true)

### Servo Channel List: -
    0 Head Rotation
    1 Neck Top
    2 Neckk Bottom
    3 Eye Right
    4 Eye Left
    5 Arm Left
    6 Arm Right
    7 Eye Brow Right
    8 Eye Brow Left
    9 Bay Door
    10
    11
    12
    13 RED RGB LED
    14 GRN RGB LED
    15 BLU RGB LED

<img width="336" height="236" alt="image" src="https://github.com/user-attachments/assets/b37ebee5-bc94-449d-93f2-da154a7ca9b7" />

Use common Cathod 10mm RGB LED

### Notes: -
    - All moving joints need to be sanded and made to work as smoothly as possible
    - Silicon from (Starlink Installations) is useful on these joints
    - Power up PSU and adjust Bucks to 5v, 6v and 2v BEFORE connecting to main loom
    - Wire up loom and electronics, then test before securing to 3d prints
    - glue electronics floor brakets so floor is level with top of Bay door opening
    - Secure electronics to 3d prints
    - Power up with no Motor or Servo loads, test buttons, leds, oled, MP3 player, xbox controller is working
    - Connect servos and put calibration firmware in ESP32 and perform calibration (https://wired.chillibasket.com/3d-printed-wall-e/)
    - Edit main wall-E firmware to add the calibration values from IDE serial monitor, transfer to ESP32 
    - Power up Wall-E, with game controler off
    - Turn on Game controller, should enrol automatically, watch Oled
    - Test and go for gold
    - Note: Theres no Battery monitor in this code. Use a Li-Po battery monitor/buzzer on Li-Po monitor plug 

<img width="591" height="444" alt="image" src="https://github.com/user-attachments/assets/9866b3a2-0881-4ab2-98a3-da6f81b261cc" />
<img width="596" height="446" alt="image" src="https://github.com/user-attachments/assets/b4e575be-7335-424b-b01d-1f6c7c4178b7" />
<img width="537" height="716" alt="image" src="https://github.com/user-attachments/assets/2d542942-494d-4c33-a97c-e5f0b9f9c0b5" />
<img width="715" height="536" alt="image" src="https://github.com/user-attachments/assets/bef63678-cfd8-4655-83e4-620f541ff4cb" />

### Arduino IDE will require these library's
    - BLE-Game_Client by Tomaz bekas V0.12.1
    - ESP_Software_Serial by Dirk Kaar SNDVAPS 8.3.1
    - Adafruit SSD1306 by Adfruit v2.5.17
    - Adafruit PWM Servo Driver lib by Adfruit v 3.03 
    - Adafruit PCF8574 by Adfruit v 1.1.2
    - DFRobotDFPlayerMini by DFRobot v1.0.6
    - Wire.h - I2C library driver, Could be built into IDE Library’s

Note: at time of writting the BLE-Game_Client required a small edit in a txt file to get it working. (https://github.com/tbekas/BLE-Gamepad-Client/pull/122)
It will be fixed in newer version releases, so this edit will not always be required.

## 🛠️ Usage

### Top Push Buttons Functions: -
    RED PB - Play Prev 
    GRN PB - Play Next
    ORG PB – Autonomous Servos On/Off 
    WHT PB – Bay Door Open/Close
    
### BLE Gamepad - Xbox controler - Button Map :-
    Tracks turn left/right  	Left Stick X (hoz)      
    Tracks forward/back       Left Stick Y (Vert) 
    Head rotation           	Right Stick X
    Head Up/Down            	Right Stick Y
    Neck Bottom             	Button A + Dpad Up/Dwn
    Lhs Eye                 	Button X + Dpad Up/Dwn
    Lhs Eyebrow             	Button X + Dpad Left/Right
    Rhs Eye                 	Button B + Dpad Up/Dwn
    Rhs Eyebrow             	Button B + Dpad Left/Right
    Bay Door Open/Close      	Button Y + Dpad Left Right
    Left Arm               	  Left Trigger  + Dpad Up/Dwn
    Right Arm              	  Right Trigger + Dpad Up/Dwn
    Play Sound              	Left Bumper + Dpad Left/Right - can crash
    Motor dead zone         	Left Bumper + Dpad Up/Dwn - 0 to 250
    LEDS move               	Right Bumper + Dpad Left/Right - loop x 4
    Steering Offset         	Right Bumper + Dpad Up/Dwn -  -100 to 100
    Animation Number          ViewButton +  Dpad Up/Dwn
    Automonus Mode    	      MenuButton On/Off toggel
    Dpad Up                 	adjust up
    Dpad Down               	adjust down
    Dpad Left               	adjust left
    Dpad Right              	adjust right
    Left Stick Button
    Right Stick Button
    ShareButton
    XboxButton

## 📜 License
Distributed under the MIT License. See `LICENSE` for more information.
