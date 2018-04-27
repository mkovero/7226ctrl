#include <SoftTimer.h>

void pollstick(Task* me);
void gearup();
void geardown();
void pollkeys();
void polltrans(Task* me);
int evaluateGear(float ratio);
void pollBoostControl();
void adaptSPC(int mapId, int xVal, int yVal);