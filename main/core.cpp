#include <Arduino.h>
#include "include/config.h"
#include "include/pins.h"
#include "include/calc.h"
#include "include/maps.h"
#include "include/sensors.h"
#include "include/input.h"

// CORE
// input:pollstick -> core:decideGear -> core:gearChange[Up|Down] -> core:switchGearStart -> core:boostControl
// input:polltrans -> core:switchGearStop
// poll -> evaluateGear

// Obvious internals
int gear = 2; // Start on gear 2
int vehicleSpeed = 100;
int fuelMaxRPM = 2000; // if fuelCtrl is enabled, this is the limit to turn off pumps.

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
  int boostControlVal;
  int boostSensorVal = boostRead() / maxBoostPressure * 255;
  shiftStartTime = millis(); // Beginning to count shiftStartTime
  shiftBlocker = true;       // Blocking any other shift operations during the shift
  if (debugEnabled)
  {
    Serial.print("switchGearStart: Begin of gear change current/solenoid: ");
    Serial.print(gear);
    Serial.print("-");
    Serial.println(cSolenoid);
  }

  int spcModVal = adaptSPC(lastMapVal, lastXval, lastYval);

  if (trans)
  {
    if (boostLimit)
    {
      pollBoostControl();
    }
    // Send PWM signal to SPC(Shift Pressure Control)-solenoid along with MPC(Modulation Pressure Control)-solenoid.
    spcSetVal = (100 - (spcVal + spcModVal)) * 2.55;
    spcPercentVal = spcVal;
    mpcVal = (100 - mpcVal) * 2.55;
    analogWrite(spc, spcSetVal);
    analogWrite(mpc, mpcVal);
    analogWrite(cSolenoid, 255); // Beginning of gear change
    if (debugEnabled)
    {
      Serial.print("switchGearStart: spcPressure/mpcPressure/boostControlVal: ");
      Serial.print(spcSetVal);
      Serial.print("-");
      Serial.println(mpcVal);
    }
  }
  cSolenoidEnabled = cSolenoid;
}

// End of gear change phase
void switchGearStop(int cSolenoid, int newGear)
{
  analogWrite(cSolenoid, 0); // turn shift solenoid off
  analogWrite(spc, 0);       // let go of SPC-pressure
  shiftBlocker = false;
  gear = newGear; // we can happily say we're on new gear
  if (debugEnabled)
  {
    Serial.print("switchGearStop: End of gear change current/solenoid: ");
    Serial.print(gear);
    Serial.print("-");
    Serial.println(cSolenoid);
  }
  shiftStartTime = 0;
}

// upshift parameter logic gathering
void gearchangeUp(int newGear)
{
  if (shiftBlocker == false)
  {
    if (debugEnabled)
    {
      Serial.print("gearChangeUp: performing change from prev->new: ");
      Serial.print(gear);
      Serial.print("->");
      Serial.println(newGear);
    }
  }
  else
  {
    Serial.println("gearChangeUp: Blocking change");
  }
  int atfTemp = atfRead();
  int trueLoad = loadRead();

  switch (newGear)
  {
  case 1:
    gear = 1;
    break;
  case 2:
    if (debugEnabled)
    {
      Serial.print("gearchangeUp->switchGearStart: Solenoid y3 requested with spcMap12/mpcMap12, load/atfTemp ");
      Serial.print(trueLoad);
      Serial.print("-");
      Serial.println(atfTemp);
    }
    if (!sensors)
    {
      switchGearStart(y3, 100, 100);
    }
    if (sensors)
    {
      lastMapVal = 100;
      switchGearStart(y3, readMap(spcMap12, trueLoad, atfTemp), readMap(mpcMap12, trueLoad, atfTemp));
    }
    break;
  case 3:
    if (debugEnabled)
    {
      Serial.print("gearchangeUp->switchGearStart: Solenoid y4 requested with spcMap23/mpcMap23, load/atfTemp ");
      Serial.print(trueLoad);
      Serial.print("-");
      Serial.println(atfTemp);
    }
    if (!sensors)
    {
      switchGearStart(y4, 100, 100);
    }
    if (sensors)
    {
      lastMapVal = 130;
      switchGearStart(y4, readMap(spcMap23, trueLoad, atfTemp), readMap(mpcMap23, trueLoad, atfTemp));
    }
    break;
  case 4:
    if (debugEnabled)
    {
      Serial.print("gearchangeUp->switchGearStart: Solenoid y5 requested with spcMap34/mpcMap34, load/atfTemp ");
      Serial.print(trueLoad);
      Serial.print("-");
      Serial.println(atfTemp);
    }
    if (!sensors)
    {
      switchGearStart(y5, 100, 100);
    }
    if (sensors)
    {
      lastMapVal = 160;
      switchGearStart(y5, readMap(spcMap34, trueLoad, atfTemp), readMap(mpcMap34, trueLoad, atfTemp));
    }
    break;
  case 5:
    if (debugEnabled)
    {
      Serial.print("gearchangeUp->switchGearStart: Solenoid y3 requested with spcMap45/mpcMap45, load/atfTemp ");
      Serial.print(trueLoad);
      Serial.print("-");
      Serial.println(atfTemp);
    }
    if (!sensors)
    {
      switchGearStart(y3, 100, 100);
    }
    if (sensors)
    {
      lastMapVal = 190;
      switchGearStart(y3, readMap(spcMap45, trueLoad, atfTemp), readMap(mpcMap45, trueLoad, atfTemp));
    }
    break;
  default:
    break;
  }
}

