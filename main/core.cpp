#include <Arduino.h>
#include "include/pins.h"
#include "include/calc.h"
#include "include/maps.h"
#include "include/sensors.h"
#include "include/input.h"
#include "include/core.h"
#include <SoftTimer.h>

// CORE
// input:pollstick -> core:decideGear -> core:gearChange[Up|Down] -> core:switchGearStart -> core:boostControl
// input:polltrans -> core:switchGearStop
// poll -> evaluateGear

// Obvious internals
byte gear = 2; // Start on gear 2
byte newGear = 2;
byte pendingGear = 2;

// Shift pressure defaults
int spcSetVal = 255;
int spcPercentVal = 100;

// for timers
unsigned long int shiftStartTime = 0;
unsigned long int shiftDuration = 0;

// Solenoid used
int cSolenoidEnabled = 0;
int cSolenoid = 0; // Change solenoid pin to be controlled.
int lastMapVal;

// Gear shift logic
// Beginning of gear change phase
// Send PWM signal to defined solenoid in transmission conductor plate.
void switchGearStart(int cSolenoid, int spcVal, int mpcVal)
{
  shiftStartTime = millis(); // Beginning to count shiftStartTime
  shiftBlocker = true;       // Blocking any other shift operations during the shift

  if (debugEnabled)
  {
    Serial.print(F("switchGearStart: Begin of gear change current/solenoid: "));
    Serial.print(gear);
    Serial.print(F("-"));
    Serial.print(newGear);
    Serial.print(F("-"));
  }

  int spcModVal = adaptSPC(lastMapVal, lastXval, lastYval);

  if (trans)
  {
    // Send PWM signal to SPC(Shift Pressure Control)-solenoid along with MPC(Modulation Pressure Control)-solenoid.
    spcSetVal = (100 - (spcVal + (spcModVal/10))) * 2.55;
    spcPercentVal = spcVal;
    mpcVal = (100 - mpcVal) * 2.55;
    analogWrite(spc, spcSetVal);
    analogWrite(mpc, mpcVal);
    analogWrite(cSolenoid, 255); // Beginning of gear change

    if (debugEnabled)
    {
      Serial.print(F("switchGearStart: spcPressure/mpcPressure: "));
      Serial.print(spcSetVal);
      Serial.print(F("-"));
      Serial.println(mpcVal);
    }
  }
  cSolenoidEnabled = cSolenoid;
}

// End of gear change phase
void switchGearStop()
{
  analogWrite(cSolenoid, 0); // turn shift solenoid off
  analogWrite(spc, 0);       // let go of SPC-pressure
  gear = pendingGear; // we can happily say we're on new gear
  shiftBlocker = false;
  shiftPending = false;

  if (debugEnabled)
  {
    Serial.print(F("switchGearStop: End of gear change current/solenoid: "));
    Serial.print(gear);
    Serial.print(F("-"));
    Serial.print(newGear);
    Serial.print(F("-"));
    Serial.println(cSolenoid);
    
  }
  shiftStartTime = 0;
}

// upshift parameter logic gathering

