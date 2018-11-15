/*
    722.6 transmission controller
    Copyright (C) 2018 Markus Kovero <mui@mui.fi>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    Big thanks to 
    Tuomas Kantola for maps and related math
    Tommi Otsavaara for guiding in electronics
    Mikko Kovero and Pete for mechanical side of things
    Jami Karvanen for datalogging and frontend stuff
    Liia Ahola for pcb tracing
    Joosep Vahar for testing
    Toni Lassila for ideas
*/

#include <Arduino.h>
#include "include/pins.h"
#include "include/sensors.h"
#include "include/core.h"
#include "include/input.h"
#include "include/ui.h"
#include <EEPROM.h>
#include <SoftTimer.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <AutoPID.h>

// "Protothreading", we have time slots for different functions to be run.
Task pollDisplay(200, updateDisplay);    // 500ms to update display*/
Task pollData(200, datalog);              // 200ms to update datalogging
Task pollStick(100, pollstick);           // 100ms for checking stick position*
Task pollGear(200, decideGear);           // 200ms for deciding new gear*/
Task pollSensors(100, pollsensors);       // 100ms to update sensor values*/
Task pollTrans(50, polltrans);            // 50ms to check transmission state (this needs to be faster than stick.)
Task pollFuelControl(1000, fuelControl);  // 1000ms for fuel pump control
Task pollBoostControl(100, boostControl); // 100ms for boost control*/
Task pollFaultMon(10, faultMon);  // 10ms Fault monitor

void setup()
{
  delay(1000);

  // MPC and SPC should have frequency of 1000hz
  // TCC should have frequency of 100hz
  // Lower the duty cycle, higher the pressures.

  analogWriteFrequency(spc, 1000);     // 1khz for spc
  analogWriteFrequency(mpc, 1000);     // and mpc
  analogWriteFrequency(boostCtrl, 30); // 30hz for boost controller
  analogWriteFrequency(rpmMeter, 50);  // 50hz for w124 rpm meter

  Serial.begin(115200);

  if (radioEnabled)
  {
    Serial1.begin(9600);
    if (debugEnabled && !datalogger)
    {
      Serial.println("Radio initialized.");
    }
  }
  U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, 13, 11, 10, 9, 5);
  u8g2.begin();

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
  pinMode(fuelPumpCtrl, OUTPUT);
  pinMode(SPIcs, OUTPUT);
  pinMode(hornPin, OUTPUT);

  // Sensor input
  pinMode(boostPin, INPUT); // boost sensor
  pinMode(tpsPin, INPUT);   // throttle position sensor
  pinMode(oilPin, INPUT);   // engine coolant sensor
  pinMode(atfPin, INPUT);   // ATF temp
  pinMode(n2pin, INPUT_PULLUP);    // N2 sensor
  pinMode(n3pin, INPUT_PULLUP);    // N3 sensor
  pinMode(speedPin, INPUT); // vehicle speed
  pinMode(rpmPin, INPUT);
  pinMode(batteryPin, INPUT);

  *portConfigRegister(boostPin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(tpsPin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(atfPin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  //*portConfigRegister(n2pin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  //*portConfigRegister(n3pin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(speedPin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(rpmPin) = PORT_PCR_MUX(1) | PORT_PCR_PE;

  //For manual control
  pinMode(autoSwitch, INPUT);
  pinMode(gupSwitch, INPUT);   // gear up
  pinMode(gdownSwitch, INPUT); // gear down

  *portConfigRegister(autoSwitch) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(gupSwitch) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(gdownSwitch) = PORT_PCR_MUX(1) | PORT_PCR_PE;

  //For stick control
  pinMode(whitepin, INPUT);
  pinMode(bluepin, INPUT);
  pinMode(greenpin, INPUT);
  pinMode(yellowpin, INPUT);

  *portConfigRegister(whitepin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(bluepin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(greenpin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(yellowpin) = PORT_PCR_MUX(1) | PORT_PCR_PE;

#ifdef ASPC
  pinMode(aSpcUpSwitch, INPUT);
  pinMode(aSpcDownSwitch, INPUT);
  *portConfigRegister(aSpcUpSwitch) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(aSpcDownSwitch) = PORT_PCR_MUX(1) | PORT_PCR_PE;
#else
  pinMode(exhaustPresPin, INPUT);
  pinMode(exhaustTempPin, INPUT);
  *portConfigRegister(exhaustPresPin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(exhaustTempPin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
#endif

  // Make sure solenoids are all off.
  analogWrite(y3, 255); // 1-2/4-5 Solenoid is pulsed during ignition crank.
  analogWrite(y4, 0);
  analogWrite(y5, 0);
  analogWrite(spc, 0);
  analogWrite(mpc, 0);
  analogWrite(tcc, 0);
  analogWrite(speedoCtrl, 0); // Wake up speedometer motor so it wont stick
  
  if (rpmSpeed && fuelPumpControl)
  {
    analogWrite(fuelPumpCtrl, 255); // Wake up fuel pumps
  }

  digitalWrite(rpmPin, HIGH); // pull-up
  digitalWrite(SPIcs, LOW);

  attachInterrupt(digitalPinToInterrupt(n2pin), N2SpeedInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(n3pin), N3SpeedInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(speedPin), vehicleSpeedInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(rpmPin), rpmInterrupt, RISING);

/* This is for erasing EEPROM on start.
  for (int i = 0; i < EEPROM.length(); i++) {
     EEPROM.write(i, 0);
  }
*/
       
  if (debugEnabled && !datalogger)
  {
    Serial.println(F("Started."));
  }

  // initialize timers
  SoftTimer.add(&pollDisplay);
  SoftTimer.add(&pollData);
  SoftTimer.add(&pollStick);
  SoftTimer.add(&pollGear);
  SoftTimer.add(&pollSensors);
  SoftTimer.add(&pollTrans);
  SoftTimer.add(&pollFuelControl);
  SoftTimer.add(&pollBoostControl);
}
