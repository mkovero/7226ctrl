#include <Arduino.h>
#include "include/config.h"

// Environment configuration, THIS IS DEPRECATED, CONFIG NOW RESIDES IN EEPROM AND IS VIRGIN INITIALIZED FROM serial_config.cpp.
boolean trans,stickCtrl,radioEnabled,manual,fullAuto,tccLock,evalGear,tpsSensor,boostSensor,exhaustPresSensor,w124speedo,w124rpm,boostLimit = false;
boolean fuelPumpControl, rpmSpeed, diffSpeed, adaptive, batteryMonitor, truePower, debugEnabled, datalogger, exhaustTempSensor = false;

struct ConfigParam config = {
  .boostMax = 700, // boost sensor max kpa
  .boostDrop = 50, // kpa to drop on shifts
  .boostSpring = 120, // kpa for wastegate spring pressure
  .fuelMaxRPM = 2000, // RPM limit to turn on fuel pumps
  .maxRPM = 7000, // Max engine RPM
  .tireWidth = 195,
  .tireProfile = 65,
  .tireInches = 15,
  .rearDiffTeeth = 29, // number of teeth in diff
  .nextShiftDelay = 2000, // ms. to wait before next shift to avoid accidental overshifting.
  .stallSpeed = 2200, // torque converter stall speed
  .batteryLimit = 11500, // battery voltage limit in 11.5v
  .firstTccGear = 2, // first gear when tcc is used.
  .triggerWheelTeeth = 6, // number of teeth in trigger wheel for RPM calculation
  .tpsAgre = 2, // 1-10 how aggressive slope tps has
  .diffRatio = 3.27,
  .maxSlip = 0.5 // Maximum allowed slip before error
};

// End of environment conf

// Do not change any of these.
// Default for blocking gear switches (do not change.)
boolean shiftBlocker = false;
boolean shiftPending = false;
// everything starts with ignition (do not change.)
boolean ignition = true;
boolean carRunning = false;
byte page = 1; // first page to show in UI
boolean drive = false;
// fault mode for speed sensors
boolean speedFault = false; 
// fault mode for battery fault
boolean batteryFault = false;
// fault mode for excess slip
boolean slipFault = false;
// Initial state of fuel pumps
boolean fuelPumps = true;
// Initial state of horn
boolean horn = false;