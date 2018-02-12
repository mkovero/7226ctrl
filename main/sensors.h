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