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
