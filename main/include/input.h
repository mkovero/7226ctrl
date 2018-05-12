#include <SoftTimer.h>
void pollstick(Task* me);
void gearup();
void geardown();
void pollkeys();
void polltrans(Task* me);
int evaluateGear(float ratio);
void boostControl(Task* me);
void fuelControl(Task* me);
int adaptSPC(int mapId, int xVal, int yVal);

extern int spcPercentVal;       
extern unsigned long int shiftStartTime;
extern unsigned long int shiftDuration;
extern boolean trans;
extern boolean boostSensor;
extern boolean shiftBlocker;
extern boolean debugEnabled;
extern boolean boostLimit;
extern int n2Speed;
extern int n3Speed;
extern boolean fullAuto;
extern boolean fuelPumpControl;
extern boolean shiftPending;
