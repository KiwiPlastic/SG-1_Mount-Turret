#define VERSION 17
/* 17-4-26

 Project: SG-1 Turrret     ****** ESP32-S3 Slave *******

 written By RICHRD NICHOLSON from NEW ZEALAND
 
Summary
  SG-1 Turret/Mount, remote control robot
  I2C coms between 2 x ESP32s (Master / Slave) no BLE
  This code is for Tracks and Turret control (Slave)
  Configuration and control is via 'SG-1 Mount' App for Android Phone
  use USB CLI to exercise

Features (this HW)
 - Mobile Plate Form, Tracks
 - Elevation & Rotation of Turret
 - R2D2 feed back
    - Startup (Short Beep, Beep)
    - Sonar Alarm
    - Horn in App
 - I2C Link From Turret (slave) to SG1-RMB Frankenboard (Master)
 - USB Serial CLI
 
 Hardware (this code)
 - Seeed XIAO ESP32S3
 - Pizo with Mofet PCB driver (R2D2)
 - 2 x Turret Servos RDS3225 25kg double shaft RC PWM 270 deg 500-2500msec
 - 2 x Tracks Servos MG995 8.5kg RC PWM Servo
 - 2 x 3A Buck voltage regulator
 - 2 x Pedodometer - Rotary Encoder
 
Tracks: Forward and Back
   - 2 x MG995 Servos 8.5kg. Modded to continues rotation (0-180)
   - Stop on RHS = 90 Deg RHS Track (This is specific to the each MG995)
   - Stop on LHS = 90 Deg LHS Track
  Forward
   - RHS Full forward = 180
   - LHS Full Forward = 0
  Backward
   - RHS Full Backward = 0
   - LHS Full Backward = 180

  Turret: Elevation and Rotation
  2 x RDS3225 25kg double shaft RC PWM Servo 270 deg 500-2500msec (Turret)
   - Rotation center line position = Center=78, CW=15, CCW=140
   - Elevation horizonal position = Horizontal=62 Min Down= 50 Max 75-80 
 
   ** SC15 serial servos, where removed but keep this if we need later
   sc.WritePos(SC15_ELV_ID, yElevationValue, 0, SC15_SPD_NOM);  //Servo(ID1) moves at max speed=1500, moves to position = Elevation
   sc.WritePos(SC15_ROT_ID, xRotationValue, ROT_CCW_LIMIT, ROT_CW_LIMIT);  //Servo(ID2) moves at max speed=1500, moves to position = Rotation
   sc.WritePos(SC15_ELV_ID, yElevationValue, 0, SC15_SPD_NOM);     // Servo(ID1), position = yElv moves at speed=600 (Max 1500)
   sc.WritePos(SC15_ELV_ID, ELV_UP_LIMIT, 0, SC15_SPD_NOM);        // Servo(ID1), position = 600 moves at speed=600 (Max 1500)
   sc.WritePos(SC15_ROT_ID, xRotationValue, 0, SC15_SPD_NOM);      // Servo(ID2, position = 512 moves at speed=1000 (Max 1500)

 USB - Serial Cmd Line Interface - 115200 baud
 ==============================================
  ? = Display Status
  #RT000  - 180$  Set RHS Tracks Deg, send. Get back Feedback reading.
  #LT000  - 180$  Set LHS Tracks Deg
  #RO015  - 140$  Turret Rotation Value
  #EL050  - 075$  Turret Elevation Value (75 = Max up, 50 Max down)
  #HM$      #HM$  Home, Moves Turret to Center line positions
  #ES$      Emergancy Shutdown: Battery Low Alarm, Ultrasonics Distance alarm
  #P0$      Pizo Off
  #P1$      R2D2 1
  #P2$      R2D2 2 to #P5$
  #D1$      Demo Dance
  #F$       Forward
  #B$       Backward
  #L$       Turn Left
  #R$       Turn Right"
  #EU$      Elevation Up
  #ED$      Elevavtion Down
  #RR$      Rotate Right
  #RL$      Rotate Left

  Odometer Left: CW : 0    Odometer Right: CCW : -1
  Sonar Front: 348mm    Sonar Rear: 487mm    // This comes from RMB Master#F$
  
//--------------------------------------------------------------------------------
I2C - Commands - From RMB (Master) to Turret (Slave) and Back (Master Initiates)
    TF                Track Forward
    TB                Tracks Backward
    TR                Tracks Right  
    TL                Tracks Left 
    TS                Tracks Stop
    RL                Turret Rotation Right
    RR                Turret Rotation Left
    EU                Turret Elevation Up
    ED                Turret Elevation Down
    HM                Turret Home Position
    P1                Pizo ON
    P0                Pizo OFF
    DE                Demo
    SFnnnn            Sonar Front mm value from RMB Master
    SRnnnn            Sonar Rear mm value from RMB Master
    APnnn             AI Pan degrees (Huskeylens)            
    ATnnn             AI Tilt degress (Huskeylens)

//-------------------------------------------------------------------------
Demo Mode Sequence:-
  R2D2
  Tracks Stop
  Turret Home
  Turret Up Max
  Turret Down Min
  Turret Horizontal - Home
  Turret Rotate CCW Max
  Turret Rotate CW Max
  Turret Rotate Center - Home
  Tracks Forward  150mm
  Stop
  Tracks Backward 150mms
  Stop
  Tracks Turn Right CW
  Stop
  Tracks Turn Left CCW 
  Stop
  R2D2

*/

