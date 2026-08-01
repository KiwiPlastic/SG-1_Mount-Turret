#define VERSION 40
/* 29-4-26

Project: SG-1 RMB  *********  MASTER  ESP32-S3 ***********

written By RICHRD NICHOLSON from NEW ZEALAND

Summary
  SG-1 Turret/RMB Mount, remote control robot
  Tracks, with turret mounted, Brushless Nerf type Blaster.

  Signal stage flywheels, Solenoid pusher with closed loop feedback sensors
  Configuration and control is via 'SG-1 Mount Ctrl' an App for Android Phone

  2 x ESP32s (RMB Frankenboard Master / Turret Slave) use I2C coms between them 
  HMI - Android phone app using BLE link

Features 
 - Mobile Plate Form, Tracks
 - Elevation & Rotation of Turret
 - Half Darts
 - Brushless FlyWheeel Motors 
 - Solenoid Pusher (closed loop)
 - Ultrasonic Distance sensor
 - R2D2 feed back sounds
 - Battery Volts sensor
 - I2C Link From Turret to SG1-RMB Frankenboard
 - SG1 Turret has Serial CLI
 - BLE Server for Android phone app
 - Android phone app (BLE Server), does not work on iPhone
   - BLE blue tooth
   - Status info
   - Trigger
   - Select Fire
   - Rev Ideal 
   - Joy Stick control 
   - Turret Control
   - Settings Config 
 - Huskeylens AI vision module - Object Recognition (Human)

Hardware
 - Seeed XIAO ESP32S3 (RMB Frankenboard)
 - 2 x Brushless DC Motors
 - 2 x ESC
 - Solenoid Pusher
 - Mofet PCB, for Solenoid
 - 2 x limit Switch's,  to make closed loop Solenoid = High ROF
 - 2 x HC-SR04 Ultra sonic sensors (RMB Franken Board)
 - 1 x Huskeylens I2C

//==============================================================
  Notes

  V40 Includes Huskylens, it will only compile in IDE version 1.8.19 
  Could use compile commands to block it out in order to user newer IDE
  Huskeylens function are in own tab. This is called from main loop
  
  D8/D9 Bootstap pins must be high at Power Up/Boot
  We are using Boot strapping pins for ESC output,
  ESCs need to be powered up to get a boot if there signal wires are connected to ESP32. Else will look dead.
  Can be powered up with no ESC's if signal wires are not connected

  BLE (Bluetooth Low Energy) this is different to the older Blutooth 2.0 serial and not compatableSupports Rev Ideal mode, set RevIdeal >90 for pre spin. Change Select fire Mode Switch to turn off RevIdeal

  MIT App Inventor
  https://appinventor.mit.edu/                                          // Phone App dev tool, Android phone only

  https://www.youtube.com/watch?v=RvbWl8rZOoQ&ab_channel=MoThunderz     // App development tool and training video

   See the following for generating UUIDs:  https://www.uuidgenerator.net/
   
//----------------------------------------------------------------------
  ESC HW
    ESC frq = 48Khz
    ESC min Trotel = 1040 (1 milisecond)
    ESC Max Trotel = 1960  (2 Millisecond)
    8 bit PWM resolution 0 = 254

   PWM Values for this code
   freq = 1000. 1kHz dont no why, but it works
    42 = off
    45 is lowest ideal speed, lower stalls
    84 is highest speed = ESC Max Trotel, can make number higher but it does  not go any faster

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
    APnnn             AI Pan Deg
    ATnnn             AI Tilt Deg
    
//----------------------------------------------------------------
 BLE - Phone APP, Write to RMB Frankenbord Master (Callbacks)
  Trig                Trigger -  Push Button
  SF                  Select Fire - Push Button
  REV                 Rev - Push Button
  Burst Size          Burst Size - Slider
  ESC1 - Slider       ESC - Slider
  RevIdel - Slider    Rev Idel - Slider
  SG1_CMD_UUID        Writes all SG1 commands to here, 1 x UUID its faster
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
    DE                Demo (Huskeylens Enable)

  BLE - Phone APP, Recive Status update from Master, 1 x UUID
   - BV               Battery Volts
   - SF               Select Fire - Not Required
   - AC               Ammo Count
   - BS               Burst Size - Not required
   - E1               ESC1 Power - Not required
   - RI               Rev Ideal - Not Required
   - DPS              Darts Fired
   - LB               Low Battery Flag
   - DF               Sonar Front mm 
   - DR               Sonar Rear mm 
   - OL               Odometer Left
   - OR               Odometer Right
   - FR               Feedback Rotation
   - FE               Feedback Elevation

//--------------------------------------------------------------------
 SG1-TURRET USB - Serial Cmd Line Interface - 115200 baud
 ========================================================
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

//---------------------------------------------------------------
// Restart ESP - EXAMPLE CODE
 Serial.println("Parameters saved, restarting..");
  delay(100);
  ESP.restart();

//---------------------------------------------------------------  
  gpio_set_drive_capability((gpio_num_t) 1, GPIO_DRIVE_CAP_0);
  
*/

//===================================================================================
//Load libraries

#include <ESP32Servo.h>  // ESC  libary by Kevin Harrington
ESP32PWM ESC1pwm;        

#include <BLEDevice.h>  // Blue Tooth Libary
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include <Wire.h>
#define I2C_S_ADD 0x08  // SG1 Turret-Slave address

#include <NewPing.h>  // Sonar driver

#include <Preferences.h>  // ESP flash Storage Libary
Preferences preferences;

#include "HUSKYLENS.h"   // HUSKYLENS green line >> SDA; blue line >> SCL
HUSKYLENS huskylens;
void printResult(HUSKYLENSResult result);

//======================================================================================
// Switches


//======================================================================================
// DEFAULT SETTINGS - USER EDITABLE DEFAULT VALUES - Saved to Flash.
// To default blaster settings, Hold down Trigger on startup

#define BURST_SIZE 3        // Burst Mode Size #number of darts
#define MOTOR_ESC1_PWR 84   // Stage 1 : ESC1 Defualt Setting Range: 45-84 max
#define MOTOR_REV_IDEAL 42  // 42 = Stop, 45 = Slow idel Rev, 84 = Full Speed
#define MOTOR_MIN_SPEED 45  // 42 = Stop, 45 = Slow idel Rev, 84 = Full Speed

#define BATTERY_CALFACTOR 0.7  // Adjustment for battery calibration calibration

#define PUSHER_MAX_T 80  // ms for pusher error detect

#define MIN_DWELL_TIME 1  // NOT USED YET WIP
#define AMMO_COUNTER 18   // Mag Size, NOT USED YET WIP

//--------------------------------------------------------------------------------------
// Seeed ESP32 ESP32 S3 -  Pin Definitions (BIOS)
//--------------------------------------------------------------------------------------

#define PIN_BATT_MON A0   // Bat Volts  A0/D0       IP           Analog LiPo battery reading, Resistor devideder 47K/10K
#define PIN_PUSHER_F D1   // Pusher Limit Sw Front  IP
#define PIN_PUSHER_R D2   // Pusher Limit Sw Rear   IP
#define PIN_SONAR_T_F D3  // Ultrasonic distance sensor, Trigger pin, Front
#define PIN_SDA D4        // I2C to SG-1 Turret
#define PIN_SCL D5        // I2C to SG-1 Turret
#define PIN_SONAR_T_R D6  // Ultrasonic distance sensor, Trigger, Rear  D6 should be used as O/P only, to do with boot straping
#define PIN_SONAR_E_F D7  // Ultrasonic distance sensor, Echo pin, Front
#define PIN_ESC_1 D8      // ESC output             OP          D8 is a boot strap pin. must be high at startup
#define PIN_SONAR_E_R D9  // Ultrasonic distance sensor, Echo pin, Front
#define PIN_RUN D10       // Pusher                 OP

// Motors
int MinMotorSpeed = MOTOR_MIN_SPEED;  //
int MaxMotorSpeed1 = MOTOR_ESC1_PWR;  //

// Battery Controls
#define BATTERY_2S_MIN 6.4                 //
#define BATTERY_3S_MIN 9.6                 //
#define BATTERY_4S_MIN 13.2                //
float BatteryCurrentVoltage = 99.0;        //
float BatteryOffset = BATTERY_CALFACTOR;   // asign to a floating point veriable so can be adjusted in config menu (work in progress)
float BatteryMinVoltage = BATTERY_2S_MIN;  // this is just to load the float with a value
bool BatteryFlat = false;                  //

