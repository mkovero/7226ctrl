#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Work by Markus Kovero <mui@mui.fi>
// Big thanks to Tuomas Kantola regarding maps and related math

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
float n2Speed = 0;
float n3Speed = 0;
float lastSensorTime = 0;
float boostVoltage = 0;
float boostPercentValue = 0;
float tpsPercentValue = 0;
float tpsVoltage = 0;
int trueLoad = 0;
int atfTempRaw = 0;
int atfTempCalculated = 0;
int atfTemp = 0;

//TPS linearisation map
int tpsLinearisationMap[2][2] {
{0,   100 },   //sensor value as a result %
{400, 2830 }}; //corresponding sensor voltage, mV

//Boost linearisation map
int boostLinearisationMap[2][2] {
{0,   100 },   //sensor value as a result %
{1500, 4200 }}; //corresponding sensor voltage, mV

//Automatic mode autoGear map
int gearMap[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //throttle position %
//-----------------------------------------------------------------------
{   0,       2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2 },
{  10,       2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2 },
{  20,       3,    3,    3,    3,    3,    3,    2,    2,    2,    2,    2 },
{  30,       3,    3,    3,    3,    3,    3,    3,    2,    2,    2,    2 },
{  45,       4,    4,    4,    4,    4,    4,    4,    3,    3,    3,    3 },
{  50,       4,    4,    4,    4,    4,    4,    4,    3,    3,    3,    3 },
{  60,       5,    5,    5,    5,    5,    5,    5,    4,    4,    4,    4 },
{  70,       5,    5,    5,    5,    5,    5,    5,    5,    5,    4,    4 },
{  80,       5,    5,    5,    5,    5,    5,    5,    5,    5,    4,    4 },
{  90,       5,    5,    5,    5,    5,    5,    5,    5,    5,    4,    4 },
{ 100,       5,    5,    5,    5,    5,    5,    5,    5,    5,    4,    4 },
{ 110,       5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5 },
{ 120,       5,    5,    5,    5,    5,    5,    5,    5,    5,    5,    5 }};
//vehicle speed, km/h