//=======================================================================================
//Load libraries

#include <ESP32Servo.h>  // MG995 Servo driver ESP32 ESP32S2 AnalogWirte by David Lloyd
Servo servo_tracks_RHS;  // create X servo object to control a servo
Servo servo_tracks_LHS;  // create Y servo object to control a servo
Servo servo_rotation;
Servo servo_elevation;

#include <Wire.h>     // I2C Driver
#define I2C_ADD 0x08  // I2C SLAVE Address for this module

//------------------------------------------------------------------------------------------
// Switches
#define DEMO_ON 0           // <<==== **** set to 1 for startup dance ***********
#define CLI_ON 1            // USB Command Line interface on/off WIP
#define TRACKS_SAFETY_ON 1  // Only lets traks go one revolution of rotary encoder

//------------------------------------------------------------------------------------------
// Tracks Calibration                   Range: 0-180 degress.
#define RHS_SERVO_STOP 90  // Right Track Stop poisiton
#define LHS_SERVO_STOP 90  // Left Track Stop position

int RHS_TracksValue = RHS_SERVO_STOP;  // Forward = 50 backw#F$ard = 140
int LHS_TracksValue = LHS_SERVO_STOP;  // Foreard = 140 Backward = 50

#define RHS_SERVO_FWD 180    // Max = 180 Range: 0-180 degress. RHS Stop = 98 LHS Stop = 90
#define LHS_SERVO_FWD 0      // Max = 0
#define RHS_SERVO_REV 0      // Max = 0
#define LHS_SERVO_REV 180    // Max = 180
#define RHS_SERVO_LEFT 25     // Turn Left
#define LHS_SERVO_LEFT 25     // Turn Left
#define RHS_SERVO_RIGHT 155  // Turn Right
#define LHS_SERVO_RIGHT 155  // Turn Right

// Turret Rotation Calibration
#define ROT_CENTER 75      //
#define ROT_CW_LIMIT 15    // Right
#define ROT_CCW_LIMIT 140  // Left

// Turret Elevation Calibrati0n
#define ELV_CENTER 65     //
#define ELV_DWN_LIMIT 50  //
#define ELV_UP_LIMIT 80   //

int xRotationValue = ELV_CENTER;
int yElevationValue = ROT_CENTER;

//--------------------------------------------------------------------------------------------------
// Sonar
#define SONAR_MIN_MM 100                     // 350 is good for IRL. Ultrasonics min distance =  alarm
#define MIN_ERROR 50                         // sonar false reading level
unsigned long lastUpdateTimeUtrasonics = 0;  //
long SF_mm = 2345;                           // Sonar Front value in mm
long SR_mm = 9876;                           // Sonar Rear value in mm
bool Sonar_F_Alm = false;                    //
bool Sonar_R_Alm = false;                    //

//============================ BIOS ================================================================

#define PIN_BOOT 0            // GPIO0   Boot button input can be tested on start up
#define PIN_ODOM_LHS1 D0      // GPIO1   Quad Odometer LHS Q1
#define PIN_ODOM_LHS2 D1      // GPIO2   Quad Odometer LHS Q2
#define PIN_ODOM_RHS2 D2      // GPIO3   Quad Odometer RHS Q1
#define PIN_ODOM_RHS1 D3      // GPIO4   Quad Odometer RHS Q2
#define PIN_SDA D4            // D4      I2C SDA Slave (SG1 Turret) to Master(RMB)
#define PIN_SCL D5            // D5      I2C SCL Slave (SG1 Turret) to Master (RMB)
#define PIN_PIZO D6           // D6      R2D Sounds
#define PIN_ELEVATION_PWM D7  // GPIO8   Servo Elevation (RDS3225 25kg double shaft RC servo 270 deg 500-2500msec)
#define PIN_TRACK_PWM_RHS D8  // GPIO9   Motor LHS PWM (MG995 8.5kg 360 deg Servo)
#define PIN_TRACK_PWM_LHS D9  // GPI10   Motor RHS PWM (MG995 8.5kg 360 deg Servo)
#define PIN_ROTATION_PWM D10  // GPI11   Servo Rotation (RDS3225 25kg double shaft RC servo 270 deg 500-2500msec)

//===============================================================================================
// System stuff
#define UPDATE_INTERVAL 100  // 100ms
#define ONESEC 1000          // One Second Interval

unsigned long lastUpdateTime = 0;
unsigned long lastOneSecTime = 0;
unsigned long LastBlinkLED = millis();  // start time in mSec
unsigned long LastServo = millis();
unsigned long LastTurret = millis();
unsigned long BootStart = 0;  // Just for any initial timing...
unsigned long PizoOnTime = 0;
unsigned long PizoOffTime = 0;
int stat_LED = 0;  // status of LED: 1 = ON, 0 = OFF

int incrementDeg = 5;  // Turret servo incriment amount in Deg

int DemoRunFlag = 0;
unsigned long LastDemoWaitTime;
byte DemoMulti = 1;

unsigned long lastUpdateTimeR2D2;
bool R2D2_Flag = false;
int ToneValue;

// USB Serial Comms
#define SERIAL_INPUT_BUFFER_MAX 25
char SerialInputBuffer[SERIAL_INPUT_BUFFER_MAX];

