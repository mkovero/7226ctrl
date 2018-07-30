#include <Arduino.h>
#include "include/pins.h"
#include "include/calc.h"
#include "include/core.h"
#include "include/sensors.h"
#include "include/maps.h"
#include "include/eeprom.h"
#include "include/input.h"
#include "include/config.h"
#include <SoftTimer.h>

byte wantedGear = 100;

// INPUT
// Polling for stick control
// This is W202 electronic gear stick, should work on any pre-canbus sticks.
void pollstick(Task *me)
{
  // Read the stick.
  int whiteState = digitalRead(whitepin);
  int blueState = digitalRead(bluepin);
  int greenState = digitalRead(greenpin);
  int yellowState = digitalRead(yellowpin);
  int autoState = digitalRead(autoSwitch);

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

  if (autoState == HIGH)
  {
    if (!fullAuto)
    {
      if (debugEnabled)
      {
        Serial.println(F("pollstick: Automode on "));
      }
      fullAuto = true;
    }
  }
  else
  {
    if (fullAuto)
    {
      if (debugEnabled)
      {
        Serial.println(F("pollstick: Automode off "));
      }
      fullAuto = false;
    }
  }
}

// For manual microswitch control, gear up
void gearup()
{
  if (wantedGear < 6 && !fullAuto)
  { // Do nothing if we're on N/R/P
    if (!shiftBlocker && !shiftPending && gear <= 5)
    {
      newGear++;
    }

    if (debugEnabled)
    {
      Serial.println(F("gearup: Gear up requested"));
    }
    shiftPending = true;
    gearchangeUp(newGear);
  }
}

// For manual microswitch control, gear down
void geardown()
{
  if (wantedGear < 6 && !fullAuto)
  { // Do nothing if we're on N/R/P
    if (!shiftBlocker && !shiftPending && gear <= 5)
    {
      newGear--;
    }

    if (debugEnabled)
    {
      Serial.println(F("gearup: Gear down requested"));
    }
    shiftPending = true;
    gearchangeDown(newGear);
  }
}

// Polling for manual switch keys
void pollkeys()
{
  int gupState = digitalRead(gupSwitch);     // Gear up
  int gdownState = digitalRead(gdownSwitch); // Gear down
  static int prevgdownState = 0;
  static int prevgupState = 0;

  if (gdownState != prevgdownState || gupState != prevgupState)
  {
    if (gdownState == LOW && gupState == HIGH)
    {
      prevgupState = gupState;
      if (debugEnabled)
      {
        Serial.println(F("pollkeys: Gear up button"));
      }
      gearup();
    }
    else if (gupState == LOW && gdownState == HIGH)
    {
     prevgdownState = gdownState;
      if (debugEnabled)
      {
        Serial.println(F("pollkeys: Gear down button"));
      }
      geardown();
    }
  }
}

void boostControl(Task *me)
{
  if (boostLimit)
  {
    struct SensorVals sensor = readSensors();
    struct ConfigParam config = readConfig();

    if (shiftBlocker)
    {
      // During the shift
      if (sensor.curBoost > (sensor.curBoostLim - config.boostDrop))
      {
        analogWrite(boostCtrl, 245);
      }
      else
      {
        analogWrite(boostCtrl, 255);
      }
    }
    else
    {
      // Not during the shift
      if (sensor.curBoost > sensor.curBoostLim)
      {
        analogWrite(boostCtrl, 0);
      }
      else if (sensor.curSpeed < 10)
      {
        analogWrite(boostCtrl, 0);
      }
      else
      {
        analogWrite(boostCtrl, 255);
      }
    }

    /*if (debugEnabled)
    {
      Serial.print(F("boostControl (allowedBoostPressure/bootSensor):"));
      Serial.print(sensor.curBoostLim);
      Serial.print(F("-"));
      Serial.print(sensor.curBoost);
    }*/
  }
}

void fuelControl(Task *me)
{
  if (fuelPumpControl)
  {
    struct SensorVals sensor = readSensors();
    struct ConfigParam config = readConfig();

    if (sensor.curRPM > config.fuelMaxRPM || millis() < 5000)
    {
      analogWrite(fuelPumpCtrl, 255);

      if (debugEnabled && !fuelPumps)
      {
        Serial.print(F("[fuelControl->fuelControl] Fuel Pump RPM limit hit/Prestart init, enabling pumps: "));
        Serial.println(config.fuelMaxRPM);
      }
      fuelPumps = true;
    }
  }
}

