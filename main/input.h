
// INPUT
// Polling for stick control
int pollstick()
{
  // Read the stick.
  int whiteState = digitalRead(whitepin);
  int blueState = digitalRead(bluepin);
  int greenState = digitalRead(greenpin);
  int yellowState = digitalRead(yellowpin);
  int autoState = digitalRead(autoSwitch);
  int wantedGear = 100;
  
  // Determine position
  if (whiteState == HIGH && blueState == HIGH && greenState == HIGH && yellowState == LOW)
  {
     wantedGear = 8;
  } // P
  if (whiteState == LOW && blueState == HIGH && greenState == HIGH && yellowState == HIGH)
  {
    wantedGear = 7;
  } // R
  if (whiteState == HIGH && blueState == LOW && greenState == HIGH && yellowState == HIGH)
  {
    wantedGear = 6;
  } // N
  if (whiteState == LOW && blueState == LOW && greenState == HIGH && yellowState == LOW)
  {
    wantedGear = 5;
  }
  if (whiteState == LOW && blueState == LOW && greenState == LOW && yellowState == HIGH)
  {
    wantedGear = 4;
  }
  if (whiteState == LOW && blueState == HIGH && greenState == LOW && yellowState == LOW)
  {
    wantedGear = 3;
  }
  if (whiteState == HIGH && blueState == LOW && greenState == LOW && yellowState == LOW)
  {
    wantedGear = 2;
  }
  if (whiteState == HIGH && blueState == HIGH && greenState == LOW && yellowState == HIGH)
  {
    wantedGear = 1;
  }
  if (whiteState == LOW && blueState == LOW && greenState == LOW && yellowState == LOW)
  {
    wantedGear = 100;
  }

  /*
  if ( autoState == HIGH ) {
    if ( ! fullAuto ) {
          Serial.println("pollstick: Automode on ");
          fullAuto = true; 
    }
  } else {
    if ( fullAuto ) {
          Serial.println("pollstick: Automode off ");
          fullAuto = false;
    }
  }*/
  return wantedGear;
    /*if ( debugEnabled && wantedGear != gear ) {
    Serial.print("pollstick: Stick says: ");
    Serial.print(whiteState);
    Serial.print(blueState);
    Serial.print(greenState);
    Serial.println(yellowState);
    Serial.print("pollstick: Requested gear prev/wanted/current/new: ");
    Serial.print(prevgear);
    Serial.print(wantedGear);
    Serial.println(gear);
  }*/
}

// For manual microswitch control, gear up
void gearup()
{
  int newGear;
  if (!gear > 5)
  { // Do nothing if we're on N/R/P
    if (!shiftBlocker)
    {
      newGear++;
    };
    if (gear > 4)
    {
      newGear = 5;
    } // Make sure not to switch more than 5.
    if (debugEnabled)
    {
      Serial.println("gearup: Gear up requested");
    }
    gearchangeUp(newGear);
  }
}

// For manual microswitch control, gear down
void geardown()
{
  int newGear;
  if (!gear > 5)
  { // Do nothing if we're on N/R/P
    if (!shiftBlocker)
    {
      newGear--;
    };
    if (gear < 2)
    {
      newGear = 1;
    } // Make sure not to switch less than 1.
    if (debugEnabled)
    {
      Serial.println("gearup: Gear down requested");
    }
    gearchangeDown(newGear);
  }
}

// Polling for manual switch keys
void pollkeys()
{
  int gupState = digitalRead(gupSwitch);     // Gear up
  int gdownState = digitalRead(gdownSwitch); // Gear down
  int prevgdownState = 0;
  int prevgupState = 0;

  if (gdownState != prevgdownState || gupState != prevgupState)
  {
    if (gdownState == LOW && gupState == HIGH)
    {
      int prevgupState = gupState;
      if (debugEnabled)
      {
        Serial.println("pollkeys: Gear up button");
      }
      gearup();
    }
    else if (gupState == LOW && gdownState == HIGH)
    {
      int prevgdownState = gdownState;
      if (debugEnabled)
      {
        Serial.println("pollkeys: Gear down button");
      }
      geardown();
    }
  }
}

// Polling time for transmission control
void polltrans(int newGear, int wantedGear)
{
  int atfTemp = atfRead();
  int trueLoad = loadRead();
  int oilTemp = oilRead();
  // int shiftDelay = 1000;
  int shiftDelay = readMapMem(shiftTimeMap, spcPercentVal, atfTemp);
  
  if (shiftBlocker)
  {
    // if ( sensors ) { shiftDelay = readMap(shiftTimeMap, spcPercentVal, atfTemp); }
    shiftDuration = millis() - shiftStartTime;
    if (shiftDuration > shiftDelay)
    {
      if (debugEnabled)
      {
        Serial.print("polltrans->switchGearStop: shiftDelay/spcPercentVal/atfTemp=");
        Serial.print(shiftDelay);
        Serial.print("-");
        Serial.print(spcPercentVal);
        Serial.print("-");
        Serial.println(atfTemp);
      }
      switchGearStop(cSolenoidEnabled, newGear);
    }
  }
  
  
  //Raw value for pwm control (0-255) for SPC solenoid, see page 9: http://www.all-trans.by/assets/site/files/mercedes/722.6.1.pdf
  // "Pulsed constantly while idling in Park or Neutral at approximately 40% Duty cycle" <- 102/255 = 0.4
  int mpcVal = readMap(mpcNormalMap, trueLoad, atfTemp);

  if (wantedGear > 6)
  {
    analogWrite(spc, 102);
  }
  else if (wantedGear < 6 && sensors)
  {
    mpcVal = (100 - mpcVal) * 2.55;
  }
  else if (wantedGear = 6)
  {
    mpcVal = (100 - 70) * 2.55;
  }
  if (!shiftBlocker)
  {
    analogWrite(mpc, mpcVal);
   /* if (debugEnabled)
    {
      Serial.print("polltrans: mpcVal/atfTemp");
      Serial.print(mpcVal);
      Serial.print("-");
      Serial.println(atfTemp);
    }*/
  };
}
void boostControl() {
  int allowedBoostPressure = boostControlRead();
  int allowedBoostPressureVal = allowedBoostPressure / maxBoostPressure * 255;
  if ( ! shiftBlocker ) { 
    analogWrite(boostCtrl, allowedBoostPressureVal);
  }
}
uint16_t readFreeSram()
{
  uint8_t newVariable;
  // heap is empty, use bss as start memory address
  if ((uint16_t)__brkval == 0)
    return (((uint16_t)&newVariable) - ((uint16_t)&__bss_end));
  // use heap end as the start of the memory address
  else
    return (((uint16_t)&newVariable) - ((uint16_t)__brkval));
};
