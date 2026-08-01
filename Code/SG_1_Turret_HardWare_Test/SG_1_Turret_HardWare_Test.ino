#define VERSION 3  // code version number
/* 7-4-26

 Project: SG-1 Turret - ESP32 S3
 Discription:  Program to test Turret hardware. Basicaly calibrate
    
    Turret Servos - PWM RDS3225 25kg double shaft RC 270 deg 500-2500msec
    Tracks Servos - PWM MG995, direction from -1 to +1 pwm (0-100%) convert to 0-18- deg
    Odometer Encoders 
    Blink Led
    Pizo startup beeps
    R2D2 sounds testing
    
*/
//=======================================================================
#include <ESP32Servo.h>  // MG995 Servo driver ESP32 Servo Kevin H

// Tracks Calibration                   Range: 0-180 degress.
#define RHS_SERVO_STOP 90  // Right Track Stop poisiton
#define LHS_SERVO_STOP 90  // Left Track Stop position

int RHS_TracksValue = RHS_SERVO_STOP;  // Forward = 50 backw#F$ard = 140
int LHS_TracksValue = LHS_SERVO_STOP;  // Foreard = 140 Backward = 50

#define RHS_SERVO_FWD 180    // Max = 180 Range: 0-180 degress. RHS Stop = 98 LHS Stop = 90
#define LHS_SERVO_FWD 0      // Max = 0
#define RHS_SERVO_REV 0      // Max = 0
#define LHS_SERVO_REV 180    // Max = 180
#define RHS_SERVO_LEFT 0     // Turn Left
#define LHS_SERVO_LEFT 0     // Turn Left
#define RHS_SERVO_RIGHT 180  // Turn Right
#define LHS_SERVO_RIGHT 180  // Turn Right

// Turret Rotation Calibration
#define ROT_CENTER 75      //
#define ROT_CW_LIMIT 15    // Right
#define ROT_CCW_LIMIT 140  // Left

// Turret Elevation Calibrati0n
#define ELV_CENTER 65     //
#define ELV_DWN_LIMIT 50  //
#define ELV_UP_LIMIT 80   //

int xRotationValue = ELV_CENTER;   // Range 0- 360
int yElevationValue = ROT_CENTER;  // Range
//----------------------------------------------------------------------
Servo servo_tracks_RHS;  // create X servo object to control a servo
Servo servo_tracks_LHS;  // create Y servo object to control a servo
Servo servo_rotation;
Servo servo_elevation;

//----------------------------------------------------------------------
//ESP32 PINS
#define PIN_BOOT 0            // GPIO0   Boot button input can be tested on start up
#define PIN_ODOM_LHS1 D0      // GPIO1   Quad Odometer LHS Q1
#define PIN_ODOM_LHS2 D1      // GPIO2   Quad Odometer LHS Q2
#define PIN_ODOM_RHS2 D2      // GPIO3   Quad Odometer RHS Q1
#define PIN_ODOM_RHS1 D3      // GPIO4   Quad Odometer RHS Q2
#define PIN_SDA D4            // D4  SDA          Slave (SG1 Turret) to Master(RMB)
#define PIN_SCL D5            // D5  SCL          Slave (SG1 Turret) to Master (RMB)
#define PIN_PIZO D6           // D6
#define PIN_ELEVATION_PWM D7  // GPIO8   Servo Elevation (RDS3225 25kg double shaft RC servo 270 deg 500-2500msec)
#define PIN_TRACK_PWM_RHS D8  // GPIO9   Motor LHS PWM (MG995 8.5kg 360 deg Servo)
#define PIN_TRACK_PWM_LHS D9  // GPI10   Motor RHS PWM (MG995 8.5kg 360 deg Servo)
#define PIN_ROTATION_PWM D10  // GPI11   Servo Rotation (RDS3225 25kg double shaft RC servo 270 deg 500-2500msec)

//========================================================================
#define UPDATE_INTERVAL 100  // 100ms
#define ONESEC 1000          // One Second Interval