// Polling time for transmission control
// R/N/P modulation pressure regulation
// idle SPC regulation
// Boost control
void polltrans(Task *me)
{
  struct SensorVals sensor = readSensors();

  unsigned int shiftDelay = readMap(shiftTimeMap, spcPercentVal, sensor.curAtfTemp);
  if (shiftBlocker)
  {
    shiftDuration = millis() - shiftStartTime;
    if (shiftDuration > shiftDelay)
    {
      if (debugEnabled)
      {
        Serial.print(F("[polltrans->switchGearStop] shiftDelay-spcPercentVal-atfTemp "));
        Serial.print(shiftDelay);
        Serial.print(F("-"));
        Serial.print(spcPercentVal);
        Serial.print(F("-"));
        Serial.println(atfRead());
      }
      switchGearStop();
    }
  }

  //Raw value for pwm control (0-255) for SPC solenoid, see page 9: http://www.all-trans.by/assets/site/files/mercedes/722.6.1.pdf
  // "Pulsed constantly while idling in Park or Neutral at approximately 40% Duty cycle" <- 102/255 = 0.4
  // MPC = varying with load, SPC constant 33%
  int mpcVal = readMap(mpcNormalMap, sensor.curLoad, sensor.curAtfTemp);

  if (!shiftBlocker)
  {
    // Pulsed constantly while idling in Park or Neutral at approximately 33% Duty cycle.
    if (wantedGear == 6 || wantedGear == 8)
    {
      analogWrite(spc, 85);
    }
    // Pulsed constantly while idling in Park or Neutral at approximately 40% Duty cycle, also for normal mpc operation
    if (wantedGear <= 6 || wantedGear == 8)
    {
      int mpcSetVal = (100 - mpcVal) * 2.55;
      analogWrite(mpc, mpcSetVal);
    }
    // 3-4 Shift solenoid is pulsed continuously while in Park and during selector lever movement (Garage Shifts).
    if (wantedGear > 5)
    {
      analogWrite(y5, 255);
    }
    else
    {
      // "1-2/4-5 Solenoid is pulsed during ignition crank." stop doing this after we get ourselves together.
      analogWrite(y5, 0);
    }
    if (ignition)
    {
      analogWrite(y3, 0);
      ignition = false;
    }
  }
}

int adaptSPC(int mapId, int xVal, int yVal)
{
  int modVal = 5;
  int aSpcUpState = digitalRead(aSpcUpSwitch);     // Adapt pressure up
  int aSpcDownState = digitalRead(aSpcDownSwitch); // Adapt pressure down
  static int prevaSpcUpState = 0;
  static int prevaSpcDownState = 0;
  int current = readEEPROM(mapId, xVal, yVal);

  if (aSpcDownState != prevaSpcDownState || aSpcUpState != prevaSpcUpState)
  {
    if (aSpcDownState == LOW && aSpcUpState == HIGH)
    {
      prevaSpcUpState = aSpcUpState;

      if (debugEnabled)
      {
        Serial.println(F("adaptSPC: More pressure button"));
        Serial.print(F("adaptSPC: request values: "));
        Serial.print(mapId);
        Serial.print(F("-"));
        Serial.print(xVal);
        Serial.print(F("-"));
        Serial.println(yVal);
        Serial.print(F("adaptSPC: old adapt pressure is: "));
        Serial.println(current);
      }
      current = current + modVal;
      writeEEPROM(mapId, xVal, yVal, current);

      if (debugEnabled)
      {
        Serial.print(F("adaptSPC: New adapt pressure is: "));
        Serial.println(current);
      }
    }
    else if (aSpcUpState == LOW && aSpcDownState == HIGH)
    {
      prevaSpcDownState = aSpcDownState;

      if (debugEnabled)
      {
        Serial.println(F("adaptSPC: Less pressure button"));
        Serial.print(F("adaptSPC: request values: "));
        Serial.print(mapId);
        Serial.print(F("-"));
        Serial.print(xVal);
        Serial.print(F("-"));
        Serial.println(yVal);
        Serial.print(F("adaptSPC: old adapt pressure is: "));
        Serial.println(current);
      }
      current = current - modVal;
      writeEEPROM(mapId, xVal, yVal, current);

      if (debugEnabled)
      {
        Serial.print(F("adaptSPC: New adapt pressure is: "));
        Serial.println(current);
      }
    }
  }

  return current;
}