// System Mode - NOT USED
#define SYSTEM_MODE_NORMAL 0
#define SYSTEM_MODE_LOW_BATT 1
#define SYSTEM_MODE_ANALOG_READ 2  //not uesed
#define SYSTEM_MODE_SERIAL_CMD 3
#define SYSTEM_MODE_TURRET_ROT 4
#define SYSTEM_MODE_TURRET_ELE 5
#define SYSTEM_MODE_TRACKS_LHS 6
#define SYSTEM_MODE_TRACKS_RHS 7
#define SYSTEM_MODE_ULTRASONICS 8
#define SYSTEM_MODE_RESET1 9
#define SYSTEM_MODE_RESET2 10
#define SYSTEM_MODE_RESET3 11
#define SYSTEM_MODE_RESET4 12
byte SystemMode = SYSTEM_MODE_NORMAL;

// I2C Config
char I2C_Rx_Buffer[20];
int toTransfer = 65535;  // Range 0 - 65535 no negative???
int Shift = toTransfer;
int mask = 0xFF;
char toSend = 0;
int byteSending = 1;
int output = 0;

volatile bool I2C_TF_Flag = false;
volatile bool I2C_TB_Flag = false;
volatile bool I2C_TL_Flag = false;
volatile bool I2C_TR_Flag = false;
volatile bool I2C_TS_Flag = false;
volatile bool I2C_RR_Flag = false;
volatile bool I2C_RL_Flag = false;
volatile bool I2C_EU_Flag = false;
volatile bool I2C_ED_Flag = false;
volatile bool I2C_HM_Flag = false;
volatile bool I2C_P1_Flag = false;
volatile bool I2C_P0_Flag = false;
volatile bool I2C_DE_Flag = false;
volatile bool I2C_SF_Flag = false;
volatile bool I2C_SR_Flag = false;
volatile bool I2C_AP_Flag = false;
volatile bool I2C_AP_Update_Flag = false;
volatile bool I2C_AT_Flag = false;
volatile bool I2C_AT_Update_Flag = false;

//--------------------------------------
// Left Tracks Odometer Encoder Inputs
int leftcounter = 0;
int leftcurrentStateCLK;
int leftpreviousStateCLK;
String leftencdir = "";
float LeftEncoderMulti = 2.5352;  // Convert Pulses to mm traveled. PPR=71, Distance = 180mm. 180/71 = 2.5352
int LeftTraveled;

//-------------------------------------
// Right Tracks Odometer Encoder Inputs
int rightcounter = 0;
int rightcurrentStateCLK;
int rightpreviousStateCLK;
String rightencdir = "";
float RightEncoderMulti = 2.5352;  // Convert Pulses to mm traveled. PPR=71, Distance = 180mm. 180/71 = 2.5352
int RightTraveled;

//================================ SETUP ==============================================
void setup() {
  BootStart = millis();  // Just for any initial timing..

  Serial.begin(115200);  // Serial Monitor output
  delay(2000);           // Wait....2 sec

  Serial.println(F("SG-1 Turret Starting..."));  // Display version on Serial Monitor

  //----------------------------------------------
  // PWM RC Servo config
  // using default min/max of 1000us and 2000us
  // different servos may require different min/max settings for an accurate 0 to 180 sweep

  ESP32PWM::allocateTimer(0);  //Allow allocation of all pwm timers
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  servo_tracks_RHS.setPeriodHertz(50);                     // standard 50 hz servo
  servo_tracks_RHS.attach(PIN_TRACK_PWM_RHS, 1000, 2000);  // attaches the servo on pin 18 to the servo object

  servo_tracks_LHS.setPeriodHertz(50);                     // standard 50 hz servo
  servo_tracks_LHS.attach(PIN_TRACK_PWM_LHS, 1000, 2000);  // attaches the servo on pin 18 to the servo object

  servo_rotation.setPeriodHertz(50);  // standard 50 hz servo
  servo_rotation.attach(PIN_ROTATION_PWM, 500, 2500);

  servo_elevation.setPeriodHertz(50);  // standard 50 hz servo
  servo_elevation.attach(PIN_ELEVATION_PWM, 500, 2500);

  // I2C Startup
  Wire.begin(I2C_ADD);       // join I2C bus as Slave with address 0x08
  Wire.onReceive(dataRcv);   // register an event handler for received I2C data
  Wire.onRequest(dataRqst);  // register an event handler for I2C data requests
  delay(1000);

  pinMode(LED_BUILTIN, OUTPUT);  // Im alive indicator

  pinMode(PIN_PIZO, OUTPUT);    // Pizo buzzer. turn off ASAP. is on during down load
  digitalWrite(PIN_PIZO, LOW);  // Set Pizo OFF

  // Odometer Qudrature rotary encoder           // Done in setupMotors, Kaia_Motors.ino
  pinMode(PIN_ODOM_LHS1, INPUT_PULLUP);
  pinMode(PIN_ODOM_LHS2, INPUT_PULLUP);
  pinMode(PIN_ODOM_RHS1, INPUT_PULLUP);
  pinMode(PIN_ODOM_RHS2, INPUT_PULLUP);
  leftpreviousStateCLK = digitalRead(PIN_ODOM_LHS1);   // PIN_ODOM_LHS1
  rightpreviousStateCLK = digitalRead(PIN_ODOM_RHS1);  // PIN_ODOM_RHS1
  delay(500);

  // R2D2
  randomSeed(analogRead(0));

  ProcessStartBeeps();  // Beep...Beep, Ready
  delay(500);

  //Blink LED at ONESEC
  LastBlinkLED = millis();

  // Set Tracks to stop
  ProcessStop();
  delay(500);

  // Set Barrel home
  ProcessHome();
  delay(500);

  // Startup Dance Movements    // for testing a dance
  //if (DEMO_ON == 1) {
  //  ProcessDemo();
  //}

  ProcessDebug();  // Serial output, display some values
}

