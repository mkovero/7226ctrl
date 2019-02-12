#include <Arduino.h>

#define INPUT_SIZE 256
long serialTimeout = 120000;
long lastActiveConfig;
boolean configMode,configSet,featureSet = false;
int myVersion = 20190211;
int asset,value = 0;
float fvalue = 0.00;
char input[INPUT_SIZE + 1];

/*
// Stick control
boolean stickCtrl = false;
// External radio control
boolean radioEnabled = true; 
// Manual microswitch control?
boolean manual = true;
// Full automatic mode
boolean fullAuto = false;
// torque converter lock
boolean tccLock = true;
// gear evaluation to determine real running gear, this prevents shifts if previous shift has not completed
boolean evalGear = true;


// Do we have TPS?
boolean tpsSensor = true;
// Do we have boost sensor?
boolean boostSensor = true;
// Exhaust pressure sensor
boolean exhaustPresSensor = false;

// Old style w124 speed meter controlling
boolean w124speedo = true;
// Old style w124 rpm meter pwm
boolean w124rpm = true;
// Boost control
boolean boostLimit = true;
// control fuel pumps (turn off below certain rpm)
boolean fuelPumpControl = false;
// Initial state of fuel pumps
boolean fuelPumps = true;
// Initial state of horn
boolean horn = false;

// calculate vehicleSpeed from engine RPM
boolean rpmSpeed = true; 
// calculate vehicleSpeed from diff abs sensor
boolean diffSpeed = true; 

// Adaptive pressure
boolean adaptive = false;
// Battery monitor
boolean batteryMonitor = true;

// Default power mode
boolean truePower = false;
// Output to serial console
boolean debugEnabled = true;
// Datalogging (enabling this disables debug)
boolean datalogger = true;
*/

typedef struct
{
    int boostMax; 
    int boostDrop;
    int boostSpring; 
    int fuelMaxRPM; 
    int maxRPM; 
    int tireWidth; 
    int tireProfile;
    int tireInches;
    float diffRatio;
    int rearDiffTeeth;
    int nextShiftDelay;
    float maxSlip;
    int stallSpeed;
    int batteryLimit;
    int firstTccGear;
    int triggerWheelTeeth;
    int tpsAgre;
} ConfigParam;

ConfigParam config = { 700,50,120,2000,7000,195,65,15,3.27,29,2000,0.5,2200,11500,2,6,2 };

void pollConfigMode()
{
      int key = 0;

    if (!configMode)
    {
        while (Serial.available() > 0)
        {
            if (key < INPUT_SIZE && Serial.available())
            {
                input[key] = Serial.read();
                key += 1;
            }
        }

        if (key > 0)
        {
            input[key] = 0;

            char *command = strtok(input, ";");
        while (command != 0)
        {
            char *separator = strchr(command, ':');
            if (separator != 0)
            {
                *separator = 0;
                int asset = atoi(command);
                ++separator;
                int value = atoi(separator);
               // Serial.print(asset);
               // Serial.print(value);
        
            if (asset == 13371337)
            {
                   Serial.print("69696969:");
                    Serial.println(myVersion);
                    configMode = true;
                    lastActiveConfig = millis();
                    if (debugEnabled)
                    {
                        Serial.println("Serial connection accepted");
                    }
               
            }
                    }

            command = strtok(0, ";");
        }
        }
    }
    else
    {
        if (millis() - lastActiveConfig > serialTimeout)
        {
            configMode = false;
            if (debugEnabled)
            {
                Serial.println("Serial timed out");
            } else {
                serialConfig();
            }
        }
    }
}

void getFeatures() {
Serial.print("60000:60000;");
Serial.print("1:");
Serial.print(int(stickCtrl));
Serial.print(";");
Serial.print("2:");
Serial.print(int(radioEnabled));
Serial.print(";");
Serial.print("3:");
Serial.print(int(manual));
Serial.print(";");
Serial.print("4:");
Serial.print(int(fullAuto));
Serial.print(";");
Serial.print("5:");
Serial.print(int(tccLock));
Serial.print(";");
Serial.print("6:");
Serial.print(int(evalGear));
Serial.print(";");
Serial.print("7:");
Serial.print(int(tpsSensor));
Serial.print(";");
Serial.print("8:");
Serial.print(int(boostSensor));
Serial.print(";");
Serial.print("9:");
Serial.print(int(exhaustPresSensor));
Serial.print(";");
Serial.print("10:");
Serial.print(int(w124speedo));
Serial.print(";");
Serial.print("11:");
Serial.print(int(w124rpm));
Serial.print(";");
Serial.print("12:");
Serial.print(int(fuelPumpControl));
Serial.print(";");
Serial.print("13:");
Serial.print(int(rpmSpeed));
Serial.print(";");
Serial.print("14:");
Serial.print(int(diffSpeed));
Serial.print(";");
Serial.print("15:");
Serial.print(int(adaptive));
Serial.print(";");
Serial.print("16:");
Serial.print(int(batteryMonitor));
Serial.print(";");
Serial.print("17:");
Serial.print(int(truePower));
Serial.print(";");
Serial.print("18:");
Serial.print(int(debugEnabled));
Serial.print(";");
Serial.print("19:");
Serial.println(int(datalogger));
}

