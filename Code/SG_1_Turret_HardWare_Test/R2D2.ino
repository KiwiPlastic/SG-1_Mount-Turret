// R2D2
//6-4-26 this code is from SG1-1 Truret V14

//--------------------------------------------
void ProcessR2D2(int Index) {
  if ((millis() - lastUpdateTimeR2D2) > (ONESEC)) {  //used to lock out multi command at once
    switch (Index) {
      case 1:
        Serial.println("R2d2 case 1 ");
        phrase1();
        break;
      case 2:
        Serial.println("R2d2 case 2 ");
        phrase2();
        break;
      case 3:
        Serial.println("R2d2 case 3 ");
        phrase1();
        phrase2();
        break;
      case 4:
        Serial.println("R2d2 case 4 ");
        phrase1();
        phrase2();
        phrase1();
        break;
      case 5:
        Serial.println("R2d2 case 5 ");
        phrase1();
        phrase2();
        phrase1();
        phrase2();
        phrase1();
        break;
      case 6:
        Serial.println("R2d2 case 6 ");
        phrase2();
        phrase1();
        phrase2();
        break;
    }

    K = 2000;
    temp1 = random(3, 9);
    //temp1 = 8;

    for (int i = 0; i <= temp1; i++) {
      //for (int i = 0; i <= random(3, 9); i++){
      ToneValue = K + random(-1700, 2000);
      //temp2 = -1336;
      ToneValue = constrain(ToneValue, 39, 2000);
      tone(PIN_PIZO, ToneValue);
      temp3 = random(70, 170);  // 70, 170
      //temp3 = 137;                        // 134
      delay(temp3);
      noTone(PIN_PIZO);
      temp4 = random(0, 30);  // 0, 30
      //temp4 = 5;                           //23
      delay(temp4);
    }
    noTone(PIN_PIZO);

    Serial.print("Phase 1: ");
    Serial.print(k);
    Serial.print("   ");
    Serial.print(temp5);
    Serial.print("   ");
    Serial.println(temp7);

    Serial.print("Phase 2: ");
    Serial.print(k2);
    Serial.print("   ");
    Serial.print(temp9);
    Serial.print("   ");
    Serial.println(temp11);
    
    Serial.print(K);
    Serial.print("   ");
    Serial.print(temp1);
    Serial.print("   ");
    Serial.print(temp2);
    Serial.print("   ");
    Serial.print(temp3);
    Serial.print("   ");
    Serial.println(temp4);


    lastUpdateTimeR2D2 = millis();  //Exit
  }
}

//----------------------------------------------
void phrase1() {
  k = random(1000, 2000);  //1000, 2000
  //k = 1990;                         // 1000
  temp5 = random(100, 2000);  // 100,2000
  //temp5 = 477;                      // 100

  for (int i = 0; i <= temp5; i++) {
    ToneValue = k + (-i * 2);
    ToneValue = constrain(ToneValue, 39, 2000);
    tone(PIN_PIZO, ToneValue);
    temp6 = random(.9, 2);  // .9, 2
    delay(temp6);
  }
  temp7 = random(100, 1000);  // 100, 1000
  //temp7 = 653;                      // 400
  for (int i = 0; i <= temp7; i++) {
    ToneValue = k + (i * 10);
    ToneValue = constrain(ToneValue, 39, 2000);
    tone(PIN_PIZO, ToneValue);
    temp8 = random(.9, 2);  // .9, 2
    delay(temp8);
  }
}

//---------------------------------------------
void phrase2() {
  k2 = random(1500, 2000);
  //k2 = 1500;   //1500
  temp9 = random(100, 2000);
  //temp9 = 1800;  //1800

  for (int i = 0; i <= temp9; i++) {
    ToneValue = (k2 + (i * 2));
    ToneValue = constrain(ToneValue, 39, 2000);
    tone(PIN_PIZO, ToneValue);
    temp10 = random(.9, 2);
    delay(temp10);
  }
  temp11 = random(100, 1000);
  //temp11 = 145;                   //145 max
  for (int i = 0; i <= temp11; i++) {
    ToneValue = k2 + (-i * 10);
    ToneValue = constrain(ToneValue, 39, 2000);
    tone(PIN_PIZO, ToneValue);
    temp12 = random(.9, 2);
    delay(temp12);
  }
}
