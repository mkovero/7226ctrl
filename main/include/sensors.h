#include <SoftTimer.h>

int tpsRead();
void N2SpeedInterrupt();
void N3SpeedInterrupt();
void vehicleSpeedInterrupt();
void pollsensors(Task* me);
int rpmRead();
int boostRead();
int boostLimitRead();
int loadRead();
int atfRead();
int oilRead();

extern boolean trans;
extern boolean sensors;
extern boolean tpsSensor;
extern boolean boostSensor;
extern boolean debugEnabled;
extern boolean boostLimit;
extern boolean drive;
