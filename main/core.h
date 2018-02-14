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

void decideGear(int wantedGear) {
  int moreGear = gear++;
  int lessGear = gear--;
  // Determine speed related downshift and upshift here.
  int autoGear = readMap(gearMap, tpsPercentValue, vehicleSpeed);

  if ( ! shiftBlocker && wantedGear < 6 ) {
    if ( fullAuto && autoGear > gear && autoGear == moreGear && autoGear >= wantedGear ) { newGear = autoGear; gearchangeUp(newGear); } else if ( wantedGear >= moreGear && wantedGear < 6 ) { newGear = moreGear; gearchangeUp(newGear); }
    if ( autoGear < gear && autoGear == lessGear && autoGear <= wantedGear ) { newGear = autoGear; gearchangeDown(newGear); } else if ( wantedGear <= moreGear && wantedGear < 6 ) { newGear = lessGear; gearchangeDown(lessGear); }
  } else if ( wantedGear > 5 ) { prevgear = gear; gear = wantedGear; };
  if ( debugEnabled) { Serial.println("decideGear: wantedGear/autoGear/newGear: "); Serial.print(wantedGear); Serial.print(autoGear); Serial.print(newGear); }
}


// END OF CORE