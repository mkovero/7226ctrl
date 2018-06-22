#include <Arduino.h>
#include "include/pins.h"
#include "include/calc.h"
#include "include/maps.h"
#include "include/sensors.h"
#include "include/ui.h"
#include "include/config.h"
#include "include/core.h"
#include <SoftTimer.h>
using namespace std;

// Internals
unsigned long n2SpeedPulses, n3SpeedPulses, vehicleSpeedPulses, lastSensorTime, rpmPulse, curLog, lastLog;
int n2Speed, n3Speed, rpmRevs, vehicleSpeedRevs;

// sensor smoothing
int avgAtfTemp, avgBoostValue, avgVehicleSpeedDiff, avgVehicleSpeedRPM, avgRpmValue, oldRpmValue, avgTemp;
float alpha = 0.7;

// Interrupt for N2 hallmode sensor
void N2SpeedInterrupt()
{
  n2SpeedPulses++;
}
// Interrupt for N3 hallmode sensor
void N3SpeedInterrupt()
{
  n3SpeedPulses++;
}
void vehicleSpeedInterrupt()
{
  vehicleSpeedPulses++;
}
// RPM Interrupt
void rpmInterrupt()
{
  rpmPulse++;
}

// Polling sensors
void pollsensors(Task *me)
{

  const int n2PulsesPerRev = 60;
  const int n3PulsesPerRev = 60;
  const int vehicleSpeedPulsesPerRev = 29; // number of teeths in w124 rear diff

  if (millis() - lastSensorTime >= 1000)
  {
    detachInterrupt(2); // Detach interrupts for calculation
    detachInterrupt(3);
    detachInterrupt(4);
    float elapsedTime = millis() - lastSensorTime; // need to have this float in order to get float calculation.

    if (n2SpeedPulses >= 60)
    {
      n2Speed = n2SpeedPulses / elapsedTime * 1000; // there are 60 pulses in one rev and 60 seconds in minute, so this is quite simple
      n2SpeedPulses = 0;
    }
    else
    {
      n2Speed = 0;
    }

    if (n3SpeedPulses >= 60)
    {
      n3Speed = n3SpeedPulses / elapsedTime * 1000;
      n3SpeedPulses = 0;
    }
    else
    {
      n3Speed = 0;
    }

    if (vehicleSpeedPulses >= vehicleSpeedPulsesPerRev)
    {
      vehicleSpeedRevs = vehicleSpeedPulses * 60 / vehicleSpeedPulsesPerRev / elapsedTime * 1000;
      vehicleSpeedPulses = 0;
    }

    // RPM as per elapsedTime
    rpmRevs = rpmPulse / elapsedTime * 1000 * 60;
    rpmPulse = 0;
    /*  Serial.print(n2Speed);
    Serial.print("-");
    Serial.print(n3Speed);
    int evalgear = evaluateGear();
    Serial.print("-");
    Serial.println(evalgear);*/
    lastSensorTime = millis();

    attachInterrupt(digitalPinToInterrupt(n2pin), N2SpeedInterrupt, RISING); // Attach again
    attachInterrupt(digitalPinToInterrupt(n3pin), N3SpeedInterrupt, RISING);
    attachInterrupt(digitalPinToInterrupt(speedPin), vehicleSpeedInterrupt, RISING);
    attachInterrupt(digitalPinToInterrupt(rpmPin), rpmInterrupt, RISING);
  }
}

int speedRead()
{
  struct ConfigParam config = readConfig();
  int curRPM = rpmRead();
  int vehicleSpeed, vehicleSpeedRPM, vehicleSpeedDiff, speedValue;

  // int vehicleSpeed = 0.03654 * vehicleSpeedRevs; // 225/45/17 with 3.27 rear diff
  float tireDiameter = (config.tireWidth * config.tireProfile / 2540 * 2 + config.tireInches) * 25.4;
  float tireCircumference = 3.14 * tireDiameter;
  if (rpmSpeed)
  {
    // speed based on engine rpm
    vehicleSpeedRPM = tireCircumference * curRPM / (ratioFromGear(gear) * config.diffRatio) / 1000000 * 60;
    speedValue = vehicleSpeedRPM;
  }
  if (diffSpeed)
  {
    // speed based on diff abs sensor
    vehicleSpeedDiff = tireCircumference * vehicleSpeedRevs / config.diffRatio / 1000000 * 60;
    speedValue = vehicleSpeedDiff;
  }
  if (rpmSpeed && diffSpeed)
  {
    if (vehicleSpeedRPM / vehicleSpeedDiff > 1.3 || vehicleSpeedRPM / vehicleSpeedDiff < 0.7)
    {
      if (!speedFault)
      {
        speedFault = true; // if both sensors are enabled and difference is too great, then create a fault.
        if (debugEnabled)
        {
          Serial.print(F("SPEED FAULT: detected - autoshift disabled "));
          Serial.print(vehicleSpeedDiff);
          Serial.print(F("-"));
          Serial.println(vehicleSpeedRPM);
        }
      }
    }
    else
    {
      if (speedFault)
      {
        speedFault = false; // we're in sync, good to go
        if (debugEnabled)
        {
          Serial.println(F("SPEED FAULT: recovery"));
        }
      }
    }
  }
  return speedValue;
  // return vehicleSpeedRevs;
}