unsigned long lastUpdateTime = 0;
unsigned long lastOneSecTime = 0;
unsigned long LastBlinkLED = millis();
unsigned long LastServo = millis();
unsigned long LastTurret = millis();
unsigned long BootStart = 0;  // Just for any initial timing..
unsigned long PizoOnTime = 0;
unsigned long PizoOffTime = 0;
bool stat_LED = 0;

int incrementDeg = 5;  // Turret servo incriment amount in Deg

int DemoRunFlag = 0;
unsigned long LastDemoWaitTime;

unsigned long lastUpdateTimeR2D2;
bool R2D2_Flag = false;
int K;
int k;
int k2;
int temp1;
int temp2;
int temp3;
int temp4;
int temp5;
int temp6;
int temp7;
int temp8;
int temp9; 
int temp10;
int temp11;
int temp12;
int ToneValue;
  
// USB Serial Comms
#define SERIAL_INPUT_BUFFER_MAX 25
char SerialInputBuffer[SERIAL_INPUT_BUFFER_MAX];

// System Mode - not used
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


//------------------------------------------------
// Rotary Encoder Inputs Left Odometer
int leftcounter = 0;
int leftcurrentStateCLK;
int leftpreviousStateCLK;
String leftencdir = "";

// Rotary Encoder Inputs Right Odometer
int rightcounter = 0;
int rightcurrentStateCLK;
int rightpreviousStateCLK;
String rightencdir = "";

//================================ SETUP ====================================
void setup() {
  BootStart = millis();  // Just for any initial timing..

  Serial.begin(115200);  // Serial Monitor output

  delay(2000);  // let serial start

  Serial.print(F("SG-1_Hardware_Test...Ver: "));  // Display version on Serial Monitor
  Serial.println(VERSION);

  pinMode(LED_BUILTIN, OUTPUT);  // config builtin LED

  pinMode(PIN_PIZO, OUTPUT);    // Pizo buzzer. turn off in code ASAP
  digitalWrite(PIN_PIZO, LOW);  // Set Pizo OFF

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

  //------------------------------------------------
  // Odometer Qudrature encoder

  pinMode(PIN_ODOM_LHS1, INPUT_PULLUP);
  pinMode(PIN_ODOM_LHS2, INPUT_PULLUP);

  pinMode(PIN_ODOM_RHS1, INPUT_PULLUP);
  pinMode(PIN_ODOM_RHS2, INPUT_PULLUP);

  leftpreviousStateCLK = digitalRead(PIN_ODOM_LHS1);   //PIN_ODOM_LHS1
  rightpreviousStateCLK = digitalRead(PIN_ODOM_RHS1);  //PIN_ODOM_RHS1
  delay(500);

  //--------------------------------------------------------
  // R2D2
  randomSeed(analogRead(0));

  ProcessStartBeeps();  // Beep...Beep, Ready
  delay(500);

  //Set Tracks servos MG995
  servo_tracks_RHS.write(RHS_TracksValue);  // set the servo position (degrees)
  servo_tracks_LHS.write(LHS_TracksValue);  // set the servo position (degrees)
  delay(500);

  //Set Rotation servo to centerline
  servo_rotation.write(ROT_CENTER);
  delay(500);

  //Set Elevation Servo to horizontal
  servo_elevation.write(ELV_CENTER);
  delay(500);

  ProcessDebug();
}

