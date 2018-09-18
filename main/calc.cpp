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
 
  int result = map(voltage, 450, 2670, 0, 3000); // NXP MPX5700AP (range 0-700kPa)
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

// Function to read 2d maps from flash (maps declared with PROGMEM)
int readBoostMap(const int theMap[14][6], int x, int y)
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
