#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include "include/ui.h"
#include "include/pins.h"
#include "include/sensors.h"
#include "include/eeprom.h"
#include "include/config.h"
#include <SoftTimer.h>

/*#ifdef MEGA
//U8GLIB_SSD1306_128X64 u8g(13, 11, 7, 6, 8); 
U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, 13, 11, 10, 6, 5);
#endif*/

//#ifdef TEENSY
//U8GLIB_SSD1306_128X64 u8g(9, 11, 10, 6, 5); // DSPLOUT1-5
// 9->13 would allow hardware SPI
U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, 13, 11, 10, 9, 5);
//U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, 13, 11, 10, 9, 5);
//U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, 9, 11, 10, 13, 5);
//a_SSD1306_128X64_NONAME_1_4W_HW_SPI(U8G2_R0, 10, 6, 5);
//#endif

/* 
yellow(res), yellow-brown(dc), black(clock), redyellowfat (cs), brownwhitepink (data)
5 6 9 10 11
clock, data, cs, dc [, reset]
9       11    10  6    5
res,dc,clock,cs,data


whitegreen<->black
green<->brownwhitepink
orangewhite<->redyellow
brown<->yellowbrown
blue<->yellow
*/

// UI STAGE
// Control for what user sees and how gearbox is used with
//
typedef u8g2_uint_t u8g_uint_t;
//u8g2.begin();

void draw(int wantedGear)
{
  struct SensorVals sensor = readSensors();
  if (page == 1)
  {
    u8g2.setFont(u8g2_font_logisoso16_tr);
    u8g2.setCursor(50, 20);
    if (wantedGear == 6)
    {
      u8g2.print(F("N"));
    }
    if (wantedGear == 7)
    {
      u8g2.print(F("R"));
    }
    if (wantedGear == 8)
    {
      u8g2.print(F("P"));
    }
    if (wantedGear == 5)
    {
      u8g2.print(F("D"));
    }
    if (wantedGear < 5 || (!fullAuto && wantedGear == 5))
    {
      u8g2.print(wantedGear);
    }
    if (fullAuto && wantedGear < 6)
    {

      u8g2.setCursor(60, 20);
      u8g2.print(F("("));
      u8g2.print(gear);
      u8g2.print(F(")"));
    }
    u8g2.setFont(u8g2_font_fub14_tf);
    u8g2.setCursor(60, 40);
    u8g2.print(sensor.curSpeed);
    u8g2.setCursor(45, 60);
    u8g2.print(F("km/h"));
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.setCursor(0, 10);
    u8g2.print("atfTemp:");
    u8g2.setCursor(0, 20);
    u8g2.print(sensor.curAtfTemp);
    u8g2.setCursor(0, 30);
    u8g2.print(F("oilTemp:"));
    u8g2.setCursor(0, 40);
    u8g2.print(sensor.curOilTemp);
    u8g2.setCursor(0, 50);
    u8g2.print(F("Boost:"));
    u8g2.setCursor(0, 60);
    u8g2.print(sensor.curBoost);
    u8g2.setCursor(100, 10);
    u8g2.print(F("TPS:"));
    u8g2.setCursor(100, 20);
    u8g2.print(sensor.curTps);
    u8g2.setCursor(100, 30);
    u8g2.print(F("RPM:"));
    u8g2.setCursor(100, 40);
    u8g2.print(sensor.curRPM);
  }
  else if (page == 2)
  {
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.setCursor(0, 15);
    u8g2.print(sensor.curBoost);
    u8g2.setCursor(0, 30);
    u8g2.print(sensor.curBoostLim);
  }
}

void rpmMeterUpdate()
{
  struct SensorVals sensor = readSensors();
  struct ConfigParam config = readConfig();

  int rpmPWM = map(sensor.curRPM, 0, config.maxRPM, 0, 255);
  analogWrite(rpmMeter, rpmPWM);
}

void updateSpeedo()
{
  struct SensorVals sensor = readSensors();
  //int speedPWM = map(sensor.curSpeed, 0, 255, 0, 255);
  analogWrite(speedoCtrl, sensor.curSpeed);
}

// Display update
void updateDisplay(Task *me)
{

  u8g2.clearBuffer();
  draw(wantedGear);
  u8g2.sendBuffer();

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
