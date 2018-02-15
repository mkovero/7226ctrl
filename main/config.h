// OUTPUT PINS
// SPI display setup
#define OLED_MOSI  11   //D1
#define OLED_CLK   13   //D0
#define OLED_DC    9
#define OLED_CS    10
#define OLED_RESET 8

// Solenoid output pins
const int y3 = 47;
const int y4 = 45;
const int y5 = 46;
const int mpc = 44;
const int spc = 43;
const int tcc = 42;
// END OUTPUT PINS

// INPUT PINS
// Stick input 
const int whitepin = 27;
const int bluepin = 29;
const int greenpin = 26;
const int yellowpin = 28;

// Switches
const int tempSwitch = 22;
const int gdownSwitch = 23;
const int gupSwitch = 24;

// Car sensor input pins
const int tpsPin = A0;
const int boostPin = A1;
const int n2pin = 19;
const int n3pin = 20;
const int speedPin = 21;
const int atfPin = A1;

// map & rpm and load input coming here also.
// END INPUT PINS

// Display config
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// Internals, states
int gear = 2; // Start on gear 2
int wantedGear = gear; // Gear that is requested
int newGear = gear; // Gear that is going to be changed
int prevgear = 1; // Previously changed gear
int cSolenoid = 0; // Change solenoid pin to be controlled.
int vehicleSpeed = 0;
int n2SpeedPulses = 0;
int n3SpeedPulses = 0;
int vehicleSpeedPulses = 0;
const int n2PulsesPerRev = 60;
const int n3PulsesPerRev = 60;
const int vehicleSpeedPulsesPerRev = 60;
int n2Speed = 0;
int n3Speed = 0;
float lastSensorTime = 0;
float boostVoltage = 0;
int boostPercentValue = 0;
int tpsPercentValue = 0;
float tpsVoltage = 0;
int trueLoad = 0;
int atfTempRaw = 0;
int atfTempCalculated = 0;
int atfTemp = 0;
int oilTemp = 0;
int spcVal = 0;
int mpcVal = 0;
// End of internals

// Environment configuration

// Shift delay
unsigned long int shiftDelay = 500;
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