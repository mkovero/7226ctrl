#include <SPI.h>
#include <Wire.h>
#include <U8glib.h>
#include "config.h"
#include "maps.h"
#include "calc.h"
#include "sensors.h"
#include "core.h"
#include "input.h"
#include "ui.h"

// Work by Markus Kovero <mui@mui.fi>
// Big thanks to Tuomas Kantola regarding maps and related math



void setup() {
  
  TCCR3B = TCCR3B & 0b11111000 | 0x03; // 980hz on pins 5,3,2
  TCCR4B = TCCR4B & 0b11111000 | 0x03; // 980hz on pins 8,7,6
  // MPC and SPC should have frequency of 1000hz
  // TCC should have frequency of 100hz
  // Lower the duty cycle, higher the pressures.
  Serial.begin(9600);
 
  // Solenoid outputs
  pinMode(y3, OUTPUT); // 1-2/4-5 solenoid
  pinMode(y4,OUTPUT); // 2-3
  pinMode(y5,OUTPUT); // 3-4
  pinMode(spc,OUTPUT); // shift pressure
  pinMode(mpc,OUTPUT); // modulation pressure
  pinMode(tcc,OUTPUT); // lock
  pinMode(rpmMeter,OUTPUT);

  // Sensor input
  pinMode(boostPin,INPUT); // boost sensor 
  pinMode(tpsPin,INPUT); // throttle position sensor 
  pinMode(atfPin, INPUT); // ATF temp
  pinMode(n2pin,INPUT); // N2 sensor
  pinMode(n3pin,INPUT); // N3 sensor 
  pinMode(speedPin,INPUT); // vehicle speed
  pinMode(rpmPin,INPUT);
  
  //For manual control
  pinMode(autoSwitch,INPUT);
  pinMode(gupSwitch,INPUT); // gear up
  pinMode(gdownSwitch,INPUT); // gear down
  
  //For stick control
  pinMode(whitepin,INPUT);
  pinMode(bluepin,INPUT);
  pinMode(greenpin,INPUT);
  pinMode(yellowpin,INPUT);

  // Drive solenoids are all off.
  analogWrite(y3,0);
  analogWrite(y4,0);
  analogWrite(y5,0);
  analogWrite(spc,0); // No pressure here by default.
  analogWrite(mpc,255); // We want constant pressure here.
  analogWrite(tcc,0); // No pressure here by default.
  
  Serial.println("Started.");
  updateDisplay();
}

void loop() {
  checkHealth();
    if ( stick ) { pollstick(); } // using stick
    if ( manual ) { pollkeys(); } // using manual control
    if ( trans ) { polltrans(); } // using transmission
    if ( sensors ) { pollsensors(); } // using sensors
    updateDisplay();
}


