#include <Arduino.h>
#include "include/pins.h"
#include "include/config.h"
#include "include/sensors.h"
#include "include/core.h"
#include "include/input.h"
#include "include/ui.h"
#include <EEPROM.h>
unsigned long int startTime = 0;
unsigned long int endTime = 0;

// Work by Markus Kovero <mui@mui.fi>
// Big thanks to Tuomas Kantola regarding maps and related math

void setup()
{
#ifdef MEGA
  TCCR2B = TCCR2B & 0b11111000 | 0x03; // 980hz on pins 9,10
  TCCR5B = TCCR5B & 0b11111000 | 0x05; // 30hz on pins 44-46
#endif
  // MPC and SPC should have frequency of 1000hz
  // TCC should have frequency of 100hz
  // Lower the duty cycle, higher the pressures.
  if (debugEnabled)
  {
    Serial.begin(9600);
  }
  else
  {
    Serial.begin(115200);
  }

  // Solenoid outputs
  pinMode(y3, OUTPUT);  // 1-2/4-5 solenoid
  pinMode(y4, OUTPUT);  // 2-3
  pinMode(y5, OUTPUT);  // 3-4
  pinMode(spc, OUTPUT); // shift pressure
  pinMode(mpc, OUTPUT); // modulation pressure
  pinMode(tcc, OUTPUT); // lock
  pinMode(rpmMeter, OUTPUT);
  pinMode(boostCtrl, OUTPUT);
  pinMode(speedoCtrl, OUTPUT);
  pinMode(speedoDir, OUTPUT);
#ifdef TEENSY
  analogWriteFrequency(spc, 1000); // 1khz for spc
  analogWriteFrequency(mpc, 1000); // and mpc
  analogWriteFrequency(boostCtrl, 30); // 30hz for boost controller
  analogWriteFrequency(rpmMeter, 50); // 50hz for w124 rpm meter
#endif
  // Sensor input
  pinMode(boostPin, INPUT); // boost sensor
  pinMode(tpsPin, INPUT);   // throttle position sensor
  pinMode(atfPin, INPUT);   // ATF temp
  pinMode(n2pin, INPUT);    // N2 sensor
  pinMode(n3pin, INPUT);    // N3 sensor
  pinMode(speedPin, INPUT); // vehicle speed
  pinMode(rpmPin, INPUT);

  //For manual control
  pinMode(autoSwitch, INPUT);
  pinMode(gupSwitch, INPUT);   // gear up
  pinMode(gdownSwitch, INPUT); // gear down

  //For stick control
  pinMode(whitepin, INPUT);
  pinMode(bluepin, INPUT);
  pinMode(greenpin, INPUT);
  pinMode(yellowpin, INPUT);

  // Make sure solenoids are all off.
  analogWrite(y3, 0);
  analogWrite(y4, 0);
  analogWrite(y5, 0);
  analogWrite(spc, 0);
  analogWrite(mpc, 0);
  analogWrite(tcc, 0);

  if (debugEnabled)
  {
    Serial.println("Started.");
  }
}

void loop()
{
  startTime = micros();
  int wantedGear = pollstick();
  int newGear = decideGear(wantedGear);
  polltrans(newGear, wantedGear);
  pollsensors();
  updateDisplay(wantedGear, loopTime);
  endTime = micros();
  int loopTime = endTime - startTime;
  
  if (datalogger)
  {
    datalog(loopTime);
  }
}
