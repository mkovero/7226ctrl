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

// Work by Markus Kovero <mui@mui.fi>
// Big thanks to Tuomas Kantola regarding maps and related math

// "Protothreading", we have time slots for different functions to be run.
Task pollDisplay(500, updateDisplay); // 500ms to update display*/
Task pollData(200, datalog);          // 200ms to update datalogging
Task pollStick(100, pollstick);       // 200ms for checking stick position*
Task pollGear(200, decideGear);
Task pollSensors(200, pollsensors);       // 500ms to update sensor values*/
Task pollTrans(50, polltrans);            // 50ms to check transmission state
Task pollFuelControl(1000, fuelControl);  // 1000ms for fuel pump control
Task pollBoostControl(50, boostControl); // 100ms for boost control*/

void setup()
{
  delay(1000);
#ifdef MEGA
  TCCR2B = TCCR2B & 0b11111000 | 0x03; // 980hz on pins 9,10
  TCCR5B = TCCR5B & 0b11111000 | 0x05; // 30hz on pins 44-46
#endif
  // MPC and SPC should have frequency of 1000hz
  // TCC should have frequency of 100hz
  // Lower the duty cycle, higher the pressures.
  if (debugEnabled)
  {
    Serial.begin(115200);
  }
  else
  {
    Serial.begin(115200);
  }

  //U8GLIB_SSD1306_128X64 u8g(9, 11, 10, 6, 5); // DSPLOUT1-5
  // 9->13 would allow hardware SPI
  U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, 9, 11, 10, 6, 5);



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
#ifdef TEENSY
  analogWriteFrequency(spc, 1000);     // 1khz for spc
  analogWriteFrequency(mpc, 1000);     // and mpc
  analogWriteFrequency(boostCtrl, 30); // 30hz for boost controller
  analogWriteFrequency(rpmMeter, 50);  // 50hz for w124 rpm meter
#endif
  // Sensor input
  pinMode(boostPin, INPUT); // boost sensor
  pinMode(tpsPin, INPUT);   // throttle position sensor
  pinMode(oilPin, INPUT);   // engine coolant sensor
  pinMode(atfPin, INPUT);   // ATF temp
  pinMode(n2pin, INPUT);    // N2 sensor
  pinMode(n3pin, INPUT);    // N3 sensor
  pinMode(speedPin, INPUT); // vehicle speed
  pinMode(rpmPin, INPUT);
#ifdef TEENSY
  *portConfigRegister(boostPin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(tpsPin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(atfPin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(n2pin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(n3pin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(speedPin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(rpmPin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
#endif
  //For manual control
  pinMode(autoSwitch, INPUT);
  pinMode(gupSwitch, INPUT);   // gear up
  pinMode(gdownSwitch, INPUT); // gear down
#ifdef TEENSY
  *portConfigRegister(autoSwitch) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(gupSwitch) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(gdownSwitch) = PORT_PCR_MUX(1) | PORT_PCR_PE;
#endif
  //For stick control
  pinMode(whitepin, INPUT);
  pinMode(bluepin, INPUT);
  pinMode(greenpin, INPUT);
  pinMode(yellowpin, INPUT);
#ifdef TEENSY
  *portConfigRegister(whitepin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(bluepin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(greenpin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
  *portConfigRegister(yellowpin) = PORT_PCR_MUX(1) | PORT_PCR_PE;
#endif
  // Make sure solenoids are all off.
  analogWrite(y3, 0);
  analogWrite(y4, 0);
  analogWrite(y5, 0);
  analogWrite(spc, 0);
  analogWrite(mpc, 0);
  analogWrite(tcc, 0);
  analogWrite(speedoCtrl, 255);     // Wake up speedometer motor so it wont stick
  analogWrite(fuelPumpCtrl, 255); // Wake up fuel pumps
  digitalWrite(rpmPin, HIGH);     // pull-up

  // resetEEPROM();

  attachInterrupt(digitalPinToInterrupt(n2pin), N2SpeedInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(n3pin), N3SpeedInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(speedPin), vehicleSpeedInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(rpmPin), rpmInterrupt, RISING);

  if (debugEnabled)
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
