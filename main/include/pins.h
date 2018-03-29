// OUTPUT PINS
// Solenoid output pins
#ifdef MEGA
#define y3 47
#define y4 45
#define y5 46
#define mpc 9
#define spc 10
#define tcc 41
#define rpmMeter 12
#define boostCtrl 44
#define speedoCtrl 43
#define speedoDir 42
#endif
#ifdef TEENSY
#define y3 36 // FMT3
#define y4 35
#define y5 14
#define mpc 38
#define spc 37
#define tcc 8
#define speedoCtrl 7
#define speedoDir 2 // END OF FMT3
#define rpmMeter 30 // FMT2
#define boostCtrl 4 // FMT1
#endif
// END OUTPUT PINS

// INPUT PINS
// Stick input
#define whitepin 27
#define bluepin 29
#define greenpin 26
#define yellowpin 28

// Switches
#define autoSwitch 22
#define gdownSwitch 23
#define gupSwitch 24
#define aSpcUpSwitch 18
#define aSpcDownSwitch 17
// Car sensor input pins
#define tpsPin A0
#define atfPin A1
#define boostPin A2
#define oilPin A3
#define n2pin 19
#define n3pin 20
#define speedPin 21
#define rpmPin 18