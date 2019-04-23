#include <Arduino.h>
#include <EEPROM.h>
#include "include/pins.h"
#include "include/config.h"
#include "include/sensors.h"

// Macro for sizeof for better support with 2d arrays.
#define LEN(arr) ((int)(sizeof(arr) / sizeof(arr)[0]))
int lastXval, lastYval;
int maxBoostPressure = 700; // Max pressure on boost sensor
boolean ShiftDebugEnabled = false;
int initBVoltage = analogRead(boostPin) * 5.0;
int initEVoltage = analogRead(exhaustPresPin) * 5.0;

/*if ( wantedGear == 8 ) {
 initBVoltage = analogRead(boostPin) * 5.0;
 initEVoltage = analogRead(exhaustPresPin) * 5.0;
}*/

// Calculation helpers

int pressureNormalization(int givenPressure)
{
  struct SensorVals sensor = readSensors();
  int targetVoltage = 12000;
  float pressureModifier = float(targetVoltage) / sensor.curBattery;
  if (sensor.curBattery < targetVoltage)
  {
    Serial.println("Battery voltage too low, target pressure cannot be reached.");
  }

  int normalizedPressure = float(pressureModifier) * givenPressure;
  if (normalizedPressure > 255)
  {
    normalizedPressure = 255;
    Serial.println("Pressure high compensation reached. Low battery?");
  }
  else if (normalizedPressure < 0)
  {
    normalizedPressure = 0;
    Serial.println("Pressure low compensation reached. Too high voltage?");
  }
  if (debugEnabled) {
    Serial.print("Voltage compensated: ");
    Serial.print(givenPressure);
    Serial.print(" to ");
    Serial.println(normalizedPressure);
  }
  return normalizedPressure;
}

// Mapping throttle position sensor voltage to percentage
int readTPSVoltage(int voltage)
{
  byte minLowByte = EEPROM.read(1000);
  byte minHighByte = EEPROM.read(1100);
  byte maxLowByte = EEPROM.read(2000);
  byte maxHighByte = EEPROM.read(2100);
  int minLimit = ((minLowByte << 0) & 0xFF) + ((minHighByte << 8) & 0xFF00);
  int maxLimit = ((maxLowByte << 0) & 0xFF) + ((maxHighByte << 8) & 0xFF00);
  int result = map(voltage, minLimit, maxLimit, 0, 100);
  return result;
}

// Mapping boost sensor voltage to percentage
int readBoostVoltage(int voltage)
{
  int result = voltage * 700 / 2.95;
  //int result = map(voltage, initBVoltage, 3100, 0, 3000); // NXP MPX5700AP (range 0-700kPa)
  return result;
}

int readExPresVoltage(int voltage)
{
  int result = voltage * 700 / 4.6;
  //int result = map(voltage, initEVoltage, 5000, 0, 3000); // NXP MPX5700AP (range 0-700kPa)
  return result;
}

// Mapping battery voltage to actual voltage
int readBatVoltage(int voltage)
{

  int result = map(voltage, 0, 3075, 0, 15000); // 0-15V
  return result;
}

