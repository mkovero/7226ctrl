#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <U8glib.h>
#include "include/ui.h"
#include "include/pins.h"
#include "include/sensors.h"
#include "include/eeprom.h"
#include "include/config.h"
#include <SoftTimer.h>

#ifdef MEGA
U8GLIB_SSD1306_128X64 u8g(13, 11, 7, 6, 8);
#endif
#ifdef TEENSY
U8GLIB_SSD1306_128X64 u8g(13, 11, 3, 6, 5);
#endif

// UI STAGE
// Control for what user sees and how gearbox is used with
//
void draw(int wantedGear)
{
  struct SensorVals sensor = readSensors();

  //  int freeSram = readFreeSram();
  // graphic commands to redraw the complete screen should be placed here
  u8g.setFont(u8g_font_ncenB18);
  u8g.setPrintPos(50, 20);
  if (wantedGear == 6)
  {
    u8g.print(F("N"));
  }
  if (wantedGear == 7)
  {
    u8g.print(F("R"));
  }
  if (wantedGear == 8)
  {
    u8g.print(F("P"));
  }
  if (fullAuto && wantedGear == 5)
  {
    u8g.print(F("D"));
  }
  if (wantedGear == 100)
  {
    u8g.print(F(""));
  }
  else
  {
    u8g.print(gear);
  }
  if (fullAuto && wantedGear < 6)
  {
    u8g.print(F("("));
    u8g.print(gear);
    u8g.print(F(")"));
  }
  u8g.setFont(u8g_font_fub14);
  u8g.setPrintPos(60, 40);
  u8g.print(sensor.curSpeed);
  u8g.setPrintPos(45, 60);
  u8g.print(F("km/h"));
  u8g.setFont(u8g_font_fixed_v0);
  u8g.setPrintPos(0, 10);
  u8g.print(F("atfTemp:"));
  u8g.setPrintPos(0, 20);
  u8g.print(sensor.curAtfTemp);
  u8g.setPrintPos(0, 30);
  u8g.print(F("oilTemp:"));
  u8g.setPrintPos(0, 40);
  u8g.print(sensor.curOilTemp);
  u8g.setPrintPos(0, 50);
  u8g.print(F("Boost:"));
  u8g.setPrintPos(0, 60);
  u8g.print(sensor.curBoost);
  u8g.setPrintPos(100, 10);
  u8g.print(F("TPS:"));
  u8g.setPrintPos(100, 20);
  u8g.print(sensor.curTps);
  u8g.setPrintPos(100, 30);
  u8g.print(F("RPM:"));
  u8g.setPrintPos(100, 40);
  u8g.print(sensor.curRPM);
  u8g.setPrintPos(100, 50);
}

void rpmMeterUpdate()
{
  struct SensorVals sensor = readSensors();
  int rpmPWM = map(sensor.curRPM, 0, 6500, 0, 255);
  analogWrite(rpmMeter, rpmPWM);
}

void updateSpeedo()
{
  struct SensorVals sensor = readSensors();
  int speedPWM = map(sensor.curTps, 0, 100, 0, 255);
  analogWrite(speedoCtrl, speedPWM);
}

// Display update
void updateDisplay(Task *me)
{
  u8g.firstPage();
  do
  {
    draw(wantedGear);
  } while (u8g.nextPage());
  if (w124rpm)
  {
    rpmMeterUpdate();
  }
  if (w124speedo)
  {
    updateSpeedo();
  }
}

void datalog(Task *me)
{
  if (datalogger)
  {
    struct SensorVals sensor = readSensors();
    struct ConfigParam config = readConfig();

    if (debugEnabled)
    {
      debugEnabled = false;
    }
    Serial.print(sensor.curSpeed);
    Serial.print(F(";"));
    Serial.print(sensor.curRPM);
    Serial.print(F(";"));
    Serial.print(sensor.curTps);
    Serial.print(F(";"));
    Serial.print(gear);
    Serial.print(F(";"));
    Serial.print(sensor.curOilTemp);
    Serial.print(F(";"));
    Serial.print(sensor.curAtfTemp);
    Serial.print(F(";"));
    Serial.print(sensor.curLoad);
    Serial.print(F(";"));
    Serial.print(sensor.curBoost);
    Serial.print(F(";"));
    Serial.print(sensor.curBoostLim);
    Serial.print(F(";"));
    Serial.print(lastMapVal);
  }
}
