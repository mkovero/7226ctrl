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
int ReadGearMap(int theMap[14][13], int tps, int vehicleSpeed) {
  int idx = 0; // by default near first element

  int distance = abs(theMap[0][idx] - tps); 
  for (int i = 1; i < 12; i++)
  {
    int d = abs(theMap[0][i] - tps);
    if (d < distance)
    {
      idx = i;
      distance = d;
    }
  }
  int tpscolumn = idx;

  idx = 0;
  distance = abs(theMap[idx][0] - vehicleSpeed); 
  for (int i = 1; i < 14; i++)
  {
    int d = abs(theMap[i][0] - vehicleSpeed);
    if (d < distance)
    {
      idx = i;
      distance = d;
    }
  }
  int speedcolumn = idx;

  proposedGear = theMap[speedcolumn][tpscolumn];
  return proposedGear;
}
   

//function to read MPC and SPC pressure control maps, and also shift solenoid using time map
float ReadTwoAxisMap(int theMap[14][12], double verticAxis, double horizAxis) {
  double percentuallyBetweenAtLine1 = 0;
  double percentuallyBetweenAtLine2 = 0;
  double percentuallyBetweenTwoPointsAtColumn = 0;
  double calculatedFromLine1 = 0;
  double calculatedFromLine2 = 0;
  double calculatedShiftTime = 0;
  int mapLine = 0;
  int mapColumn = 0;

  if (horizAxis < -20) { horizAxis = -20; }
  if (horizAxis > 100) { horizAxis = 100; }
  
  int i;
  for (i = 1; i < 12; i++) {
    if (verticAxis == 0) { i = 1; }
    if (verticAxis <= theMap[0][i]) { break; }
  }
  
  mapColumn = i;
  int j;
  for (j = 1; j < 14; j++) {
    if (horizAxis <= theMap[j][0]) { break; }
  }

  mapLine = j;
  percentuallyBetweenAtLine1 = ((theMap[0][i] - verticAxis) / (theMap[0][i] - theMap[0][i - 1]));
  calculatedFromLine1 = theMap[mapLine][i] - (percentuallyBetweenAtLine1 * (theMap[mapLine][i] - theMap[mapLine][i - 1]));

  if (horizAxis > theMap[j][0]) {
    percentuallyBetweenAtLine2 = ((theMap[0][i] - verticAxis) / (theMap[0][i] - theMap[0][i - 1]));
    calculatedFromLine2 = theMap[mapLine + 1][i] - (percentuallyBetweenAtLine2 * (theMap[mapLine + 1][i] - theMap[mapLine + 1][i - 1]));
    percentuallyBetweenTwoPointsAtColumn = ((horizAxis - theMap[j][0]) / (theMap[j - 1][0] - theMap[j][0]));
    calculatedShiftTime = calculatedFromLine2 - (percentuallyBetweenTwoPointsAtColumn * (calculatedFromLine1 - calculatedFromLine2));
  } else if (horizAxis < theMap[j][0]) {
    percentuallyBetweenAtLine2 = ((theMap[0][i] - verticAxis) / (theMap[0][i] - theMap[0][i - 1]));
    calculatedFromLine2 = theMap[mapLine - 1][i] - (percentuallyBetweenAtLine2 * (theMap[mapLine - 1][i] - theMap[mapLine - 1][i - 1]));
    percentuallyBetweenTwoPointsAtColumn = ((horizAxis - theMap[j][0]) / (theMap[j][0] - theMap[j - 1][0]));
    calculatedShiftTime = calculatedFromLine1 - (percentuallyBetweenTwoPointsAtColumn * (calculatedFromLine2 - calculatedFromLine1)); 
  } else {
    calculatedFromLine2 = calculatedFromLine1;
    percentuallyBetweenTwoPointsAtColumn = ((horizAxis - theMap[j][0]) / (theMap[j - 1][0] - theMap[j][0]));
    calculatedShiftTime = calculatedFromLine2 - (percentuallyBetweenTwoPointsAtColumn * (calculatedFromLine1 - calculatedFromLine2));
  }            
  return calculatedShiftTime;
}