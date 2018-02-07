#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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

//TPS linearisation map
int tpsLinearisationMap[2][2] {
{0,   100 },   //sensor value as a result %
{400, 2830 }}; //corresponding sensor voltage, mV

//Boost linearisation map
int boostLinearisationMap[2][2] {
{0,   100 },   //sensor value as a result %
{1500, 4200 }}; //corresponding sensor voltage, mV

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
  pinMode(boostPin,INPUT); // throttle position sensor 
  pinMode(tpsPin,INPUT); // throttle position sensor 
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

// UI STAGE
// Control for what user sees and how gearbox is used with
// 

// Display update
void updateDisplay() {
  display.clearDisplay();
  display.setCursor(3,0);
  if ( prevgear <= 5 ) { display.print(prevgear); };
  if ( prevgear == 6 ) { display.print("N"); };
  if ( prevgear == 7 ) { display.print("R"); };
  if ( prevgear == 8 ) { display.print("P"); };
  display.print("->");
  if ( gear <= 5 ) { display.print(gear); };
  if ( gear == 6 ) { display.print("N"); };
  if ( gear == 7 ) { display.print("R"); };
  if ( gear == 8 ) { display.print("P"); }; 
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
  int moreGear = gear+1;
  int lessGear = gear-1;
  // Determine speed related downshift and upshift here.
  
  if ( ! shiftBlocker && wantedGear < 6) {
    if ( wantedGear >= moreGear && wantedGear < 6 ) { newGear = moreGear; gearchangeUp(newGear); };
    if ( wantedGear <= lessGear && wantedGear < 6 ) { newGear = lessGear; gearchangeDown(newGear); };
  }
  else if ( wantedGear > 5 ) { prevgear = gear; gear = wantedGear; };
  if ( debugEnabled ) { Serial.println("decideGear: Blocking"); };
  
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

