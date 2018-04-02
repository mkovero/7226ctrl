#include <Arduino.h>
#include "include/config.h"
#include "include/pins.h"
#include "include/calc.h"
#include "include/maps.h"

// Internals
unsigned long n2SpeedPulses, n3SpeedPulses, vehicleSpeedPulses, vehicleSpeedRevs, lastSensorTime;
int n2Speed, n3Speed;
int speedMod = 10;
int rpmValue = 0;
int tpsPercentValue = 0;
int oilTemp = -40;
int atfTemp = -40;
int boostValue = 0;

// atf temperature sensor lowpass filtering
int atfSensorFilterWeight = 16; // higher numbers = heavier filtering
int atfSensorNumReadings = 10;  // number of readings
int atfSensorAverage = 0;       // the  running average

int tpsRead()
{
  tpsPercentValue = tpsPercentValue + 10;
  if ( tpsPercentValue >= 100 ) { tpsPercentValue = 0; }
  return tpsPercentValue;
}

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

// Polling sensors
void pollsensors()
{
  const int n2PulsesPerRev = 60;
  const int n3PulsesPerRev = 60;
  const int vehicleSpeedPulsesPerRev = 29; // number of teeths in w124 rear diff
  float diffRatio = 3.27;                  // rear diff ratio
  int vehicleSpeedRevs;

  if (millis() - lastSensorTime >= 1000)
  {
    detachInterrupt(2); // Detach interrupts for calculation
    detachInterrupt(3);
    detachInterrupt(4);

    // This is for datalog testing.
    if (vehicleSpeed >= 300)
    {
      vehicleSpeed = 0;
    }
    else
    {
      vehicleSpeed = vehicleSpeed + speedMod;
    }

    lastSensorTime = millis();

    attachInterrupt(2, N2SpeedInterrupt, RISING); // Attach again
    attachInterrupt(3, N3SpeedInterrupt, RISING);
    attachInterrupt(4, vehicleSpeedInterrupt, RISING);
  }
}

int rpmRead()
{
  rpmValue = rpmValue + 1; // Datalog testing
  if ( rpmValue >= 7000 ) { rpmValue = 0; }

  return rpmValue;
}

int boostRead()
{
  boostValue = boostValue + 5; // Datalog testing
  if ( boostValue >= 700 ) { boostValue = 0; }

  return boostValue;
}

int loadRead()
{
  int trueLoad = 0;
  int boostPercentValue = boostRead();
  int tpsPercentValue = tpsRead();

  if (boostSensor && tpsSensor)
  {
    trueLoad = (tpsPercentValue * 0.60) + (boostPercentValue * 0.40);
  }
  else if (tpsSensor && !boostSensor)
  {
    trueLoad = (tpsPercentValue * 1);
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
  atfTemp = atfTemp + 14; // Datalog testing
  if ( atfTemp >= 400 ) { atfTemp = -40; }

  return atfTemp;
}

int oilRead()
{
  oilTemp = oilTemp + 12; // Datalog testing
  if ( oilTemp >= 400 ) { oilTemp = -40; }

  return oilTemp;
}

int boostLimitRead()
{
  int oilTemp = oilRead();
  int tps = tpsRead();
  int allowedBoostPressure = readMapMem(boostControlPressureMap, tps, oilTemp);

  return allowedBoostPressure;
}