// ISR Flags
bool ISR_PUSHER_F_Flag = false;  // Interrupt triggered flag
bool ISR_PUSHER_R_Flag = false;  // Interrupt triggered flag

// System Modes
#define SYSTEM_MODE_NORMAL 0           // Ideal/ok to Fire
#define SYSTEM_MODE_LOWBATT 1          // All stop
#define SYSTEM_MODE_FLYWHEELFAIL 2     // not used wip
#define SYSTEM_MODE_DANCE 3            // demo Dance script
byte SystemMode = SYSTEM_MODE_NORMAL;  //

// Firing Controls
#define UPDATE_INTERVAL 100  // 100ms ultra sonics uses this
#define TRIG_UPDATE_INTERVAL 300
#define ONESEC 1000          // One second dah

// Selectfire
#define SINGLE 0
#define BURST 1
#define AUTO 2
byte CurrentFireMode = SINGLE;
volatile bool SF_Changed_Flag = false;   // Select Fire Changed
volatile bool MotorRunningFlag = false;  //

// Input Button Status, For ISR output
#define BTN_LOW 0
#define BTN_HIGH 1
#define BTN_ROSE 2
#define BTN_FELL 3

// RMB Main internal registers/veriables
byte TriggerButtonState = BTN_HIGH;
byte BLE_TriggerButtonState = false;
byte BLE_SF_PB_State = false;
byte BLERev_PB_State = false;
byte BurstSize = BURST_SIZE;
byte ESC1_Pwr = MOTOR_ESC1_PWR;
byte RevIdeal = MOTOR_REV_IDEAL;
byte CurrentShot_Value = 0;  // calculated value by firing ctrl repported by nofify

// Map Flash Memory loads defaults on start up, if not exist
unsigned int burstsize = BURST_SIZE;
unsigned int esc1 = MOTOR_ESC1_PWR;
unsigned int revideal = MOTOR_REV_IDEAL;

volatile bool DefaultFlag = false;     // Default flash stroage values
volatile bool PusherTickTock = false;  // To capture the first edge of each pusher in / out cycle

// Sonar - Ultrasonics Distance Sensor
//#define SONAR_MIN_MM 100  // not used? Ultrasonics min distance =  shut down
#define MAX_DISTANCE 200  // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
//#define MIN_ERROR 90      // not used? sonar false reading level
#define SONAR_NUM 2       // Number of utrosonic sensors
#define SONAR_PING 8      // Sonar ping samples to avaerage

NewPing sonar[SONAR_NUM] = {
  NewPing(PIN_SONAR_T_F, PIN_SONAR_E_F, MAX_DISTANCE),  // NewPing setup of pins and maximum distance.
  NewPing(PIN_SONAR_T_R, PIN_SONAR_E_R, MAX_DISTANCE),  // NewPing setup of pins and maximum distance.
};

unsigned long lastUpdateTimeUtrasonics = 0;  // polling is UPDATE_INTERVAL = 100
float SF_duration, SF_distance;              // Sonar floatinng value cm
float SR_duration, SR_distance;              // Sonar floating value cm
long SF_mm;                                  // Sonar Front value in mm
long SR_mm;                                  // Sonar Rear value in mm
//bool Sonar_F_Alm = false;
//bool Sonar_R_Alm = false;

//I2C Commands (Cmd)
#define RS 0          // Rest ( can send sonar)
#define TS 1          // Tracks Stop
#define TF 2          // Tracks Forward
#define TB 3          // Tracks Backward
#define TL 4          // Trakcs Left Turn
#define TR 5          // Tracks Right turn
#define EU 6          // Elevation Up
#define ED 7          // Elevation Down
#define RR 8          // Rotation Right
#define RL 9          // Rotation left
#define HM 10         // Home turret poistion to center lines
#define P1 11         // Pizo On ( R2d2)
#define P0 12         // Pizo Off
#define DE 13         // Demo (SG-1 Turret runs and exerise)
#define SF 14         // Write Sonar Ditance Front
#define SR 15         // Write Sonar Distance Rear
#define AP 16         // AI Huskeylens PAN
#define AT 17         // AI Huskeylens Tilt
byte Cmd_Slave = RS;  // << change this to switch Commands

int I2C_ValueSF = 666;  // Write value to slave
int I2C_ValueSR = 999;  // Write value to slave

int toTransfer = 65535;  // value to send to Slave. Range 0 - 65535 no negative???
int Shift = toTransfer;
int mask = 0xFF;
char toSend = 0;
int output = 0;
unsigned long LastMillisI2C_Req;  // Request data from slave
unsigned long LastMillisI2C_CMD;  // Write I2C CMD to slave

// SOME STUFF
volatile bool PusherFront = false;  // Status flags for manual testing
volatile bool PusherRear = false;

int stat_LED = 0;            // status of LED: 1 = ON, 0 = OFF
unsigned long LastBlinkLED;  // start time in milliseconds
unsigned long Last_BLE_Trigger;  // start time in milliseconds

unsigned long LastBLECycle;  //
byte LatstSonar = SF;
byte BLE_LastSonar = SF;

// HUSKLENS AI
volatile bool Huskylens_Enable = false;
unsigned long LastMillisHuskylens;  

// Turret Rotation Calibration (Pan)
#define PAN_CENTER 75       //
#define PAN_RIGHT_LIMIT 15   // Right
#define PAN_LEFT_LIMIT 140  // Left

// Turret Elevation Calibration (Tilt)
#define TILT_CENTER 65     //
#define TILT_DWN_LIMIT 50  //
#define TILT_UP_LIMIT 80   //

// Camera resolution (HuskyLens default)
#define FRAME_WIDTH  320
#define FRAME_HEIGHT 240

// Current servo angles
int panAngle = PAN_CENTER;
int tiltAngle = TILT_CENTER;

// Tuning (adjust if movement too fast/slow)
float panGain  = 0.05;
float tiltGain = 0.05;

//================================== BLE Server Config ==================================
bool BLE_deviceConnected = false;     //
bool BLE_oldDeviceConnected = false;  //
unsigned long BLE_TimeConnected = 0;  //
bool NotifyFlag = false;              // Used by RMB to send Notify to App
char BLEValue[2];                     // Buffer for Callbacks to read characteristics values

volatile bool BLE_TriggerPB_Flag = false;    // callback sets these flags
volatile bool BLE_RevPB_Flag = false;
volatile bool BLE_Burst_Flag = false;
volatile bool BLE_ESCPwr_Flag = false;
volatile bool BLE_RevIdeal_Flag = false;
volatile bool BLE_TF_Flag = false;          // Sg1 Turrect commands
volatile bool BLE_TB_Flag = false;
volatile bool BLE_TL_Flag = false;
volatile bool BLE_TR_Flag = false;
volatile bool BLE_TS_Flag = false;
volatile bool BLE_RR_Flag = false;
volatile bool BLE_RL_Flag = false;
volatile bool BLE_EU_Flag = false;
volatile bool BLE_ED_Flag = false;
volatile bool BLE_HM_Flag = false;
volatile bool BLE_P1_Flag = false;
volatile bool BLE_P0_Flag = false;
volatile bool BLE_DE_Flag = false;

uint32_t BLE_BatVoltsValue = 0;                //
uint32_t BLE_SF_ModeValue = 0;                 //
uint32_t BLE_AmmoCounterValue = AMMO_COUNTER;  // <<<< chnage this to get working
uint32_t BLE_BurstSizeValue = BurstSize;       //
uint32_t BLE_ESC1Value = ESC1_Pwr;             //
uint32_t BLE_DFValue = 0;                      //
uint32_t BLE_DRValue = 0;                      //
uint32_t BLE_RevIdealValue = RevIdeal;         //
uint32_t BLE_TriggerValue = 0;                 //

volatile byte BLE_LastBatVolts = 99;     //
volatile byte BLE_LastSF_Mode = 99;      //
volatile byte BLE_LastAmmoCounter = 99;  //
volatile byte BLE_LastBurstSize = 99;    //
volatile byte BLE_LastESC1 = 99;         //
volatile byte BLE_LastDF = 99;           //
volatile byte BLE_LastDR = 99;           //
volatile byte BLE_LastRevIdeal = 99;     //
volatile byte BLE_LastCurrentShot = 99;  //
volatile byte BLE_LastBatteryFlat = 99;  //
volatile byte BLE_LastRev_PB = 99;       //
volatile byte BLE_Last_SF_PB = 99;       //
//volatile bool BLE_LastP1 = false;        // Last Pizo status. Tracking for toggel from APP