// downshift parameter logic gathering
void gearchangeDown(int newGear)
{
  if (shiftBlocker == false)
  {
    if (debugEnabled)
    {
      Serial.print("gearChangeDown: performing change from prev->new: ");
      Serial.print(gear);
      Serial.print("->");
      Serial.println(newGear);
    }
  }
  else
  {
    Serial.println("gearChangeDown: Blocking change");
  }
  int atfTemp = atfRead();
  int trueLoad = loadRead();
  switch (newGear)
  {
  case 1:
    if (debugEnabled)
    {
      Serial.print("gearchangeDown->switchGearStart: Solenoid y3 requested with spcMap21/mpcMap21, load/atfTemp ");
      Serial.print(trueLoad);
      Serial.print("-");
      Serial.println(atfTemp);
    }
    if (!sensors)
    {
      switchGearStart(y3, 100, 100);
    }
    if (sensors)
    {
      lastMapVal = 210;
      switchGearStart(y3, readMap(spcMap21, trueLoad, atfTemp), readMap(mpcMap21, trueLoad, atfTemp));
    }
    break;
  case 2:
    if (debugEnabled)
    {
      Serial.print("gearchangeDown->switchGearStart: Solenoid y4 requested with spcMap32/mpcMap32, load/atfTemp ");
      Serial.print(trueLoad);
      Serial.print("-");
      Serial.println(atfTemp);
    }
    if (!sensors)
    {
      switchGearStart(y4, 100, 100);
    }
    if (sensors)
    {
      lastMapVal = 240;
      switchGearStart(y4, readMap(spcMap32, trueLoad, atfTemp), readMap(mpcMap32, trueLoad, atfTemp));
    }
    break;
  case 3:
    if (debugEnabled)
    {
      Serial.print("gearchangeDown->switchGearStart: Solenoid y5 requested with spcMap43/mpcMap43, load/atfTemp ");
      Serial.print(trueLoad);
      Serial.print("-");
      Serial.println(atfTemp);
    }
    if (!sensors)
    {
      switchGearStart(y5, 100, 100);
    }
    if (sensors)
    {
      lastMapVal = 270;
      switchGearStart(y5, readMap(spcMap43, trueLoad, atfTemp), readMap(mpcMap43, trueLoad, atfTemp));
    }
    break;
  case 4:
    if (debugEnabled)
    {
      Serial.print("gearchangeDown->switchGearStart: Solenoid y3 requested with spcMap54/mpcMap54, load/atfTemp ");
      Serial.print(trueLoad);
      Serial.print("-");
      Serial.println(atfTemp);
    }
    if (!sensors)
    {
      switchGearStart(y3, 100, 100);
    }
    if (sensors)
    {
      lastMapVal = 300;
      switchGearStart(y3, readMap(spcMap54, trueLoad, atfTemp), readMap(mpcMap54, trueLoad, atfTemp));
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
int decideGear(int wantedGear)
{

  int moreGear = gear + 1;
  int lessGear = gear - 1;
  int atfTemp = atfRead();
  int tpsPercentValue = tpsRead();
  // Determine speed related downshift and upshift here.
  int autoGear = readMap(gearMap, tpsPercentValue, vehicleSpeed);

  if (!shiftBlocker && wantedGear < 6)
  {
    if (autoGear > gear && wantedGear > gear)
    {
      int newGear = moreGear;
      if (debugEnabled)
      {
        Serial.println("");
        Serial.print("decideGear->gearchangeUp: tpsPercent/vehicleSpeed/atfTemp: ");
        Serial.print(tpsPercentValue);
        Serial.print("-");
        Serial.print(vehicleSpeed);
        Serial.print("-");
        Serial.println(atfTemp);
      }
      if (debugEnabled)
      {
        Serial.print("decideGear->gearchangeUp: wantedGear/autoGear/newGear/gear: ");
        Serial.print(wantedGear);
        Serial.print("-");
        Serial.print(autoGear);
        Serial.print("-");
        Serial.print(newGear);
        Serial.print("-");
        Serial.println(gear);
      }
      gearchangeUp(newGear);
      return newGear;
    }
    if (autoGear < gear || wantedGear < gear)
    {
      int newGear = lessGear;
      if (debugEnabled)
      {
        Serial.println("");
        Serial.print("decideGear->gearchangeDown: tpsPercent/vehicleSpeed/atfTemp: ");
        Serial.print(tpsPercentValue);
        Serial.print("-");
        Serial.print(vehicleSpeed);
        Serial.print("-");
        Serial.println(atfTemp);
      }
      if (debugEnabled)
      {
        Serial.print("decideGear->gearchangeDown: wantedGear/autoGear/newGear/gear: ");
        Serial.print(wantedGear);
        Serial.print("-");
        Serial.print(autoGear);
        Serial.print("-");
        Serial.print(newGear);
        Serial.print("-");
        Serial.println(gear);
      }
      gearchangeDown(newGear);
      return newGear;
    }
  }
}

void fuelCtrl() {
  curRPM = readRPM();

  if ( curRPM > fuelMaxRPM) {
    analogWrite(fuelPumpCtrl, 0);
    if ( debugEnabled ) { Serial.print("Fuel Pump RPM limit hit: "); Serial.println(fuelMaxRPM); }
  }
}

// END OF CORE
