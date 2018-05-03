#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <U8glib.h>
#include "include/ui.h"
#include "include/pins.h"
#include "include/sensors.h"
#include "include/eeprom.h"
#include <SoftTimer.h>
int loopTime = 0;


#ifdef MEGA
U8GLIB_SSD1306_128X64 u8g(13, 11, 7, 6, 8);
#endif
#ifdef TEENSY
U8GLIB_SSD1306_128X64 u8g(13, 11, 3, 6, 5);
#endif

// UI STAGE
// Control for what user sees and how gearbox is used with
//
void draw(int wantedGear, int loopTime)
{
  int atfTemp = atfRead();
  int tpsPercentValue = tpsRead();
  int oilTemp = oilRead();
  int boostPressure = boostRead();
  int rpmValue = rpmRead();
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
  u8g.print(vehicleSpeed);
  u8g.setPrintPos(45, 60);
  u8g.print(F("km/h"));
  u8g.setFont(u8g_font_fixed_v0);
  u8g.setPrintPos(0, 10);
  u8g.print(F("atfTemp:"));
  u8g.setPrintPos(0, 20);
  u8g.print(atfTemp);
  u8g.setPrintPos(0, 30);
  u8g.print(F("oilTemp:"));
  u8g.setPrintPos(0, 40);
  u8g.print(oilTemp);
  u8g.setPrintPos(0, 50);
  u8g.print(F("Boost:"));
  u8g.setPrintPos(0, 60);
  u8g.print(boostPressure);
  u8g.setPrintPos(100, 10);
  u8g.print(F("TPS:"));
  u8g.setPrintPos(100, 20);
  u8g.print(tpsPercentValue);
  u8g.setPrintPos(100, 30);
  u8g.print(F("RPM:"));
  u8g.setPrintPos(100, 40);
  u8g.print(rpmValue);
  u8g.setPrintPos(100, 50);
  u8g.print(F("Looptime:"));
  u8g.setPrintPos(100, 60);
  u8g.print(loopTime);
}

void rpmMeterUpdate()
{
  int rpmValue = rpmRead();
  int rpmPWM = map(rpmValue, 0, 6500, 0, 255);
  analogWrite(rpmMeter, rpmPWM);
}
void updateSpeedo()
{
  int tpsPos = tpsRead();
  int speedPWM = map(tpsPos,0,100,0,255);
  analogWrite(speedoCtrl, speedPWM);
/*  int blipDuration = micros() - blipOntime;
  int blipDelay = tpsPos * 1000;
  if (blipDuration > blipDelay && tpsPos > 5)
  {
    blipOntime = micros();
    digitalWrite(speedoCtrl, HIGH);
  }
  else
  {
    digitalWrite(speedoCtrl, LOW);
  }*/
}

// Display update
void updateDisplay(Task* me)
{
  u8g.firstPage();
  do
  {
    draw(wantedGear, loopTime);
  } while (u8g.nextPage());
  if ( w124rpm ) { rpmMeterUpdate(); }
  if ( w124speedo ) { updateSpeedo(); }
}

void datalog(Task* me)
{
  int atfTemp = atfRead();
  int tpsPercentValue = tpsRead();
  int oilTemp = oilRead();
  int boostPressure = boostRead();
  int rpmValue = rpmRead();
  int boostPressureLimit = boostLimitRead();
  int load = loadRead();

  if (debugEnabled)
  {
    debugEnabled = false;
  }
  Serial.print(vehicleSpeed);
  Serial.print(F(";"));
  Serial.print(rpmValue);
  Serial.print(F(";"));
  Serial.print(tpsPercentValue);
  Serial.print(F(";"));
  Serial.print(gear);
  Serial.print(F(";"));
  Serial.print(oilTemp);
  Serial.print(F(";"));
  Serial.print(atfTemp);
  Serial.print(F(";"));
  Serial.print(load);
  Serial.print(F(";"));
  Serial.print(boostPressure);
  Serial.print(F(";"));
  Serial.print(boostPressureLimit);
  Serial.print(F(";"));
  Serial.print(lastMapVal);
  Serial.print(F(";"));
  Serial.println(loopTime);
}