// BLE Characteristic Labels
BLEServer* pServer = 0;                 // Server Advertising
BLECharacteristic* pStatus_Update = 0;  // Notify Status Update to App
BLECharacteristic* pTrigger = 0;        // R/W Trigger PB
BLECharacteristic* pSF_PB = 0;          // R/W SF PB value
BLECharacteristic* pRev = 0;            // R/W Rev PB
BLECharacteristic* pBurst_Size = 0;     // R/W Burst Size via slider
BLECharacteristic* pESC1 = 0;           // R/W ESC1 Pwr via slider
BLECharacteristic* pRevIdeal = 0;       // R/W Rev Ideal Spd via slider
BLECharacteristic* pSG1 = 0;            // R/W SG1 Command

BLEDescriptor* pDescr;
BLE2902* pBLE2902;

// BLE Characteristics UUID  See the following for generating UUIDs: https://www.uuidgenerator.net/
static BLEUUID BLESERVICE_UUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");  // Severice advertisment
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"              //
#define STATUS_UPDATE_UUID "38ed8a4d-f45d-4323-956a-759072589315"        // Status Update Notifiyer to app
#define FIRE_PB_UUID "b377ef36-7247-46fa-aca3-d21da530c782"              // Trigge Push Button R/W
#define SF_PB__UUID "9c3d379d-28e7-4974-a563-04d7c0b24a51"               // SelectFire setting R/W
#define REV_PB_UUID "738faa41-e3ee-418d-b370-e3ba613dc99b"               // Rev Push button R/W
#define BURST_UUID "e3223119-9445-4e96-a4a1-85358c4046a2"                // Burst Size Slider setting R/W
#define ESC1_UUID "7460002d-70cc-4ed0-98b4-1e778842e64e"                 // ESC1 Slider setting R/W
#define REVIDEAL_UUID "e8c8936e-fc18-4bb7-8004-73ad956a37eb"             // Rev Idel Mode Slider setting R/W
#define SG1_CMD_UUID "e95e6412-3ef4-4aaa-89f7-8599453faabe"              // SG1 TurreCommand, from App to Master R/W

// --------------- BLE CallBacks - Connect/Disconnect -------------------
class ESP32ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    BLE_deviceConnected = true;
    BLE_TimeConnected = millis();
  };
  void onDisconnect(BLEServer* pServer) {
    BLE_deviceConnected = false;
  }
};

//-------------------- BLE CallBacks - Trigger Push Button ----------------------
class CharacteristicTrigger : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pChar) {
    String pChar_value_stdstr = String(pChar->getValue());
    String pChar_value_string = String(pChar_value_stdstr.c_str());  // convert to auduino string format
    BLEValue[0] = (pChar_value_string[0]);                           // BLEValue[] is general use
    
    if (strcmp(BLEValue, "T") == 0) {                                // Test for trigger,if true ...
      BLEValue[0] = 0;                                               // Reset value to random number
      if (millis() - Last_BLE_Trigger >= (TRIG_UPDATE_INTERVAL)) {   // Debounce can hit here 3x / sec
        Last_BLE_Trigger = millis();                                      //
        if (CurrentFireMode == AUTO & BLE_TriggerButtonState == true) {   // if firing
          BLE_TriggerButtonState = false;                                 // reset trigger
          //Serial.println("BLE Trigger OFF");
          return;
        } else if (BLE_TriggerButtonState == false) {
          BLE_TriggerButtonState = true;  
          //Serial.println("BLE Trigger ON");
        }
      }
    }
  }
};

//-------------------- BLE CallBacks - Rev Push Button -------------------------------
class CharacteristicRevPB : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pChar) {
    String pChar_value_stdstr = String(pChar->getValue());
    String pChar_value_string = String(pChar_value_stdstr.c_str());  // convert to auduino string format
    BLEValue[0] = (pChar_value_string[0]);                           // BLEValue[] is general use
    if (strcmp(BLEValue, "R") == 0) {                                // Test for Rev PB, if true ...
      BLEValue[0] = 0;                                               // Reset value to random number
      BLE_RevPB_Flag = true;                                         // set flag, stops debounce
    }
  }
};

//---------------------- BLE CallBacks - Select Fire_Push Button -------------------------
class CharacteristicSF_PB : public BLECharacteristicCallbacks {      // Select fire, via PB in app
  void onWrite(BLECharacteristic* pChar) override {                  //
    String pChar_value_stdstr = String(pChar->getValue());           //
    String pChar_value_string = String(pChar_value_stdstr.c_str());  // convert to auduino string format
    BLEValue[0] = (pChar_value_string[0]);                           // BLEValue[] is general use

    if (strcmp(BLEValue, "S") == 0) {  // Test for Single Fire,if true ...
      BLEValue[0] = 0;                 // Reset value to random number
      BLE_SF_PB_State = SINGLE;        // set Select fire value
    }
    if (strcmp(BLEValue, "B") == 0) {  // Test for Burst,if  true ...
      BLEValue[0] = 0;                 // Reset value to random number
      BLE_SF_PB_State = BURST;         //
    }
    if (strcmp(BLEValue, "A") == 0) {  // Test for Auto,if true ...
      BLEValue[0] = 0;                 // Reset value to random number
      BLE_SF_PB_State = AUTO;          //
    }
  }
};

//---------------------- BLE CallBacks - Burst Size Slider -------------------------
class CharacteristicBurstSize : public BLECharacteristicCallbacks {  // Burst Size via slider in app
  void onWrite(BLECharacteristic* pChar) override {                  //
    String pChar_value_stdstr = String(pChar->getValue());
    String pChar_value_string = String(pChar_value_stdstr.c_str());  // convert to auduino string format
    int pChar_value_int = pChar_value_string.toInt();                // convert string to interger
    //Serial.println("Burst Size: " + String(pChar_value_int));      // Debug: displays slider value as interger
    BurstSize = (pChar_value_int-1);                                     // set Burst Size
    burstsize = BurstSize;                                           // copy to flash variable
    //preferences.putUInt("burstsize", burstsize);                   // save value to Flash
  }
};

//---------------------- BLE CallBack - ESC Pwr Slider-------------------------
class CharacteristicESC1 : public BLECharacteristicCallbacks {  // ESC1 Pwr via slider in app
  void onWrite(BLECharacteristic* pChar) override {             //
    String pChar_value_stdstr = String(pChar->getValue());
    String pChar_value_string = String(pChar_value_stdstr.c_str());  // convert to auduino string format
    int pChar_value_int = pChar_value_string.toInt();                // convert string to interger
    //Serial.println("ESC1: " + String(pChar_value_int));              // Debug: displays slider value as interger
    ESC1_Pwr = pChar_value_int;                                      // save value to ESC1 Power
    MaxMotorSpeed1 = ESC1_Pwr;
    esc1 = ESC1_Pwr;
    preferences.putUInt("esc1", esc1);  // save value to Flash
  }
};

//---------------------- BLE CallBack - Rev Ideal Slider -------------------------
class CharacteristicRevIdeal : public BLECharacteristicCallbacks {  //Rev Ideal via slider in app
  void onWrite(BLECharacteristic* pChar) override {
    String pChar_value_stdstr = String(pChar->getValue());           //
    String pChar_value_string = String(pChar_value_stdstr.c_str());  // convert to auduino string format
    int pChar_value_int = pChar_value_string.toInt();                // convert string to interger
    //Serial.println("RevIdelSpd: " + String(pChar_value_int));            // Debug: displays slider value as interger
    RevIdeal = pChar_value_int;                                      // save value to Ideal Rev Spd
    MinMotorSpeed = RevIdeal;
    revideal = RevIdeal;
    preferences.putUInt("revideal", revideal);  // save value to Flash
  }
};