//============================ MAIN LOOP ==================================================
void loop() {

  ProcessUtrasonics();     // get Sonar reading, from I2C Callback incoming data from RMB Master
  ProcessLeftEncoder();    // read Left Tracks Odemeter (needs to move to interupt)
  ProcessRightEncoder();   // read Right Tracks Odemeter (needs to move to interupt)
  ProcessRMB_I2C();        // RMB I2C callbacks sets flags , process incomeing data
  ProcessSerialInput();    // Chk for USB serial CLI characters
  ProcessSerialCommand();  // Process incomeing USB serial CLI command
  ProcessBlinkLED();       // Im alive indicator
  ProcessDemo();           // Runs Demo Sequance if DemoFlag > 0
}

//=========================================================================================
// I2C EVENT HANDELER: Received Data
// The Master sends a Cmd and value. Process here for max responce
// after reciving a cmd the Master requests data. we perload that buffer here
// I2C fails if we do Servo movements while in here. Need to flag new value to be sent etc
//---------------------------------------------------------------------------------------
void dataRcv(int numBytes) {
  while (Wire.available()) {  // read all bytes received
    char c = Wire.read();     // Receive a byte as character
    I2C_Rx_Buffer[0] = (c);   // Save to RX buffer [0]
    c = Wire.read();          // Receive a byte as character
    I2C_Rx_Buffer[1] = (c);   // Save to RX buffer [1]
    c = Wire.read();          // Receive a byte as character
    I2C_Rx_Buffer[2] = (c);   // Save to RX buffer [2]

    if (strcmp(I2C_Rx_Buffer, "#TF") == 0) {  // Tracks Forward
      I2C_TF_Flag = true;                     // set flag
      //Serial.println("I2C TF Flag");
    }
    if (strcmp(I2C_Rx_Buffer, "#TB") == 0) {  // Tracks Backward
      I2C_TB_Flag = true;
      //Serial.println("I2C TB Flag");  // set flag
    }
    if (strcmp(I2C_Rx_Buffer, "#TL") == 0) {  // Tracks Left
      I2C_TL_Flag = true;
      //Serial.println("I2C TL Flag ");  // set flag
    }
    if (strcmp(I2C_Rx_Buffer, "#TR") == 0) {  // Tracks Right
      I2C_TR_Flag = true;                     // set flag
      //Serial.println("I2C TR Flag");
    }
    if (strcmp(I2C_Rx_Buffer, "#TS") == 0) {  // Tracks Stop
      I2C_TS_Flag = true;
      //Serial.println("I2C TS Flag");
    }
    if (strcmp(I2C_Rx_Buffer, "#RR") == 0) {  // Rotation Right
      I2C_RR_Flag = true;                     // set flag
      //Serial.println("I2C RR Flag");
    }
    if (strcmp(I2C_Rx_Buffer, "#RL") == 0) {  // Rotation Left
      I2C_RL_Flag = true;                     // set flag
      //Serial.println("I2C RL Flag");
    }
    if (strcmp(I2C_Rx_Buffer, "#EU") == 0) {  // Elevation Up
      I2C_EU_Flag = true;                     // set flag
      //Serial.println("I2C EU Flag");
    }
    if (strcmp(I2C_Rx_Buffer, "#ED") == 0) {  // Elevation Down
      I2C_ED_Flag = true;                     // set flag
      //Serial.println("I2C ED Flag");
    }
    if (strcmp(I2C_Rx_Buffer, "#AP") == 0) {  // AI Pan (Rotation)
      I2C_AP_Flag = true;                     // set flag
      //Serial.println("I2C AP Flag");
    }
    if (strcmp(I2C_Rx_Buffer, "#AT") == 0) {  // AI Tilt (Elevation)
      I2C_AT_Flag = true;                     // set flag
      //Serial.println("I2C AT Flag");
    }
    if (strcmp(I2C_Rx_Buffer, "#HM") == 0) {  // HOME Turret to center lines
      I2C_HM_Flag = true;
      //Serial.println("I2C HM Flag");
    }
    if (strcmp(I2C_Rx_Buffer, "#P1") == 0) {  // PIZO ON
      I2C_P1_Flag = true;
      //Serial.println(F("I2C P1 Flag"));
    }
    if (strcmp(I2C_Rx_Buffer, "#P0") == 0) {  // PIZO OFF
      I2C_P0_Flag = true;
      //Serial.println(F("I2C P0 Flag"));
    }
    if (strcmp(I2C_Rx_Buffer, "#DE") == 0) {  // Tracks Right
      I2C_DE_Flag = true;                     // set flag
      //Serial.println("I2C DE Flag");
    }
    if (strcmp(I2C_Rx_Buffer, "#SF") == 0) {  // SONAR FRONT
      I2C_SF_Flag = true;
      //Serial.println("I2C Sonar Front");
    }
    if (strcmp(I2C_Rx_Buffer, "#SR") == 0) {  // SONAR REAR
      I2C_SR_Flag = true;
      //Serial.println("I2C Sonar Rear ");
    }

    // keep reciving couse will be a number (int) following the command
    c = Wire.read();         // Receive a byte as character
    I2C_Rx_Buffer[3] = (c);  // Save to RX buffer [3]
    c = Wire.read();         // Receive a byte as character
    I2C_Rx_Buffer[4] = (c);  // Save to RX buffer [4]
    c = Wire.read();         // Receive a byte as character
    I2C_Rx_Buffer[5] = (c);  // Save to RX buffer [5]
    c = Wire.read();         // Receive a byte as character
    I2C_Rx_Buffer[6] = (c);  // Save to RX buffer [6]
    c = Wire.read();         // This is not needed but is not harming
    I2C_Rx_Buffer[7] = (c);  // Save to RX buffer [7]
                             //Serial.println(I2C_Rx_Buffer);                  // Print the buffer
  }
  // we know have 4 bytes to convert to an interger number in I2C_Rx_Buffer [3][4][5][6]
  output = I2C_Rx_Buffer[3];          // load output an interger
  output |= (I2C_Rx_Buffer[4] << 8);  // Load and move bits one byte across
  output |= (I2C_Rx_Buffer[5] << 8);
  output |= (I2C_Rx_Buffer[6] << 8);

  //Serial.print(output);                             // RESULT, number recived from Master if there was one
  
  if (I2C_SF_Flag == true) {  // SONAR FRONT flag
    SF_mm = output;           // save value to input
    toTransfer = SF_mm;       // load feed back
    //Serial.println ("I2C SF = " + String(SF_mm));
    I2C_SF_Flag = false;
  }
  if (I2C_SR_Flag == true) {  // SONAR REAR flag
    SR_mm = output;           // save value to input
    toTransfer = SR_mm;       // load feed back
    //Serial.println ("I2C SR = " + String(SR_mm));
    I2C_SR_Flag = false;
  }
  if (I2C_AP_Flag == true) {  // AI Pan
    xRotationValue = output;           // save value to input
    toTransfer = xRotationValue;       // load feed back
    //Serial.println ("I2C AP = " + String(xRotationValue));
    I2C_AP_Flag = false;
    I2C_AP_Update_Flag = true;
  }
  if (I2C_AT_Flag == true) {  // AI Tilt
    yElevationValue = output;           // save value to inpit
    toTransfer = yElevationValue;       // load feed back
    //Serial.println ("I2C AT = " + String(yElevationValue));
    I2C_AT_Flag = false;
    I2C_AT_Update_Flag = true;
  }
  output = 0;  // Reset for next recive

  I2C_Rx_Buffer[0] = 0;  // clear RX Buffer. tidy latter. 7 bytes per send allow 8
  I2C_Rx_Buffer[1] = 0;
  I2C_Rx_Buffer[2] = 0;
  I2C_Rx_Buffer[3] = 0;
  I2C_Rx_Buffer[4] = 0;
  I2C_Rx_Buffer[5] = 0;
  I2C_Rx_Buffer[6] = 0;
  I2C_Rx_Buffer[7] = 0;
}

