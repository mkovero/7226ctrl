#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Solenoid output
int y3 = 2;
int y4 = 3;
int y5 = 4;
int mpc = 5;
int spc = 6;
int tcc = 7;

// pins 8-12 reserved for oled
#define OLED_MOSI  11   //D1
#define OLED_CLK   12   //D0
#define OLED_DC    9
#define OLED_CS    8
#define OLED_RESET 10

// Stick input
int whitepin = 27;
int bluepin = 29;
int greenpin = 33;
int yellowpin = 35;

// Car sensor input
int tpspin = A0;
// map & rpm and load input coming here also.

// Internals
int gear = 1;
int prevgear = 1;
int *pin;
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
static unsigned long thisMicros = 0;
static unsigned long lastMicros = 0;

// Switches
int tempSwitch = 22;
int gdownSwitch = 23;
int gupSwitch = 24;

// States
int prevtempState = 0;
int tempState = 0;
int gdownState = 0;
int gupState = 0;
int prevgdownState = 0;
int prevgupState = 0;
int tccState = 0;
int prevtccState = 0;
int whiteState = 0;
int blueState = 0;
int greenState = 0;
int yellowState = 0;

// Shit delay
int shiftDelay = 1000;
int shiftStartTime = 0;
int shiftDuration = 0;

// Do we want torque lock?
boolean tccEnabled = false; // no

// Are we in a real car?
boolean incar = false; // no.

// Do we use stick control?
boolean stick = true; // yes.

// Manual control?
boolean manual = true;

// Actual transmission there?
boolean trans = true;

boolean switchBlocker = false;

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
  display.setTextSize(1);
  display.setTextColor(WHITE);
 
  // Solenoid outputs
  pinMode(y3, OUTPUT); // 1-2/4-5 solenoid
  pinMode(y4,OUTPUT); // 2-3
  pinMode(y5,OUTPUT); // 3-4
  pinMode(spc,OUTPUT); // shift pressure
  pinMode(mpc,OUTPUT); // modulation pressure
  pinMode(tcc,OUTPUT); // lock

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

  //Internals
  pinMode(tpspin,INPUT); // throttle position sensor
  
  Serial.println("Started.");
}

// UI STAGE
// Control for what user sees and how gearbox is used with
// 

// Display update
void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(gear);
  display.display();
}

// INPUT
// Polling for stick control
void pollstick() {
  // Read the stick.
  whiteState = digitalRead(whitepin);
  blueState = digitalRead(bluepin);
  greenState = digitalRead(greenpin);
  yellowState = digitalRead(yellowpin);
  // Determine position
  if (whiteState == HIGH && blueState == HIGH && greenState == HIGH && yellowState == LOW ) { gear = 8; } // P
  if (whiteState == LOW && blueState == HIGH && greenState == HIGH && yellowState == HIGH ) { gear = 7; } // R
  if (whiteState == HIGH && blueState == LOW && greenState == HIGH && yellowState == HIGH ) { gear = 6; } // N
  if (whiteState == LOW && blueState == LOW && greenState == HIGH && yellowState == LOW ) { gear = 5; }
  if (whiteState == LOW && blueState == LOW && greenState == LOW && yellowState == HIGH ) { gear = 4; }
  if (whiteState == LOW && blueState == HIGH && greenState == LOW && yellowState == LOW ) { gear = 3; }
  if (whiteState == HIGH && blueState == LOW && greenState == LOW && yellowState == LOW ) { gear = 2; }
  if (whiteState == HIGH && blueState == HIGH && greenState == LOW && yellowState == HIGH ) { gear = 1; }
  gearchange();
}
// For manual control, gear up
void gearup() {
  prevgear = gear;
  gear = gear+1;
  if (gear > 4) { gear = 5; } // Make sure not to switch more than 5.
  if ( ! prevgear == gear) { gearchange(); }
}

// For manual control, gear down
void geardown() {
  prevgear = gear;
  gear = gear-1;
  if (gear < 2) { gear = 1; } // Make sure not to switch less than 1.
  if ( ! prevgear == gear) { gearchange(); }
}

// Polling for manual switch keys
void pollkeys() {
  gupState == digitalRead(gupSwitch); // Gear up
  gdownState == digitalRead(gdownSwitch); // Gear down

  if (gdownState != prevgdownState || gupState != prevgupState ) {
    if (gdownState == LOW && gupState == HIGH) {
      prevgupState = gupState;
      gearup();
    } else if (gupState == LOW && gdownState == HIGH) {
      prevgdownState = gdownState;
      geardown();
    }
  }
}

// END OF UI STAGE / INPUT

// CORE
// no pressure alteration happening yet
//  
// gearSwitch logic
void switchGear() {
   switchBlocker = true;
   switchGearStart();
}

void switchGearStart() {
   analogWrite(spc,255); // We could change shift pressure here 
   analogWrite(*pin,255); // Beginning of gear change
}

void switchGearStop() {
   analogWrite(*pin,0); // End of gear change
   analogWrite(spc,0); // let go of shift pressure
   switchBlocker = false;
}

void polltrans() {
   shiftDuration = millis() - shiftStartTime;
   if ( shiftDuration > shiftDelay) { switchGearStop(); };
}

// Solenoid logic
void gearchange() {
  prevgear = gear; // Make sure previous gear is known
  shiftStartTime = millis(); 
  if ( switchBlocker == false ) { 
    switch (gear) {
      case 1: 
        if ( prevgear == 2 ) { pin = &y3; switchGear(); gear = 1; };
        break;
      case 2:
        if ( prevgear == 1 ) { pin = &y3; switchGear(); gear = 2; };
        if ( prevgear == 3 ) { pin = &y5; switchGear(); gear = 2; };
        break;
      case 3:
        if ( prevgear == 2 ) { pin = &y5; switchGear(); gear = 3; };
        if ( prevgear == 4 ) { pin = &y4; switchGear(); gear = 3; };
        break;
      case 4:
        if ( prevgear == 3 ) { pin = &y4; switchGear(); gear = 4; };
        if ( prevgear == 5 ) { pin = &y3; switchGear(); gear = 4; };
        break;
      case 5:
        if ( prevgear == 4 ) { pin = &y3; switchGear(); gear = 5; };
        break;
      case 6:
        // mechanical "N" gear
        break;
      case 7:
        // mechanical "R" gear
        break;
      case 8:
        // mechanical "P" gear
        break;
      default:
      break;
    }
  }
  updateDisplay();
}
// END OF CORE


void loop() {
  // Get temperature
  tempState = digitalRead(tempSwitch);
  // If we have a temperature, we can assume P/N switch has moved to R/N. (Lever switch and temp sensor are in series)
  if ( tempState == HIGH ) {
    if ( stick == true ) { pollstick(); } // using stick
    if ( manual == true ) { pollkeys(); } // using manual control
    if ( trans == true ) { polltrans(); } // using transmission

  }
}