//---------------------- BLE CallBacks - SG1_CMD_UUID >> SG-1 Turret  -------------------------
class CharacteristicSG1_Cmd : public BLECharacteristicCallbacks {  //Multi command input, app buttons trigger cmds
  void onWrite(BLECharacteristic* pChar) override {
    String pChar_value_stdstr = String(pChar->getValue());           //
    String pChar_value_string = String(pChar_value_stdstr.c_str());  // convert to auduino string format

    if (pChar_value_string == "TF") {  // Tracks Forward
      //Serial.println("BLE Tracks Forward");
      BLE_TF_Flag = true;  // set flag
      return;
    }

    if (pChar_value_string == "TB") {  // Tracks Backward
      //Serial.println("BLE Track Backward");
      BLE_TB_Flag = true;  // set flag
      return;
    }

    if (pChar_value_string == "TL") {  // Tracks Left turn
      //Serial.println("BLE Track Left");
      BLE_TL_Flag = true;  // set flag
      return;
    }

    if (pChar_value_string == "TR") {  // Tracks Right turn
      //Serial.println("BLE Track Right");
      BLE_TR_Flag = true;  // set flag
      return;
    }

    if (pChar_value_string == "TS") {  // Tracks Stop (JS Center)
      //Serial.println("BLE Track Stop");
      BLE_TS_Flag = true;  // set flag
      return;
    }

    if (pChar_value_string == "RR") {  // Turret Rotate Right CW
      //Serial.println("BLE Rotation Right");
      BLE_RR_Flag = true;  // set flag
      return;
    }

    if (pChar_value_string == "RL") {  // Turret Rotate Left CCW
      //Serial.println("BLE Rotate Left");
      BLE_RL_Flag = true;  // set flag, next value is for EL
      return;
    }

    if (pChar_value_string == "EU") {  // Turret Elevation Up
      //Serial.println("BLE Elevation Up");
      BLE_EU_Flag = true;  // set flag
      return;
    }
    if (pChar_value_string == "ED") {  // Turret Elevation Down
      //Serial.println("BLE Elevation Down");
      BLE_ED_Flag = true;  // set flag
      return;
    }

    if (pChar_value_string == "HM") {  // Home Turret
      BLE_HM_Flag = true;              // set flag
      return;
    }

    if (pChar_value_string == "P1") {  // R2D2 Sound
      BLE_P1_Flag = true;              // set flag
      return;
    }

    if (pChar_value_string == "P0") {  // Pizo off just in case
      BLE_P0_Flag = true;              // set flag
      return;
    }

    if (pChar_value_string == "DE") {  // Demo
      //Serial.println("BLE Demo");
      BLE_DE_Flag = true;  // set flag
      return;
    }
    // EXAMPLE CODE to get value out
    //if (BLE_CB_RT_Flag == true) {                        // second loop colect value
    //  int pChar_value_int = pChar_value_string.toInt();  // convert string to interger
    //  BLE_RT_Value = pChar_value_int;                    // Save Number to Buffer, needs chking for valid data
    //  BLE_RT_ValueFlag = true;                           // set flag to process
    //  BLE_CB_RT_Flag = false;                            // reset callback voids flag
    //  return;                                            // get out of here
    //}
  }
};

//-----------------------------------------------------------------------
bool isBootButtonPressed(uint8_t sec) {
  if (!digitalRead(0))
    Serial.println("BOOT button pressed. Keep pressing to load defaults.");
  else
    return false;

  uint32_t msec = sec * 1000;
  unsigned long start_time_ms = millis();
  while (!digitalRead(0)) {
    delay(50);
    //digitalWrite(cfg.LED_PIN, !digitalRead(cfg.LED_PIN));
    if (millis() - start_time_ms > msec) {
      return true;
    }
  }
  return false;
}

