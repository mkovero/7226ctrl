#include <SoftTimer.h>
void draw(int wantedGear, int loopTime);
void rpmMeterUpdate();
void updateDisplay(Task* me);
void datalog(Task* me);
void updateSpeedo();

extern byte gear;         
extern boolean debugEnabled;
extern boolean fullAuto;
extern boolean datalogger;
extern boolean w124speedo;
extern boolean w124rpm;
extern boolean infoBoost;
extern byte wantedGear,page;
extern int lastMapVal;
extern boolean shiftPending,truePower;
extern float ratio;
extern int n2Speed, n3Speed;
extern float gearSlip;
extern unsigned long fuelIn, fuelOut;
#ifdef __arm__
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__