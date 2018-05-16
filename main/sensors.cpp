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
unsigned long n2SpeedPulses, n3SpeedPulses, vehicleSpeedPulses, lastSensorTime, rpmPulse;
int n2Speed, n3Speed, rpmRevs, vehicleSpeedRevs;

// sensor smoothing
int avgAtfTemp, avgBoostValue, avgVehicleSpeed, avgRpmValue;

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

    if (n2SpeedPulses >= 60)
    {
      n2Speed = n2SpeedPulses / 60;
      n2SpeedPulses = 0;
    }
    else
    {
      n2Speed = 0;
    }

    if (n3SpeedPulses >= 60)
    {
      n3Speed = n3SpeedPulses / 60;
      n3SpeedPulses = 0;
    }
    else
    {
      n3Speed = 0;
    }

    if (vehicleSpeedPulses >= vehicleSpeedPulsesPerRev)
    {
      vehicleSpeedRevs = vehicleSpeedPulses / vehicleSpeedPulsesPerRev;
      vehicleSpeedPulses = 0;
    }

    if (rpmPulse >= 60)
    {
      rpmRevs = rpmPulse * 60;
      rpmPulse = 0;
    }

    lastSensorTime = millis();

    attachInterrupt(2, N2SpeedInterrupt, RISING); // Attach again
    attachInterrupt(3, N3SpeedInterrupt, RISING);
    attachInterrupt(4, vehicleSpeedInterrupt, RISING);
    attachInterrupt(5, rpmInterrupt, RISING);
  }
}

int speedRead()
{
  struct ConfigParam config = readConfig();
  int curRPM = rpmRead();
  int vehicleSpeed = 100;

  // int vehicleSpeed = 0.03654 * vehicleSpeedRevs; // 225/45/17 with 3.27 rear diff
  float tireDiameter = (config.tireWidth * config.tireProfile / 2540 * 2 + config.tireInches) * 25.4;
  float tireCircumference = 3.14 * tireDiameter;
  if (rpmSpeed)
  {
    // speed based on engine rpm
    vehicleSpeed = tireCircumference * curRPM / (ratioFromGear(gear) * config.diffRatio) / 1000000 * 60;
  }
  else if (diffSpeed)
  {
    // speed based on diff abs sensor
    vehicleSpeed = tireCircumference * vehicleSpeedRevs / config.diffRatio / 1000000 * 60;
  }

  avgVehicleSpeed = (avgVehicleSpeed * 1 + vehicleSpeed) / 2;
  return avgVehicleSpeed;
}

int tpsRead()
{
  int tpsPercentValue = 0;
  if (tpsSensor)
  {
    //reading TPS
    float tpsVoltage = analogRead(tpsPin) * 4.89;
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
  avgRpmValue = (avgRpmValue * 5 + rpmRevs) / 10;

  return avgRpmValue;
}

int oilRead()
{
  // wip
  int oilTemp = 100;
  return oilTemp;
}

int boostRead()
{
  int boostValue = 0;
  if (boostSensor)
  {
    //reading MAP/boost
    float boostVoltage = analogRead(boostPin) * 4.89;
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