// 10-4-26ProcessTurret

// ProcessStop
// ProcessForward
// ProcessBackward
// ProcessTurnRight
// ProcessTurnLeft
// ProcessElevationUp
// ProcessElevationDown
// ProcessRotateRight
// ProcessRotateLeft
// ProcessHome
// ProcessElevHoz
// ProcessRotCenter
// ProcessElevUpMax
// ProcessElevDownmin
// ProcessRotLHSmax
// ProcessRotRHSmax
// ProcessDemo

//----------------------------------------------------------------
void ProcessStop() {
  Serial.print("Tracks_Stoppped LHS : ");
  RHS_TracksValue = RHS_SERVO_STOP;         // the stop (center) position of Track servo RHS = 99
  LHS_TracksValue = LHS_SERVO_STOP;         // the stop (center) position of Track servo LHS = 91
  servo_tracks_RHS.write(RHS_TracksValue);  // set the servo position (degrees)
  servo_tracks_LHS.write(LHS_TracksValue);  // set the servo position (degrees)

  LeftTraveled = int((leftcounter * LeftEncoderMulti));
  Serial.print(LeftTraveled);
  Serial.print(" mm     ");
  RightTraveled = int(((rightcounter)*RightEncoderMulti));
  Serial.print("RHS : ");
  Serial.print(RightTraveled);
  Serial.println(" mm");

  LastDemoWaitTime = millis();
}

//---------------------------------------------------------------
void ProcessForward() {
  Serial.println("Tracks_Forward");
  if (Sonar_F_Alm != true){
    RHS_TracksValue = RHS_SERVO_FWD;
    LHS_TracksValue = LHS_SERVO_FWD;
    rightcounter = 0;
    leftcounter = 0;
    servo_tracks_RHS.write(RHS_TracksValue);  // set the servo position (degrees)
    servo_tracks_LHS.write(LHS_TracksValue);  // set the servo position (degrees)
    LastDemoWaitTime = millis();
  }
}

//---------------------------------------------------------------
void ProcessBackward() {
  Serial.println("Tracks_Reverse");
  if (Sonar_R_Alm != true){
    RHS_TracksValue = RHS_SERVO_REV;
    LHS_TracksValue = LHS_SERVO_REV;
    rightcounter = 0;
    leftcounter = 0;
    servo_tracks_RHS.write(RHS_TracksValue);  // set the servo position (degrees)
    servo_tracks_LHS.write(LHS_TracksValue);  // set the servo position (degrees)
    LastDemoWaitTime = millis();
  }
}

//---------------------------------------------------------------
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

//--------------------------------------------------------------
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

//---------------------------------------------------------------------------------------------
void ProcessElevationUp() {
  Serial.print("Barrel Elevation Up:");
  Serial.println(yElevationValue);
  yElevationValue = (yElevationValue + incrementDeg);
  if (yElevationValue >= ELV_UP_LIMIT){
      yElevationValue = ELV_UP_LIMIT;
  }
  servo_elevation.write(yElevationValue);
}

//--------------------------------------------------------------------------------------------
void ProcessElevationDown() {
  Serial.print("Barrel_Elevation Down: ");
  Serial.println(yElevationValue);
  yElevationValue = (yElevationValue - incrementDeg);
  if (yElevationValue <= ELV_DWN_LIMIT){
      yElevationValue = ELV_DWN_LIMIT;
  }
  servo_elevation.write(yElevationValue);
}

//-------------------------------------------------------------------
void ProcessRotateRight() {
  Serial.print("Barrel_Turn Right: ");
  Serial.println(xRotationValue);
  xRotationValue = (xRotationValue - incrementDeg);
  if (xRotationValue <= ROT_CW_LIMIT) {
      xRotationValue = ROT_CW_LIMIT;
  }
  servo_rotation.write(xRotationValue);
}

//-------------------------------------------------------------------
void ProcessRotateLeft() {
  Serial.print("Barrel Turn Left: ");
  Serial.println(xRotationValue);
  xRotationValue = (xRotationValue + incrementDeg);
  if (xRotationValue >= ROT_CCW_LIMIT) {
      xRotationValue = ROT_CCW_LIMIT;
  }
  servo_rotation.write(xRotationValue);
}

//----------------------------------
void ProcessAI_Rotate(){
  servo_rotation.write(xRotationValue);
  }

//----------------------------------
void ProcessAI_Elevation(){
  servo_elevation.write(yElevationValue);
  }
  
//----------------------------------------------------------------
void ProcessHome() {
  Serial.println("Barrel Home");
  ProcessElevHoz();
  ProcessRotCenter();
  LastDemoWaitTime = millis();
}

//--------------------------------------------------------------
void ProcessElevHoz() {
  Serial.println("Barrel_Horizontal");  // Set Barrel Horizontal
  yElevationValue = ELV_CENTER;
  servo_elevation.write(yElevationValue);
  LastDemoWaitTime = millis();
}
//-----------------------------------------------------------
void ProcessRotCenter() {
  Serial.println("Barrel Centered");  // Set Barrel on rotational center line
  xRotationValue = ROT_CENTER;
  servo_rotation.write(xRotationValue);
  LastDemoWaitTime = millis();
}

//--------------------------------------------------------------
void ProcessElevUpmax() {
  Serial.println("Barrel_Up_Limit");  // Set Barrel pointing Up
  yElevationValue = ELV_UP_LIMIT;
  servo_elevation.write(yElevationValue);
  LastDemoWaitTime = millis();
}

//---------------------------------------------------------------
void ProcessElevDownmin() {
  Serial.println("Barrel_Down_Limit");  //Set Barrel pointing Down
  yElevationValue = ELV_DWN_LIMIT;
  servo_elevation.write(yElevationValue);
  LastDemoWaitTime = millis();
}

