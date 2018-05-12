#include <SoftTimer.h>

void switchGearStart(int cSolenoid, int spcVal, int mpcVal);
void switchGearStop();
void gearchangeUp(int newGear);
void gearchangeDown(int newGear);
void decideGear(Task* me);

extern byte gear;         
extern int cSolenoid;  
extern int spcSetVal;
extern int spcPercentVal;
extern unsigned long int shiftStartTime;
extern unsigned long int shiftDuration;
extern int cSolenoidEnabled;
extern int lastMapVal,lastXval,lastYval;
extern boolean trans;
extern boolean sensors;
extern boolean shiftBlocker;
extern boolean debugEnabled;
extern byte wantedGear;
extern byte newGear;
extern byte pendingGear;
extern boolean shiftPending;