//============================ LOOP =======================================
void loop() {
  ProcessLeftEncoder();
  ProcessRightEncoder();

  ProcessSerialInput();    // Chk USB serial CLI
  ProcessSerialCommand();  // Process incomeing USB serial command

  ProcessBlinkLED();
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
  //SerialInputBuffer[2] = 0;
  

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

  // Tone On- to | test sends fixed tone freq to pizo range 39 - 2000hz
  if ((strcmp(CommandHeader, "to") == 0) && (SystemMode == SYSTEM_MODE_NORMAL)) {
    char IntValue[5] = { SerialInputBuffer[2], SerialInputBuffer[3], SerialInputBuffer[4], SerialInputBuffer[5],0 };
    ToneValue = constrain(atoi(IntValue),39,2000);
    Serial.print("Tone = ");
    Serial.println(ToneValue);
    tone(PIN_PIZO, ToneValue);
  }
  // Tone OFF - t9
  if ((strcmp(CommandHeader, "t9") == 0) && (SystemMode == SYSTEM_MODE_NORMAL)) {
    Serial.println(F("tone Off"));
    noTone(PIN_PIZO);
  }

  // Pizo ON - P1 (R2DR2 Case 1)
  if ((strcmp(CommandHeader, "P1") == 0) && (SystemMode == SYSTEM_MODE_NORMAL)) {
    Serial.println(F("Serial PIZO1 ON"));
    ProcessR2D2(1);
    //digitalWrite(PIN_PIZO, HIGH);  // Set Pizo ON
  }

  // Pizo ON - P2 (R2DR2 Case 2)
  if ((strcmp(CommandHeader, "P2") == 0) && (SystemMode == SYSTEM_MODE_NORMAL)) {
    Serial.println(F("Serial PIZO2 ON"));
    ProcessR2D2(2);
  }
  // Pizo ON - P3 (R2DR2 Case 3)
  if ((strcmp(CommandHeader, "P3") == 0) && (SystemMode == SYSTEM_MODE_NORMAL)) {
    Serial.println(F("Serial PIZO3 ON"));
    ProcessR2D2(3);
  }

  // Pizo ON - P4 (R2DR2 Case 4)
  if ((strcmp(CommandHeader, "P4") == 0) && (SystemMode == SYSTEM_MODE_NORMAL)) {
    Serial.println(F("Serial PIZO4 ON"));
    ProcessR2D2(4);
  }

  // Pizo ON - P5 (R2DR2 Case 5)
  if ((strcmp(CommandHeader, "P5") == 0) && (SystemMode == SYSTEM_MODE_NORMAL)) {
    Serial.println(F("Serial PIZO5 ON"));
    ProcessR2D2(5);
  }

  // Pizo ON - P6 (R2DR2 Case 6)
  if ((strcmp(CommandHeader, "P6") == 0) && (SystemMode == SYSTEM_MODE_NORMAL)) {
    Serial.println(F("Serial PIZO6 ON"));
    ProcessR2D2(6);
  }

  // Pizo OFF - P0
  if ((strcmp(CommandHeader, "P0") == 0) && (SystemMode == SYSTEM_MODE_NORMAL)) {
    Serial.println(F("Serial PIZO OFF"));
    digitalWrite(PIN_PIZO, LOW);  // Set Pizo OFF
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
  // Demo Dance - D
  if (strcmp(CommandHeader, "D1") == 0) {
    Serial.println(F("Serial Demo"));
    ProcessDemo();
  }
  // Display Settings - DS
  if (strcmp(CommandHeader, "DS") == 0) {
    ProcessDebug();
  }
}

//----------------------------------------------------------------------------------------------------
void ProcessStop() {
  Serial.println("Tracks_Stop");
  RHS_TracksValue = RHS_SERVO_STOP;         // the stop (center) position of Track servo RHS = 99
  LHS_TracksValue = LHS_SERVO_STOP;         // the stop (center) position of Track servo LHS = 91
  servo_tracks_RHS.write(RHS_TracksValue);  // set the servo position (degrees)
  servo_tracks_LHS.write(LHS_TracksValue);  // set the servo position (degrees)
  LastDemoWaitTime = millis();
}

//---------------------------------------------------------------------------------------------------
void ProcessForward() {
  Serial.println("Tracks_Forward");
  RHS_TracksValue = RHS_SERVO_FWD;
  LHS_TracksValue = LHS_SERVO_FWD;
  rightcounter = 0;
  leftcounter = 0;
  servo_tracks_RHS.write(RHS_TracksValue);  // set the servo position (degrees)
  servo_tracks_LHS.write(LHS_TracksValue);  // set the servo position (degrees)
  LastDemoWaitTime = millis();
}

//--------------------------------------------------------------------------------------------------
void ProcessBackward() {
  Serial.println("Tracks_Reverse");
  RHS_TracksValue = RHS_SERVO_REV;
  LHS_TracksValue = LHS_SERVO_REV;
  rightcounter = 0;
  leftcounter = 0;
  servo_tracks_RHS.write(RHS_TracksValue);  // set the servo position (degrees)
  servo_tracks_LHS.write(LHS_TracksValue);  // set the servo position (degrees)
  LastDemoWaitTime = millis();
}

//------------------------------------------------------------------------------------------------
void ProcessTurnRight() {
  Serial.println("Tracks Turn_Right");
  RHS_TracksValue = RHS_SERVO_LEFT;
  LHS_TracksValue = LHS_SERVO_LEFT;
  rightcounter = 0;
  leftcounter = 0;
  servo_tracks_RHS.write(RHS_TracksValue);  // set the servo position (degrees)
  servo_tracks_LHS.write(LHS_TracksValue);  // set the servo position (degrees)
  LastDemoWaitTime = millis();
}

//-----------------------------------------------------------------------------------------------
void ProcessTurnLeft() {
  Serial.println("Tracks Turn_Left");
  RHS_TracksValue = RHS_SERVO_RIGHT;
  LHS_TracksValue = LHS_SERVO_RIGHT;
  rightcounter = 0;
  leftcounter = 0;
  servo_tracks_RHS.write(RHS_TracksValue);  // set the servo position (degrees)
  servo_tracks_LHS.write(LHS_TracksValue);  // set the servo position (degrees)
  LastDemoWaitTime = millis();
}

//-----------------------------------------------------------------------------------------------
void ProcessHome() {
  Serial.println("Servo Home_Barrel");
  ProcessElevHoz();
  ProcessRotCenter();
  LastDemoWaitTime = millis();
}

//----------------------------------------------------------------------------------------------
void ProcessElevHoz() {
  Serial.println("Servo Barrel_Horizontal");  // Set Barrel Horizontal
  yElevationValue = ELV_CENTER;
  servo_elevation.write(yElevationValue);
  LastDemoWaitTime = millis();
}
//---------------------------------------------------------------------------------------------
void ProcessRotCenter() {
  Serial.println("Servo Barrel Centered");  // Set Barrel on rotational center line
  xRotationValue = ROT_CENTER;
  servo_rotation.write(xRotationValue);
  LastDemoWaitTime = millis();
}

//---------------------------------------------------------------------------------------------
void ProcessElevationUp() {
  Serial.println("Servo Barrel_Elevation Up");
  Serial.print("yElevationValue: ");
  Serial.println(yElevationValue);
  yElevationValue = (yElevationValue + incrementDeg);
  servo_elevation.write(yElevationValue);
}

//--------------------------------------------------------------------------------------------
void ProcessElevationDown() {
  Serial.println("Servo Barrel_Elevation Down");
  Serial.print("yElevationValue: ");
  Serial.println(yElevationValue);
  yElevationValue = (yElevationValue - incrementDeg);
  servo_elevation.write(yElevationValue);
}

//-------------------------------------------------------------------
void ProcessRotateRight() {
  Serial.println("Servo Barrel_Turn Right");
  Serial.print("xRotationValue: ");
  Serial.println(xRotationValue);
  xRotationValue = (xRotationValue + incrementDeg);
  servo_rotation.write(xRotationValue);
}

//-------------------------------------------------------------------
void ProcessRotateLeft() {
  Serial.println("Servo Barrel_Turn Left");
  Serial.print("xRotationValue: ");
  Serial.println(xRotationValue);
  xRotationValue = (xRotationValue - incrementDeg);
  servo_rotation.write(xRotationValue);
}

//=========================================================================
void ProcessLeftEncoder() {
  leftcurrentStateCLK = digitalRead(PIN_ODOM_LHS1);  // Read the current state of PIN_ODOM_LHS1

  // If the previous and the current state of the PIN_ODOM_LHS1 are different then a pulse has occurred
  if (leftcurrentStateCLK != leftpreviousStateCLK) {

    // If the PIN_ODOM_LHS2 state is different than the PIN_ODOM_LHS1 state then
    // the encoder is rotating counterclockwise (CCW)
    if (digitalRead(PIN_ODOM_LHS2) != leftcurrentStateCLK) {
      leftcounter--;
      leftencdir = "CCW";
    } else {      // Encoder is rotating clockwise
      leftcounter++;
      leftencdir = "CW";
    }
    Serial.print("Left Direction: ");
    Serial.print(leftencdir);
    Serial.print(" -- Value: ");
    Serial.println(leftcounter);
  }
  leftpreviousStateCLK = leftcurrentStateCLK;  // Update previousStateCLK with the current state
}

//=========================================================================
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
    } else {     // Encoder is rotating clockwise
      rightcounter++;
      rightencdir = "CW";
    }
    Serial.print("Right Direction: ");
    Serial.print(rightencdir);
    Serial.print(" -- Value: ");
    Serial.println(rightcounter);
  }
  // Update previousStateCLK with the current state
  rightpreviousStateCLK = rightcurrentStateCLK;
}

