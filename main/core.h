// CORE
// no pressure alteration happening yet
//  
// gearSwitch logic
void switchGearStart(int cSolenoid, int spcVal, int mpcVal) {
   shiftStartTime = millis(); 
   shiftBlocker = true;
   Serial.println("blocker");
   Serial.print(shiftBlocker);
   if ( debugEnabled ) { Serial.print("switchGearStart: Begin of gear change current/solenoid: "); Serial.print(gear); Serial.println(cSolenoid); }
  if ( trans ) { 
   spcVal = (100 - spcVal) * 2.55;
   mpcVal = (100 - mpcVal) * 2.55;
   analogWrite(spc,spcVal); // We could change shift pressure here 
   analogWrite(mpc,mpcVal);
   analogWrite(cSolenoid,255); // Beginning of gear change
   if ( debugEnabled ) { Serial.print("switchGearStart: spcPressure/mpcPressure: "); Serial.print(spcVal); Serial.println(mpcVal); }
  }
   cSolenoidEnabled = cSolenoid;
}

void switchGearStop(int cSolenoid) {
   analogWrite(cSolenoid,0); // End of gear change
   analogWrite(spc,0); // let go of shift pressure
   shiftBlocker = false;
   if ( debugEnabled ) { Serial.print("switchGearStop: End of gear change current/solenoid: "); Serial.print(gear); Serial.println(cSolenoid); }
   prevgear = gear; // Make sure previous gear is known
   if ( newGear < 6 ) { gear = newGear; }
   shiftStartTime = 0;
}


void gearchangeUp(int newGear) {
  if ( shiftBlocker == false ) { 
    int atfTemp = atfRead();
    int trueLoad = loadRead();

      switch (newGear) {
      case 1: 
        prevgear = gear;
        gear = 1; 
        break;
      case 2:
        if ( ! sensors ) { switchGearStart(y3,100, 100); }
        if ( sensors ) { switchGearStart(y3,readMap(spcMap12, trueLoad, atfTemp),readMap(mpcMap12, trueLoad, atfTemp)); }
        if ( debugEnabled ) { Serial.print("Solenoid y3 requested with spcMap12/mpcMap12, load/atfTemp "); Serial.print(trueLoad); Serial.println(atfTemp); }
        break;
      case 3:
        if ( ! sensors ) { switchGearStart(y4,100, 100); }
        if ( sensors ) { switchGearStart(y4,readMap(spcMap23, trueLoad, atfTemp),readMap(mpcMap23, trueLoad, atfTemp)); }
        if ( debugEnabled ) { Serial.print("Solenoid y4 requested with spcMap23/mpcMap23, load/atfTemp "); Serial.print(trueLoad); Serial.println(atfTemp); }
        break;
      case 4:
        if ( ! sensors ) { switchGearStart(y5,100, 100); }
        if ( sensors ) { switchGearStart(y5,readMap(spcMap34, trueLoad, atfTemp),readMap(mpcMap34, trueLoad, atfTemp)); }
        if ( debugEnabled ) { Serial.print("Solenoid y5 requested with spcMap34/mpcMap34, load/atfTemp "); Serial.print(trueLoad); Serial.println(atfTemp); }
        break;
      case 5:
        if ( ! sensors ) { switchGearStart(y3,100, 100); }
        if ( sensors ) { switchGearStart(y3,readMap(spcMap45, trueLoad, atfTemp),readMap(mpcMap45, trueLoad, atfTemp)); }
        if ( debugEnabled ) { Serial.print("Solenoid y3 requested with spcMap45/mpcMap45, load/atfTemp "); Serial.print(trueLoad); Serial.println(atfTemp); }
        break;
      default:
      break;
    }
    if ( debugEnabled ) { 
      Serial.print("gearChangeUp: performing change from prev->new: "); 
      Serial.print(prevgear);
      Serial.print("->");
      Serial.println(newGear);
    }
  } else {
    Serial.println("gearChangeUp: Blocking change");  
  }
}