//-----------------------------------------------------------------
void dataRqst() {
  // I2C EVENT HANDELER:
  // function that executes whenever data is requested by master (master send, reciver respond)
  // this function is registered as an event, see setup()
  // sends back value in range 0-65535 (interger)
  // will keep outputing the last value loaded in to variable 'toTransfer'
  // might work for monitoring movement feedback
  // NOTE: picks up change in value on next loop toTransfer is loaded to shift at end of
  // this. needs tidy maybe fix to make faster. this be the random number after upload

  // send integervalue value to Master
  if (byteSending == 1) {  // send packet 1
    toSend = Shift & mask;
    Shift = Shift >> 8;
    Wire.write(toSend);
    byteSending = 2;
  } else if (byteSending == 2) {  // send packet 2
    toSend = Shift & mask;
    Shift = Shift >> 8;
    Wire.write(toSend);
    byteSending = 3;
  } else if (byteSending == 3) {  // send packet 3
    toSend = Shift & mask;
    Shift = Shift >> 8;
    Wire.write(toSend);
    byteSending = 4;
  } else if (byteSending == 4) {  //send packet 4
    toSend = Shift & mask;
    Shift = Shift >> 8;
    Wire.write(toSend);
    byteSending = 1;     //initialization for next turn
    Shift = toTransfer;  // load value for next send
    mask = 0xFF;
    toSend = 0;
  }
}
//====================================================================================================

// Sonar Obstical Alram Value comes from I2C RMB Master-----------------------------------------
void ProcessUtrasonics() {
  if (Sonar_F_Alm == false) {
    if ((SF_mm) <= SONAR_MIN_MM) {          // Sonar Front within min distance
      if ((SF_mm >= MIN_ERROR)) {           // Sonar > min error
        Sonar_F_Alm = true;                 // set Front alarm flag
        ProcessStop();                      // Stop Tracks
        ProcessR2D2(1);                     // Sound Alarm
        Serial.print("Sonar Alarm Front : ");  // dslpay msg
        Serial.print((SF_mm));
        Serial.println(" mm");
      }
    }
  }
  if (Sonar_R_Alm == false) {
    if ((SR_mm) <= SONAR_MIN_MM) {         // Sonar Rear min distance
      if ((SR_mm >= MIN_ERROR)) {          // error samples
        Sonar_R_Alm = true;                // set Rear alarm flag
        ProcessStop();                     // stop tracks
        ProcessR2D2(1);                    // Sound alarm
        Serial.print("Sonar Alarm Rear : ");  // display msg
        Serial.print((SR_mm));
        Serial.println(" mm");
      }
    }
  }
  if ((SF_mm >= SONAR_MIN_MM) && (Sonar_F_Alm == true)) {  // reset Front
    Sonar_F_Alm = false;                                   // reset Front alarm flag
    Serial.println("Sonar Alarm Front clear");
  }
  if ((SR_mm >= SONAR_MIN_MM) && (Sonar_R_Alm == true)) {  // Reset Rear
    Sonar_R_Alm = false;                                   // reset Rear alarm flag
    Serial.println("Sonar Alarm Rear clear");
  }
}

