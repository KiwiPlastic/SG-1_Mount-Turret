/*
  HUSKYLENS

  Huskeylens_Enabale Flag must be true. Set via App = 'Demo' Btn

  Then Huskylens out put is used as targeting data
  Drone moves to follow target.

  Having trouble doing a learn for Object Tracking. But Object Recognishtion is working with pre defind ones
  ID = 0 is a pre configured ID eg person, horse. with cordinates.
  need to work out how to get Id numbers from them

  Screen Cordinates: X = 320 wide. Y = 240 high. Org 0,0 top Left. Center 160,120

  Outputs coordinates relative to these screen numbers

  Example files:
  HUSKYLENS_OBJECT_TRACKING_Exp1.ino    //Has good stuff in it, including PID

  HUSKYLENS_UTILITIES.ino (writing images to flash)

  Huskelens v2 Vision Tracker I2C.ino This ones simple, it works

  To switch the algorithm on HUSKYLENS:
    // huskylens.writeAlgorithm(ALGORITHM_FACE_RECOGNITION);
    // huskylens.writeAlgorithm(ALGORITHM_OBJECT_TRACKING);
    // huskylens.writeAlgorithm(ALGORITHM_OBJECT_RECOGNITION);
    // huskylens.writeAlgorithm(ALGORITHM_LINE_TRACKING);
    // huskylens.writeAlgorithm(ALGORITHM_COLOR_RECOGNITION);
    // huskylens.writeAlgorithm(ALGORITHM_TAG_RECOGNITION);

    // if (huskylens.request())                    //request all blocks and arrows from HUSKYLENS
    // if (huskylens.requestBlocks())           //request only blocks from HUSKYLENS
    // if (huskylens.requestArrows())           //request only arrows from HUSKYLENS
    // if (huskylens.requestLearned())          //request blocks and arrows tangged ID != 0 from HUSKYLENS
    // if (huskylens.requestBlocksLearned())    //request blocks tangged ID != ID0 from HUSKYLENS
    // if (huskylens.requestArrowsLearned())    //request arrows tangged ID != ID0 from HUSKYLENS
    // if (huskylens.request(ID1))              //request blocks and arrows tangged ID == ID1 from HUSKYLENS
    // if (huskylens.requestBlocks(ID1))        //request blocks tangged ID == ID1 from HUSKYLENS
    // if (huskylens.requestArrows(ID1))        //request arrows tangged ID == ID1 from HUSKYLENS
    // if (huskylens.request(ID2))              //request blocks and arrows tangged ID == ID2 from HUSKYLENS
    // if (huskylens.requestBlocks(ID2))        //request blocks tangged ID == ID2 from HUSKYLENS
    // if (huskylens.requestArrows(ID2))        //request arrows tangged ID == ID2 from HUSKYLENS

    // Serial.println("###################################");
    // Serial.println(String()+F("Count of learned IDs:")+huskylens.countLearnedIDs());//The count of (faces, colors, objects or lines) you have learned on HUSKYLENS.
    // Serial.println(String()+F("frame number:")+huskylens.frameNumber());//The number of frame HUSKYLENS have processed.
*/

// range test
bool isInside(int value, int min, int max) {              // isInside(value, min, max)
  return (value >= min && value <= max);                  // range test value
}

int errorX = 0;
int errorY = 0;

//==============================================================
// this is the AI targeting output, turn into movements here
void HuskeyLens() {
  if (Huskylens_Enable == true) {
   // if ((millis() - LastMillisHuskylens) > ONESEC) {
      //LastMillisHuskylens = millis();

      if (!huskylens.request()) Serial.println(F("Fail to request objects from HUSKYLENS!"));
      else if (!huskylens.isLearned()) {
        Serial.println(F("Object not learned!"));
      }
      else if (!huskylens.available()){
        Serial.println(F("Object disappeared!"));
        //this needs a hold off delay??
        //Cmd_Slave = HM;  // I2c command, home turret
        errorX = 0;
        errorY = 0;
      }
      else
      {
        HUSKYLENSResult result = huskylens.read();            // load and print array (xCenter,yCenter, width, height)
        int x = result.xCenter;
        int y = result.yCenter;
        Serial.print("Object Center: ");
        Serial.print(x);
        Serial.print(", ");
        Serial.println(y);

        // Calculate error from center
        errorX = x - FRAME_WIDTH / 2;
        errorY = y - FRAME_HEIGHT / 2;
        
        Serial.print("Object Error: ");
        Serial.print(errorX);
        Serial.print(", ");
        Serial.println(errorY);
        
        // Adjust angles
        panAngle  -= errorX * panGain;  //0.05
        tiltAngle += errorY * tiltGain;  //0.05

        // Constrain angles
        panAngle  = constrain(panAngle, 15, 140);
        tiltAngle = constrain(tiltAngle, 50, 80);

        // Move servos
        Serial.print("panAngle = ");
        Serial.println(panAngle);
        Serial.print("tiltAngle = ");
        Serial.println(tiltAngle);
        if (Cmd_Slave == RS) { 
          Cmd_Slave = AP;       // Flag to I2C AI Husklens cordinates to send
        }
      }
    //}
  }
}

//------------------------------------------------------
void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK) {
    Serial.println(String() + F("Block:xCenter=") + result.xCenter + F(",yCenter=") + result.yCenter + F(",width=") + result.width + F(",height=") + result.height + F(",ID=") + result.ID);
  }
  else if (result.command == COMMAND_RETURN_ARROW) {
    Serial.println(String() + F("Arrow:xOrigin=") + result.xOrigin + F(",yOrigin=") + result.yOrigin + F(",xTarget=") + result.xTarget + F(",yTarget=") + result.yTarget + F(",ID=") + result.ID);
  }
  else {
    Serial.println("Object unknown!");
  }
}
