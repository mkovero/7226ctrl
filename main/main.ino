#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <calc.h>
#include <input.h>
#include <maps.h>
#include <ui.h>
#include <core.h>
#include <config.h>
// Work by Markus Kovero <mui@mui.fi>
// Big thanks to Tuomas Kantola regarding maps and related math

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


