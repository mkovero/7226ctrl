#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <U8glib.h>
#include "config.h"
#include "pins.h"
#include "sensors.h"

U8GLIB_SSD1306_128X64 u8g(13, 11, 7, 6, 8);
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
  int freeSram = readFreeSram();
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
void updateDisplay(int wantedGear, int loopTime)
{
  /*  display.clearDisplay();
  display.setCursor(3,0);
  display.setTextSize(5);
  if ( ! fullAuto ) {
    if ( prevgear <= 5 ) { display.print(prevgear); };
    if ( prevgear == 6 ) { display.print("N"); };
    if ( prevgear == 7 ) { display.print("R"); };
    if ( prevgear == 8 ) { display.print("P"); };
    display.print("->");
    if ( gear <= 5 ) { display.print(gear); };
    if ( gear == 6 ) { display.print("N"); };
    if ( gear == 7 ) { display.print("R"); };
    if ( gear == 8 ) { display.print("P"); }; 
  } else {
    if ( gear == 5 ) { display.print("D"); };
    if ( gear == 6 ) { display.print("N"); };
    if ( gear == 7 ) { display.print("R"); };
    if ( gear == 8 ) { display.print("P"); }; 
  }
  display.setTextSize(1);
  display.println(atfTemp);
  display.print(", ");
  display.print(vehicleSpeed);
  display.print(", ");
  display.print(tpsPercentValue);
  display.display();*/
  u8g.firstPage();
  do
  {
    draw(wantedGear,loopTime);
  } while (u8g.nextPage());
  rpmMeterUpdate();
}
