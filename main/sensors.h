

int tpsRead() {
 int tpsPercentValue = 0;
 if ( tpsSensor ) {
    //reading TPS
    float tpsVoltage = analogRead(tpsPin) * 4.89;
    tpsPercentValue = readTPSVoltage(tpsVoltage);

    if (tpsPercentValue > 100 ) { tpsPercentValue = 100; } 
    if (tpsPercentValue < 0 ) { tpsPercentValue = 0; }
  } else {
    tpsPercentValue = 0;
  }
  return tpsPercentValue;
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
  const int n2PulsesPerRev = 60;
  const int n3PulsesPerRev = 60;
  const int vehicleSpeedPulsesPerRev = 60;

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
      vehicleSpeed = tpsRead();
    }

    lastSensorTime = millis();

    attachInterrupt(2, N2SpeedInterrupt, RISING); // Attach again
    attachInterrupt(3, N3SpeedInterrupt, RISING);
    attachInterrupt(4, vehicleSpeedInterrupt, RISING);
    
  }
}



int rpmRead() {
 int rpmValue = 0;
  return rpmValue;
}

int boostRead() {  
  int boostPercentValue = 0;
  if ( boostSensor ) { 
    //reading MAP/boost
    float boostVoltage = analogRead(boostPin) * 4.89;
    boostPercentValue = readBoostVoltage(boostVoltage);
    
    if (boostPercentValue > 100 ) { boostPercentValue = 100; } 
    if (boostPercentValue < 0 ) { boostPercentValue = 0; }
  } else {
    boostPercentValue = 0;
  }
  return boostPercentValue;
}

int loadRead() { 
 int trueLoad = 0;
 int boostPercentValue = boostRead();
 int tpsPercentValue = tpsRead();

  if ( boostSensor && tpsSensor ) { trueLoad = (tpsPercentValue * 0.60) + (boostPercentValue * 0.40); }
    else if ( tpsSensor && ! boostSensor ) {  trueLoad = (tpsPercentValue * 1); } 
    else if ( ! tpsSensor ) {  trueLoad = 100; }

  return trueLoad;
}
  //reading oil temp sensor / pn-switch (same input pin, see page 27: http://www.all-trans.by/assets/site/files/mercedes/722.6.1.pdf)
int atfRead() {
  int atfTempCalculated = 0;
  int atfTempRaw = analogRead(atfPin);
  int atfTemp = 0;
  int sensorReading = 0;  
  int i = 0;

  if (atfTempRaw > 1015 ) { drive = false; atfTempCalculated = 9999; atfTemp = 0; }
    else { drive = true; 
    atfTempCalculated = (0.0309*atfTempRaw * atfTempRaw) - 44.544*atfTempRaw + 16629; 
    atfTemp = -0.000033059* atfTempCalculated * atfTempCalculated + 0.2031 * atfTempCalculated - 144.09; //same as above
    if ( i < atfSensorNumReadings ) {
    sensorReading = atfTemp;
    atfSensorAverage = atfSensorAverage + (sensorReading - atfSensorAverage) / atfSensorFilterWeight;
    i++;
    }
  }
  return atfSensorAverage;
}

int oilRead() {
  // wip
  int oilTemp = 80;
  return oilTemp;
}
