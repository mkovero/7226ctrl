// Solenoid output
int y3 = 4;
int y4 = 7;
int y5 = 8;
int mpc = 5;
int spc = 6;
int tcc = 3;

// Stick input
int whitepin = 14;
int bluepin = 15;
int greenpin = 16;
int yellowpin = 17;

// Car sensor input
int tpspin = 18;
// map & rpm and load input coming here also.

// Internals
int gear = 1;
int prevgear = 1;
const char* message = "unused";
const char* pin = "nothing";

// Switches
int tempSwitch = 22;
int gdownSwitch = 20;
int gupSwitch = 21;
int tccSwitch = 2;

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
int shiftdelay = 1000;

// Do we want torque lock?
boolean tccEnabled = false; // no

// Are we in a real car?
boolean incar = false; // no.

// Do we use stick control?
boolean stick = true; // yes.

LiquidCrystal lcd(19, 13, 12, 11, 10, 9);

void setup() {
  
   // TCCR2B = TCCR2B & 0b11111000 | 0x07;
  // MPC and SPC should have frequency of 1000hz
  // TCC should have frequency of 100hz
  // Lower the duty cycle, higher the pressures.
  Serial.begin(9600);
  
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
  pinMode(tccSwitch,INPUT); // torque lock
  
  //For stick control
  pinMode(whitepin,INPUT);
  pinMode(bluepin,INPUT);
  pinMode(greenpin,INPUT);
  pinMode(yellowpin,INPUT);

  // Drive solenoids are all off.
  digitalWrite(y3,HIGH);
  digitalWrite(y4,HIGH);
  digitalWrite(y5,HIGH);
  digitalWrite(spc,HIGH); // No pressure here by default.
  digitalWrite(mpc,LOW); // We want constant pressure here.
  digitalWrite(tcc,HIGH); // No pressure here by default.

  //Internals
  pinMode(tpspin,INPUT); // throttle position sensor
  
  lcd.begin(16, 2);
  Serial.println("Started.");
}

void updateLCD() {
  delay(500); // get rid of delay, actually get proper OLED.
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Vaihde: ");
  lcd.print(gear);
  lcd.print(" ");
  lcd.print(prevgear);
  lcd.print("->");
  lcd.print(gear);
  lcd.setCursor(0,1);
}

// Drive solenoid state change (replace delay() with something else that does not block everything.)
void switchGear(const char* pin, const char* message) {
   digitalWrite(spc,LOW); // We could change shift pressure here LOW = maxed out
   digitalWrite(pin,HIGH); // Beginning of gear change
   delay(shiftdelay);
   digitalWrite(pin,LOW); // End of gear change
   digitalWrite(spc,HIGH); // let go of shift pressure
   Serial.println(message);
}

// For manual control, gear up
void gearup() {
  if (gear > 4) { gear = 5; } // Make sure not to switch more than 5.
  prevgear = gear;
  gear = gear+1;
  gearchange(gear);
}

// For manual control, gear down
void geardown() {
  if (gear < 2) { gear = 1; } // Make sure not to switch less than 1.
  prevgear = gear;
  gear = gear-1;
  gearchange(gear);
}

// General gear change solenoid logic
gearchange(int gear) {
  prevgear = gear; // Make sure previous gear is known
 
  switch (gear) {
    case 1: 
      if ( prevgear == 2 ) { switchGear(y3, "from 2->1"); gear = 1; }
      break;
    case 2:
      if ( prevgear == 1 ) { switchGear(y3, "from 1->2"); gear = 2; }
      if ( prevgear == 3 ) { switchGear(y5, "from 3->2"); gear = 2; }
      break;
    case 3:
      if ( prevgear == 2 ) { switchGear(y5, "from 2->3"); gear = 3; }
      if ( prevgear == 4 ) { switchGear(y4, "from 4->3"); gear = 3; }
      break;
    case 4:
      if ( prevgear == 3 ) { switchGear(y4, "from 3->4"); gear = 4; }
      if ( prevgear == 5 ) { switchGear(y3, "from 5->4"); gear = 4; }
      break;
    case 5:
      if ( prevgear == 4 ) { switchGear(y3, "from 4->5"); gear = 5; }
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

// Polling for stick control
pollstick() {
  // Read the stick.
  whiteState = digitalRead(whitepin);
  blueState = digitalRead(bluepin);
  greenState = digitalRead(greenpin);
  yellowState = digitalRead(yellowpin);
  // Determine position
  if (whiteState == HIGH && blueState == HIGH && greenState == HIGH && yellowState == LOW ) { gearchange(8); } // P 
  if (whiteState == LOW && blueState == HIGH && greenState == HIGH && yellowState == HIGH ) { gearchange(7); } // R
  if (whiteState == HIGH && blueState == LOW && greenState == HIGH && yellowState == HIGH ) { gearchange(6); } // N
  if (whiteState == LOW && blueState == LOW && greenState == HIGH && yellowState == LOW ) { gearchange(5); }
  if (whiteState == LOW && blueState == LOW && greenState == LOW && yellowState == HIGH ) { gearchange(4); }
  if (whiteState == LOW && blueState == HIGH && greenState == LOW && yellowState == LOW ) { gearchange(3); }
  if (whiteState == HIGH && blueState == LOW && greenState == LOW && yellowState == LOW ) { gearchange(2); }
  if (whiteState == HIGH && blueState == HIGH && greenState == LOW && yellowState == HIGH ) { gearchange(1); }
}

// Polling for manual switch keys
pollkeys() {
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

void loop() {
  // Get temperature
  tempState = digitalRead(tempSwitch);
  // If we have a temperature, we can assume P/N switch has moved to R/N. (Lever switch and temp sensor are in series)
  if ( tempState == HIGH ) {
    if ( stick == true) { pollstick(); } // using stick
    if ( manual == true) { pollkeys(); } // using manual control
    if ( tccEnabled == true { tccState = digitalRead(tccSwitch); } // Torque lock requested 
  }

}