void setFeatures(int asset, int value)
{
    lastActiveConfig = millis();
    switch (asset)
    {
    case 1:
         stickCtrl = boolean(value);
         break;
    case 2:
         radioEnabled = boolean(value);
        break;
    case 3:
         manual = boolean(value);
        break;
            case 4:
         fullAuto = boolean(value);
        break;
            case 5:
         tccLock = boolean(value);
        break;
            case 6:
         evalGear = boolean(value);
        break;
            case 7:
         tpsSensor = boolean(value);
        break;
            case 8:
         boostSensor = boolean(value);
        break;
            case 9:
         exhaustPresSensor = boolean(value);
        break;
            case 10:
         w124speedo = boolean(value);
        break;
            case 11:
         w124rpm = boolean(value);
        break;
            case 12:
         fuelPumpControl = boolean(value);
        break;
            case 13:
         rpmSpeed = boolean(value);
        break;
            case 14:
         diffSpeed = boolean(value);
        break;
            case 15:
         adaptive = boolean(value);
        break;
            case 16:
         batteryMonitor = boolean(value);
        break;
            case 17:
         truePower = boolean(value);
        break;
            case 18:
         debugEnabled = boolean(value);
        break;
            case 19:
         datalogger = boolean(value);
        break;        
    default:
        break;
    }
      /*    if (debugEnabled) {
          Serial.print(asset);
          Serial.print(":");
          Serial.println(value);
         }*/
}
void setConfigFloat(int asset, float value, ConfigParam* config) {
      lastActiveConfig = millis();
    
  switch (asset)
    {
        case 58:
         config->diffRatio = value;

        break;
         case 61:
         config->maxSlip = value;

        break;
        default:
        break;
    }
}

void setConfig(int asset, int value, ConfigParam* config) {
    lastActiveConfig = millis();
 
    switch (asset)
    {
    case 50:
         config->boostMax = value;
         break;
    case 51:
         config->boostDrop = value;
        break;
    case 52:
         config->boostSpring = value;
        break;
            case 53:
         config->fuelMaxRPM = value;
        break;
            case 54:
         config->maxRPM = value;
        break;
            case 55:
         config->tireWidth = value;
        break;
            case 56:
         config->tireProfile = value;
        break;
            case 57:
         config->tireInches = value;
        break;
            case 59:
         config->rearDiffTeeth = value;
        break;
            case 60:
         config->nextShiftDelay = value;
        break;
            case 62:
         config->stallSpeed = value;
        break;
            case 63:
         config->batteryLimit = value;
        break;
            case 64:
         config->firstTccGear = value;
        break;
            case 65:
         config->triggerWheelTeeth = value;
        break;
            case 66:
         config->tpsAgre = value;
        break;
   
    default:
        break;
    }
         /* if (debugEnabled) {
          Serial.print(asset);
          Serial.print(":");
          Serial.println(value);
         }*/
}

void getConfig(ConfigParam* config) {
    Serial.print("50000:50000;");
Serial.print("50:");
Serial.print(config->boostMax);
Serial.print(";");
Serial.print("51:");
Serial.print(config->boostDrop);
Serial.print(";");
Serial.print("52:");
Serial.print(config->boostSpring);
Serial.print(";");
Serial.print("53:");
Serial.print(config->fuelMaxRPM);
Serial.print(";");
Serial.print("54:");
Serial.print(config->maxRPM);
Serial.print(";");
Serial.print("55:");
Serial.print(config->tireWidth);
Serial.print(";");
Serial.print("56:");
Serial.print(config->tireProfile);
Serial.print(";");
Serial.print("57:");
Serial.print(config->tireInches);
Serial.print(";");
Serial.print("58:");
Serial.print(config->diffRatio);
Serial.print(";");
Serial.print("59:");
Serial.print(config->rearDiffTeeth);
Serial.print(";");
Serial.print("60:");
Serial.print(config->nextShiftDelay);
Serial.print(";");
Serial.print("61:");
Serial.print(config->maxSlip);
Serial.print(";");
Serial.print("62:");
Serial.print(config->stallSpeed);
Serial.print(";");
Serial.print("63:");
Serial.print(config->batteryLimit);
Serial.print(";");
Serial.print("64:");
Serial.print(config->firstTccGear);
Serial.print(";");
Serial.print("65:");
Serial.print(config->triggerWheelTeeth);
Serial.print(";");
Serial.print("66:");
Serial.println(config->tpsAgre);
}

void serialConfig()
{
    int key = 0;

    // Start receiving command from Serial
    while (Serial.available() > 0)
    {
        delay(3);

        if (key < INPUT_SIZE && Serial.available())
        {
            input[key] = Serial.read();
            key += 1;
        }
    }

    if (key > 0)
    {
        input[key] = 0;

        char *command = strtok(input, ";");
        while (command != 0)
        {
            char *separator = strchr(command, ':');
            if (separator != 0)
            {
                *separator = 0;
                int asset = atoi(command);
                ++separator;
           
                int value = atoi(separator);
            
                
            
                if ( asset == 7777 ) {
                  Serial.println("Ending communication..");
                  configMode = false;
                } else if ( asset == 8989 ) {
                  getFeatures();
                  getConfig(&config);
                } else if ( asset == 60000) {
                  configSet = false; featureSet = true;
                } else if ( asset == 50000) {
                  featureSet = false; configSet = true;
                } 
                if (featureSet) {
                    setFeatures(asset, value);
                }

                if (configSet) {
                  if (asset == 58 || asset == 61) {
                   float fvalue = atof(separator);
                   setConfigFloat(asset, fvalue, &config);
                  // Serial.print(asset);
                  } else {
                   setConfig(asset, value, &config);
                  }
                }
            }
            command = strtok(0, ";");
        }
        Serial.println("69696969");
    }
}
/*
void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
pollConfigMode();
if (configMode) {
  serialConfig();
}
//delay(1000);
}

*/
