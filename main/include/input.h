int pollstick();
void gearup();
void geardown();
void pollkeys();
void polltrans(int newGear, int wantedGear);
int evaluateGear(float ratio);
void pollBoostControl();
int adaptSPC(int mapId, int xVal, int yVal);