//--------------------------------------------------------------------
void ProcessLeftEncoder() {
  leftcurrentStateCLK = digitalRead(PIN_ODOM_LHS1);  // Read the current state of PIN_ODOM_LHS1

  // If the previous and the current state of the PIN_ODOM_LHS1 are different then a pulse has occurred
  if (leftcurrentStateCLK != leftpreviousStateCLK) {

    // If the PIN_ODOM_LHS2 state is different than the PIN_ODOM_LHS1 state then
    // the encoder is rotating counterclockwise
    if (digitalRead(PIN_ODOM_LHS2) != leftcurrentStateCLK) {
      leftcounter--;
      leftencdir = "CCW";
    } else {
      // Encoder is rotating clockwise
      leftcounter++;
      leftencdir = "CW";
    }
    //Serial.print("Left Direction: ");
    //Serial.print(leftencdir);
    //Serial.print(" -- Value: ");
    //Serial.println(leftcounter);

    // SAFETY MOVEMENT LIMITER
    if (TRACKS_SAFETY_ON == 1) {
      if (leftcounter >= 71) {
        ProcessStop();
      }
      if (leftcounter <= -71) {
        ProcessStop();
      }
    }
  }
  leftpreviousStateCLK = leftcurrentStateCLK;  // Update previousStateCLK with the current state
}

//-----------------------------------------------------------------------
void ProcessRightEncoder() {
  // Read the current state of PIN_ODOM_RHS1
  rightcurrentStateCLK = digitalRead(PIN_ODOM_RHS1);

  // If the previous and the current state of the PIN_ODOM_LHS1 are different then a pulse has occurred
  if (rightcurrentStateCLK != rightpreviousStateCLK) {

    // If the PIN_ODOM_LHS2 state is different than the PIN_ODOM_LHS1 state then
    // the encoder is rotating counterclockwise
    if (digitalRead(PIN_ODOM_RHS2) != rightcurrentStateCLK) {
      rightcounter--;
      rightencdir = "CCW";
    } else {  // Encoder is rotating clockwise
      rightcounter++;
      rightencdir = "CW";
    }
    //Serial.print("Right Direction: ");
    //Serial.print(rightencdir);
    //Serial.print(" -- Value: ");
    //Serial.println(rightcounter);
  }
  // Update previousStateCLK with the current state
  rightpreviousStateCLK = rightcurrentStateCLK;
}

//-----------------------------------------------------------
void ProcessRMB_I2C() {
  if (I2C_TF_Flag == true) {
    //Serial.println("I2C TF Flag");
    ProcessForward();
    I2C_TF_Flag = false;
    return;
  }
  if (I2C_TB_Flag == true) {
    //Serial.println("I2C TB Flag");  // set flag
    ProcessBackward();
    I2C_TB_Flag = false;
    return;
  }
  if (I2C_TL_Flag == true) {
    //Serial.println("I2C TL Flag ");  // set flag
    ProcessTurnLeft();
    I2C_TL_Flag = false;
    return;
  }
  if (I2C_TR_Flag == true) {
    //Serial.println("I2C TR Flag");
    ProcessTurnRight();
    I2C_TR_Flag = false;
    return;
  }
  if (I2C_TS_Flag == true) {
    //Serial.println("I2C TS Flag");
    ProcessStop();
    I2C_TS_Flag = false;
    return;
  }
  if (I2C_EU_Flag == true) {
    //Serial.println("I2C EU Flag");
    ProcessElevationUp();
    I2C_EU_Flag = false;
    return;
  }
  if (I2C_ED_Flag == true) {
    //Serial.println("I2C ED Flag");
    ProcessElevationDown();
    I2C_ED_Flag = false;
    return;
  }
  if (I2C_RR_Flag == true) {
    //Serial.println("I2C RR Flag");
    ProcessRotateRight();
    I2C_RR_Flag = false;
    return;
  }
  if (I2C_RL_Flag == true) {
    //Serial.println("I2C RL Flag");
    ProcessRotateLeft();
    I2C_RL_Flag = false;
    return;
  }
  if (I2C_AP_Update_Flag == true) {
    Serial.print("I2C AI Pan Deg : ");
    Serial.println(xRotationValue);
    ProcessAI_Rotate();
    I2C_AP_Update_Flag = false;
    return;
  }
  if (I2C_AT_Update_Flag == true) {
    Serial.print("I2C AI Tilt Deg : ");
    Serial.println(yElevationValue);
    ProcessAI_Elevation();
    I2C_AT_Update_Flag = false;
    return;
  }
  if (I2C_HM_Flag == true) {
    ProcessHome();
    I2C_HM_Flag = false;
    return;
  }
  if (I2C_P1_Flag == true) {
    //Serial.println("I2C P1 Flag");
    ProcessR2D2(1);  //pizo on;
    I2C_P1_Flag = false;
    return;
  }
  if (I2C_P0_Flag == true) {
    //Serial.println("I2C P0 Flag");
    digitalWrite(PIN_PIZO, LOW);  // Set Pizo OFF
    I2C_P0_Flag = false;
    return;
  }
  if (I2C_DE_Flag == true) {
    //Serial.println("I2C DE Flag");
    DemoRunFlag = 1;
    I2C_DE_Flag = false;
    return;
  }
}

