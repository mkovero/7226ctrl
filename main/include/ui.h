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
extern byte wantedGear;
extern int lastMapVal;

#ifdef __arm__
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__