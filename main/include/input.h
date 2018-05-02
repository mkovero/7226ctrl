#include <SoftTimer.h>
void pollstick(Task* me);
void gearup();
void geardown();
void pollkeys();
void polltrans(Task* me);
int evaluateGear(float ratio);
void pollBoostControl();
int adaptSPC(int mapId, int xVal, int yVal);