//MPC map in normal drive (=outside shifts. This is just to make a better mileage, MPC pressure could be 100% outside shifts too.)
int mpcNormalMap[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //load %
//-----------------------------------------------------------------------
{ -20,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{ -10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{   0,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  20,      20,   25,   30,   30,   35,   53,   83,   93,   93,   95,  100 },
{  30,      15,   20,   25,   25,   30,   53,   80,   90,   90,   95,  100 },
{  40,      10,   15,   20,   20,   28,   53,   80,   85,   85,   90,  100 },
{  50,      10,   15,   20,   20,   28,   53,   80,   85,   85,   90,  100 },
{  60,      10,   15,   20,   20,   27,   53,   80,   85,   85,   90,  100 },
{  70,      10,   15,   20,   20,   27,   53,   80,   85,   85,   90,  100 },
{  80,       9,   15,   20,   20,   27,   53,   80,   85,   85,   90,  100 },
{  90,       9,   15,   20,   20,   27,   53,   80,   85,   85,   90,  100 },
{ 100,       9,   15,   20,   20,   27,   53,   80,   85,   85,   90,  100 }};
//oil temp

//Shift solenoid using time map, ms. Time to keep current flowing to shift pressure solenoid (SPC), shift solenoid,
//and to use shift-depedent maps for modulating pressure control (MPC) solenoid, instead of above map.
int shiftTimeMap[14][12] {
{999,       0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //shift pressure %
//-----------------------------------------------------------------------
{ -20,    1100, 1100, 1100, 1100, 1100, 1100, 1100, 1100,  900,  800,  800 },
{ -10,    1100, 1100, 1100, 1100, 1100, 1100, 1100,  900,  900,  800,  800 },
{   0,    1100, 1100, 1100, 1100, 1100, 1100,  900,  900,  900,  800,  800 },
{  10,    1100, 1100, 1100, 1100, 1100, 1100,  900,  900,  900,  800,  800 },
{  20,    1100, 1100, 1100, 1000, 1000, 1000,  900,  900,  900,  800,  800 },
{  30,    1100, 1100, 1100, 1000, 1000,  900,  900,  800,  700,  700,  600 },
{  40,    1000, 1000, 1000, 1000,  900,  800,  700,  700,  700,  500,  500 },
{  50,    1000, 1000, 1000,  900,  900,  800,  700,  700,  500,  500,  500 },
{  60,    1000, 1000, 1000,  900,  800,  700,  600,  500,  450,  450,  450 },
{  70,    1000, 1000,  900,  900,  800,  700,  600,  500,  450,  320,  320 },
{  80,    1000,  900,  900,  800,  800,  700,  600,  500,  320,  300,  300 },
{  90,    1000,  900,  800,  800,  800,  700,  600,  500,  320,  300,  300 },
{ 100,    1000,  900,  800,  800,  800,  700,  600,  500,  320,  300,  300 }};
//oil temp




//Shift maps
//******************************************* 1 -> 2 *******************************************

//SPC map, 1 -> 2
int spcMap12[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //load %
//-----------------------------------------------------------------------
{ -20,      36,   41,   51,   61,   61,   71,   78,   88,  100,  100,  100 },
{ -10,      36,   41,   51,   61,   61,   71,   78,   88,  100,  100,  100 },
{   0,      36,   41,   51,   61,   61,   71,   78,   88,  100,  100,  100 },
{  10,      36,   41,   51,   61,   61,   71,   78,   88,  100,  100,  100 },
{  20,      23,   28,   38,   48,   48,   58,   68,   68,  100,  100,  100 },
{  30,      18,   23,   33,   43,   43,   53,   63,   63,  100,  100,  100 },
{  40,      17,   22,   32,   42,   42,   52,   62,   62,  100,  100,  100 },
{  50,      16,   21,   31,   41,   41,   51,   61,   61,  100,  100,  100 },
{  60,      15,   20,   25,   25,   25,   50,   60,   60,  100,  100,  100 },
{  70,      15,   20,   25,   25,   25,   50,   60,   60,  100,  100,  100 },
{  80,      12,   17,   22,   22,   22,   47,   60,   60,  100,  100,  100 },
{  90,      12,   17,   22,   22,   22,   47,   60,   60,  100,  100,  100 },
{ 100,      12,   17,   22,   22,   22,   47,   60,   60,  100,  100,  100 }};
//oil temp

//MPC map, 1 -> 2
int mpcMap12[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //load %
//-----------------------------------------------------------------------
{ -20,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{ -10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{   0,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  20,      20,   25,   30,   30,   35,   53,   83,   93,   93,   95,  100 },
{  30,      15,   20,   25,   25,   30,   50,   80,   90,   90,   95,  100 },
{  40,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  50,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  60,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  70,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  80,       8,   13,   18,   18,   23,   50,   75,   85,   85,   90,  100 },
{  90,       8,   13,   18,   18,   23,   50,   75,   85,   85,   90,  100 },
{ 100,       8,   13,   18,   18,   23,   50,   75,   85,   85,   90,  100 }};
//oil temp




//******************************************* 2 -> 3 *******************************************

//SPC map, 2 -> 3
int spcMap23[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //load %
//-----------------------------------------------------------------------
{ -20,      41,   41,   46,   56,   66,   66,   73,   83,  100,  100,  100 },
{ -10,      41,   41,   46,   56,   66,   66,   73,   83,  100,  100,  100 },
{   0,      41,   41,   46,   56,   66,   66,   73,   83,  100,  100,  100 },
{  10,      41,   41,   46,   56,   66,   66,   73,   83,  100,  100,  100 },
{  20,      33,   33,   38,   48,   58,   58,   68,   78,  100,  100,  100 },
{  30,      30,   30,   35,   45,   55,   58,   65,   75,  100,  100,  100 },
{  40,      27,   27,   32,   42,   52,   58,   62,   72,  100,  100,  100 },
{  50,      26,   26,   31,   41,   51,   58,   61,   71,  100,  100,  100 },
{  60,      23,   23,   25,   38,   40,   58,   61,   71,  100,  100,  100 },
{  70,      23,   23,   25,   38,   40,   58,   61,   71,  100,  100,  100 },
{  80,      21,   21,   23,   36,   38,   58,   61,   71,  100,  100,  100 },
{  90,      21,   21,   23,   36,   38,   58,   61,   71,  100,  100,  100  },
{ 100,      21,   21,   23,   36,   38,   58,   61,   71,  100,  100,  100  }};
//oil temp

//MPC map, 2 -> 3
int mpcMap23[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //load %
//-----------------------------------------------------------------------
{ -20,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{ -10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{   0,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  20,      20,   25,   30,   30,   35,   53,   83,   93,   93,   95,  100 },
{  30,      15,   20,   25,   25,   30,   50,   80,   90,   90,   95,  100 },
{  40,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  50,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  60,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  70,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  80,       8,   13,   18,   18,   23,   50,   75,   85,   85,   90,  100 },
{  90,       8,   13,   18,   18,   23,   50,   75,   85,   85,   90,  100 },
{ 100,       8,   13,   18,   18,   23,   50,   75,   85,   85,   90,  100 }};
//oil temp



//******************************************* 3 -> 4 *******************************************

//SPC map, 3 -> 4
int spcMap34[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //load %
//-----------------------------------------------------------------------
{ -20,      58,   75,   95,  100,  100,  100,  100,  100,  100,  100,  100 },
{ -10,      58,   75,   95,  100,  100,  100,  100,  100,  100,  100,  100 },
{   0,      58,   75,   95,  100,  100,  100,  100,  100,  100,  100,  100 },
{  10,      58,   75,   95,  100,  100,  100,  100,  100,  100,  100,  100 },
{  20,      58,   75,   95,  100,  100,  100,  100,  100,  100,  100,  100 },
{  30,      58,   75,   95,  100,  100,  100,  100,  100,  100,  100,  100 },
{  40,      58,   75,   95,  100,  100,  100,  100,  100,  100,  100,  100 },
{  50,      58,   75,   95,  100,  100,  100,  100,  100,  100,  100,  100 },
{  60,      58,   75,   95,  100,  100,  100,  100,  100,  100,  100,  100 },
{  70,      58,   75,   95,  100,  100,  100,  100,  100,  100,  100,  100 },
{  80,      58,   75,   95,  100,  100,  100,  100,  100,  100,  100,  100 },
{  90,      58,   75,   95,  100,  100,  100,  100,  100,  100,  100,  100 },
{ 100,      58,   75,   95,  100,  100,  100,  100,  100,  100,  100,  100 }};
//oil temp

//MPC map, 3 -> 4
int mpcMap34[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //load %
//-----------------------------------------------------------------------
{ -20,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{ -10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{   0,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  20,      20,   25,   30,   30,   35,   53,   83,   93,   93,   95,  100 },
{  30,      50,   75,   75,   75,   75,   75,   85,   95,  100,  100,  100 },
{  40,      50,   75,   75,   95,   95,  100,  100,  100,  100,  100,  100 },
{  50,      50,   75,   75,   95,   95,  100,  100,  100,  100,  100,  100 },
{  60,      50,   75,   75,   95,   95,  100,  100,  100,  100,  100,  100 },
{  70,      50,   75,   75,   95,   95,  100,  100,  100,  100,  100,  100 },
{  80,      50,   75,   75,   95,   95,  100,  100,  100,  100,  100,  100 },
{  90,      50,   75,   75,   95,   95,  100,  100,  100,  100,  100,  100 },
{ 100,      50,   75,   75,   95,   95,  100,  100,  100,  100,  100,  100 }};
//oil temp



//******************************************* 4 -> 5 *******************************************

//SPC map, 4 -> 5
int spcMap45[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //load %
//-----------------------------------------------------------------------
{ -20,      55,   65,   65,   65,   65,   65,   75,   75,  100,  100,  100 },
{ -10,      55,   65,   65,   65,   65,   65,   75,   75,  100,  100,  100 },
{   0,      55,   65,   65,   65,   65,   65,   75,   75,  100,  100,  100 },
{  10,      55,   65,   65,   65,   65,   65,   75,   75,  100,  100,  100 },
{  20,      50,   60,   60,   60,   60,   60,   70,   70,  100,  100,  100 },
{  30,      43,   53,   53,   53,   53,   53,   63,   63,  100,  100,  100 },
{  40,      42,   52,   52,   52,   52,   52,   62,   62,  100,  100,  100 },
{  50,      40,   50,   50,   50,   50,   50,   60,   60,  100,  100,  100 },
{  60,      40,   50,   50,   50,   50,   50,   60,   60,  100,  100,  100 },
{  70,      40,   50,   50,   50,   50,   50,   60,   60,  100,  100,  100 },
{  80,      40,   50,   50,   50,   50,   50,   60,   60,  100,  100,  100 },
{  90,      40,   50,   50,   50,   50,   50,   60,   60,  100,  100,  100 },
{ 100,      40,   50,   50,   50,   50,   50,   60,   60,  100,  100,  100 }};
//oil temp

//MPC map, 4 -> 5
int mpcMap45[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //load %
//-----------------------------------------------------------------------
{ -20,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{ -10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{   0,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  20,      20,   25,   30,   30,   35,   53,   83,   93,   93,   95,  100 },
{  30,      15,   20,   25,   25,   30,   50,   80,   90,   90,   95,  100 },
{  40,      10,   16,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  50,      10,   16,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  60,      10,   16,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  70,      10,   16,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  80,      10,   16,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  90,      10,   16,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{ 100,      10,   16,   20,   20,   25,   50,   75,   85,   85,   90,  100 }};
//oil temp



//******************************************* 5 -> 4 *******************************************

//SPC map, 5 -> 4
int spcMap54[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //load %
//-----------------------------------------------------------------------
{ -20,      58,   68,   93,   98,  100,  100,  100,  100,  100,  100,  100 },
{ -10,      58,   68,   93,   98,  100,  100,  100,  100,  100,  100,  100 },
{   0,      58,   68,   93,   98,  100,  100,  100,  100,  100,  100,  100 },
{  10,      58,   68,   93,   98,  100,  100,  100,  100,  100,  100,  100 },
{  20,      58,   68,   93,   98,  100,  100,  100,  100,  100,  100,  100 },
{  30,      58,   68,   93,   98,  100,  100,  100,  100,  100,  100,  100 },
{  40,      44,   54,   84,   94,   95,  100,  100,  100,  100,  100,  100 },
{  50,      43,   53,   83,   93,   95,  100,  100,  100,  100,  100,  100 },
{  60,      42,   52,   82,   92,   95,  100,  100,  100,  100,  100,  100 },
{  70,      42,   52,   82,   92,   95,  100,  100,  100,  100,  100,  100 },
{  80,      42,   52,   82,   92,   95,  100,  100,  100,  100,  100,  100 },
{  90,      42,   52,   82,   92,   95,  100,  100,  100,  100,  100,  100 },
{ 100,      42,   52,   82,   92,   95,  100,  100,  100,  100,  100,  100 }};
//oil temp

//MPC map, 5 -> 4
int mpcMap54[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //load %
//-----------------------------------------------------------------------
{ -20,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{ -10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{   0,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  20,      20,   25,   30,   30,   35,   53,   83,   93,   93,   95,  100 },
{  30,      15,   20,   25,   25,   30,   50,   80,   90,   90,   95,  100 },
{  40,      15,   17,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  50,      15,   17,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  60,      15,   17,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  70,      15,   17,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  80,      15,   17,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  90,      15,   17,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{ 100,      15,   17,   20,   20,   25,   50,   75,   85,   85,   90,  100 }};
//oil temp



//******************************************* 4 -> 3 *******************************************

//SPC map: 4 -> 3
int spcMap43[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //load %
//-----------------------------------------------------------------------
{ -20,      51,   51,   51,   51,   61,   81,   88,   88,  100,  100,  100 },
{ -10,      51,   51,   51,   51,   61,   81,   88,   88,  100,  100,  100 },
{   0,      51,   51,   51,   51,   61,   81,   88,   88,  100,  100,  100 },
{  10,      51,   51,   51,   51,   61,   81,   88,   88,  100,  100,  100 },
{  20,      48,   48,   48,   48,   58,   78,   85,   85,  100,  100,  100 },
{  30,      38,   38,   38,   38,   43,   63,   73,   73,  100,  100,  100 },
{  40,      30,   30,   30,   30,   35,   55,   65,   65,  100,  100,  100 },
{  50,      28,   28,   28,   28,   33,   53,   63,   63,  100,  100,  100 },
{  60,      26,   26,   26,   26,   31,   51,   61,   61,  100,  100,  100 },
{  70,      25,   25,   25,   25,   30,   50,   60,   60,  100,  100,  100 },
{  80,      25,   25,   25,   25,   30,   50,   60,   60,  100,  100,  100 },
{  90,      25,   25,   25,   25,   30,   50,   60,   60,  100,  100,  100 },
{ 100,      25,   25,   25,   25,   30,   50,   60,   60,  100,  100,  100 }};
//oil temp

//MPC map, 4 -> 3
int mpcMap43[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //load %
//-----------------------------------------------------------------------
{ -20,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{ -10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{   0,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  20,      20,   25,   30,   30,   35,   53,   83,   93,   93,   95,  100 },
{  30,      15,   20,   25,   25,   30,   50,   80,   90,   90,   95,  100 },
{  40,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  50,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  60,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  70,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  80,       8,   13,   18,   18,   23,   50,   75,   85,   85,   90,  100 },
{  90,       8,   13,   18,   18,   23,   50,   75,   85,   85,   90,  100 },
{ 100,       8,   13,   18,   18,   23,   50,   75,   85,   85,   90,  100 }};
//oil temp



//******************************************* 3 -> 2 *******************************************

//SPC map, 3 -> 2
int spcMap32[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //load %
//-----------------------------------------------------------------------
{ -20,      50,   50,   50,   50,   70,   70,   80,   80,  100,  100,  100 },
{ -10,      50,   50,   50,   50,   70,   70,   80,   80,  100,  100,  100 },
{   0,      50,   50,   50,   50,   70,   70,   80,   80,  100,  100,  100 },
{  10,      50,   50,   50,   50,   70,   70,   80,   80,  100,  100,  100 },
{  20,      48,   48,   48,   48,   68,   68,   78,   85,  100,  100,  100 },
{  30,      40,   40,   40,   40,   60,   60,   70,   70,  100,  100,  100 },
{  40,      34,   34,   34,   34,   54,   54,   64,   64,  100,  100,  100 },
{  50,      32,   32,   32,   32,   52,   52,   62,   62,  100,  100,  100 },
{  60,      31,   31,   31,   31,   51,   51,   61,   61,  100,  100,  100 },
{  70,      30,   30,   30,   30,   50,   50,   60,   60,  100,  100,  100 },
{  80,      28,   28,   28,   28,   48,   50,   60,   60,  100,  100,  100 },
{  90,      28,   28,   28,   28,   48,   50,   60,   60,  100,  100,  100 },
{ 100,      28,   28,   28,   28,   48,   50,   60,   60,  100,  100,  100 }};
//oil temp

//MPC map, 3 -> 2
int mpcMap32[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //load %
//-----------------------------------------------------------------------
{ -20,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{ -10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{   0,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  20,      20,   25,   30,   30,   35,   53,   83,   93,   93,   95,  100 },
{  30,      15,   20,   25,   25,   30,   50,   80,   90,   90,   95,  100 },
{  40,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  50,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  60,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  70,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  80,       8,   13,   18,   18,   23,   50,   75,   85,   85,   90,  100 },
{  90,       8,   13,   18,   18,   23,   50,   75,   85,   85,   90,  100 },
{ 100,       8,   13,   18,   18,   23,   50,   75,   85,   85,   90,  100 }};
//oil temp



//******************************************* 2 -> 1 *******************************************

//SPC map, 2 -> 1
int spcMap21[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //load %
//-----------------------------------------------------------------------
{ -20,      45,   45,   45,   50,   70,   70,   80,   80,  100,  100,  100 },
{ -10,      45,   45,   45,   50,   70,   70,   80,   80,  100,  100,  100 },
{   0,      45,   45,   45,   50,   70,   70,   80,   80,  100,  100,  100 },
{  10,      45,   45,   45,   50,   70,   70,   80,   80,  100,  100,  100 },
{  20,      43,   43,   43,   48,   68,   68,   78,   78,  100,  100,  100 },
{  30,      28,   28,   28,   33,   53,   53,   63,   63,  100,  100,  100 },
{  40,      26,   26,   26,   31,   51,   51,   61,   61,  100,  100,  100 },
{  50,      25,   25,   25,   30,   50,   50,   60,   60,  100,  100,  100 },
{  60,      23,   23,   23,   30,   50,   50,   60,   60,  100,  100,  100 },
{  70,      23,   23,   23,   30,   50,   50,   60,   60,  100,  100,  100 },
{  80,      19,   21,   21,   28,   48,   50,   60,   60,  100,  100,  100 },
{  90,      19,   21,   21,   28,   48,   50,   60,   60,  100,  100,  100 },
{ 100,      19,   21,   21,   28,   48,   50,   60,   60,  100,  100,  100 }};
//oil temp


//MPC map, 2 -> 1
int mpcMap21[14][12] {
{999,        0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100 },   //load %
//-----------------------------------------------------------------------
{ -20,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{ -10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{   0,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  10,      23,   28,   33,   33,   38,   56,   86,   96,   96,   98,  100 },
{  20,      20,   25,   30,   30,   35,   53,   83,   93,   93,   95,  100 },
{  30,      15,   20,   25,   25,   30,   50,   80,   90,   90,   95,  100 },
{  40,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  50,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  60,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  70,      10,   15,   20,   20,   25,   50,   75,   85,   85,   90,  100 },
{  80,       8,   13,   18,   18,   23,   50,   75,   85,   85,   90,  100 },
{  90,       8,   13,   18,   18,   23,   50,   75,   85,   85,   90,  100 },
{ 100,       8,   13,   18,   18,   23,   50,   75,   85,   85,   90,  100 }};
//oil temp

Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

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

boolean fullAuto = true;

boolean drive = false;

// End of environment conf

void setup() {
  
   // TCCR2B = TCCR2B & 0b11111000 | 0x07;
  // MPC and SPC should have frequency of 1000hz
  // TCC should have frequency of 100hz
  // Lower the duty cycle, higher the pressures.
  Serial.begin(9600);

  display.begin(SSD1306_SWITCHCAPVCC);
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setTextSize(5);
  display.setTextColor(WHITE);
 
  // Solenoid outputs
  pinMode(y3, OUTPUT); // 1-2/4-5 solenoid
  pinMode(y4,OUTPUT); // 2-3
  pinMode(y5,OUTPUT); // 3-4
  pinMode(spc,OUTPUT); // shift pressure
  pinMode(mpc,OUTPUT); // modulation pressure
  pinMode(tcc,OUTPUT); // lock

  // Sensor input
  pinMode(boostPin,INPUT); // boost sensor 
  pinMode(tpsPin,INPUT); // throttle position sensor 
  pinMode(atfPin, INPUT); // ATF temp
  pinMode(n2pin,INPUT); // N2 sensor
  pinMode(n3pin,INPUT); // N3 sensor 
  pinMode(speedPin,INPUT); // vehicle speed
  
  //For manual control
  pinMode(gupSwitch,INPUT); // gear up
  pinMode(gdownSwitch,INPUT); // gear down
  
  //For stick control
  pinMode(whitepin,INPUT);
  pinMode(bluepin,INPUT);
  pinMode(greenpin,INPUT);
  pinMode(yellowpin,INPUT);

  // Drive solenoids are all off.
  analogWrite(y3,0);
  analogWrite(y4,0);
  analogWrite(y5,0);
  analogWrite(spc,0); // No pressure here by default.
  analogWrite(mpc,255); // We want constant pressure here.
  analogWrite(tcc,0); // No pressure here by default.
  
  Serial.println("Started.");
  updateDisplay();
}

// Calculation helpers
float ReadSensorMap(int sensorMap[2][2], double voltage) { 
  double percentuallyBetweenTwoPointsAtMap = 0;
  double result = 0;
  
  for (int i = 0; i < 2; i++) {
    if (voltage <= sensorMap[1][i]) {
      if (voltage <= sensorMap[1][0]) {
        result = sensorMap[0][0];
      } else {
        percentuallyBetweenTwoPointsAtMap = ((sensorMap[1][i] - voltage) / (sensorMap[1][i] - sensorMap[1][i - 1]));
        result = sensorMap[0][i] - (percentuallyBetweenTwoPointsAtMap * (sensorMap[0][i] - sensorMap[0][i - 1]));
      }
        break;
    }

    if (voltage >= sensorMap[1][1]) {
      result = sensorMap[0][1];
    }
  }
  return result;
}

//function to read automode proposedGear map
int ReadGearMap(int theMap[14][12], double tps, double vehicleSpeed) {
  double percentuallyBetweenAtLine1 = 0;
  double percentuallyBetweenAtLine2 = 0;
  double percentuallyBetweenTwoPointsAtColumn = 0;
  double calculatedFromLine1 = 0;
  double calculatedFromLine2 = 0;
  double proposedGear = 0;
  int mapLine = 0;
  int mapColumn = 0;

  int i;
  for (i = 1; i < 12; i++) { 
    if (tps == 0) { i = 1; } 
      if (tps <= theMap[0][i]) { break; } }
       
  mapColumn = i; int j;
  for (j = 1; j < 14; j++) { 
    if (vehicleSpeed <= theMap[j][0]) { break; } }

  mapLine = j;
  percentuallyBetweenAtLine1 = ((theMap[0][i] - tps) / (theMap[0][i] - theMap[0][i - 1]));
  calculatedFromLine1 = theMap[mapLine][i] - (percentuallyBetweenAtLine1 * (theMap[mapLine][i] - theMap[mapLine][i - 1]));

  if (vehicleSpeed > theMap[j][0]) {
    percentuallyBetweenAtLine2 = ((theMap[0][i] - tps) / (theMap[0][i] - theMap[0][i - 1]));
    calculatedFromLine2 = theMap[mapLine + 1][i] - (percentuallyBetweenAtLine2 * (theMap[mapLine + 1][i] - theMap[mapLine + 1][i - 1]));
    percentuallyBetweenTwoPointsAtColumn = ((vehicleSpeed - theMap[j][0]) / (theMap[j - 1][0] - theMap[j][0]));
    wantedGear = calculatedFromLine2 - (percentuallyBetweenTwoPointsAtColumn * (calculatedFromLine1 - calculatedFromLine2));
  } else if (vehicleSpeed < theMap[j][0]) {
    percentuallyBetweenAtLine2 = ((theMap[0][i] - tps) / (theMap[0][i] - theMap[0][i - 1]));
    calculatedFromLine2 = theMap[mapLine - 1][i] - (percentuallyBetweenAtLine2 * (theMap[mapLine - 1][i] - theMap[mapLine - 1][i - 1]));
    percentuallyBetweenTwoPointsAtColumn = ((vehicleSpeed - theMap[j][0]) / (theMap[j][0] - theMap[j - 1][0]));
    wantedGear = calculatedFromLine1 - (percentuallyBetweenTwoPointsAtColumn * (calculatedFromLine2 - calculatedFromLine1));
  } else {
    calculatedFromLine2 = calculatedFromLine1;
    percentuallyBetweenTwoPointsAtColumn = ((vehicleSpeed - theMap[j][0]) / (theMap[j - 1][0] - theMap[j][0]));
    wantedGear = calculatedFromLine2 - (percentuallyBetweenTwoPointsAtColumn * (calculatedFromLine1 - calculatedFromLine2));
  }            
  return proposedGear;
}
   

//function to read MPC and SPC pressure control maps, and also shift solenoid using time map
float ReadTwoAxisMap(int theMap[14][12], double verticAxis, double horizAxis) {
  double percentuallyBetweenAtLine1 = 0;
  double percentuallyBetweenAtLine2 = 0;
  double percentuallyBetweenTwoPointsAtColumn = 0;
  double calculatedFromLine1 = 0;
  double calculatedFromLine2 = 0;
  double calculatedShiftTime = 0;
  int mapLine = 0;
  int mapColumn = 0;

  if (horizAxis < -20) { horizAxis = -20; }
  if (horizAxis > 100) { horizAxis = 100; }
  
  int i;
  for (i = 1; i < 12; i++) {
    if (verticAxis == 0) { i = 1; }
    if (verticAxis <= theMap[0][i]) { break; }
  }
  
  mapColumn = i;
  int j;
  for (j = 1; j < 14; j++) {
    if (horizAxis <= theMap[j][0]) { break; }
  }

  mapLine = j;
  percentuallyBetweenAtLine1 = ((theMap[0][i] - verticAxis) / (theMap[0][i] - theMap[0][i - 1]));
  calculatedFromLine1 = theMap[mapLine][i] - (percentuallyBetweenAtLine1 * (theMap[mapLine][i] - theMap[mapLine][i - 1]));

  if (horizAxis > theMap[j][0]) {
    percentuallyBetweenAtLine2 = ((theMap[0][i] - verticAxis) / (theMap[0][i] - theMap[0][i - 1]));
    calculatedFromLine2 = theMap[mapLine + 1][i] - (percentuallyBetweenAtLine2 * (theMap[mapLine + 1][i] - theMap[mapLine + 1][i - 1]));
    percentuallyBetweenTwoPointsAtColumn = ((horizAxis - theMap[j][0]) / (theMap[j - 1][0] - theMap[j][0]));
    calculatedShiftTime = calculatedFromLine2 - (percentuallyBetweenTwoPointsAtColumn * (calculatedFromLine1 - calculatedFromLine2));
  } else if (horizAxis < theMap[j][0]) {
    percentuallyBetweenAtLine2 = ((theMap[0][i] - verticAxis) / (theMap[0][i] - theMap[0][i - 1]));
    calculatedFromLine2 = theMap[mapLine - 1][i] - (percentuallyBetweenAtLine2 * (theMap[mapLine - 1][i] - theMap[mapLine - 1][i - 1]));
    percentuallyBetweenTwoPointsAtColumn = ((horizAxis - theMap[j][0]) / (theMap[j][0] - theMap[j - 1][0]));
    calculatedShiftTime = calculatedFromLine1 - (percentuallyBetweenTwoPointsAtColumn * (calculatedFromLine2 - calculatedFromLine1)); 
  } else {
    calculatedFromLine2 = calculatedFromLine1;
    percentuallyBetweenTwoPointsAtColumn = ((horizAxis - theMap[j][0]) / (theMap[j - 1][0] - theMap[j][0]));
    calculatedShiftTime = calculatedFromLine2 - (percentuallyBetweenTwoPointsAtColumn * (calculatedFromLine1 - calculatedFromLine2));
  }            
  return calculatedShiftTime;
}

// UI STAGE
// Control for what user sees and how gearbox is used with
// 

// Display update
void updateDisplay() {
  display.clearDisplay();
  display.setCursor(3,0);
  display.setTextSize(5);
  if ( ! fullAuto ) {
    if ( prevgear <= 5 ) { display.print(prevgear); };
    if ( prevgear == 6 ) { display.print("N"); };
    if ( prevgear == 7 ) { display.print("R"); };
    if ( prevgear == 8 ) { display.print("P"); };
    display.print("->");
    if ( gear <= 5 ) { display.print(gear); };
    if ( gear == 6 ) { display.print("N"); };
    if ( gear == 7 ) { display.print("R"); };
    if ( gear == 8 ) { display.print("P"); }; 
  } else {
    if ( gear == 5 ) { display.print("D"); };
    if ( gear == 6 ) { display.print("N"); };
    if ( gear == 7 ) { display.print("R"); };
    if ( gear == 8 ) { display.print("P"); }; 
  }
  display.setTextSize(1);
  display.println(atfTemp);
  display.print(", ");
  display.print(vehicleSpeed);
  display.print(", ");
  display.print(tpsPercentValue);
  display.display();
}

// INPUT
// Polling for stick control
void pollstick() {
  // Read the stick.
  int whiteState = digitalRead(whitepin);
  int blueState = digitalRead(bluepin);
  int greenState = digitalRead(greenpin);
  int yellowState = digitalRead(yellowpin);

  // Determine position
  if (whiteState == HIGH && blueState == HIGH && greenState == HIGH && yellowState == LOW ) { wantedGear = 8; } // P
  if (whiteState == LOW && blueState == HIGH && greenState == HIGH && yellowState == HIGH ) { wantedGear = 7; } // R
  if (whiteState == HIGH && blueState == LOW && greenState == HIGH && yellowState == HIGH ) { wantedGear = 6; } // N
  if (whiteState == LOW && blueState == LOW && greenState == HIGH && yellowState == LOW ) { wantedGear = 5; }
  if (whiteState == LOW && blueState == LOW && greenState == LOW && yellowState == HIGH ) { wantedGear = 4; }
  if (whiteState == LOW && blueState == HIGH && greenState == LOW && yellowState == LOW ) { wantedGear = 3; }
  if (whiteState == HIGH && blueState == LOW && greenState == LOW && yellowState == LOW ) { wantedGear = 2; }
  if (whiteState == HIGH && blueState == HIGH && greenState == LOW && yellowState == HIGH ) { wantedGear = 1; }

  decideGear(wantedGear);
  
  if ( debugEnabled && wantedGear != gear ) {
    Serial.println("pollstick: Stick says: ");
    Serial.print(whiteState);
    Serial.print(blueState);
    Serial.print(greenState);
    Serial.print(yellowState);
    Serial.println("pollstick: Requested gear prev/wanted/current/new: ");
    Serial.print(prevgear);
    Serial.print(wantedGear);
    Serial.print(gear);
    Serial.print(newGear);
  }
}

// Polling for manual switch keys
void pollkeys() {
  int gupState = digitalRead(gupSwitch); // Gear up
  int gdownState = digitalRead(gdownSwitch); // Gear down
  int prevgdownState = 0;
  int prevgupState = 0;
  
  if (gdownState != prevgdownState || gupState != prevgupState ) {
    if (gdownState == LOW && gupState == HIGH) {
      int prevgupState = gupState;
      if ( debugEnabled ) { Serial.println("pollkeys: Gear up button"); }
      gearup();
    } else if (gupState == LOW && gdownState == HIGH) {
      int prevgdownState = gdownState;
      if ( debugEnabled ) { Serial.println("pollkeys: Gear down button"); }
      geardown();
    }
  }
}
// Polling time for transmission control
void polltrans() {
   if ( shiftBlocker ) { 
    shiftDuration =  millis() - shiftStartTime;
    if ( shiftDuration > shiftDelay) { switchGearStop(cSolenoidEnabled); };
   }

   //Raw value for pwm control (0-255) for SPC solenoid, see page 9: http://www.all-trans.by/assets/site/files/mercedes/722.6.1.pdf
   // "Pulsed constantly while idling in Park or Neutral at approximately 40% Duty cycle" <- 102/255 = 0.4
   if ( gear > 6 ) {
     analogWrite(spc, 102); 
   }
}

// Interrupt for N2 hallmode sensor
void N2SpeedInterrupt() {
 n2SpeedPulses++;
}

// Interrupt for N3 hallmode sensor
void N3SpeedInterrupt() {
  n3SpeedPulses++;
}
void vehicleSpeedInterrupt() {
  vehicleSpeedPulses++;
}
  

// Polling sensors
void pollsensors() {
  if ( millis() - lastSensorTime >= 1000 ) {
    detachInterrupt(2); // Detach interrupts for calculation
    detachInterrupt(3);
    detachInterrupt(4);
    
    if ( n2SpeedPulses >= 60 ) {
      n2Speed = n2SpeedPulses / 60;
      n2SpeedPulses = 0;
    } else {
      n2Speed = 0;
    }
    
    if ( n3SpeedPulses >= 60 ) {
      n3Speed = n3SpeedPulses / 60;
      n3SpeedPulses = 0;
    } else {
      n3Speed = 0;
    }
    
    if ( vehicleSpeedPulses >= 60 ) {
      vehicleSpeed = vehicleSpeedPulses / 60;
      vehicleSpeedPulses = 0;
    } else {
      vehicleSpeed = 0;
    }

    float lastSensorTime = millis();

    attachInterrupt(2, N2SpeedInterrupt, RISING); // Attach again
    attachInterrupt(3, N3SpeedInterrupt, RISING);
    attachInterrupt(4, vehicleSpeedInterrupt, RISING);
    
  }
  //reading oil temp sensor / pn-switch (same input pin, see page 27: http://www.all-trans.by/assets/site/files/mercedes/722.6.1.pdf)
  atfTempRaw = analogRead(atfPin);
  if (atfTempRaw > 1015 ) { drive = false; atfTempCalculated = 9999; atfTemp = 0; }
  else { drive = true; 
    atfTempCalculated = (0.0309*atfTempRaw * atfTempRaw) - 44.544*atfTempRaw + 16629; 
    atfTemp = -0.000033059* atfTempCalculated * atfTempCalculated + 0.2031 * atfTempCalculated - 144.09; //same as above
  }
    
 if ( tpsSensor ) {
    //reading TPS
    tpsVoltage = analogRead(tpsPin) * ( 5000 / 1023.00 );
    tpsPercentValue = ReadSensorMap (tpsLinearisationMap, tpsVoltage);

    if (tpsPercentValue > 100 ) { tpsPercentValue = 100; } 
    if (tpsPercentValue < 0 ) { tpsPercentValue = 0; }
  }
  
  if ( boostSensor ) { 
    //reading MAP/boost
    boostVoltage = analogRead(boostPin) * ( 5000 / 1023.00 );
    boostPercentValue = ReadSensorMap (boostLinearisationMap, boostVoltage);
    
    if (boostPercentValue > 100 ) { boostPercentValue = 100; } 
    if (boostPercentValue < 0 ) { boostPercentValue = 0; }
  }
  if ( boostSensor && tpsSensor ) { trueLoad = (tpsPercentValue * 0.60) + (boostPercentValue * 0.40); }
    else if ( tpsSensor && ! boostSensor ) { trueLoad = (tpsPercentValue * 1); } 
    else if ( ! tpsSensor ) { trueLoad = 100; }
}

// For manual microswitch control, gear up
void gearup() {
  if ( ! gear > 5 ) {  // Do nothing if we're on N/R/P
    if ( ! shiftBlocker) { newGear++; };
    if (gear > 4) { newGear = 5; } // Make sure not to switch more than 5.
    if ( debugEnabled ) { Serial.println("gearup: Gear up requested"); }
      gearchangeUp(newGear); 
  }
}

// For manual microswitch control, gear down
void geardown() {
  if ( ! gear > 5 ) {  // Do nothing if we're on N/R/P
    if ( ! shiftBlocker) { newGear--; };
    if (gear < 2) { newGear = 1; } // Make sure not to switch less than 1.
    if ( debugEnabled ) { Serial.println("gearup: Gear down requested"); }
      gearchangeDown(newGear); 
  }
}

// END OF UI STAGE / INPUT

// CORE
// no pressure alteration happening yet
//  
// gearSwitch logic
void switchGearStart(int cSolenoid) {
   shiftStartTime = millis(); 
   shiftBlocker = true;
   Serial.println("blocker");
   Serial.print(shiftBlocker);
   if ( debugEnabled ) { Serial.println("switchGearStart: Begin of gear change current/new/solenoid: "); Serial.print(gear); Serial.print(newGear); Serial.print(cSolenoid); }
   analogWrite(spc,255); // We could change shift pressure here 
   analogWrite(cSolenoid,255); // Beginning of gear change
   cSolenoidEnabled = cSolenoid;
}

void switchGearStop(int cSolenoid) {
   analogWrite(cSolenoid,0); // End of gear change
   analogWrite(spc,0); // let go of shift pressure
   shiftBlocker = false;
   if ( debugEnabled ) { Serial.println("switchGearStop: End of gear change current/new/solenoid: "); Serial.print(gear); Serial.print(newGear); Serial.print(cSolenoid); }
   prevgear = gear; // Make sure previous gear is known
   gear = newGear;
   shiftStartTime = 0;
}

void decideGear(int wantedGear) {
  int moreGear = gear++;
  int lessGear = gear--;
  // Determine speed related downshift and upshift here.
  int autoGear = ReadGearMap(gearMap, tpsPercentValue, vehicleSpeed);

  if ( ! shiftBlocker && wantedGear < 6 ) {
    if ( fullAuto && autoGear > gear && autoGear == moreGear && autoGear >= wantedGear ) { newGear = autoGear; gearchangeUp(newGear); } else if ( wantedGear >= moreGear && wantedGear < 6 ) { newGear = moreGear; gearchangeUp(newGear); }
    if ( autoGear < gear && autoGear == lessGear && autoGear <= wantedGear ) { newGear = autoGear; gearchangeDown(newGear); } else if ( wantedGear <= moreGear && wantedGear < 6 ) { newGear = lessGear; gearchangeDown(lessGear); }
  } else if ( wantedGear > 5 ) { prevgear = gear; gear = wantedGear; };
  if ( debugEnabled) { Serial.println("decideGear: wantedGear/autoGear/newGear: "); Serial.print(wantedGear); Serial.print(autoGear); Serial.print(newGear); }
}

void gearchangeUp(int newGear) {
  if ( shiftBlocker == false ) { 
      switch (newGear) {
      case 1: 
        prevgear = gear;
        gear = 1; 
        break;
      case 2:
        switchGearStart(y3);
        break;
      case 3:
        switchGearStart(y5);
        break;
      case 4:
        switchGearStart(y4);
        break;
      case 5:
        switchGearStart(y3);
        break;
      default:
      break;
    }
    if ( debugEnabled ) { 
      Serial.println("gearChangeUp: performing change from prev->new: "); 
      Serial.print(prevgear);
      Serial.print("->");
      Serial.print(newGear);
    }
  } else {
    Serial.println("gearChangeUp: Blocking change");  
  }
}

void gearchangeDown(int newGear) {
  if ( shiftBlocker == false ) { 
      switch (newGear) {
      case 1: 
        switchGearStart(y3);
        break;
      case 2:
        switchGearStart(y5);
        break;
      case 3:
        switchGearStart(y4);
        break;
      case 4:
        switchGearStart(y3); 
        break;
      case 5:
        prevgear = gear;
        gear = 5; 
        break;
      default:
      break;
    }
    if ( debugEnabled ) { 
      Serial.println("gearChangeDown: performing change from prev->new: "); 
      Serial.print(prevgear);
      Serial.print("->");
      Serial.print(newGear);
    }
  } else {
    Serial.println("gearChangeDown: Blocking change");  
  }
}

// END OF CORE

void checkHealth() {
  // Get temperature
  int tempState = digitalRead(tempSwitch);
  int prevtempState = 0;
  if ( tempState == HIGH  ) { health == true; };
}

void loop() {
  checkHealth();
  if (( incar && health ) || ( ! incar )) {
    if ( stick ) { pollstick(); } // using stick
    if ( manual ) { pollkeys(); } // using manual control
    if ( trans ) { polltrans(); } // using transmission
    if ( sensors ) { pollsensors(); } // using sensors
    updateDisplay();

  }
}


