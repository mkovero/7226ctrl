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
int freeMemory();