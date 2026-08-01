// 12-4-26

//--------------------------------------------
void ProcessR2D2(int Index) {
  switch (Index) {
    case 1: phrase1(); break;
    case 2: phrase2(); break;
    case 3:
      phrase1();
      phrase2();
      break;
    case 4:
      phrase1();
      phrase2();
      phrase1();
      break;
    case 5:
      phrase1();
      phrase2();
      phrase1();
      phrase2();
      phrase1();
      break;
    case 6:
      phrase2();
      phrase1();
      phrase2();
      break;
  }

  int K = 2000;

  for (int i = 0; i <= random(3, 9); i++) {
    ToneValue = K + random(-1700, 2000);
    ToneValue = constrain(ToneValue, 39, 2000);
    tone(PIN_PIZO, ToneValue);
    delay(random(70, 170));
    noTone(PIN_PIZO);
    delay(random(0, 30));
  }
  noTone(PIN_PIZO);
  delay(500);           // delay before can be recalled, adds a pause
  //lastUpdateTimeR2D2 = millis();
}

//--------------------------------------
void phrase1() {
  int k = random(1000, 2000);
  for (int i = 0; i <= random(100, 2000); i++) {
    ToneValue = k + (-i * 2);
    ToneValue = constrain(ToneValue, 39, 2000);
    tone(PIN_PIZO, ToneValue);
    delay(random(.9, 2));
  }

  for (int i = 0; i <= random(100, 1000); i++) {
    ToneValue = k + (i * 10);
    ToneValue = constrain(ToneValue, 39, 2000);
    tone(PIN_PIZO, ToneValue);
    delay(random(.9, 2));
  }
}
void phrase2() {
  int k = random(1000, 2000);
  for (int i = 0; i <= random(100, 2000); i++) {
    ToneValue = (k + (i * 2));
    ToneValue = constrain(ToneValue, 39, 2000);
    tone(PIN_PIZO, ToneValue);
    delay(random(.9, 2));
  }
  for (int i = 0; i <= random(100, 1000); i++) {
    ToneValue = k + (-i * 10);
    ToneValue = constrain(ToneValue, 39, 2000);
    tone(PIN_PIZO, ToneValue);
    delay(random(.9, 2));
  }
}