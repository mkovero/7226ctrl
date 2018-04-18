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
#define speedoCtrl 43
#define speedoDir 42
#endif

#ifdef TEENSY
#define y3 36 // FMT3
#define y4 35
#define y5 34 //ex 14
#define mpc 38
#define spc 37
#define tcc 8
#define speedoCtrl 7 // ex 7
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
#define autoSwitch 5 // ex. 22
#define gdownSwitch 25 // ex. 23
#define gupSwitch 24

#ifdef MEGA
#define aSpcUpSwitch 39 // ex 18
#define aSpcDownSwitch 40 // ex 17
#endif

#ifdef TEENSY
#define aSpcUpSwitch 32 // ex 18
#define aSpcDownSwitch 31 // ex 17
#endif
// Car sensor input pins
#define tpsPin A0
#define atfPin A1
#define boostPin A2
#define oilPin A3
#define n2pin 19
#define n3pin 20
#define speedPin 21
#define rpmPin 18