//--------------------------------------------------------------------
void ProcessSerialInput() {
  bool SerialDataAvailable = false;
  if (Serial.available() != 0)
    SerialDataAvailable = true;

  if (!SerialDataAvailable) return;  // Ignore when there is no serial input

  static byte CurrentBufferPosition = 0;

  while (Serial.available() > 0) {
    char NextByte = 0;
    if (Serial.available() != 0)
      NextByte = Serial.read();
    else
      NextByte = 0;  // WTF is this happening??
    switch (NextByte) {
      case '#':  // Starting new command
        CurrentBufferPosition = 0;
        break;
      case '$':  // Ending command
        return;  // Jump out.. There's more data in the buffer, but we can read that next time around.
        break;
      case '?':  // Presume help - Simulate DS
        SerialInputBuffer[0] = 'D';
        SerialInputBuffer[1] = 'S';
        return;
        break;
      default:                                                                                                        // Just some stuff coming through
        SerialInputBuffer[CurrentBufferPosition] = NextByte;                                                          // Insert into the buffer
        CurrentBufferPosition++;                                                                                      // Move the place to the right
        if (CurrentBufferPosition >= SERIAL_INPUT_BUFFER_MAX) CurrentBufferPosition = (SERIAL_INPUT_BUFFER_MAX - 1);  // Capture Overflows.
    }
  }
  return;
}

//------------------------------------------------------------
void ProcessSerialCommand() {

  char CommandHeader[3];  // Place the header into this buffer

  CommandHeader[0] = SerialInputBuffer[0];
  CommandHeader[1] = SerialInputBuffer[1];
  CommandHeader[2] = 0;

  SerialInputBuffer[0] = 0;  // clear the input buffer to stop looping
  SerialInputBuffer[1] = 0;

  // Left Track - LT
  if ((strcmp(CommandHeader, "LT") == 0) && (SystemMode == SYSTEM_MODE_NORMAL)) {
    char IntValue[4] = { SerialInputBuffer[2], SerialInputBuffer[3], SerialInputBuffer[4], 0 };
    LHS_TracksValue = constrain(atoi(IntValue), 0, 180);
    Serial.print("Serial LT = ");
    Serial.println(LHS_TracksValue);
    servo_tracks_LHS.write(LHS_TracksValue);  // set the servo position (degrees)
    //SystemMode = SYSTEM_MODE_TRACKS_LHS;
  }

  // Right Track - RT
  if ((strcmp(CommandHeader, "RT") == 0) && (SystemMode == SYSTEM_MODE_NORMAL)) {
    char IntValue[4] = { SerialInputBuffer[2], SerialInputBuffer[3], SerialInputBuffer[4], 0 };
    RHS_TracksValue = constrain(atoi(IntValue), 0, 180);
    Serial.print("Serial RT = ");
    Serial.println(RHS_TracksValue);
    servo_tracks_RHS.write(RHS_TracksValue);  // set the servo position (degrees)
    //SystemMode = SYSTEM_MODE_TRACKS_RHS;
  }

  // Elevation - EL
  if ((strcmp(CommandHeader, "EL") == 0) && (SystemMode == SYSTEM_MODE_NORMAL)) {
    char IntValue[4] = { SerialInputBuffer[2], SerialInputBuffer[3], SerialInputBuffer[4], 0 };
    yElevationValue = constrain(atoi(IntValue), ELV_DWN_LIMIT, ELV_UP_LIMIT);  // horizonal position = 512. Down elevation = 600max (550 on angels). Up elevation = 350 min.
    Serial.print("Serial EL = ");
    Serial.println(yElevationValue);
    servo_elevation.write(yElevationValue);
    //SystemMode = SYSTEM_MODE_TURRET_ELE;
  }

  // Rotation - RO
  if ((strcmp(CommandHeader, "RO") == 0) && (SystemMode == SYSTEM_MODE_NORMAL)) {
    char IntValue[4] = { SerialInputBuffer[2], SerialInputBuffer[3], SerialInputBuffer[4], 0 };
    xRotationValue = constrain(atoi(IntValue), ROT_CW_LIMIT, ROT_CCW_LIMIT);
    //xRotationValue = (atoi(IntValue));
    Serial.print("Serial RO = ");
    Serial.println(xRotationValue);
    servo_rotation.write(xRotationValue);
    //SystemMode = SYSTEM_MODE_TURRET_ROT;
  }

  // Pizo OFF - P0
  if ((strcmp(CommandHeader, "P0") == 0) && (SystemMode == SYSTEM_MODE_NORMAL)) {
    Serial.println(F("Serial Pizo Off"));
    digitalWrite(PIN_PIZO, LOW);  // Set Pizo OFF
  }

  //  Pizo On - P1 R2D2
  if ((strcmp(CommandHeader, "P1") == 0) && (SystemMode == SYSTEM_MODE_NORMAL)) {
    Serial.println(F("Serial R2D2-1"));
    //digitalWrite(PIN_PIZO, HIGH);  // Set Pizo ON
    ProcessR2D2(1);
  }

  // Pizo - P2 R2D2
  if ((strcmp(CommandHeader, "P2") == 0) && (SystemMode == SYSTEM_MODE_NORMAL)) {
    Serial.println(F("Serial R2D2-2"));
    ProcessR2D2(2);
  }

  // Home - Turret Center lines - HM
  if (strcmp(CommandHeader, "HM") == 0) {
    Serial.println(F("Serial Home"));
    ProcessHome();
  }

  // Emergancy Stop - Low bat, distance etc - ES
  if (strcmp(CommandHeader, "ES") == 0) {
    Serial.println(F("Serial Emergancy Stop"));
    ProcessStop();
  }

  // Move Forward - F
  if (strcmp(CommandHeader, "F") == 0) {
    Serial.println(F("Move Forward"));
    ProcessForward();
  }

  // Move Backward - B
  if (strcmp(CommandHeader, "B") == 0) {
    Serial.println(F("Move Backward"));
    ProcessBackward();
  }

  // Turn Left - L
  if (strcmp(CommandHeader, "L") == 0) {
    Serial.println(F("Turn Left"));
    ProcessTurnLeft();
  }

  // Turn Right
  if (strcmp(CommandHeader, "R") == 0) {
    Serial.println(F("Turn Right"));
    ProcessTurnRight();
  }

  // Elevation UP
  if (strcmp(CommandHeader, "EU") == 0) {
    Serial.println(F("Elevation Up"));
    ProcessElevationUp();
  }
  // Elevation Down
  if (strcmp(CommandHeader, "ED") == 0) {
    Serial.println(F("Elevation Down"));
    ProcessElevationDown();
  }
  // Rotate CW
  if (strcmp(CommandHeader, "RR") == 0) {
    Serial.println(F("Rotate Right"));
    ProcessRotateRight();
  }
  // Rotate CCW
  if (strcmp(CommandHeader, "RL") == 0) {
    Serial.println(F("Rotate Left"));
    ProcessRotateLeft();
  }
  // Demo Dance - D1
  if (strcmp(CommandHeader, "D1") == 0) {
    Serial.println(F("Serial Demo"));
    DemoRunFlag = 1;
    //ProcessDemo();
  }
  // Display Settings - DS
  if (strcmp(CommandHeader, "DS") == 0) {
    ProcessDebug();
  }
}

