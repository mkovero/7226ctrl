#define LEN(arr) ((int) (sizeof (arr) / sizeof (arr)[0]))

// Calculation helpers
float ReadSensorMap(int sensorMap[2][2], double voltage) { 
  double percentuallyBetweenTwoPointsAtMap = 0;
  double result = 0;
  
  for (int i = 0; i < 2; i++) {
    if (voltage <= sensorMap[1][i]) {
      if (voltage <= sensorMap[1][0]) {
        result = sensorMap[0][0];
      } else {
        percentuallyBetweenTwoPointsAtMap = ((sensorMap[1][i] - voltage) / (sensorMap[1][i] - sensorMap[1][i - 1]));
        result = sensorMap[0][i] - (percentuallyBetweenTwoPointsAtMap * (sensorMap[0][i] - sensorMap[0][i - 1]));
      }
        break;
    }

    if (voltage >= sensorMap[1][1]) {
      result = sensorMap[0][1];
    }
  }
  return result;
}

//function to read automode proposedGear map
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