#include <Arduino.h>
#include "include/config.h"

// Environment configuration, THIS IS DEPRECATED, CONFIG NOW RESIDES IN EEPROM AND IS VIRGIN INITIALIZED FROM serial_config.cpp.
boolean boostLimit,boostLimitShift,stickCtrl,radioEnabled,manual,fullAuto,tccLock,evalGear,tpsSensor,boostSensor,exhaustPresSensor,w124speedo,w124rpm = false;
boolean fuelPumpControl, rpmSpeed, diffSpeed, adaptive, batteryMonitor, truePower, datalogger, exhaustTempSensor, resistiveStick = false;
boolean debugEnabled = true;

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
   .highRPMshiftLimit = 5000,
  .lowRPMshiftLimit = 1000,
  .diffRatio = 3.27,
  .maxSlip = 0.5, // Maximum allowed slip before error
  .transSloppy = 1.2, // multiplier for pressures coping with old transmissions
  .oneTotwo = 35,
  .twoTothree = 72,
  .threeTofour = 80,
  .fourTofive = 80,
  .fiveTofour = 65,
  .fourTothree = 65,
  .threeTotwo = 17,
  .twoToone = 35
};

// End of environment conf

// Do not change any of these.
// Default for blocking gear switches (do not change.)
boolean trans = true;
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
