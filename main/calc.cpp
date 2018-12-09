#include <Arduino.h>

// Macro for sizeof for better support with 2d arrays.
#define LEN(arr) ((int)(sizeof(arr) / sizeof(arr)[0]))
int lastXval, lastYval;
int maxBoostPressure = 700; // Max pressure on boost sensor

// Calculation helpers

// Mapping throttle position sensor voltage to percentage
int readTPSVoltage(int voltage)
{

  int result = map(voltage, 1900, 800, 0, 100);
  return result;
}

// Mapping boost sensor voltage to percentage
int readBoostVoltage(int voltage)
{

  int result = map(voltage, 680, 2670, 0, 3000); // NXP MPX5700AP (range 0-700kPa)
  return result;
}
// Mapping battery voltage to actual voltage
int readBatVoltage(int voltage)
{

  int result = map(voltage, 0, 3375, 0, 15000); // 0-15V
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

  int calculatedPoint = 0;
  int mapValue = pgm_read_word_near(&theMap[yidx][xidx]);
  int prevMapValue = pgm_read_word_near(&theMap[yidx][xidx - 1]);
  int prevMapValue2 = pgm_read_word_near(&theMap[yidx - 1][xidx]);
  int nextMapValue = pgm_read_word_near(&theMap[yidx + 1][xidx]);
  int fuzzyMapValue = pgm_read_word_near(&theMap[yidx + 1][xidx - 1]);
  int fuzzyMapValue2 = pgm_read_word_near(&theMap[yidx - 1][xidx - 1]);
  int curY = pgm_read_word_near(&theMap[yidx][0]);
  int curX = pgm_read_word_near(&theMap[0][xidx]);
  int prevY = pgm_read_word_near(&theMap[yidx - 1][0]);
  int prevX = pgm_read_word_near(&theMap[0][xidx - 1]);

  float betweenL1 = (curX - x) / (curX - prevX);                              //*
  float calculatedLine1 = mapValue - (betweenL1 * (mapValue - prevMapValue)); //*

  if (y > curY)
  {
    float calculatedLine2 = nextMapValue - (betweenL1 * (nextMapValue - fuzzyMapValue));         //*
    float twoPoints = (y - curY) / (prevY - curY);                                               //*
    float calculatedPoint = calculatedLine2 - (twoPoints * (calculatedLine1 - calculatedLine2)); //*
  }
  if (y < curY)
  {
    float calculatedLine2 = prevMapValue2 - (betweenL1 * (prevMapValue2 - fuzzyMapValue2)); //*
    float twoPoints = (y - curY) / (curY - prevY);
    calculatedPoint = calculatedLine2 - (twoPoints * (calculatedLine1 - calculatedLine2));
  }
  else
  {
    float calculatedLine2 = calculatedLine1;
    int twoPoints = (y - curY) / (prevY - curY);
    calculatedPoint = calculatedLine2 - (twoPoints * (calculatedLine1 - calculatedLine2));
  }

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