//======================================================================================================
void setup() {
  unsigned long BootStart = millis();  // Just for any initial timing..

  //-------------- Pusher Outout ------------------
  pinMode(PIN_RUN, OUTPUT);    // Pusher activates on Boot, so turn off ASAP
  digitalWrite(PIN_RUN, LOW);  // Set Pusher OFF
  pinMode(0, INPUT);           // ESP32 Boot sw

//-------------- I2C ----------------------------
  Wire.begin();  // join I2C bus as the Master
  while (!huskylens.begin(Wire)){
        Serial.println(F("Begin failed!"));
        Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>I2C)"));
        Serial.println(F("2.Please recheck the connection."));
        delay(100);
  }
  //huskylens.writeAlgorithm(ALGORITHM_OBJECT_TRACKING);        // Switch the algorithm to object tracking.
  huskylens.writeAlgorithm(ALGORITHM_OBJECT_RECOGNITION);       // this uses pre configured recognistion
  
  //----------- Im Alive LED ------------------------
  pinMode(LED_BUILTIN, OUTPUT);  // Config Built in Led as an output
  LastBlinkLED = millis();

  //-------------- PWM -----------------------------
  // Allow allocation of all PWM timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  //-------------- USB ----------------------------
  Serial.begin(115200);  // Needs time to start
  delay(1000);

  //-------------- ESC ----------------------------
  ESC1pwm.attachPin(PIN_ESC_1, 1000, 10);  // 1KHz 8 bit
  delay(2000);                             // wait improtant... for startup tones....

  ESC1pwm.writeScaled(0);  // Set min output to ESC = 0.000
  delay(2000);             // wait important....

  ESC1pwm.writeScaled(0.040);  // set min pwm to esc, makes startup tone
  delay(2000);

  //-------------- Bat, Pusher Limits, Sonar --------------
  analogReadResolution(12);      // Set analog input resolution to max, 12-bits
  pinMode(PIN_BATT_MON, INPUT);  // Analog Battery Monitor Input

  pinMode(PIN_PUSHER_F, INPUT_PULLUP);  // Pusher Limit Switch Front
  attachInterrupt(digitalPinToInterrupt(PIN_PUSHER_F), ISR_PUSHER_F, FALLING);

  pinMode(PIN_PUSHER_R, INPUT_PULLUP);  // Pusher Limit Switch Rear
  attachInterrupt(digitalPinToInterrupt(PIN_PUSHER_R), ISR_PUSHER_R, FALLING);

  pinMode(PIN_SONAR_T_F, OUTPUT);  // Front Ultrasonic Trig Pin
  pinMode(PIN_SONAR_E_F, INPUT);   // Front Ultrasonic Echo Pin

  pinMode(PIN_SONAR_T_R, OUTPUT);  // Rear Ultrasonic Trig Pin
  pinMode(PIN_SONAR_E_R, INPUT);   // Rear Ultrasonic Echo Pin

  //-------------- BLE SEVER startup -------------
  // Create the BLE Device
  BLEDevice::init("SG-1 RMB MOUNT");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ESP32ServerCallbacks());  // BLE Sevre Callback

  // Create the BLE Service
  BLEService* pService = pServer->createService(BLESERVICE_UUID, 30, 0);  // the 30 defines the number of chars to be used, increase as needed

  // Create a BLE Characteristic
  pStatus_Update = pService->createCharacteristic(
    STATUS_UPDATE_UUID,
    BLECharacteristic::PROPERTY_NOTIFY);

  pTrigger = pService->createCharacteristic(
    FIRE_PB_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pBurst_Size = pService->createCharacteristic(
    BURST_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pESC1 = pService->createCharacteristic(
    ESC1_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pSF_PB = pService->createCharacteristic(
    SF_PB__UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pRevIdeal = pService->createCharacteristic(
    REVIDEAL_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pRev = pService->createCharacteristic(
    REV_PB_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pSG1 = pService->createCharacteristic(
    SG1_CMD_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  // Create a BLE Descriptor
  pDescr = new BLEDescriptor((uint16_t)0x2901);
  pDescr->setValue("A very interesting variable");
  pStatus_Update->addDescriptor(pDescr);

  pBLE2902 = new BLE2902();
  pBLE2902->setNotifications(true);

  // Add all Descriptors here
  pStatus_Update->addDescriptor(pBLE2902);
  pTrigger->addDescriptor(new BLE2902());
  pBurst_Size->addDescriptor(new BLE2902());
  pESC1->addDescriptor(new BLE2902());
  pSF_PB->addDescriptor(new BLE2902());
  pRevIdeal->addDescriptor(new BLE2902());
  pRev->addDescriptor(new BLE2902());
  pSG1->addDescriptor(new BLE2902());

  // add Callbacks
  pTrigger->setCallbacks(new CharacteristicTrigger());       // Tigger PB from Client
  pSF_PB->setCallbacks(new CharacteristicSF_PB());           // SF PB From Client
  pRev->setCallbacks(new CharacteristicRevPB());             // Rev PB from Client
  pBurst_Size->setCallbacks(new CharacteristicBurstSize());  // Burst Count Slider value
  pESC1->setCallbacks(new CharacteristicESC1());             // ESC1 Slider value
  pRevIdeal->setCallbacks(new CharacteristicRevIdeal());     // RevSpd Slider value
  pSG1->setCallbacks(new CharacteristicSG1_Cmd());           // SG1 Command

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();

  Serial.println("BLE Waiting...");

  //------------------------------------------------
  Serial.print(F("SG1_RMB (Frankenboard) ...Ver: "));  // Show version on USB serial on BOOT
  Serial.println(VERSION);
  
  #define RESET_SETTINGS_HOLD_SEC 3  // 3 seconds
  if (isBootButtonPressed(RESET_SETTINGS_HOLD_SEC)) {
    DefaultFlag = true;
  }
  
  //------------------------------------------------
  // Pre-charge the battery indicator and Wait for the sync ~ 10 seconds
  while (BatteryCurrentVoltage >= 99.0) {
    ProcessBatteryMonitor();  // Check battery voltage occasionally
    delay(10);  // slow it down a bit
  }

  //-------------------------------------------------
  // Config Flash Stroarge
  preferences.begin("rmb-app", false);  // config Flash for R/W. True = Read only.

  // Load default values. Not used yet
  if (DefaultFlag == true) {                                         // If DefaultFlag load Default values to Flash
    preferences.putUInt("burstsize", BURST_SIZE);                    // save value to Flash
    preferences.putUInt("esc1", MOTOR_ESC1_PWR);                     // save value to Flash
    preferences.putUInt("revideal", MOTOR_REV_IDEAL);                // save value to Flash
    DefaultFlag = false;                                             // Reste flag
  }

  // Read Flash Storage, if no values (new chip) use default values
  burstsize = preferences.getUInt("burstsize", BURST_SIZE);  // Read Flash Storage
  esc1 = preferences.getUInt("esc1", MOTOR_ESC1_PWR);
  revideal = preferences.getUInt("revideal", MOTOR_REV_IDEAL);

  BurstSize = burstsize;           // copy flash reading to Main system variable
  BLE_BurstSizeValue = BurstSize;  // Main System variable to BLE variable

  ESC1_Pwr = esc1;            // copy flash reading to Main system variable
  BLE_ESC1Value = ESC1_Pwr;   // Main System variable to BLE variable
  MaxMotorSpeed1 = ESC1_Pwr;  // Copy Main system variable to ESC speed control

  RevIdeal = revideal;           // copy flash reading to Main system variable
  BLE_RevIdealValue = RevIdeal;  // Main System variable to BLE variable
  MinMotorSpeed = RevIdeal;      // Copy Main system variable to ESC speed control

  //------------------------------------------------------
  // Calculate battery Min from bat types
  if (BatteryCurrentVoltage >= BATTERY_4S_MIN) {
    BatteryMinVoltage = BATTERY_4S_MIN;
    Serial.println("Bat Type: 4S");
    BatteryFlat = false;
  } else if (BatteryCurrentVoltage >= BATTERY_3S_MIN) {
    BatteryMinVoltage = BATTERY_3S_MIN;
    Serial.println("Bat Type: 3S");
    BatteryFlat = false;
  } else {
    BatteryMinVoltage = BATTERY_2S_MIN;
    Serial.println("Bat Type: 2S");
    BatteryFlat = false;
  }

  //-------------------------------------------------------
  // Ready Go...
  Serial.println("Bat Min = " + String(BatteryMinVoltage));  // Debug
  Serial.println("Bat Volts = " + String(BatteryCurrentVoltage));
  Serial.println("Flash Burst Setting = " + String(burstsize));  // Show Flash settinngs on Serial
  Serial.println("Flash ESC Power = " + String(esc1));
  Serial.println("Flash Rev Ideal = " + String(revideal));
}

//=============== Interupt Servce Routines =================
void ISR_PUSHER_F() {
  if (!PusherTickTock)  // Switch the tick-tock
    PusherTickTock = true;
  PusherFront = true;
}
//-----------------------------------------------------------
void ISR_PUSHER_R() {
  if (PusherTickTock)  // Switch the tick-tock
    PusherTickTock = false;
  PusherRear = true;
}

//================================ MAIN LOOP =============================================================
void loop() {
  ProcessBatteryMonitor();    // Check battery voltage
  ProcessSystemMode();        // Handle the system mode (Low battery chk)
  ProcessUtrasonics();        // Read Ultra sonic sensors
  ProcessRevSwitch();         // process Rev switch from BLE
  ProcessSelectFire();        // process Select Fire from BLE
  ProcessFiring();            // Handle any firing here
  HuskeyLens();               // enable by Huskylens_Enable flag (set by Demo PB in App)
  ProcessBLERecive();         // processs BLE flags set from cmd in App (Callbacks)
  ProcessBLE();               // BLE Server, sends stuff back to App
  ProcessWriteToSlave();      // I2C send Cmd to Slave
  ProcessRequestFromSlave();  // I2C Request slave to send value
  ProcessBlinkLED();          // Im Alive flash on boarrd LED. coould us for pulses status output???
  }

//========================================================================================================

//----------------------- Process Ultra Sonic (Sonar) distance sensors HC-SR04 -----------------------------------------
void ProcessUtrasonics() {
  if (millis() - lastUpdateTimeUtrasonics > UPDATE_INTERVAL) {  //used to lock out multi command at once
    lastUpdateTimeUtrasonics = millis();

    int iterations = SONAR_PING;  // how many sonic pings to average

    SF_duration = sonar[0].ping_median(iterations);  // get Sonar Front reading (float)
    SF_distance = (SF_duration / 2) * 0.0343;        // convert to distance in cm
    SF_mm = (int(SF_distance * 10));                 // save float to int as mm
    BLE_DFValue = SF_mm;                             // load ble staty value

    SR_duration = sonar[1].ping_median(iterations);  // get sonar Rear reading (float)
    SR_distance = (SR_duration / 2) * 0.0343;        // covert to distance in cm
    SR_mm = (int(SR_distance * 10));                 // save float to int as mm
    BLE_DRValue = SR_mm;

    // Debug
    //Serial.print("SF_distance = ");
    //Serial.print(SF_mm);                                     // Distance will be 0 when out of set max range.
    //Serial.print(" mm            ");

    //Serial.print("SR_distance = ");
    //Serial.print(SR_mm);                                     // Distance will be 0 when out of set max range.
    //Serial.println(" mm");
  }
}

//----------------------------------------------------------------------------------
// Process BLE Recived Char from App. Callbacks sets a flag, action it here. some things r done in callback
void ProcessBLERecive() {
  if (BLE_RevPB_Flag == true) {                                
    BLERev_PB_State = !BLERev_PB_State;
    //Serial.println("Rev_PB = " + String(BLERev_PB_State));  // Debug
    BLE_RevPB_Flag = false;
  }  

  if (BLE_TF_Flag == true) {
    Serial.println("BLE Tracks forward ");
    Cmd_Slave = TF;       // I2c Tracks forward command
    BLE_TF_Flag = false;  // Reset flag
  }

  if (BLE_TB_Flag == true) {
    Serial.println("BLE Track Backward");
    Cmd_Slave = TB;  // I2c command
    BLE_TB_Flag = false;
  }

  if (BLE_TL_Flag == true) {
    Serial.println("BLE Track Left");
    Cmd_Slave = TL;  // I2c command
    BLE_TL_Flag = false;
  }

  if (BLE_TR_Flag == true) {
    Serial.println("BLE Track Right");
    Cmd_Slave = TR;  // I2c command
    BLE_TR_Flag = false;
  }

  if (BLE_TS_Flag == true) {
    Serial.println("BLE Track Stop");
    Cmd_Slave = TS;  // I2c command
    BLE_TS_Flag = false;
  }

  if (BLE_RR_Flag == true) {
    Serial.println("BLE Rotate Right CW");
    Cmd_Slave = RR;  // I2c command
    BLE_RR_Flag = false;
  }

  if (BLE_RL_Flag == true) {
    Serial.println("BLE Rotate Left CCW");
    Cmd_Slave = RL;  // I2c command
    BLE_RL_Flag = false;
  }

  if (BLE_EU_Flag == true) {
    Serial.println("BLE Elevation Up");
    Cmd_Slave = EU;  // I2c command
    BLE_EU_Flag = false;
  }

  if (BLE_ED_Flag == true) {
    Serial.println("BLE Elevaion Down");
    Cmd_Slave = ED;  // I2c command
    BLE_ED_Flag = false;
  }

  if (BLE_HM_Flag == true) {
    Serial.println("BLE Turret Home");
    Cmd_Slave = HM;  // I2c command
    BLE_HM_Flag = false;
  }

  if (BLE_P1_Flag == true) {
    Serial.println("BLE P1 Horn");
    Cmd_Slave = P1;  // I2c command
    BLE_P1_Flag = false;
  }

  if (BLE_P0_Flag == true) {
    Serial.println("BLE P0 off");
    Cmd_Slave = P0;  // I2c command
    BLE_P0_Flag = false;
  }

  if (BLE_DE_Flag == true) {
    Serial.println("BLE HuskyLens Enable/Disable");
    Cmd_Slave = P1;  // I2c command. Trig R2D2 sound
    Huskylens_Enable = !Huskylens_Enable;
    //Cmd_Slave = DE;  // I2c command, triggers SG1-Turret Demo moves
    BLE_DE_Flag = false;
  }
}

//---------------------- Process BLE, send value to phone app -----------------------------------------------
void ProcessBLE() {
  //unsigned long CurrentMillis = millis();                     // Single call to millis()
  int BLE_NotifyFlag = 0;  // process one value per cycle

  if (millis() - LastBLECycle >= (UPDATE_INTERVAL)) {
    LastBLECycle = millis();

    if (NotifyFlag == true)  // If NotifiyFlag, BLE is UP
    {
      BLE_NotifyFlag = false;  // Notify works best at 1 value per scan. Set buffer empty

      if ((BLE_LastBatVolts != BLE_BatVoltsValue) && (BLE_NotifyFlag == false))  // if Bat volts changed
      {
        String pChar_value_string = ("[BV" + String(BLE_BatVoltsValue) + "]");
        BLE_LastBatVolts = BLE_BatVoltsValue;
        //Serial.println("BLE Bat volts : " + String(pChar_value_string));

        pStatus_Update->setValue(pChar_value_string.c_str());
        pStatus_Update->notify();
        BLE_NotifyFlag = true;
      }

      if ((BLE_LastSF_Mode != BLE_SF_ModeValue) && (BLE_NotifyFlag == false))  // If Select Fire Mode Changed
      {
        String pChar_value_string = ("[SF" + String(BLE_SF_ModeValue) + "]");
        BLE_LastSF_Mode = BLE_SF_ModeValue;
        //Serial.println("BLE SELL Fire : " + String(pChar_value_string));

        pStatus_Update->setValue(pChar_value_string.c_str());
        pStatus_Update->notify();
        BLE_NotifyFlag = true;
      }

      if ((BLE_LastAmmoCounter != BLE_AmmoCounterValue) && (BLE_NotifyFlag == false))  // If Select Fire Mode Chnaged
      {
        String pChar_value_string = ("[AC" + String(BLE_AmmoCounterValue) + "]");
        BLE_LastAmmoCounter = BLE_AmmoCounterValue;
        //Serial.println("BLE AmmoCount : " + String(pChar_value_string));

        pStatus_Update->setValue(pChar_value_string.c_str());
        pStatus_Update->notify();
        BLE_NotifyFlag = true;
      }

      if ((BLE_LastCurrentShot != CurrentShot_Value) && (BLE_NotifyFlag == false))  // If Darts Fired changed
      {
        String pChar_value_string = ("[DPS" + String(CurrentShot_Value) + "]");
        BLE_LastCurrentShot = CurrentShot_Value;
        //Serial.println("BLE DPS : " + String(pChar_value_string));

        pStatus_Update->setValue(pChar_value_string.c_str());
        pStatus_Update->notify();
        BLE_NotifyFlag = true;
      }

      if ((BatteryFlat != BLE_LastBatteryFlat) && (BLE_NotifyFlag == false))  // Low Battery Alarm
      {
        String pChar_value_string = ("[LB" + String(BatteryFlat) + "]");
        BLE_LastBatteryFlat = BatteryFlat;
        //Serial.println("BLE Low Bat = " + String(pChar_value_string));

        pStatus_Update->setValue(pChar_value_string.c_str());
        pStatus_Update->notify();
        BLE_NotifyFlag = true;
      }

      // this is slider value x 3, not useing this feeback in app know, its overhead
      if ((BLE_LastBurstSize != BLE_BurstSizeValue) && (BLE_NotifyFlag == false))  // If Burst size changed
      {
        String pChar_value_string = ("[BS" + String(BLE_BurstSizeValue) + "]");
        BLE_LastBurstSize = BLE_BurstSizeValue;
        //Serial.println("BLE BurstSize : " + String(pChar_value_string));

        pStatus_Update->setValue(pChar_value_string.c_str());
        pStatus_Update->notify();
        BLE_NotifyFlag = true;
      }

      if ((BLE_LastESC1 != BLE_ESC1Value) && (BLE_NotifyFlag == false))  // If ESC1 pwr changed
      {
        String pChar_value_string = ("[E1" + String(BLE_ESC1Value) + "]");
        BLE_LastESC1 = BLE_ESC1Value;
        //Serial.println("BLE ESC1 : " + String(pChar_value_string));

        pStatus_Update->setValue(pChar_value_string.c_str());
        pStatus_Update->notify();
        BLE_NotifyFlag = true;
      }

      if ((BLE_LastRevIdeal != BLE_RevIdealValue) && (BLE_NotifyFlag == false))  // If RevIdeal pwr changed
      {
        String pChar_value_string = ("[RI" + String(BLE_RevIdealValue) + "]");
        BLE_LastRevIdeal = BLE_RevIdealValue;
        //Serial.println("BLE RevIdeal : " + String(pChar_value_string));

        pStatus_Update->setValue(pChar_value_string.c_str());
        pStatus_Update->notify();
        BLE_NotifyFlag = true;
      }

      // The sonar values change constanly, these must be here at the end
      if ((BLE_NotifyFlag == false) && (BLE_LastSonar == SR))  // if ideal and last reading sent was Dis Rear
      {
        String pChar_value_string = ("[DF" + String(BLE_DFValue) + "]");
        //Serial.println("BLE DF : " + String(pChar_value_string));

        pStatus_Update->setValue(pChar_value_string.c_str());
        pStatus_Update->notify();
        BLE_LastSonar = SF;
        BLE_NotifyFlag = true;
      }

      if ((BLE_NotifyFlag == false) && (BLE_LastSonar == SF))  // if ideal and last reading sent was Dis Front
      {
        String pChar_value_string = ("[DR" + String(BLE_DRValue) + "]");
        //Serial.println("BLE DR : " + String(pChar_value_string));

        pStatus_Update->setValue(pChar_value_string.c_str());
        pStatus_Update->notify();
        BLE_LastSonar = SR;
        BLE_NotifyFlag = true;
      }                                                 
    }  // end if device connected (Notify Flag)
  }

  // Disconnecting
  if (!BLE_deviceConnected && BLE_oldDeviceConnected) {  // If disconnect
    delay(500);                                          // give the bluetooth stack the chance to get things ready
    NotifyFlag = false;                                  // Reset Notify flag

    BLE_BurstSizeValue = BurstSize;  // Save values else on reconnect will be wrong (This is Not Flash memory)
    BLE_LastBurstSize = BurstSize;   //

    BLE_ESC1Value = ESC1_Pwr;
    BLE_LastESC1 = ESC1_Pwr;

    BLE_RevIdealValue = RevIdeal;
    BLE_LastRevIdeal = RevIdeal;

    pServer->startAdvertising();  // restart advertising
    Serial.println("********** Start Advertising *****************");
    BLE_oldDeviceConnected = BLE_deviceConnected;
  }

  // connecting
  if (BLE_deviceConnected && !BLE_oldDeviceConnected)  // do stuff here on connecting - preload IO values
  {
    if (millis() > (BLE_TimeConnected + ONESEC))  // provide a startup delay
    {
      Serial.println("Connected");
      BLE_oldDeviceConnected = BLE_deviceConnected;  // deviceConnected flag = true or flase

      BLE_LastBatVolts = 99;
      BLE_LastSF_Mode = 99;
      BLE_LastAmmoCounter = 99;
      BLE_LastBurstSize = 99;
      BLE_LastESC1 = 99;
      BLE_LastDF = 99;
      BLE_LastDR = 99;
      BLE_LastRevIdeal = 99;
      BLE_LastBatteryFlat = 99;  // reset notifiy compare values

      NotifyFlag = true;  // allow Notifys to start
    }
  }
}

//---------------------------------------- I2C -----------------------------------------------------
// I2C - We send a cmd string to the slave, when we do a requiest we get the associated value back
// Write new value to Slave, SG-1. 0-65535 this would be tracks, elevation or rotation
void ProcessWriteToSlave() {
  if (millis() - LastMillisI2C_CMD >= (UPDATE_INTERVAL)) {
    LastMillisI2C_CMD = millis();

    Wire.beginTransmission(I2C_S_ADD);  // informs the bus that we will be sending data to slave device 8 (0x08)

    if (Cmd_Slave == RS) {                   // if ideal, send sonar readings
      Wire.write((const uint8_t*)"#SR", 3);  // 3 bytes Tag --- Sonar Rear
      toTransfer = SR_mm;                    // set value
      Shift = toTransfer;
      toSend = 0;
      ProcessSendInt();
      Wire.endTransmission();                // Terminate line
      Wire.beginTransmission(I2C_S_ADD);     // --- Send Sonar Front
      Wire.write((const uint8_t*)"#SF", 3);  // 3 bytes Tag
      toTransfer = SF_mm;
      Shift = toTransfer;
      toSend = 0;
      ProcessSendInt();
      Cmd_Slave = RS;
    }
    if (Cmd_Slave == TF) {                   // Tracks forward
      Wire.write((const uint8_t*)"#TF", 3);  // 3 bytes Tag
      //Cmd_Slave = RS;                        // Clear I2C flag, no send back
    }
    if (Cmd_Slave == TB) {                   // Tracks Backward
      Wire.write((const uint8_t*)"#TB", 3);  // 3 bytes Tag
      //Cmd_Slave = RS;
    }
    if (Cmd_Slave == TL) {                   // Tracks Left
      Wire.write((const uint8_t*)"#TL", 3);  // 3 bytes Tag
      Cmd_Slave = RS;
    }
    if (Cmd_Slave == TR) {                   // Tracks Right
      Wire.write((const uint8_t*)"#TR", 3);  // 3 bytes Tag
      Cmd_Slave = RS;
    }
    if (Cmd_Slave == TS) {                   // Tracks Stop
      Wire.write((const uint8_t*)"#TS", 3);  // 3 bytes Tag
      Cmd_Slave = RS;
    }
    if (Cmd_Slave == RR) {                   // Rotate Right
      Wire.write((const uint8_t*)"#RR", 3);  // 3 bytes Tag
      Cmd_Slave = RS;
    }
    if (Cmd_Slave == RL) {                   // Rotate Left
      Wire.write((const uint8_t*)"#RL", 3);  // 3 bytes Tag
      Cmd_Slave = RS;
    }
    if (Cmd_Slave == EU) {                   // Elevation Up
      Wire.write((const uint8_t*)"#EU", 3);  // 3 bytes Tag
      Cmd_Slave = RS;
    }
    if (Cmd_Slave == ED) {                   // Elevation Down
      Wire.write((const uint8_t*)"#ED", 3);  // 3 bytes Tag
      Cmd_Slave = RS;
    }
    if (Cmd_Slave == AT) {                   // if AI Tilt send readings
      Wire.write((const uint8_t*)"#AT", 3);  // 3 bytes Tag 
      toTransfer = tiltAngle;                // set value
      Shift = toTransfer;
      toSend = 0;
      ProcessSendInt();
      Cmd_Slave = RS;
    }
    if (Cmd_Slave == AP) {                   // if AI PAN send readings
      Wire.write((const uint8_t*)"#AP", 3);  // 3 bytes Tag
      toTransfer = panAngle;                 // set value
      Shift = toTransfer;
      toSend = 0;
      ProcessSendInt();
      Cmd_Slave = AT;
    }
    if (Cmd_Slave == HM) {                   // Home Turret to centers
      Wire.write((const uint8_t*)"#HM", 3);  // 3 bytes Tag
      Cmd_Slave = RS;                        // Clear I2C flag, no send back
    }
    if (Cmd_Slave == P1) {                   // Pizo ON
      Wire.write((const uint8_t*)"#P1", 3);  // 3 bytes Tag
      Cmd_Slave = RS;
    }
    if (Cmd_Slave == P0) {                   // Pizo OFF
      Wire.write((const uint8_t*)"#P0", 3);  // 3 bytes Tag
      Cmd_Slave = RS;
    }
    if (Cmd_Slave == DE) {                   // Demo
      Wire.write((const uint8_t*)"#DE", 3);  // 3 bytes Tag
      Cmd_Slave = RS;
    }

    // when tracks are going, sonar is not updated via I2C need to send sonar with tracks, 
    if ((Cmd_Slave == TF) || (Cmd_Slave == TB)) {  // Write value for Left & Right Track (X/Y Axis)
      Wire.write((const uint8_t*)"#SR", 3);  // 3 bytes Tag
      toTransfer = SR_mm;                    // set value
      Shift = toTransfer;
      Serial.println("I2C M2S SR : " + String(toTransfer));
      toSend = 0;
      ProcessSendInt();
      Wire.endTransmission();  // Terminate line

      Wire.beginTransmission(I2C_S_ADD);     // Know send Sonar Front
      Wire.write((const uint8_t*)"#SF", 3);  // 3 bytes Tag
      toTransfer = SF_mm;
      Shift = toTransfer;
      Serial.println("I2C M2S SF : " + String(toTransfer));
      toSend = 0;
      ProcessSendInt();
      Cmd_Slave = RS;
    }
    Wire.endTransmission();  // informs the bus and the slave device that we have finished sending data
    // initialization for next turn
    Shift = toTransfer;
    mask = 0xFF;
    toSend = 0;
  }
}

//-------------------------------------------
// I2C - send interger
void ProcessSendInt() {
  toSend = Shift & mask;
  Shift = Shift >> 8;
  Wire.write(toSend);
  //send 2nd byte
  toSend = Shift & mask;
  Shift = Shift >> 8;
  Wire.write(toSend);
  // send 3rd byte
  toSend = Shift & mask;
  Shift = Shift >> 8;
  Wire.write(toSend);
  // send forth byte
  toSend = Shift & mask;
  Shift = Shift >> 8;
  Wire.write(toSend);
}

//-------------------------------------------
// I2C - Request data from slave
void ProcessRequestFromSlave() {

  if (millis() - LastMillisI2C_Req >= (UPDATE_INTERVAL * 2)) {
    LastMillisI2C_Req = millis();
    Wire.requestFrom(I2C_S_ADD, 1);  // request from slave 0x08, 1 byte
    while (Wire.available())         // read response from slave 0x08
    {
      char received = Wire.read();
      output = received;
    }
    for (int i = 0; i < 3; i++)  // next 3 bytes
    {
      Wire.requestFrom(I2C_S_ADD, 1);
      while (Wire.available()) {
        char received = Wire.read();
        output |= (received << 8);
      }
    }
    // **** KEEP AS EXAMPLE HOW TO DO *****
    //if (Cmd_Slave == SR) {                      // Get distance front
    //BLE_DFValue = output;                       // seams to work like this
    //Serial.print("DF Recived on I2C = ");
    //Serial.println (BLE_DFValue);
    //Cmd_Slave = RS;
    //return;
    //}
    //if (Cmd_Slave == SF) {
    //BLE_DRValue = output;
    //Serial.print("DR Recived on I2C = ");
    //Serial.println (BLE_DRValue);
    //Cmd_Slave = RS;
    //return;
    //}
  }
}

// ====================== RMB Start ========================
// Process System Mode 
void ProcessSystemMode() {
  static byte LastSystemMode = 99;

  if (BatteryFlat) {
    SystemMode = SYSTEM_MODE_LOWBATT;
    //send Emergancy Stop to turret here
  } else {
    SystemMode = SYSTEM_MODE_NORMAL;
  }

  LastSystemMode = SystemMode;

  if (SystemMode == SYSTEM_MODE_NORMAL) {  // Optic Gate Readout for testing PCB
    if (PusherRear == true) {
      byte pr = digitalRead(PIN_PUSHER_R);
      //Serial.println("Pusher Rear = " + String(pr));
      PusherRear = false;
    }
    if (PusherFront == true) {
      byte pf = digitalRead(PIN_PUSHER_F);
      //Serial.println("Pusher Front = " + String(pf));
      PusherFront = false;
    }
  }
}

//------------------------- Process BLE Rev Switch  ------------------------------------------------------
void ProcessRevSwitch() {
  if (SystemMode == SYSTEM_MODE_NORMAL) {
    if (BLE_LastRev_PB != BLERev_PB_State) {
      //Serial.println("Rev_PB = " + String(BLERev_PB_State));  // Debug
      if (BLERev_PB_State == false) {                         // off, not pressed, Stop motors
        MinMotorSpeed = MOTOR_REV_IDEAL;                      // stop
        StopMotors();
      }
      if (BLERev_PB_State == true) {  // on, pressed, start motors at Rev Ideal
        if (RevIdeal >= MOTOR_MIN_SPEED) {
          byte Temp1 = MaxMotorSpeed1;  // save value
          MinMotorSpeed = RevIdeal;
          MaxMotorSpeed1 = RevIdeal;
          StartMotors();
          MaxMotorSpeed1 = Temp1;  // restore values
        }
      }
      BLE_LastRev_PB = BLERev_PB_State;
    }
  }
}


//----------------------------------------- Process SelectFire ---------------------------------------------------
void ProcessSelectFire() {
  if (BLE_Last_SF_PB != BLE_SF_PB_State) {
    if (BLE_SF_PB_State == BURST) {
      CurrentFireMode = BURST;
      BLE_SF_ModeValue = BURST;  // BLE notify BURST
      Serial.println("Burst Mode");
    } else if (BLE_SF_PB_State == SINGLE) {
      CurrentFireMode = SINGLE;
      BLE_SF_ModeValue = SINGLE;  // BLE notify SINGLE shot
      Serial.println("Single Shot");
    } else {
      CurrentFireMode = AUTO;
      BLE_SF_ModeValue = AUTO;  // BLE notify AUTO
      Serial.println("Full Auto");
    }
    SF_Changed_Flag = true;  //indicate SF has changed to stop RevIdeal
    BLE_Last_SF_PB = BLE_SF_PB_State;
    //Serial.println("SF PB = " + String(BLE_SF_PB_State)); // Debug
  }
  if ((SF_Changed_Flag == true) && (RevIdeal >= MOTOR_MIN_SPEED) && (MotorRunningFlag == true)) {  // turn off motors if RevIdeal > 42
    MinMotorSpeed = MOTOR_REV_IDEAL;
    SF_Changed_Flag = false;
    MotorRunningFlag = false;
    StopMotors();
  }
  SF_Changed_Flag = false;
}

//---------------------------------- Process Firing control logic ----------------------------------------------------
void ProcessFiring() {
  bool StartFiringCycle = false;  //
  bool Trigger_On_Flag = false;   // internaal flag 

  static unsigned long RevStart = 0;       //
  unsigned long CurrentMillis = millis();  // Single call to millis() for better performance

  if ((BLE_TriggerButtonState == true) && (StartFiringCycle == false))  // Check if the BLE trigger is true
    StartFiringCycle = true;

  // Only fire in normal mode
  if (StartFiringCycle && (SystemMode != SYSTEM_MODE_NORMAL)) {  // return if not Normal Mode
    return;
  }

  if (StartFiringCycle) {                                      // Start Fireing sequence - we have a Trigger
    Serial.println("Bat = " + String(BatteryCurrentVoltage));  // debug
    if (RevIdeal >= MOTOR_MIN_SPEED)                           // pre set minMotor to RevIdeal
      MinMotorSpeed = RevIdeal;
    RevStart = millis();
    StartMotors();                     // Start motors
    while (millis() - RevStart < 200)  //
    {
      delay(1);
    }

    unsigned long CurrentShot = 0;
    unsigned long DPSStart = millis();
    do                 // Firing loop start, time critical, dont fuck with it
    {                  //
      FireSolenoid();  // fire solenoid and return
      CurrentShot++;   //
      if ((CurrentFireMode == SINGLE && CurrentShot == 1)) {
        BLE_TriggerButtonState = false;
        Trigger_On_Flag = false;
      }
      if (BLE_TriggerButtonState == true)                                                                                                   // test for trigget still present
        Trigger_On_Flag = true;                                                                                                             // if trigger set flag
      else                                                                                                                                  //
        Trigger_On_Flag = false;                                                                                                            // Reset trigger flag
    } while ((CurrentFireMode == AUTO && Trigger_On_Flag) || (CurrentFireMode == BURST && (CurrentShot <= BurstSize) && Trigger_On_Flag));  // LOOP

    CurrentMillis = millis();  // update current millis
    StopMotors();
    PusherRear = false;  // clear int flags
    PusherFront = false;
    CurrentShot_Value = CurrentShot;  // this will be the BLE load

    Serial.println("Darts Fired = " + String(CurrentShot));

    BLE_TriggerButtonState = false;
  }
}

// ------------------------------------ Solenoid Firing Sequence -------------------------------------------------
void FireSolenoid() {
  unsigned long StartThrow = millis();
  unsigned long StartTimer = millis();
  bool Failed = false;

  digitalWrite(PIN_RUN, HIGH);  // Turn On Solenoid

  StartTimer = millis();  // Wait for the sensor to report an extended pusher
  Failed = true;
  PusherTickTock = false;
  while (millis() - StartTimer <= PUSHER_MAX_T) {
    if (PusherTickTock) {  // Solenoid Tick Tock
      Failed = false;
      break;
    }
  }
  digitalWrite(PIN_RUN, LOW);  // Turn Off Solenoid
  //Serial.println("PF_Failed = " + String(Failed));  // Debug. 1 = error no hit

  StartTimer = millis();  // Wait for the sensor to report an extended pusher
  Failed = true;
  while (millis() - StartTimer <= PUSHER_MAX_T) {
    if (!PusherTickTock) {
      Failed = false;
      break;
    }
  }
  digitalWrite(PIN_RUN, LOW);  // Turn Off Solenoid
  //Serial.println("PR_Failed = " + String(Failed));  // Debug. 1 = error no interupt signal
}

//------------------------------------- Start Fly Wheel Motors --------------------------------------------------------
void StartMotors() {
  float PWMValue1 = (0.001 * MaxMotorSpeed1);  // convert to float
  ESC1pwm.writeScaled(PWMValue1);              // start motor
  MotorRunningFlag = true;                     // For RevIdeal Flag
  //Serial.println(MaxMotorSpeed1);
  //Serial.println(PWMValue1,3);
}

//---------------------------------------Stop Motors ------------------------------------------------------------
void StopMotors() {
  float PWMValue1 = (0.001 * MinMotorSpeed);  // convert to float
  ESC1pwm.writeScaled(PWMValue1);             // stop motors
  //Serial.println("Motor OFF");
}

//------------------------------------ Process Battery Monitor -------------------------------------------------
void ProcessBatteryMonitor() {
#define NUM_SAMPLES 8
  static byte CollectedSamples = 0;
  static float SampleAverage = 0;
  uint32_t Vbatt = 0;

  if ((SystemMode == SYSTEM_MODE_NORMAL) || (SystemMode == SYSTEM_MODE_LOWBATT))  // Only read if system mode is normal or low batery
  {
    if (CollectedSamples < NUM_SAMPLES) {
      CollectedSamples++;
      SampleAverage += analogReadMilliVolts(PIN_BATT_MON);  // Read ADC with correction
    } else {
      BatteryCurrentVoltage = ((6 * SampleAverage / 8 / 1000.0) - BatteryOffset);  // attenuation ratio 1/6, mV --> v
      BLE_BatVoltsValue = int(BatteryCurrentVoltage * 10);                         //convert to int for BLE value

      if ((BLE_BatVoltsValue > (BLE_LastBatVolts + 3)) || (BLE_BatVoltsValue < (BLE_LastBatVolts - 3)))  //Provide hysteresis
      {
        //BatVoltsValue = BatVoltsValue;                                                //update, dumy as already loaded above
      } else {
        BLE_BatVoltsValue = BLE_LastBatVolts;  // dont update
      }

      if (BatteryCurrentVoltage < BatteryMinVoltage) {
        if (BatteryCurrentVoltage > 1.6)  // If the current voltage is 0, we are probably debugging
        {
          BatteryFlat = true;
          Serial.println("BAT FLAT V = " + String(BatteryCurrentVoltage));
          Serial.println("BatMin = " + String(BatteryMinVoltage));
          Cmd_Slave = P1;  // turn pizo on
        } else {
          BatteryFlat = false;
          Cmd_Slave = P0;  // turn pizo off
        }
      } else {
        BatteryFlat = false;
        //Cmd_Slave = P0;         // turn pizo off
      }
      CollectedSamples = 0;
      SampleAverage = 0;
    }
  }
}

//------------------------------------------------
// Blink LED l
void ProcessBlinkLED() {
  if ((millis() - LastBlinkLED) > ONESEC) {
    stat_LED = !stat_LED;
    LastBlinkLED = millis();

    // Display Readings at 1 sec interval
    //Serial.print ("SF: ");
    //Serial.print (SF_mm);
    //Serial.print ("mm");
    //Serial.print ("     ");
    //Serial.print ("SR: ");
    //Serial.print (SR_mm);
    //Serial.print("mm");
    //Serial.print ("     ");
    //Serial.print ("Bat_V: ");
    //Serial.println (BatteryCurrentVoltage);
  }
  digitalWrite(LED_BUILTIN, stat_LED);  //flip flop on/off indicate alive
}
