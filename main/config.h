// OUTPUT PINS
// SPI display setup
#define OLED_MOSI  11   //D1
#define OLED_CLK   13   //D0
#define OLED_DC    9
#define OLED_CS    10
#define OLED_RESET 8

// Solenoid output pins
// #define y3 47
#define y3 2
// #define y4 45
#define y4 3
// #define y5 46
#define y5 5
// #define mpc 44
#define mpc 6
// #define spc 43
#define spc 7
#define tcc 42
#define rpmMeter 12
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

// Car sensor input pins
#define tpsPin A0
#define atfPin A1
#define boostPin A2
#define oilPin A3
#define n2pin 19
#define n3pin 20
#define speedPin 21
#define rpmPin 18

// map & rpm and load input coming here also.
// END INPUT PINS

// Display configuration
U8GLIB_SSD1306_128X64 u8g(13, 11, 10, 9,8);

// Internals, states
int gear = 2; // Start on gear 2
int wantedGear = gear; // Gear that is requested
int newGear = gear; // Gear that is going to be changed
int prevgear = 1; // Previously changed gear
int cSolenoid = 0; // Change solenoid pin to be controlled.
int vehicleSpeed = 200;
unsigned long lastSensorTime = 0;
int spcVal = 0;
int n2SpeedPulses = 0;
int n3SpeedPulses = 0;
int vehicleSpeedPulses = 0;
extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern void *__brkval;
// End of internals

// Environment configuration

// Shift delay
unsigned long int shiftStartTime = 0;
unsigned long int shiftDuration = 0;
int cSolenoidEnabled = 0;

// Are we in a real car?
boolean incar = false; // no.

// Do we use stick control?
boolean stick = true; // yes.

// Manual microswitch control?
boolean manual = false;

// Actual transmission there?
boolean trans = true;

// Are we using sensors?
boolean sensors = true;

// Do we have TPS?
boolean tpsSensor = true;

// Do we have boost sensor?
boolean boostSensor = false;

// Default for blocking gear switches (do not change.)
boolean shiftBlocker = false;

// Default for health (do not change.)
boolean health = false;

// Output to serial console
boolean debugEnabled = true;

// Full automatic mode
boolean fullAuto = true; 

boolean drive = false;


// End of environment conf