//-------------------------------------------------------------
void ProcessDemo() {         // Demo Dance 
  if (DemoRunFlag >= 1) {    // start else = 0
    if (DemoRunFlag == 1) {  // Stop in case tracks are moving
      ProcessStop();
      ProcessHome();  // Set turret to horizontal home poition
      DemoRunFlag++;
      return;
    }
  }
}

//==========================================================================
// Process Startup Beeps
void ProcessStartBeeps() {
  PizoOnTime = millis();
  digitalWrite(PIN_PIZO, HIGH);  // Set Pizo ON
  while (millis() - PizoOnTime < 100) {
    //Serial.println("pizo on");
  }
  PizoOffTime = millis();
  digitalWrite(PIN_PIZO, LOW);  // Set Pizo OFF
  while (millis() - PizoOffTime < 100) {
    //Serial.println("pizo off");
  }
  PizoOnTime = millis();
  digitalWrite(PIN_PIZO, HIGH);  // Set Pizo ON
  while (millis() - PizoOnTime < 100) {
    //Serial.println("pizo on");
  }
  digitalWrite(PIN_PIZO, LOW);  // Set Pizo OFF
  //Serial.println("pizo OFF");
}

//===========================================================================
// Blink Built in LED (Yellow) im alive
void ProcessBlinkLED() {
  if ((millis() - LastBlinkLED) > ONESEC) {
    stat_LED = !stat_LED;
    LastBlinkLED = millis();
  }
  digitalWrite(LED_BUILTIN, stat_LED);  //flip flop on/off indicate alive
}

//-----------------------
// Process Debug - Print Menu & Variables, send ?
void ProcessDebug() {
  Serial.print("=========== Ver: ");
  Serial.print(VERSION);
  Serial.println(" ============");
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
  //Serial.println("#P1$ Pizo On ");
  Serial.println("#P0$ Pizo Off ");
  Serial.println("#D1$ Demo On ");
  Serial.println("#F$ Forward ");
  Serial.println("#B$ Backward");
  Serial.println("#L$ Turn Left");
  Serial.println("#R$ Turn Right");
  Serial.println("EU$ Elevation Up");
  Serial.println("#ED$ Elevavtion Down");
  Serial.println("#RR$ Rotate Right");
  Serial.println("#RL$ Rotate Left");
  Serial.println("#P1$ R2D2 1");
  Serial.println("#P2$ R2D2 2 to P5");
  Serial.println("#t1$ Tone Value Hz 39-2000");
  Serial.println("#t9$ Tone Off");
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


//===========================================================================