int tpsRead()
{
  int tpsPercentValue = 0;
  if (tpsSensor)
  {

    float tpsVoltage = analogRead(tpsPin) * 3.00;
    tpsPercentValue = readTPSVoltage(tpsVoltage);

    if (tpsPercentValue > 100)
    {
      tpsPercentValue = 100;
    }
    if (tpsPercentValue < 0)
    {
      tpsPercentValue = 0;
    }
  }
  else
  {
    tpsPercentValue = 0;
  }
  return tpsPercentValue;
}

int rpmRead()
{
  struct ConfigParam config = readConfig();

  if (rpmRevs > config.maxRPM)
  {
    rpmRevs = config.maxRPM;
  }
  // Sensor smoothing if needed.
  /*avgRpmValue = alpha*oldRpmValue + (1-alpha)*rpmRevs;
  oldRpmValue = avgRpmValue;*/
  return rpmRevs;
}

int oilRead()
{
  // wip
  // w124 temp sensor B = 3500 roughly, 2.0kohm at 25c
/*
  Steinhart-Hart coefficients
a[0] = 1.689126553357672e-03
a[1] = 8.951863613981253e-05
a[2] = 2.411208545519697e-05
a[3] = -9.456539654701360e-07
*/
  float c1 = 1.689126553357672e-03, c2 = 8.951863613981253e-05, c3 = 2.411208545519697e-05, c4 = -9.456539654701360e-07;
  float tempRead = analogRead(oilPin);
  avgTemp = (avgTemp * 5 + tempRead) / 10;
  int R2 = 2250 * (1023.0 / (float)avgTemp);
  float logR2 = log(R2);
  float T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  // float T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2 + c4 * logR2 * logR2 * logR2));
  float oilTemp = T - 273.15;
  return oilTemp;
}

int boostRead()
{
  int boostValue = 0;
  if (boostSensor)
  {
    //reading MAP/boost
    float boostVoltage = analogRead(boostPin) * 3.00;
    boostValue = readBoostVoltage(boostVoltage);
    avgBoostValue = (avgBoostValue * 5 + boostValue) / 10;
  }
  return avgBoostValue;
}

int boostLimitRead(int oilTemp, int tps)
{
  int allowedBoostPressure = readMap(boostControlPressureMap, tps, oilTemp);
  return allowedBoostPressure;
}

int loadRead(int boostSensor, int allowedBoostPressure, int tpsPercentValue)
{
  int trueLoad = 0;
  int boostPercentValue = 100 * boostSensor / allowedBoostPressure;

  if (boostSensor && tpsSensor)
  {
    trueLoad = (tpsPercentValue * 0.60) + (boostPercentValue * 0.40);
  }
  else if (tpsSensor && !boostSensor)
  {
    trueLoad = tpsPercentValue;
  }
  else if (!tpsSensor)
  {
    trueLoad = 100;
  }

  return trueLoad;
}

//reading oil temp sensor / pn-switch (same input pin, see page 27: http://www.all-trans.by/assets/site/files/mercedes/722.6.1.pdf)
int atfRead()
{
  /*
  Steinhart-Hart coefficients
a[0] = 1.428001776691670e-02
a[1] = 3.123372804552903e-04
a[2] = -5.605468817359506e-04
a[3] = 4.141869911401698e-05
*/
  int atfTempCalculated = 0;
  int atfTempRaw = analogRead(atfPin);
  int atfTemp = 0;

  if (atfTempRaw > 1015)
  {
    drive = false;
    atfTempCalculated = 9999;
    atfTemp = 0;
  }
  else
  {
    drive = true;
    atfTempCalculated = (0.0309 * atfTempRaw * atfTempRaw) - 44.544 * atfTempRaw + 16629;
    atfTemp = -0.000033059 * atfTempCalculated * atfTempCalculated + 0.2031 * atfTempCalculated - 144.09; //same as above
    avgAtfTemp = (avgAtfTemp * 9 + atfTemp) / 10;
  }

  if (avgAtfTemp < -40)
  {
    avgAtfTemp = 0;
  }
  if (avgAtfTemp > 120)
  {
    avgAtfTemp = 120;
  }

  return avgAtfTemp;
}

int freeMemory()
{
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char *>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif // __arm__
}

struct SensorVals readSensors()
{
  struct SensorVals sensor;
  sensor.curOilTemp = oilRead();
  sensor.curAtfTemp = atfRead();
  sensor.curBoost = boostRead();
  sensor.curBoostLim = boostLimitRead(sensor.curOilTemp, sensor.curTps);
  sensor.curTps = tpsRead();
  sensor.curLoad = loadRead(sensor.curBoost, sensor.curBoostLim, sensor.curTps);
  sensor.curRPM = rpmRead();
  sensor.curSpeed = speedRead();
  return sensor;
}
