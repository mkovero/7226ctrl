#include <Arduino.h>
#include "config.h"
#include "pins.h"
#include "calc.h"
#include "core.h"
#include "sensors.h"
#include "maps.h"

// INPUT
// Polling for stick control
// This is W202 electronic gear stick, should work on any pre-canbus sticks.
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
/* if ( debugEnabled) {
    Serial.print("pollstick: Stick says: ");
    Serial.print(whiteState);
    Serial.print(blueState);
    Serial.print(greenState);
    Serial.println(yellowState);
    Serial.print("pollstick: Requested gear prev/wanted/current/new: ");
    Serial.print(wantedGear);
    Serial.println(gear);
  }*/
  return wantedGear;
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
// R/N/P modulation pressure regulation
// idle SPC regulation
// Boost control
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

  if (boostLimit)
  {
    boostControl();
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
     if (debugEnabled)
    {
      Serial.print("polltrans: mpcVal/atfTemp");
      Serial.print(mpcVal);
      Serial.print("-");
      Serial.println(atfTemp);
    }
  };
}

int evaluateGear(float ratio)
{
  int evaluatedGear = 0;
  int n3n2 = n3Speed / n2Speed;
  int incomingShaftSpeed = 0;
  int measuredGear = 0;
  if (n3Speed == 0)
  {
    incomingShaftSpeed = n2Speed * 1.64;
  }
  else
  {
    incomingShaftSpeed = n2Speed;
    //when gear is 2, 3 or 4, n3 speed is not zero, and then incoming shaft speed (=turbine speed) equals to n2 speed)
  }

  if (3.4 < ratio && n3n2 < 0.50)
  {
    measuredGear = 1;
  }
  else if (2.05 < ratio && ratio < 2.20 && n3n2 >= 0.50)
  {
    measuredGear = 2;
  }
  else if (1.38 < ratio && ratio < 1.45 && n3n2 >= 0.50)
  {
    measuredGear = 3;
  }
  else if (0.97 < ratio && ratio < 1.05 && n3n2 >= 0.50)
  {
    measuredGear = 4;
  }
  else if (ratio < 0.90 && n3n2 < 0.50)
  {
    measuredGear = 5;
  }

  if (measuredGear != 0)
  {
    return measuredGear;
  }
}
