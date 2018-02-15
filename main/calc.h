#define LEN(arr) ((int) (sizeof (arr) / sizeof (arr)[0]))

// Calculation helpers
int readTPSVoltage(int voltage) { 
  result = map(voltage,400,2830,0,100);
  return result;
}

int readBoostVoltage(int voltage) { 
  result = map(voltage,1500,4200,0,100);
  return result;
}

//function to read 2d maps
int readMap(int theMap[14][12], int x, int y) {
  int xidx = 0; // by default near first element
  int xelements = LEN(theMap[0]);

  int distance = abs(theMap[0][xidx] - x); 
  for (int i = 1; i < xelements; i++)
  {
    int d = abs(theMap[0][i] - x);
    if (d < distance)
    {
      xidx = i;
      distance = d;
    }
  }
  int yidx = 0; // by default near first element
  int yelements = LEN(*theMap);

  distance = abs(theMap[yidx][0] - y); 
  for (int i = 1; i < yelements; i++)
  {
    int d = abs(theMap[i][0] - y);
    if (d < distance)
    {
      yidx = i;
      distance = d;
    }
  }

  int mapValue = theMap[yidx][xidx];
  return mapValue;
}