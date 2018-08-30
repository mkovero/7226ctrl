#include <Arduino.h>
#include "include/config.h"

// Environment configuration

// Do we use stick control?
boolean stick = true; // yes.

// Manual microswitch control?
boolean manual = true;

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
boolean fullAuto = false;

// Stick control
boolean stickCtrl = true;

// Boost control
boolean boostLimit = true;

boolean drive = false;

// Datalogging (enabling this disables debug)
boolean datalogger = false;

// Old style w124 speed meter controlling
boolean w124speedo = true;

// Old style w124 rpm meter pwm
boolean w124rpm = true;

// control fuel pumps (turn off below certain rpm)
boolean fuelPumpControl = false;

// calculate vehicleSpeed from engine RPM
boolean rpmSpeed = false; 

// calculate vehicleSpeed from diff abs sensor
boolean diffSpeed = false; 

// fault mode for speed sensors
boolean speedFault = false; 

// gear evaluation to determine real running gear, this prevents shifts if previous shift has not completed
boolean evalGear = false;

// radio control
boolean radioEnabled = true; 

// everything starts with ignition (do not change.)
boolean ignition = true;
// state of fuel pumps
boolean fuelPumps = false;
// state of horn
boolean horn = false;

byte page = 1; // first page to show in UI

struct ConfigParam readConfig()
{
  struct ConfigParam config;
  config.boostMax = 700; // boost sensor max kpa
  config.boostDrop = 50; // kpa to drop on shifts
  config.boostSpring = 120; // kpa for wastegate spring pressure
  config.fuelMaxRPM = 2000; // RPM limit to turn on fuel pumps
  config.maxRPM = 7000; // Max engine RPM
  config.tireWidth = 225;
  config.tireProfile = 45;
  config.tireInches = 17;
  config.diffRatio = 3.27;
  return config;
}

// End of environment conf

