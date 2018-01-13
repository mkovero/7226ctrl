#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// pins 8-12 reserved for oled
// SPI display setup
#define OLED_MOSI  11   //D1
#define OLED_CLK   13   //D0
#define OLED_DC    9
#define OLED_CS    10
#define OLED_RESET 8

// Solenoid output
const int y3 = 2;
const int y4 = 3;
const int y5 = 4;
const int mpc = 5;
const int spc = 6;
const int tcc = 7;

// Stick input
const int whitepin = 27;
const int bluepin = 29;
const int greenpin = 33;
const int yellowpin = 35;

// Car sensor input
const int tpspin = A0;
// map & rpm and load input coming here also.

// Internals
int gear = 2; // Start on gear 2
int prevgear = 2;
int const *pin;
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
static unsigned long thisMicros = 0;
static unsigned long lastMicros = 0;

// Switches
const int tempSwitch = 22;
const int gdownSwitch = 23;
const int gupSwitch = 24;

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
  display.setTextSize(5);
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
  display.setCursor(3,0);
  if ( ! prevgear > 5 ) { display.print(prevgear); };
  if ( prevgear == 6 ) { display.print("N"); };
  if ( prevgear == 7 ) { display.print("R"); };
  if ( prevgear == 8 ) { display.print("P"); };
  display.print("->");
  if ( ! gear > 5 ) { display.print(gear); };
  if ( gear == 6 ) { display.print("N"); };
  if ( gear == 7 ) { display.print("R"); };
  if ( gear == 8 ) { display.print("P"); };
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
  prevgear = gear; // Make sure previous gear is known

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

// For manual control, gear up
void gearup() {
  if ( ! gear > 5 ) {  // Do nothing if we're on N/R/P
    prevgear = gear;
    gear++;
    if (gear > 4) { gear = 5; } // Make sure not to switch more than 5.
    if ( ! prevgear == gear) { gearchange(); }
  }
}

// For manual control, gear down
void geardown() {
  if ( ! gear > 5 ) {  // Do nothing if we're on N/R/P
    prevgear = gear;
    gear--;
    if (gear < 2) { gear = 1; } // Make sure not to switch less than 1.
    if ( ! prevgear == gear) { gearchange(); }
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
 
   //Raw value for pwm control (0-255) for SPC solenoid, see page 9: http://www.all-trans.by/assets/site/files/mercedes/722.6.1.pdf
   // "Pulsed constantly while idling in Park or Neutral at approximately 40% Duty cycle" <- 102/255 = 0.4
   if ( gear > 5 ) {
     analogWrite(spc, 102); 
   }
}

void gearchange() {
  shiftStartTime = millis(); 
  if ( switchBlocker == false ) { 
    switch (gear) {
      case 1: 
        if ( prevgear == 2 ) { pin = &y3; switchGear(); };
        break;
      case 2:
        if ( prevgear == 1 ) { pin = &y3; switchGear(); };
        if ( prevgear == 3 ) { pin = &y5; switchGear(); };
        break;
      case 3:
        if ( prevgear == 2 ) { pin = &y5; switchGear(); };
        if ( prevgear == 4 ) { pin = &y4; switchGear(); };
        break;
      case 4:
        if ( prevgear == 3 ) { pin = &y4; switchGear(); };
        if ( prevgear == 5 ) { pin = &y3; switchGear(); };
        break;
      case 5:
        if ( prevgear == 4 ) { pin = &y3; switchGear(); };
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
  if (( incar && tempState == HIGH ) || ( ! incar )) {
    if ( stick ) { pollstick(); } // using stick
    if ( manual ) { pollkeys(); } // using manual control
    if ( trans ) { polltrans(); } // using transmission
  }
}
