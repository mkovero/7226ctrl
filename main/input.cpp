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
#include <AutoPID.h>

byte wantedGear = 100;

// INPUT

// Pid tuning parameters
const double Kp = 7; //80,21 Pid Proporional Gain. Initial ramp up i.e Spool, Lower if over boost
double Ki = 20;      //40,7 Pid Integral Gain. Overall change while near Target Boost, higher value means less change, possible boost spikes
const double Kd = 0; //100, 1 Pid Derivative Gain.
boolean garageShift = false;
double garageTime, lastShift, lastInput;

/*
const double Kp = 200; 
double Ki = 100;      
const double Kd = 25; //This is not necessarily good idea.
*/
double pidBoost, boostPWM, pidBoostLim, hornPressTime;

//Load PID controller
AutoPID myPID(&pidBoost, &pidBoostLim, &boostPWM, 0, 255, Kp, Ki, Kd);

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
    gear = 2; // force reset gear to 2
    shiftPending = false;
    shiftBlocker = false;
  } // P
  if (whiteState == LOW && blueState == HIGH && greenState == HIGH && yellowState == HIGH)
  {
    wantedGear = 7;
    gear = 2; // force reset gear to 2
  }           // R
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
    if (!stickCtrl)
    {
      if (debugEnabled)
      {
        Serial.println(F("pollstick: Stick control on "));
      }
      stickCtrl = true;
    }
  }
  else
  {
    if (stickCtrl)
    {
      if (debugEnabled)
      {
        Serial.println(F("pollstick: Stick control off "));
      }
      stickCtrl = false;
    }
  }
}