//------------------------------------------------------------
void ProcessRotLHSmax() {
  Serial.println("Barrel_Port_Limit");  // Barrel move CCW
  xRotationValue = ROT_CCW_LIMIT;
  servo_rotation.write(xRotationValue);
  LastDemoWaitTime = millis();
}

//-------------------------------------------------------------
void ProcessRotRHSmax() {
  Serial.println("Barrel_Starboard_Limit");  // Barrel move CW
  xRotationValue = ROT_CW_LIMIT;
  servo_rotation.write(xRotationValue);
  LastDemoWaitTime = millis();
}

//--------------------------------------------------------------
void ProcessDemo() {       // Demo Dance 1
  if (DemoRunFlag >= 1) {  // start else = 0
    if (DemoRunFlag == 1) {
      Serial.print(" At Demo Step");
      ProcessStop();  // Stop in case tracks are moving
      ProcessHome();  // Set turret to horizontal home poition
      DemoRunFlag++;
      return;
    }
    if (DemoRunFlag == 2) {  // wait 1 sec
      DemoWaitTime(DemoMulti);
    }
    if (DemoRunFlag == 3) {  // Point Barrel Up all the way
      ProcessElevUpmax();
      DemoRunFlag++;
      return;
    }
    if (DemoRunFlag == 4) {  // Wait 1
      DemoWaitTime(DemoMulti);
    }
    if (DemoRunFlag == 5) {  // Point barrel Down all the way
      ProcessElevDownmin();
      DemoRunFlag++;
      return;
    }
    if (DemoRunFlag == 6) {  // Wait 1
      DemoWaitTime(DemoMulti);
    }
    if (DemoRunFlag == 7) {
      ProcessElevHoz();  // Set Barrel Back at horizontal position
      DemoRunFlag++;
      return;
    }
    if (DemoRunFlag == 8) {  // wait 1
      DemoWaitTime(DemoMulti);
    }
    if (DemoRunFlag == 9) {
      ProcessRotLHSmax();  // Rotatre Barrel CCW port
      DemoRunFlag++;
      return;
    }
    if (DemoRunFlag == 10) {  // wait 1
      DemoWaitTime(DemoMulti);
    }
    if (DemoRunFlag == 11) {
      ProcessRotRHSmax();  // Rotate Barrel CW Starbord
      DemoRunFlag++;
      return;
    }
    if (DemoRunFlag == 12) {  // wait 1
      DemoWaitTime(DemoMulti);
    }
    if (DemoRunFlag == 13) {
      ProcessRotCenter();  // Barrel back to center
      DemoRunFlag++;
      return;
    }
    if (DemoRunFlag == 14) {  // wait 1
      DemoWaitTime(DemoMulti);
    }
    if (DemoRunFlag == 15) {
      ProcessForward();  // Move Tracks forward
      DemoRunFlag++;
      return;
    }
    if (DemoRunFlag == 16) {  // Travel x mm
      LeftTraveled = int((leftcounter * LeftEncoderMulti));
      if (LeftTraveled >= 150) {
        ProcessStop();
        DemoRunFlag++;
        return;
      }
    }
    if (DemoRunFlag == 17) {  // wait x 2 sec
      DemoMulti = 2;
      DemoWaitTime(DemoMulti);
    }

    if (DemoRunFlag == 18) {
      ProcessBackward();  // Move tracks Backward
      DemoRunFlag++;
      return;
    }
    if (DemoRunFlag == 19) {  // Travel 150 mm and stop
      LeftTraveled = int((leftcounter * LeftEncoderMulti));
      if (LeftTraveled <= -150) {
        ProcessStop();
        DemoRunFlag++;
        return;
      }
    }
    if (DemoRunFlag == 20) {  // wait 1
      DemoMulti = 1;
      DemoWaitTime(DemoMulti);
    }
    if (DemoRunFlag == 21) {
      ProcessTurnRight();  // Spin Clock Wise
      DemoRunFlag++;
      return;
    }
    if (DemoRunFlag == 22) {  // wait 2
      DemoMulti = 2;
      DemoWaitTime(DemoMulti);
    }
    if (DemoRunFlag == 23) {
      ProcessStop();  // Stop Tracks
      DemoRunFlag++;
      return;
    }
    if (DemoRunFlag == 24) {  // wait 1
      DemoMulti = 1;
      DemoWaitTime(DemoMulti);
    }
    if (DemoRunFlag == 25) {
      ProcessTurnLeft();  // Spin Counter Clock Wise
      DemoRunFlag++;
      return;
    }
    if (DemoRunFlag == 26) {  // wait 2
      DemoMulti = 2;
      DemoWaitTime(DemoMulti);
    }
    if (DemoRunFlag == 27) {
      ProcessStop();  // Stop Tracks
      DemoRunFlag++;
      return;
    }
    if (DemoRunFlag == 28) {  // wait 1
      DemoMulti = 1;
      DemoWaitTime(DemoMulti);
    }
    if (DemoRunFlag == 29) {
      ProcessElevUpmax();  // Point Barrel Up all the way
      DemoRunFlag++;
      return;
    }
    if (DemoRunFlag == 30) {  // wait 1
      DemoWaitTime(DemoMulti);
    }
    if (DemoRunFlag == 31) {
      ProcessHome();  // Home
      DemoRunFlag++;
    }
    if (DemoRunFlag == 32) {
      Serial.println("R2D2");  //R2D2
      ProcessR2D2(1);          // Finish
      DemoRunFlag = 0;
    }
  }
}
// -----------------------
void DemoWaitTime(int Multi) {
  if ((millis() - LastDemoWaitTime) > (ONESEC * Multi)) {
    DemoRunFlag++;
    DemoMulti = 1;
  }
}
//-------------------------------