void gearchangeUp(int newGear)
{
  struct SensorVals sensor = readSensors();
  if (shiftBlocker == false && shiftPending == true)
  {
    pendingGear = newGear;
    if (debugEnabled)
    {
      Serial.print(F("gearChangeUp: performing change from prev->new: "));
      Serial.print(gear);
      Serial.print(F("->"));
      Serial.println(newGear);
    }
  }
  else
  {
    Serial.println(F("gearChangeUp: Blocking change"));
  }

  switch (newGear)
  {
  case 1:
    gear = 1;
    break;
  case 2:
    if (debugEnabled)
    {
      Serial.print(F("gearchangeUp->switchGearStart: Solenoid y3 requested with spcMap12/mpcMap12, load/atfTemp "));
      Serial.print(sensor.curLoad);
      Serial.print(F("-"));
      Serial.println(sensor.curAtfTemp);
    }
    if (!sensors)
    {
      switchGearStart(y3, 100, 100);
    }
    if (sensors)
    {
      lastMapVal = 100;
      switchGearStart(y3, readMap(spcMap12, sensor.curLoad, sensor.curAtfTemp), readMap(mpcMap12, sensor.curLoad, sensor.curAtfTemp));
    }
    break;
  case 3:
    if (debugEnabled)
    {
      Serial.print(F("gearchangeUp->switchGearStart: Solenoid y4 requested with spcMap23/mpcMap23, load/atfTemp "));
      Serial.print(sensor.curLoad);
      Serial.print(F("-"));
      Serial.println(sensor.curAtfTemp);
    }
    if (!sensors)
    {
      switchGearStart(y4, 100, 100);
    }
    if (sensors)
    {
      lastMapVal = 130;
      switchGearStart(y4, readMap(spcMap23, sensor.curLoad, sensor.curAtfTemp), readMap(mpcMap23, sensor.curLoad, sensor.curAtfTemp));
    }
    break;
  case 4:
    if (debugEnabled)
    {
      Serial.print(F("gearchangeUp->switchGearStart: Solenoid y5 requested with spcMap34/mpcMap34, load/atfTemp "));
      Serial.print(sensor.curLoad);
      Serial.print(F("-"));
      Serial.println(sensor.curAtfTemp);
    }
    if (!sensors)
    {
      switchGearStart(y5, 100, 100);
    }
    if (sensors)
    {
      lastMapVal = 160;
      switchGearStart(y5, readMap(spcMap34, sensor.curLoad, sensor.curAtfTemp), readMap(mpcMap34, sensor.curLoad, sensor.curAtfTemp));
    }
    break;
  case 5:
    if (debugEnabled)
    {
      Serial.print(F("gearchangeUp->switchGearStart: Solenoid y3 requested with spcMap45/mpcMap45, load/atfTemp "));
      Serial.print(sensor.curLoad);
      Serial.print(F("-"));
      Serial.println(sensor.curAtfTemp);
    }
    if (!sensors)
    {
      switchGearStart(y3, 100, 100);
    }
    if (sensors)
    {
      lastMapVal = 190;
      switchGearStart(y3, readMap(spcMap45, sensor.curLoad, sensor.curAtfTemp), readMap(mpcMap45, sensor.curLoad, sensor.curAtfTemp));
    }
    break;
  default:
    break;
  }
}