// Function to read 2d maps from flash (maps declared with PROGMEM)
int readMap(const int theMap[14][12], int x, int y)
{
  int xidx = 0; // by default near first element
  int xelements = LEN(theMap[0]);

  int distance = abs(pgm_read_word_near(&theMap[0][xidx]) - x); // distance comparison
  for (int i = 1; i < xelements; i++)
  {
    int d = abs(pgm_read_word_near(&theMap[0][i]) - x);
    if (d < distance)
    {
      xidx = i;
      distance = d;
    }
  }
  int yidx = 0; // by default near first element
  int yelements = LEN(*theMap);

  distance = abs(pgm_read_word_near(&theMap[yidx][0]) - y);
  for (int i = 1; i < yelements; i++)
  {
    int d = abs(pgm_read_word_near(&theMap[i][0]) - y);
    if (d < distance)
    {
      yidx = i;
      distance = d;
    }
  }
  lastXval = xidx;
  lastYval = yidx;
  int mapValue = pgm_read_word_near(&theMap[yidx][xidx]);

  return mapValue;
}
int readTempMap(const int theMap[23][2], int y)
{

  int yidx = 0; // by default near first element
  int yelements = 23;

  for (int i = 1; i < yelements; i++)
  {
    int curVal = pgm_read_dword_near(&theMap[i][0]);

    if (y <= curVal)
    {
      yidx = i;
      break;
    }
  }

  int curY = pgm_read_dword_near(&theMap[yidx][0]);              // valittu Y // 1644
  int mapValue = pgm_read_dword_near(&theMap[yidx][1]);          // valittu X // 100
  int prevXMapValue = pgm_read_dword_near(&theMap[yidx - 1][1]); // edellinen X // 90
  int prevYMapValue = pgm_read_dword_near(&theMap[yidx - 1][0]); // edellinen Y // 1545

  float betweenL1 = ((float(curY) - y) / (curY - prevYMapValue)) * (mapValue - prevXMapValue) + prevXMapValue;
  // valittu Y - annettu luku = xyz -> (xyz / (valittu Y - edellinen Y)) * (valittu X - edellinen X) + edellinen x
  // 1700 - 1200 = (500/(1700-1170))*(30-40)+40
  // 1700 - 1200 = (500/(1700-1170))*(40-30)+30 // inverted
  return betweenL1;
}

int readTempMapInverted(const int theMap[14][2], int y)
{

  int yidx = 0; // by default near first element
  int yelements = 14;

  for (int i = 1; i < yelements; i++)
  {
    int curVal = pgm_read_dword_near(&theMap[i][0]);

    if (y <= curVal)
    {
      yidx = i;
      break;
    }
  }

  int curY = pgm_read_dword_near(&theMap[yidx][0]);              // valittu Y
  int mapValue = pgm_read_dword_near(&theMap[yidx][1]);          // valittu X
  int prevXMapValue = pgm_read_dword_near(&theMap[yidx - 1][1]); // edellinen X
  int prevYMapValue = pgm_read_dword_near(&theMap[yidx - 1][0]); // edellinen Y

  float betweenL1 = ((float(curY) - y) / (curY - prevYMapValue)) * (prevXMapValue - mapValue) + mapValue;
  // valittu Y - annettu luku = xyz -> (xyz / (valittu Y - edellinen Y)) * (valittu X - edellinen X) + edellinen x
  // 1700 - 1200 = (500/(1700-1170))*(30-40)+40
  // 1700 - 1200 = (500/(1700-1170))*(40-30)+30 // inverted
  return betweenL1;
}

