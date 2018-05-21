// OUTPUT PINS
// Solenoid output pins

// Color placeholder
// Orange / Orange-White = 12V key on (fused)
// Brown / Brown-White = 5V key on
// Blue = Speedo control voltage
// Blue-White = NA
// Green = Fuel pump control ground
// Green-White = TPS signal

#define MEGA // we're running this on arduino mega other option is TEENSY

#ifdef MEGA
#define y3 47
#define y4 45
#define y5 46
#define mpc 9
#define spc 10
#define tcc 41
#define rpmMeter 12
#define boostCtrl 44
#define speedoCtrl 5
#define fuelPumpCtrl 4
#endif

#ifdef TEENSY
#define y3 36 // FMT3, orange<->brown/red
#define y4 35 // orange <-> brown/grey
#define y5 34 //ex 14, orange <-> brown/black
#define mpc 38 // red <-> brown/pink
#define spc 37 // red <-> brown/yellow
#define tcc 8 // pink <-> brown/yellow/white
#define speedoCtrl 7 // ex 7, blue <-> blue/green
#define rpmMeter 30 // FMT2 missing
#define boostCtrl 4 // FMT1, green <-> green/white/yellow
#define fuelPumpCtrl 3 // missing
#endif
// END OUTPUT PINS

// INPUT PINS
// Stick input
#define whitepin 27 // 0.5kohm <-> yellow <-> missing
#define bluepin 29 // 0.5kohm <-> yellow <-> missing
#define greenpin 26 // 0.5kohm <-> yellow <-> missing
#define yellowpin 28 // 0.5kohm <-> yellow <-> missing

// Switches
#define autoSwitch 51 // ex. 22, 0.5kohm <-> yellow <-> missing
#define gdownSwitch 25 // ex. 23 <-> NC
#define gupSwitch 24 // <-> NC

#ifdef MEGA
#define aSpcUpSwitch 39 // ex 18 <-> missing
#define aSpcDownSwitch 40 // ex 17 <-> missing
#endif

#ifdef TEENSY
#define aSpcUpSwitch 32 // ex 18
#define aSpcDownSwitch 31 // ex 17
#endif
// Car sensor input pins, black
#define tpsPin A0 // voltage div 5/3 <-> black <-> missing = 1kohm/1.8kohm div
#define atfPin A1 // voltage div 5/3 <-> black <-> missing = 1kohm/1.8kohm div
#define boostPin A2 // voltage div 5/3 <-> black <-> missing = 1kohm/1.8kohm div
#define oilPin A3 // voltage div 12/3 <-> black <-> missing = 1kohm/380ohm div
#define n2pin 19 // voltage div 5/3 <-> black <-> missing = 1kohm/1.8kohm div
#define n3pin 20 // voltage div 5/3 <-> black <-> missing = 1kohm/1.8kohm div
#define speedPin 21 // voltage div 12/3 <-> black <-> missing = 1kohm/380ohm div
#define rpmPin 18 // voltage div 12/3 <-> black <-> missing = 1kohm/380ohm div