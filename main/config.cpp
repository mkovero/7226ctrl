#include <Arduino.h>



// END INPUT PINS

// Internals, states
int gear = 2;          // Start on gear 2
int cSolenoid = 0;     // Change solenoid pin to be controlled.
int vehicleSpeed = 100;
unsigned long lastSensorTime = 0;
int spcSetVal = 255;
int spcPercentVal = 100;
int n2SpeedPulses = 0;
int n3SpeedPulses = 0;
int vehicleSpeedPulses = 0;
int n2Speed = 0;
int n3Speed = 0;
extern unsigned int __bss_end;
unsigned int __heap_start;
void *__brkval;
int atfSensorFilterWeight = 16; // higher numbers = heavier filtering
int atfSensorNumReadings = 10;  // number of readings
int atfSensorAverage = 0;             // the  running average
int shiftDropPressure = 50;
int boostcount = 0;
// End of internals

// Environment configuration

// Shift delay
unsigned long int shiftStartTime = 0;
unsigned long int shiftDuration = 0;
int cSolenoidEnabled = 0;

// Are we in a real car?
boolean incar = false; // no.

// Do we use stick control?
boolean stick = true; // yes.

// Manual microswitch control?
boolean manual = false;

// Actual transmission there?
boolean trans = true;

// Are we using sensors?
boolean sensors = true;

// Do we have TPS?
boolean tpsSensor = true;

// Do we have boost sensor?
boolean boostSensor = true;

// Default for blocking gear switches (do not change.)
boolean shiftBlocker = false;

// Default for health (do not change.)
boolean health = false;

// Output to serial console
boolean debugEnabled = true;

// Full automatic mode
boolean fullAuto = true;

// Boost control
boolean boostLimit = true;

boolean drive = false;

// End of environment conf
