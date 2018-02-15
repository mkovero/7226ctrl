// CORE
// no pressure alteration happening yet
//  
// gearSwitch logic
void switchGearStart(int cSolenoid, int spcVal, int mpcVal) {
   shiftStartTime = millis(); 
   shiftBlocker = true;
   Serial.println("blocker");
   Serial.print(shiftBlocker);
   if ( debugEnabled ) { Serial.println("switchGearStart: Begin of gear change current: %d, new: %d, solenoid: %d", gear, newGear, cSolenoid); }
  if ( trans ) { 
   spcVal = (100 - spcVal) * 2.55;
   mpcVal = (100 - mpcVal) * 2.55;
   analogWrite(spc,spcVal); // We could change shift pressure here 
   analogWrite(mpc,mpcVal);
   analogWrite(cSolenoid,255); // Beginning of gear change
   if ( debugEnabled ) { Serial.println("switchGearStart: spcPressure: %d, mpcPressure %d", spcVal, mpcVal); }
  }
   cSolenoidEnabled = cSolenoid;
}

void switchGearStop(int cSolenoid) {
   analogWrite(cSolenoid,0); // End of gear change
   analogWrite(spc,0); // let go of shift pressure
   shiftBlocker = false;
   if ( debugEnabled ) { Serial.println("switchGearStop: End of gear change current: %d, new: %d, solenoid: %d", gear, newGear, cSolenoid); }
   prevgear = gear; // Make sure previous gear is known
   gear = newGear;
   shiftStartTime = 0;
}


void gearchangeUp(int newGear) {
  if ( shiftBlocker == false ) { 
      switch (newGear) {
      case 1: 
        prevgear = gear;
        gear = 1; 
        break;
      case 2:
        if ( ! sensors ) { switchGearStart(y3,100, 100); }
        if ( sensors ) { switchGearStart(y3,readMap(spcMap12, trueLoad, atfTemp),readMap(mpcMap12, trueLoad, atfTemp)); }
        if ( debugEnabled ) { Serial.println("Solenoid %d requested with spcMap12/mpcMap12, load is %d and atfTemp %d", y3, trueLoad, atfTemp); }
        break;
      case 3:
        if ( ! sensors ) { switchGearStart(y4,100, 100); }
        if ( sensors ) { switchGearStart(y4,readMap(spcMap23, trueLoad, atfTemp),readMap(mpcMap23, trueLoad, atfTemp)); }
        if ( debugEnabled ) { Serial.println("Solenoid %d requested with spcMap23/mpcMap23, load is %d and atfTemp %d", y4, trueLoad, atfTemp); }
        break;
      case 4:
        if ( ! sensors ) { switchGearStart(y5,100, 100); }
        if ( sensors ) { switchGearStart(y5,readMap(spcMap34, trueLoad, atfTemp),readMap(mpcMap34, trueLoad, atfTemp)); }
        if ( debugEnabled ) { Serial.println("Solenoid %d requested with spcMap34/mpcMap34, load is %d and atfTemp %d", y5, trueLoad, atfTemp); }
        break;
      case 5:
        if ( ! sensors ) { switchGearStart(y3,100, 100); }
        if ( sensors ) { switchGearStart(y3,readMap(spcMap45, trueLoad, atfTemp),readMap(mpcMap45, trueLoad, atfTemp)); }
        if ( debugEnabled ) { Serial.println("Solenoid %d requested with spcMap45/mpcMap45, load is %d and atfTemp %d", y3, trueLoad, atfTemp); }
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
        if ( ! sensors ) { switchGearStart(y3,100, 100); }
        if ( sensors ) { switchGearStart(y3,readMap(spcMap21, trueLoad, atfTemp),readMap(mpcMap21, trueLoad, atfTemp)); }
        if ( debugEnabled ) { Serial.println("Solenoid %d requested with spcMap21/mpcMap21, load is %d and atfTemp %d", y3, trueLoad, atfTemp); }
        break;
      case 2:
        if ( ! sensors ) { switchGearStart(y4,100, 100); }
        if ( sensors ) { switchGearStart(y4,readMap(spcMap32, trueLoad, atfTemp),readMap(mpcMap32, trueLoad, atfTemp)); }
        if ( debugEnabled ) { Serial.println("Solenoid %d requested with spcMap32/mpcMap32, load is %d and atfTemp %d", y4, trueLoad, atfTemp); }
        break;
      case 3:
        if ( ! sensors ) { switchGearStart(y5,100, 100); }
        if ( sensors ) { switchGearStart(y5,readMap(spcMap43, trueLoad, atfTemp),readMap(mpcMap43, trueLoad, atfTemp)); }
        if ( debugEnabled ) { Serial.println("Solenoid %d requested with spcMap43/mpcMap43, load is %d and atfTemp %d", y5, trueLoad, atfTemp); }
        break;
      case 4:
        if ( ! sensors ) { switchGearStart(y3,100, 100); }
        if ( sensors ) { switchGearStart(y3,readMap(spcMap54, trueLoad, atfTemp),readMap(mpcMap54,trueLoad, atfTemp)); }
        if ( debugEnabled ) { Serial.println("Solenoid %d requested with spcMap54/mpcMap54, load is %d and atfTemp %d", y3, trueLoad, atfTemp); }
        break;
      case 5:
        prevgear = gear;
        gear = 5; 
        break;
      default:
      break;
    }
    if ( debugEnabled ) { 
      Serial.println("gearChangeDown: performing change from prev: %d new: %d ", prevgear, newgear); 
    }
  } else {
    Serial.println("gearChangeDown: Blocking change");  
  }
}

void decideGear(int wantedGear) {
  int moreGear = gear++;
  int lessGear = gear--;
  // Determine speed related downshift and upshift here.
  int autoGear = readMap(gearMap, tpsPercentValue, vehicleSpeed);

  if ( ! shiftBlocker && wantedGear < 6 ) {
    if ( fullAuto && autoGear > gear && autoGear <= wantedGear ) { 
      newGear = moreGear; gearchangeUp(newGear); 
    } 
    if ( autoGear < gear || wantedGear < gear ) { 
      newGear = lessGear; gearchangeDown(newGear); 
    } 
  } else if ( wantedGear > 5 ) { prevgear = gear; gear = wantedGear; };
  if ( debugEnabled ) { Serial.println("decideGear: wantedGear=%d, autoGear=%d, newGear=%d",wantedGear,autoGear,newGear); }
}


// END OF CORE