void gearchangeDown(int newGear) {
  if ( shiftBlocker == false ) { 
    int atfTemp = atfRead();
    int trueLoad = loadRead();
      switch (newGear) {
      case 1: 
        if ( ! sensors ) { switchGearStart(y3,100, 100); }
        if ( sensors ) { switchGearStart(y3,readMap(spcMap21, trueLoad, atfTemp),readMap(mpcMap21, trueLoad, atfTemp)); }
        if ( debugEnabled ) { Serial.print("Solenoid y3 requested with spcMap21/mpcMap21, load/atfTemp "); Serial.print(trueLoad); Serial.println(atfTemp); }
        break;
      case 2:
        if ( ! sensors ) { switchGearStart(y4,100, 100); }
        if ( sensors ) { switchGearStart(y4,readMap(spcMap32, trueLoad, atfTemp),readMap(mpcMap32, trueLoad, atfTemp)); }
        if ( debugEnabled ) { Serial.print("Solenoid y4 requested with spcMap32/mpcMap32, load/atfTemp "); Serial.print(trueLoad); Serial.println(atfTemp); }
        break;
      case 3:
        if ( ! sensors ) { switchGearStart(y5,100, 100); }
        if ( sensors ) { switchGearStart(y5,readMap(spcMap43, trueLoad, atfTemp),readMap(mpcMap43, trueLoad, atfTemp)); }
        if ( debugEnabled ) { Serial.print("Solenoid y5 requested with spcMap43/mpcMap43, load/atfTemp "); Serial.print(trueLoad); Serial.println(atfTemp); }
        break;
      case 4:
        if ( ! sensors ) { switchGearStart(y3,100, 100); }
        if ( sensors ) { switchGearStart(y3,readMap(spcMap54, trueLoad, atfTemp),readMap(mpcMap54,trueLoad, atfTemp)); }
        if ( debugEnabled ) { Serial.print("Solenoid y3 requested with spcMap54/mpcMap54, load/atfTemp "); Serial.print(trueLoad); Serial.println(atfTemp); }
        break;
      case 5:
        prevgear = gear;
        gear = 5; 
        break;
      default:
      break;
    }
   if ( debugEnabled ) { 
      Serial.print("gearChangeDown: performing change from prev->new: "); 
      Serial.print(prevgear);
      Serial.print("->");
      Serial.println(newGear);
    }
  } else {
    Serial.println("gearChangeDown: Blocking change");  
  }
}

void decideGear(int wantedGear) {
  
  int moreGear = gear+1;
  int lessGear = gear-1;
  int tpsPercentValue = tpsRead();
  // Determine speed related downshift and upshift here.
  int autoGear = readMap(gearMap, tpsPercentValue, vehicleSpeed);

  if ( ! shiftBlocker && wantedGear < 6 ) {
    if ( (fullAuto && autoGear > gear) || (! fullAuto && wantedGear > gear) ) { 
      newGear = moreGear; gearchangeUp(newGear); 
    } 
    if ( autoGear < gear || wantedGear < gear ) { 
      newGear = lessGear; gearchangeDown(newGear); 
    } 
    if ( debugEnabled) { Serial.print("decideGear: tpsPercent/vehicleSpeed: "); Serial.print(tpsPercentValue); Serial.print("-"); Serial.println(vehicleSpeed); }
    if ( debugEnabled) { Serial.print("decideGear: wantedGear/autoGear/newGear/gear: "); Serial.print(wantedGear); Serial.print("-"); Serial.print(autoGear); Serial.print("-"); Serial.print(newGear); Serial.print("-"); Serial.println(gear); }
  }
}

int evaluateGear(float ratio) {
  int evaluatedGear = 0;
  int n3n2 = n3Speed / n2Speed;
  int incomingShaftSpeed = 0;
  int measuredGear = 0;
  if (n3Speed == 0) { 
    incomingShaftSpeed = n2Speed * 1.64;
  } else {
    incomingShaftSpeed = n2Speed; 
    //when gear is 2, 3 or 4, n3 speed is not zero, and then incoming shaft speed (=turbine speed) equals to n2 speed)
  }

  if ( 3.4 < ratio && n3n2 < 0.50) {
  measuredGear = 1;
  }
  else if ( 2.05 < ratio && ratio < 2.20 && n3n2 >= 0.50) {
  measuredGear = 2;
  }
  else if ( 1.38 < ratio && ratio < 1.45 && n3n2 >= 0.50 ) {
  measuredGear = 3;
  }
  else if ( 0.97 < ratio && ratio < 1.05 && n3n2 >= 0.50) {
  measuredGear = 4;
  }
  else if ( ratio < 0.90 && n3n2 < 0.50) {
  measuredGear = 5;
  }
 
  if (measuredGear != 0)
  {
  return measuredGear;
  }
}
// END OF CORE
