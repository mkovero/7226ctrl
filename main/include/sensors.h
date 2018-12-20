#include <SoftTimer.h>

struct SensorVals
{
    int curOilTemp, curExTemp, curBoost, curExPres, curAtfTemp, curRPM, curBoostLim, curEvalGear, curBattery, curPresDiff, fuelUsed, fuelUsedAvg;
    byte curTps, curLoad, curSpeed;
    float curSlip, curRatio;
};

void N2SpeedInterrupt();
void N3SpeedInterrupt();
void vehicleSpeedInterrupt();
void rpmInterrupt();
void fuelInInterrupt();
void fuelOutInterrupt();
void pollsensors(Task* me);
int speedRead();
int tpsRead();
int rpmRead();
int boostRead();
int exhaustPressureRead();
int exhaustTempRead();
int boostLimitRead(int oilTemp, int tps);
int loadRead(int curTps, int curBoost, int curBoostLim, int curRPM);
int atfRead();
int oilRead();
int batteryRead();
struct SensorVals readSensors();

extern boolean trans;
extern boolean sensors;
extern boolean tpsSensor;
extern boolean boostSensor;
extern boolean debugEnabled;
extern boolean boostLimit;
extern boolean drive;
extern boolean diffSpeed;
extern boolean rpmSpeed;
extern boolean batteryMonitor, exhaustPresSensor, carRunning;
extern int evalGearVal;