#include <Arduino.h>

// Environment configuration

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
boolean shiftPending = false;

// Output to serial console
boolean debugEnabled = true;

// Full automatic mode
boolean fullAuto = true;

// Boost control
boolean boostLimit = true;

boolean drive = false;

// Datalogging (enabling this disables debug)
boolean datalogger = false;

// Old style w124 speed meter controlling
boolean w124speedo = true;

boolean w124rpm = true;

// control fuel pumps
boolean fuelPumpControl = true;

// End of environment conf