// For manual microswitch control, gear up
void gearUp()
{
  if (wantedGear < 6 && !fullAuto && !stickCtrl && gear < 5)
  { // Do nothing if we're on N/R/P
    if (!shiftBlocker)
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
void gearDown()
{
  if (wantedGear < 6 && !fullAuto && !stickCtrl && gear > 1)
  { // Do nothing if we're on N/R/P
    if (!shiftBlocker)
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
      gearUp();
    }
    else if (gupState == LOW && gdownState == HIGH)
    {
      prevgdownState = gdownState;
      if (debugEnabled)
      {
        Serial.println(F("pollkeys: Gear down button"));
      }
      gearDown();
    }
  }
}

void hornOn()
{
  // Simple horn control
  digitalWrite(hornPin, HIGH);
  horn = true;
  hornPressTime = millis();
  if (debugEnabled)
  {
    Serial.println("Horn pressed");
  }
}

void hornOff()
{
  digitalWrite(hornPin, LOW);
  horn = false;
  if (debugEnabled)
  {
    Serial.println("Horn depressed");
  }
}

void boostControl(Task *me)
{
  if (boostLimit)
  {
    struct SensorVals sensor = readSensors();
    struct ConfigParam config = readConfig();
    pidBoost = sensor.curBoost;
    pidBoostLim = sensor.curBoostLim;
    myPID.setBangBang(100, 50);
    myPID.setTimeStep(100);

    if (shiftBlocker && !slipFault)
    {
      // During the shift
      if (preShift && sensor.curBoostLim > config.boostDrop)
      {
        pidBoostLim = sensor.curBoostLim - config.boostDrop;
      }
      else
      {
        pidBoostLim = 0;
      }
      Ki = 5; // New integral gain value; we want change of pressure to be aggressive here.
    }
    else
    {
      pidBoostLim = sensor.curBoostLim;
      Ki = 20; // New integral gain value; we want change of pressure to be more modest.
    }

    // Just a sanity check to make sure PID library is not doing anything stupid.
    if (sensor.curBoostLim > 0 && !slipFault && truePower)
    {
      myPID.run();
      //   analogWrite(boostCtrl, boostPWM);
      //  if (debugEnabled) { Serial.print("BoostPWM = "); Serial.println(boostPWM); }
    }
    else
    {
      analogWrite(boostCtrl, 0);
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

    if ((sensor.curRPM > config.fuelMaxRPM || millis() < 5000) && !fuelPumps)
    {
      analogWrite(fuelPumpCtrl, 255);

      if (debugEnabled)
      {
        Serial.print(F("[fuelControl->fuelControl] Fuel Pump RPM limit hit/Prestart init, enabling pumps: "));
        Serial.println(config.fuelMaxRPM);
      }
      fuelPumps = true;
    }
    else if (sensor.curRPM < config.fuelMaxRPM && fuelPumps && millis() > 5000)
    {
      analogWrite(fuelPumpCtrl, 0);
      if (debugEnabled)
      {
        Serial.print(F("[fuelControl->fuelControl] Fuel Pump RPM disabled due low rpm/timelimit "));
      }
      fuelPumps = false;
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
    if (shiftDuration > shiftDelay && shiftDone)
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
    if (preShift && !preShiftDone)
    {
      doPreShift();
    }
    else if (!preShift && preShiftDone)
    {
      doShift();
    }
    else if (postShift && !postShiftDone)
    {
      doPostShift();
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
      garageShift = true;
      garageTime = millis();
    }
    // Pulsed constantly while idling in Park or Neutral at approximately 40% Duty cycle, also for normal mpc operation
    if (wantedGear <= 6 || wantedGear == 8)
    {
      int mpcSetVal = (100 - mpcVal) * 2.55;
      // analogWrite(mpc, mpcSetVal);
    }

    if ((wantedGear == 7 || (wantedGear < 6 && !shiftPending)) && garageShift && (millis() - garageTime > 1000))
    {
      analogWrite(spc, 0);
      garageShift = false;
    }

    // 3-4 Shift solenoid is pulsed continuously while in Park and during selector lever movement (Garage Shifts).
    if (wantedGear > 5)
    {
      analogWrite(y5, 255);
    }
    else if (!shiftBlocker && wantedGear < 6)
    {
      analogWrite(y5, 0);
    }

    if (sensor.curTps < 40 && sensor.curSpeed > 80 && gear == 5)
    {
      analogWrite(tcc, 255);
    }
    else
    {
      analogWrite(tcc, 0);
    }

    // "1-2/4-5 Solenoid is pulsed during ignition crank." stop doing this after we get ourselves together.
    if (ignition)
    {
      analogWrite(y3, 0);
      ignition = false;
    }
    /*
    if (evaluateGear() < 6 && wantedGear < 6)
    {
      gear = evaluateGear();
    }*/
  }

  if (radioEnabled)
  {
    radioControl();
  }
  if (manual)
  {
    pollkeys();
  }
  if (horn && (millis() - hornPressTime > 300))
  {
    hornOff();
  }
}

int adaptSPC(int mapId, int xVal, int yVal)
{
  int current = 0;
#ifdef ASPC
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
#endif
  return current;
}

void radioControl()
{
  static byte readData;
  static byte pwrCounter = 1;

  if (Serial1.available() > 0)
  {
    readData = Serial1.read();
    
    if (readData == 100 && !shiftPending && gear < 5)
    {
      lastShift = millis();
      shiftPending = true;
      gearUp();
      readData = 0;
    }
    else if (readData == 100 && shiftPending)
    {
      readData = 0;
      Serial.println("Steering: shift pending");
    }
    else if (readData == 200 && !shiftPending && gear > 1)
    {
      lastShift = millis();
      shiftPending = true;
      gearDown();
      readData = 0;
    }
    else if (readData == 200 && shiftPending)
    {
      readData = 0;
      Serial.println("Steering: shift pending");
    }
    else if (readData == 55)
    {
      hornOn();
      readData = 0;
    }
    else if (readData == 150)
    {
      if (page < 3)
      {
        page++;
      }
      else if (page > 3)
      {
        page = 1;
      }
      readData = 0;
    }
    else if (readData == 151)
    {
      if (page > 1)
      {
        page--;
      }
      else if (page < 1)
      {
        page = 3;
      }
      readData = 0;
    }
    else if (readData == 249)
    {
      if (millis() - lastInput > 1000) {
      pwrCounter = pwrCounter+1;
      lastInput = millis();
      }  
      
      if (truePower && pwrCounter > 5)
      {
        truePower = false;
        pwrCounter = 1;
      }
      else if (!truePower && pwrCounter > 5)
      {
        truePower = true;
        pwrCounter = 1;
      }

    }
  }
}
