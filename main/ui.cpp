#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <U8glib.h>
#include "include/config.h"
#include "include/pins.h"
#include "include/sensors.h"
#include "include/eeprom.h"
#include <SoftTimer.h>

#ifdef MEGA
U8GLIB_SSD1306_128X64 u8g(13, 11, 7, 6, 8);
#endif
#ifdef TEENSY
U8GLIB_SSD1306_128X64 u8g(13, 11, 3, 6, 5);
#endif
int loopTime = 0;
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
    u8g.print("N");
  }
  if (wantedGear == 7)
  {
    u8g.print("R");
  }
  if (wantedGear == 8)
  {
    u8g.print("P");
  }
  if (fullAuto && wantedGear == 5)
  {
    u8g.print("D");
  }
  if (wantedGear == 100)
  {
    u8g.print("");
  }
  else
  {
    u8g.print(gear);
  }
  if (fullAuto && wantedGear < 6)
  {
    u8g.print("(");
    u8g.print(gear);
    u8g.print(")");
  }
  u8g.setFont(u8g_font_fub14);
  u8g.setPrintPos(60, 40);
  u8g.print(vehicleSpeed);
  u8g.setPrintPos(45, 60);
  u8g.print("km/h");
  u8g.setFont(u8g_font_fixed_v0);
  u8g.setPrintPos(0, 10);
  u8g.print("atfTemp:");
  u8g.setPrintPos(0, 20);
  u8g.print(atfTemp);
  u8g.setPrintPos(0, 30);
  u8g.print("oilTemp:");
  u8g.setPrintPos(0, 40);
  u8g.print(oilTemp);
  u8g.setPrintPos(0, 50);
  u8g.print("Boost:");
  u8g.setPrintPos(0, 60);
  u8g.print(boostPressure);
  u8g.setPrintPos(100, 10);
  u8g.print("TPS:");
  u8g.setPrintPos(100, 20);
  u8g.print(tpsPercentValue);
  u8g.setPrintPos(100, 30);
  u8g.print("RPM:");
  u8g.setPrintPos(100, 40);
  u8g.print(rpmValue);
  u8g.setPrintPos(100, 50);
  u8g.print("Looptime:");
  u8g.setPrintPos(100, 60);
  u8g.print(loopTime);
}

void rpmMeterUpdate()
{
  int rpmValue = rpmRead();
  int rpmPWM = map(rpmValue, 0, 6500, 0, 255);
  analogWrite(rpmMeter, rpmPWM);
}

// Display update
void updateDisplay(Task *me)
{
  u8g.firstPage();
  do
  {
    draw(wantedGear, loopTime);
  } while (u8g.nextPage());
  rpmMeterUpdate();
}

void updateSpeedo()
{
  digitalWrite(speedoDir, LOW); // can change direction, on w124 direction should be opposite as to clock
  int speedMillis = 300 - vehicleSpeed;
  long unsigned lastSpeedoMillis;

  if (speedMillis > millis() - lastSpeedoMillis)
  {
    digitalWrite(speedoCtrl, HIGH);
    lastSpeedoMillis = millis();
  }
  else
  {
    digitalWrite(speedoCtrl, LOW);
  }
}

void datalog(Task *me)
{
  if (datalogger)
  {
    int atfTemp = atfRead();
    int tpsPercentValue = tpsRead();
    int oilTemp = oilRead();
    int boostPressure = boostRead();
    int rpmValue = rpmRead();
    int boostPressureLimit = boostLimitRead();
    int load = loadRead();
    int lastModVal = readEEPROM(lastMapVal, lastXval, lastYval);

    if (debugEnabled)
    {
      debugEnabled = false;
    }
    Serial.print(vehicleSpeed);
    Serial.print(";");
    Serial.print(rpmValue);
    Serial.print(";");
    Serial.print(tpsPercentValue);
    Serial.print(";");
    Serial.print(gear);
    Serial.print(";");
    Serial.print(oilTemp);
    Serial.print(";");
    Serial.print(atfTemp);
    Serial.print(";");
    Serial.print(load);
    Serial.print(";");
    Serial.print(boostPressure);
    Serial.print(";");
    Serial.print(boostPressureLimit);
    Serial.print(";");
    Serial.print(lastMapVal);
    Serial.print(";");
    Serial.print(lastModVal);
    Serial.print(";");
    Serial.print(wantedGear);
    Serial.print(";");
    Serial.println(newGear);
  }
}
