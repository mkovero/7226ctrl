#include <SoftTimer.h>

void switchGearStart(int cSolenoid, int spcVal, int mpcVal);
void switchGearStop();
void gearchangeUp(int newGear);
void gearchangeDown(int newGear);
void decideGear(Task* me);

extern int gear;         
extern int cSolenoid;  
extern int vehicleSpeed;
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
extern int wantedGear;
extern int newGear;