//-------------------------------------------
// Process Startup Beeps (2 x Short Beep)
void ProcessStartBeeps() {
  PizoOnTime = millis();
  digitalWrite(PIN_PIZO, HIGH);  // Set Pizo ON
  while (millis() - PizoOnTime < 100) {
  }
  PizoOffTime = millis();
  digitalWrite(PIN_PIZO, LOW);  // Set Pizo OFF
  while (millis() - PizoOffTime < 100) {
  }
  PizoOnTime = millis();
  digitalWrite(PIN_PIZO, HIGH);  // Set Pizo ON
  while (millis() - PizoOnTime < 100) {
  }
  digitalWrite(PIN_PIZO, LOW);  // Set Pizo OFF
}

//------------------------------------------------
//  Blink Built in LED (Yellow) im alive
void ProcessBlinkLED() {
  if ((millis() - LastBlinkLED) > ONESEC) {
    stat_LED = !stat_LED;
    LastBlinkLED = millis();
  }
  digitalWrite(LED_BUILTIN, stat_LED);
}

//-----------------------
// Process Debug - Print Menu & Variables, send ?
void ProcessDebug() {
  Serial.print("========== Ver: ");
  Serial.print(VERSION);
  Serial.println(" ==========");
  Serial.println("? = Display Status");
  Serial.print("#RT000 - 180$ RHS Tracks Deg = ");
  Serial.println(RHS_TracksValue);
  Serial.print("#LT000 - 180$ LHS Tracks Deg = ");
  Serial.println(LHS_TracksValue);
  Serial.print("#RO015 - 140$  xRotationValue = ");
  Serial.println(xRotationValue);
  Serial.print("#EL050 - 075$  yElevationValue = ");
  Serial.println(yElevationValue);
  Serial.println("#HM$ Home to center line ");
  Serial.println("#ES$ Emergancy Stop ");
  Serial.println("#P0$ Pizo Off ");
  Serial.println("#P1$ R2D2 (1) ");
  Serial.println("#P2$ R2D2 (2) ");
  Serial.println("#D1$ Demo On ");
  Serial.println("#F$ Forward ");
  Serial.println("#B$ Backward");
  Serial.println("#L$ Turn Left");
  Serial.println("#R$ Turn Right");
  Serial.println("EU$ Elevation Up");
  Serial.println("#ED$ Elevavtion Down");
  Serial.println("#RR$ Rotate Right");
  Serial.println("#RL$ Rotate Left");
  Serial.println("#P1$ R2D2 random");
  Serial.print("Sonar Front: ");
  Serial.print(SF_mm);
  Serial.print("mm");
  Serial.print("    ");
  Serial.print("Sonar Rear: ");
  Serial.print(SR_mm);
  Serial.print("mm");
  Serial.print("    ");
  Serial.print("Odometer Left: ");
  Serial.print(leftencdir);
  Serial.print(" : ");
  Serial.print(leftcounter);
  Serial.print("    ");
  Serial.print("Odometer Right: ");
  Serial.print(rightencdir);
  Serial.print(" : ");
  Serial.println(rightcounter);
  Serial.println("===============================");

  //Serial.print ("\t");
  //Serial.print("SystemMode = ");
  //Serial.println(SystemMode);
}
