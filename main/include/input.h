#include <SoftTimer.h>

void pollstick(Task* me);
void gearUp();
void gearDown();
void polltrans(Task* me);
void boostControl(Task* me);
void fuelControl(Task* me);
int adaptSPC(int mapId, int xVal, int yVal);
void adaptSPCup();
void adaptSPCdown();
void radioControl();
void onReleased(unsigned long pressTimespanMs);

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
extern boolean ignition;
extern boolean fuelPumps;
extern boolean radioEnabled;
extern boolean stickCtrl;
extern boolean horn;
extern boolean manual;
extern boolean truePower;
extern byte page;