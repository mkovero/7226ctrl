#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <U8x8lib.h>
#include "include/ui.h"
#include "include/pins.h"
#include "include/sensors.h"
#include "include/eeprom.h"
#include "include/config.h"
#include <SoftTimer.h>

#ifdef MEGA
//U8GLIB_SSD1306_128X64 u8g(13, 11, 7, 6, 8); 
U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, 13, 11, 10, 6, 5);
#endif
#ifdef TEENSY
//U8GLIB_SSD1306_128X64 u8g(9, 11, 10, 6, 5); // DSPLOUT1-5
U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, 13, 11, 10, 6, 5);
#endif
/*
9 needs to be 13
6 4. redyellowfat
10 3. black
5 5. bronwhitepink
11 2. yellow-brown
9 1. yellow
*/
// yellow, yellow-brown, black, redyellowfat, brownwhitepink
// 13 = green <-> brown/white/pink
// 11 = red <-> brown/white
// 3 = red <-> green/red
// 6 = green <-> red/yellow fat
// 5 = red <-> black

// UI STAGE
// Control for what user sees and how gearbox is used with
//
typedef u8g2_uint_t u8g_uint_t;

void draw(int wantedGear)
{
  struct SensorVals sensor = readSensors();

  //  int freeSram = readFreeSram();
  // graphic commands to redraw the complete screen should be placed here
  u8g2.setFont(u8g_font_ncenB18);
  if (wantedGear == 6)
  {
  u8g2.drawStr(60, 40, "N");
  }
  if (wantedGear == 7)
  {
  u8g2.drawStr(60, 40, "R");
  }
  if (wantedGear == 8)
  {
  u8g2.drawStr(60, 40, "P");
  }
  if (fullAuto && wantedGear == 5)
  {
  u8g2.drawStr(60, 40, "D");
  }
  if (wantedGear == 100)
  {
  u8g2.drawStr(60, 40, "");
  }
  else
  {
  u8g2.drawStr(60, 40, gear);
  }
  if (fullAuto && wantedGear < 6)
  {
  u8g2.drawStr(60, 40, "(");
  u8g2.drawStr(60, 42, gear);
  u8g2.drawStr(60, 44, ")");
  }
  u8g2.setFont(u8g2_font_fub14_tf);
  u8g2.drawStr(60, 40, sensor.curSpeed);
  u8g2.drawStr(45, 60, "km/h");
  u8g2.setFont(u8g2_font_5x8_tn);
  u8g2.drawStr(0, 10, "atfTemp:");
  u8g2.drawStr(0, 20, sensor.curAtfTemp);
  u8g2.drawStr(0, 30, "oilTemp:");
  u8g2.drawStr(0, 40, sensor.curOilTemp);
  u8g2.drawStr(0, 50, "Boost:");
  u8g2.drawStr(0, 60, sensor.curBoost);
  u8g2.drawStr(100, 10, "TPS:");
  u8g2.drawStr(100, 20, sensor.curTps);
  u8g2.drawStr(100, 30, "RPM:");
  u8g2.drawStr(100, 40, sensor.curRPM);
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
  u8g2.begin();
  u8g2.firstPage();
  do
  {
    draw(wantedGear);
  } while (u8g2.nextPage());
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
    Serial.println(lastMapVal);
  }
}