int readPercentualMap(const int theMap[14][12], int x, int y)
{

  if (y < -20)
  {
    y = -20;
  }

  if (y > 100)
  {
    y = 100;
  }

  int xidx = 0; // by default near first element
  int yidx = 0; // by default near first element
  int xelements = 12;
  int yelements = 14;

  for (int i = 1; i < xelements; i++)
  {
    int curVal = pgm_read_dword_near(&theMap[0][i]);

    if (x <= curVal)
    {
      xidx = i;
      break;
    }
  }

  for (int i = 1; i < yelements; i++)
  {
    int curVal = pgm_read_dword_near(&theMap[i][0]);

    if (y <= curVal)
    {
      yidx = i;
      break;
    }
  }

  int calculatedPoint = 0;
  int mapValue = pgm_read_dword_near(&theMap[yidx][xidx]);
  int prevMapValue = pgm_read_dword_near(&theMap[yidx][xidx - 1]);
  int prevMapValue2 = pgm_read_dword_near(&theMap[yidx - 1][xidx]);
  int nextMapValue = pgm_read_dword_near(&theMap[yidx + 1][xidx]);
  int fuzzyMapValue = pgm_read_dword_near(&theMap[yidx + 1][xidx - 1]);
  int fuzzyMapValue2 = pgm_read_dword_near(&theMap[yidx - 1][xidx - 1]);
  int curY = pgm_read_dword_near(&theMap[yidx][0]);
  int curX = pgm_read_dword_near(&theMap[0][xidx]);
  int prevY = pgm_read_dword_near(&theMap[yidx - 1][0]);
  int prevX = pgm_read_dword_near(&theMap[0][xidx - 1]);

  float betweenL1 = ((float(curX) - x) / (curX - prevX));                     //*
  float calculatedLine1 = mapValue - (betweenL1 * (mapValue - prevMapValue)); //*
  if (ShiftDebugEnabled)
  {

    Serial.print("mapvalue: ");
    Serial.println(mapValue);
    Serial.print("prevmapvalue: ");
    Serial.println(prevMapValue);
    Serial.print("betweenL1: ");
    Serial.println(betweenL1);
    Serial.print("x: ");
    Serial.println(x);
    Serial.print("curX: ");
    Serial.println(curX);
    Serial.print("prevX: ");
    Serial.println(prevX);
  }
  if (y > curY)
  {
    double calculatedLine2 = nextMapValue - (betweenL1 * (nextMapValue - fuzzyMapValue));
    float twoPoints = ((float(y) - curY) / (prevY - curY));
    calculatedPoint = calculatedLine2 - (twoPoints * (calculatedLine1 - calculatedLine2));

    if (ShiftDebugEnabled)
    {
      Serial.print("y: ");
      Serial.println(y);
      Serial.print("curY: ");
      Serial.println(curY);
      Serial.print("prevY: ");
      Serial.println(prevY);
      Serial.print("twopoints: ");
      Serial.println(twoPoints);
      Serial.print("calculatedLine2: ");
      Serial.println(calculatedLine2);
      Serial.print("calculatedLine1: ");
      Serial.println(calculatedLine1);
      Serial.print("1calculatedPoint: ");
      Serial.println(calculatedPoint);
    }
  }
  else if (y < curY)
  {
    double calculatedLine2 = prevMapValue2 - (betweenL1 * (prevMapValue2 - fuzzyMapValue2)); //*
    float twoPoints = ((float(y) - curY) / (curY - prevY));
    calculatedPoint = calculatedLine2 - (twoPoints * (calculatedLine1 - calculatedLine2));

    if (ShiftDebugEnabled)
    {
      Serial.print("y: ");
      Serial.println(y);
      Serial.print("curY: ");
      Serial.println(curY);
      Serial.print("prevY: ");
      Serial.println(prevY);
      Serial.print("twopoints: ");
      Serial.println(twoPoints);
      Serial.print("calculatedLine2: ");
      Serial.println(calculatedLine2);
      Serial.print("calculatedLine1: ");
      Serial.println(calculatedLine1);
      Serial.print("2calculatedPoint: ");
      Serial.println(calculatedPoint);
    }
  }
  else
  {
    double calculatedLine2 = calculatedLine1;
    float twoPoints = ((float(y) - curY) / (prevY - curY));
    calculatedPoint = calculatedLine2 - (twoPoints * (calculatedLine1 - calculatedLine2));

    if (ShiftDebugEnabled)
    {
      Serial.print("y: ");
      Serial.println(y);
      Serial.print("curY: ");
      Serial.println(curY);
      Serial.print("prevY: ");
      Serial.println(prevY);
      Serial.print("twopoints: ");
      Serial.println(twoPoints);
      Serial.print("calculatedLine2: ");
      Serial.println(calculatedLine2);
      Serial.print("3calculatedPoint: ");
      Serial.println(calculatedPoint);
    }
  }
  //calculatedPoint = calculatedPoint * config.transSloppy;
  
 /* if (calculatedPoint > 100)
  {
    calculatedPoint = 100;
  }
  if (calculatedPoint < 1)
  {
    calculatedPoint = 0;
  }*/
  return calculatedPoint;
}

// Function to read 2d maps from flash (maps declared with PROGMEM)
int readGearMap(const int theMap[14][6], int x, int y)
{
  int yidx = 0; // by default near first element
  int yelements = LEN(*theMap);

  int distance = abs(pgm_read_word_near(&theMap[yidx][0]) - y);
  for (int i = 1; i < yelements; i++)
  {
    int d = abs(pgm_read_word_near(&theMap[i][0]) - y);
    if (d < distance)
    {
      yidx = i;
      distance = d;
    }
  }
  int mapValue = pgm_read_word_near(&theMap[yidx][x]);

  return mapValue;
}