// downshift parameter logic gathering
void gearchangeDown(int newGear)
{
  struct SensorVals sensor = readSensors();
  if (shiftBlocker == false && shiftPending == true)
  {
    pendingGear = newGear;
    if (debugEnabled)
    {
      Serial.print(F("gearChangeDown: performing change from prev->new: "));
      Serial.print(gear);
      Serial.print(F("->"));
      Serial.println(newGear);
    }
  }
  else
  {
    Serial.println(F("gearChangeDown: Blocking change"));
  }

  switch (newGear)
  {
  case 1:
    if (debugEnabled)
    {
      Serial.print(F("gearchangeDown->switchGearStart: Solenoid y3 requested with spcMap21/mpcMap21, load/atfTemp "));
      Serial.print(sensor.curLoad);
      Serial.print(F("-"));
      Serial.println(sensor.curAtfTemp);
    }
    if (!sensors)
    {
      switchGearStart(y3, 100, 100);
    }
    if (sensors)
    {
      lastMapVal = 210;
      switchGearStart(y3, readMap(spcMap21, sensor.curLoad, sensor.curAtfTemp), readMap(mpcMap21, sensor.curLoad, sensor.curAtfTemp));
    }
    break;
  case 2:
    if (debugEnabled)
    {
      Serial.print(F("gearchangeDown->switchGearStart: Solenoid y4 requested with spcMap32/mpcMap32, load/atfTemp "));
      Serial.print(sensor.curLoad);
      Serial.print(F("-"));
      Serial.println(sensor.curAtfTemp);
    }
    if (!sensors)
    {
      switchGearStart(y4, 100, 100);
    }
    if (sensors)
    {
      lastMapVal = 240;
      switchGearStart(y4, readMap(spcMap32, sensor.curLoad, sensor.curAtfTemp), readMap(mpcMap32, sensor.curLoad, sensor.curAtfTemp));
    }
    break;
  case 3:
    if (debugEnabled)
    {
      Serial.print(F("gearchangeDown->switchGearStart: Solenoid y5 requested with spcMap43/mpcMap43, load/atfTemp "));
      Serial.print(sensor.curLoad);
      Serial.print(F("-"));
      Serial.println(sensor.curAtfTemp);
    }
    if (!sensors)
    {
      switchGearStart(y5, 100, 100);
    }
    if (sensors)
    {
      lastMapVal = 270;
      switchGearStart(y5, readMap(spcMap43, sensor.curLoad, sensor.curAtfTemp), readMap(mpcMap43, sensor.curLoad, sensor.curAtfTemp));
    }
    break;
  case 4:
    if (debugEnabled)
    {
      Serial.print(F("gearchangeDown->switchGearStart: Solenoid y3 requested with spcMap54/mpcMap54, load/atfTemp "));
      Serial.print(sensor.curLoad);
      Serial.print(F("-"));
      Serial.println(sensor.curAtfTemp);
    }
    if (!sensors)
    {
      switchGearStart(y3, 100, 100);
    }
    if (sensors)
    {
      lastMapVal = 300;
      switchGearStart(y3, readMap(spcMap54, sensor.curLoad, sensor.curAtfTemp), readMap(mpcMap54, sensor.curLoad, sensor.curAtfTemp));
    }
    break;
  case 5:
    gear = 5;
    break;
  default:
    break;
  }
}

// Logic for automatic new gear, this makes possible auto up/downshifts.
void decideGear(Task *me)
{
  int moreGear = gear + 1;
  int lessGear = gear - 1;
  struct SensorVals sensor = readSensors();

  // Determine speed related downshift and upshift here.
  int autoGear = readMap(gearMap, sensor.curTps, sensor.curSpeed);

  if (!shiftBlocker && !shiftPending && wantedGear < 6)
  {
    if (autoGear > gear && wantedGear > gear)
    {
      int newGear = moreGear;
      if (debugEnabled)
      {
        Serial.println(F(""));
        Serial.print(F("decideGear->gearchangeUp: tpsPercent/vehicleSpeed: "));
        Serial.print(sensor.curTps);
        Serial.print(F("-"));
        Serial.println(sensor.curSpeed);
      }
      if (debugEnabled)
      {
        Serial.print(F("decideGear->gearchangeUp: wantedGear/autoGear/newGear/gear: "));
        Serial.print(wantedGear);
        Serial.print(F("-"));
        Serial.print(autoGear);
        Serial.print(F("-"));
        Serial.print(newGear);
        Serial.print(F("-"));
        Serial.println(gear);
      }
      shiftPending = true;
      gearchangeUp(newGear);
    }
    if (autoGear < gear || wantedGear < gear)
    {
      int newGear = lessGear;
      if (debugEnabled)
      {
        Serial.println(F(""));
        Serial.print(F("decideGear->gearchangeDown: tpsPercent/vehicleSpeed: "));
        Serial.print(sensor.curTps);
        Serial.print(F("-"));
        Serial.println(sensor.curSpeed);
      }
      if (debugEnabled)
      {
        Serial.print(F("decideGear->gearchangeDown: wantedGear/autoGear/newGear/gear: "));
        Serial.print(wantedGear);
        Serial.print(F("-"));
        Serial.print(autoGear);
        Serial.print(F("-"));
        Serial.print(newGear);
        Serial.print(F("-"));
        Serial.println(gear);
      }
      shiftPending = true;
      gearchangeDown(newGear);
    }
  }
}
// END OF CORE
