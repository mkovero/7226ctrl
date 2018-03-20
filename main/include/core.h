void switchGearStart(int cSolenoid, int spcVal, int mpcVal);
void switchGearStop(int cSolenoid, int newGear);
void gearchangeUp(int newGear);
void gearchangeDown(int newGear);
int decideGear(int wantedGear);
void boostControl();