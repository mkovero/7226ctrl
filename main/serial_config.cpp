#include <Arduino.h>
#include <EEPROM.h>
#include "include/config.h"
#include "include/sensors.h"
#include "include/serial_config.h"

#define INPUT_SIZE 256
long serialTimeout = 120000;
long lastActiveConfig;
boolean configMode, configSet, featureSet, upGear, downGear, tpsInit0, tpsInit1 = false;
int myVersion = 20190211;
int asset, value = 0;
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

boolean exhaustTempSensor = false; (20)
struct ConfigParam
{
    int boostMax, boostDrop, boostSpring, fuelMaxRPM, maxRPM, tireWidth, tireProfile, tireInches, rearDiffTeeth, nextShiftDelay, stallSpeed, batteryLimit, firstTccGear, triggerWheelTeeth, tpsAgre;
    float diffRatio, maxSlip;
};

struct ConfigParam config = {
  .boostMax = 700, // boost sensor max kpa
  .boostDrop = 50, // kpa to drop on shifts
  .boostSpring = 120, // kpa for wastegate spring pressure
  .fuelMaxRPM = 2000, // RPM limit to turn on fuel pumps
  .maxRPM = 7000, // Max engine RPM
  .tireWidth = 195,
  .tireProfile = 65,
  .tireInches = 15,
  .rearDiffTeeth = 29, // number of teeth in diff
  .nextShiftDelay = 2000, // ms. to wait before next shift to avoid accidental overshifting.
  .stallSpeed = 2200, // torque converter stall speed
  .batteryLimit = 11500, // battery voltage limit in 11.5v
  .firstTccGear = 2, // first gear when tcc is used.
  .triggerWheelTeeth = 6, // number of teeth in trigger wheel for RPM calculation
  .tpsAgre = 2, // 1-10 how aggressive slope tps has
  .diffRatio = 3.27,
  .maxSlip = 0.5 // Maximum allowed slip before error
};

//ConfigParam config = { 700, 50, 120, 2000, 7000, 195, 65, 15, 3.27, 29, 2000, 0.5, 2200, 11500, 2, 6, 2 };
*/
void initConfig()
{
    byte virginByte = EEPROM.read(4090);
    if (virginByte != 69)
    {
        // Virgin config set
        setFeatures(1, 1);
        setFeatures(2, 0);
        setFeatures(3, 1);
        setFeatures(4, 0);
        setFeatures(5, 0);
        setFeatures(6, 0);
        setFeatures(7, 0);
        setFeatures(8, 0);
        setFeatures(9, 0);
        setFeatures(10, 0);
        setFeatures(11, 0);
        setFeatures(12, 0);
        setFeatures(13, 0);
        setFeatures(14, 0);
        setFeatures(15, 0);
        setFeatures(16, 1);
        setFeatures(17, 1);
        setFeatures(18, 1);
        setFeatures(19, 1);
        setFeatures(20, 0);
        setFeatures(21, 0);
        setFeatures(22, 0);
        setFeatures(23, 0);
        setConfig(50, 700);
        setConfig(51, 50);
        setConfig(52, 120);
        setConfig(53, 2000);
        setConfig(54, 7000);
        setConfig(55, 195);
        setConfig(56, 55);
        setConfig(57, 15);
        setConfigFloat(58, 3.27);
        setConfig(59, 27);
        setConfig(60, 2000);
        setConfigFloat(61, 0.5);
        setConfig(62, 2200);
        setConfig(63, 11500);
        setConfig(64, 5);
        setConfig(65, 6);
        setConfig(66, 2);
        setConfigFloat(67, 1.00);
        setUpGear(1, 35);
        setUpGear(2, 72);
        setUpGear(3, 80);
        setUpGear(4, 80);
        setDownGear(2, 35);
        setDownGear(3, 17);
        setDownGear(4, 65);
        setDownGear(5, 65);

        EEPROM.put(4090, 69);
        Serial.println("Virgin init");
    }
    else
    {
        int features[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
        int config[] = {50, 51, 52, 53, 54, 55, 56, 57, 59, 60, 62, 63, 64, 65, 66};
        int configF[] = {58, 61, 67};
        int upGears[] = {1,2,3,4};
        int downGears[] = {2,3,4,5};

        for (int i = 0; i < sizeof features / sizeof features[0]; i++)
        {
            asset = features[i] * 11;
            byte featureVal;
            EEPROM.get(asset, featureVal);
            setFeatures(features[i], featureVal);
        }
        for (int i = 0; i < sizeof config / sizeof config[0]; i++)
        {
            asset = config[i] * 10;
            int configVal;
            EEPROM.get(asset, configVal);
            setConfig(config[i], configVal);
        }
        for (int i = 0; i < sizeof configF / sizeof configF[0]; i++)
        {
            asset = configF[i] * 10;
            float configFVal;
            EEPROM.get(asset, configFVal);
            setConfigFloat(configF[i], configFVal);
        }
        for (int i = 0; i < sizeof upGears / sizeof upGears[0]; i++)
        {
            asset = upGears[i] * 199;
            byte featureVal;
            EEPROM.get(asset, featureVal);
            setUpGear(upGears[i], featureVal);
        }
        for (int i = 0; i < sizeof downGears / sizeof downGears[0]; i++)
        {
            asset = downGears[i] * 197;
            byte featureVal;
            EEPROM.get(asset, featureVal);
            setDownGear(downGears[i], featureVal);
        }
    }
}
void pollConfigMode()
{
    int key = 0;
    int readData;
    static int intData;
    if (!configMode)
    {
        if (Serial.available() > 0)
        {
            readData = Serial.read();

            if (readData == 72)
            {
                Serial.print("69696969:");
                Serial.println(myVersion);
                configMode = true;
                lastActiveConfig = millis();
                readData = 0;
                if (debugEnabled)
                {
                    Serial.println("Serial connection accepted");
                }
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
            }
            else
            {
                serialConfig();
            }
        }
    }
}

void getFeatures()
{
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
    Serial.print(int(datalogger));
    Serial.print(";");
    Serial.print("20:");
    Serial.print(int(exhaustTempSensor));
    Serial.print(";");
    Serial.print("21:");
    Serial.print(int(boostLimit));
    Serial.print(";");
    Serial.print("22:");
    Serial.print(int(boostLimitShift));
    Serial.print(";");
    Serial.print("23:");
    Serial.println(int(resistiveStick));
}

void setFeatures(int asset, int value)
{
    lastActiveConfig = millis();
    if (asset > 0 && asset < 40)
    {
        int assetLocation = asset * 11;
        if (debugEnabled)
        {
            Serial.print("Setting feature: ");
            Serial.print(assetLocation);
            Serial.print(":");
            Serial.print(asset);
            Serial.print(":");
            Serial.println(value);
        }
        EEPROM.write(assetLocation, value);
    }

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
    case 20:
        exhaustTempSensor = true;
        break;
    case 21:
        boostLimit = boolean(value);
        break;
    case 22:
        boostLimitShift = boolean(value);
        break;
    case 23:
        resistiveStick = boolean(value);
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
void setConfigFloat(int asset, float value)
{
    lastActiveConfig = millis();

    if (asset > 49 && asset < 70)
    {
        int assetLocation = asset * 10;
        if (debugEnabled)
        {
            Serial.print("Setting configF: ");
            Serial.print(assetLocation);
            Serial.print(":");
            Serial.print(asset);
            Serial.print(":");
            Serial.println(value);
        }
        EEPROM.put(assetLocation, value);
    }

    switch (asset)
    {
    case 58:
        config.diffRatio = value;

        break;
    case 61:
        config.maxSlip = value;
        break;
    case 67:
        config.transSloppy = value;
        break;
    default:
        break;
    }
}

void setUpGear(int asset, int value)
{
    lastActiveConfig = millis();

    if (asset > 0 && asset < 6)
    {
        int assetLocation = asset * 199;
        if (debugEnabled)
        {
            Serial.print("Setting upGear: ");
            Serial.print(assetLocation);
            Serial.print(":");
            Serial.print(asset);
            Serial.print(":");
            Serial.println(value);
        }
        EEPROM.put(assetLocation, value);
    }

    switch (asset)
    {
    case 1:
        config.oneTotwo = value;
        break;
    case 2:
        config.twoTothree = value;
        break;
    case 3:
        config.threeTofour = value;
        break;
    case 4:
        config.fourTofive = value;
        break;
    case 5:
        break;
    default:
        break;
    }
}

void setDownGear(int asset, int value)
{
    lastActiveConfig = millis();

    if (asset > 0 && asset < 6)
    {
        int assetLocation = asset * 197;
        if (debugEnabled)
        {
            Serial.print("Setting downGear: ");
            Serial.print(assetLocation);
            Serial.print(":");
            Serial.print(asset);
            Serial.print(":");
            Serial.println(value);
        }
        EEPROM.put(assetLocation, value);
    }

    switch (asset)
    {
    case 1:
        break;
    case 2:
        config.twoToone = value;
        break;
    case 3:
        config.threeTotwo = value;
        break;
    case 4:
        config.fourTothree = value;
        break;
    case 5:
        config.fiveTofour = value;
        break;
    default:
        break;
    }
}

void getGears()
{
      Serial.print("440:440;");
    Serial.print("1:");
    Serial.print(config.oneTotwo);
    Serial.print(";");
    Serial.print("2:");
    Serial.print(config.twoTothree);
    Serial.print(";");
    Serial.print("3:");
    Serial.print(config.threeTofour);
    Serial.print(";");
    Serial.print("4:");
    Serial.println(config.fourTofive);
    Serial.print("550:550;");
    Serial.print("2:");
    Serial.print(config.twoToone);
    Serial.print(";");
    Serial.print("3:");
    Serial.print(config.threeTotwo);
    Serial.print(";");
    Serial.print("4:");
    Serial.print(config.fourTothree);
    Serial.print(";");
    Serial.print("5:");
    Serial.println(config.fiveTofour);
}
void setConfig(int asset, int value)
{
    lastActiveConfig = millis();

    if (asset > 49 && asset < 70)
    {
        int assetLocation = asset * 10;
        if (debugEnabled)
        {
            Serial.print("Setting config: ");
            Serial.print(assetLocation);
            Serial.print(":");
            Serial.print(asset);
            Serial.print(":");
            Serial.println(value);
        }
        EEPROM.put(assetLocation, value);
    }

    switch (asset)
    {
    case 50:
        config.boostMax = value;
        break;
    case 51:
        config.boostDrop = value;
        break;
    case 52:
        config.boostSpring = value;
        break;
    case 53:
        config.fuelMaxRPM = value;
        break;
    case 54:
        config.maxRPM = value;
        break;
    case 55:
        config.tireWidth = value;
        break;
    case 56:
        config.tireProfile = value;
        break;
    case 57:
        config.tireInches = value;
        break;
    case 59:
        config.rearDiffTeeth = value;
        break;
    case 60:
        config.nextShiftDelay = value;
        break;
    case 62:
        config.stallSpeed = value;
        break;
    case 63:
        config.batteryLimit = value;
        break;
    case 64:
        config.firstTccGear = value;
        break;
    case 65:
        config.triggerWheelTeeth = value;
        break;
    case 66:
        config.tpsAgre = value;
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

void getConfig()
{
    Serial.print("50000:50000;");
    Serial.print("50:");
    Serial.print(config.boostMax);
    Serial.print(";");
    Serial.print("51:");
    Serial.print(config.boostDrop);
    Serial.print(";");
    Serial.print("52:");
    Serial.print(config.boostSpring);
    Serial.print(";");
    Serial.print("53:");
    Serial.print(config.fuelMaxRPM);
    Serial.print(";");
    Serial.print("54:");
    Serial.print(config.maxRPM);
    Serial.print(";");
    Serial.print("55:");
    Serial.print(config.tireWidth);
    Serial.print(";");
    Serial.print("56:");
    Serial.print(config.tireProfile);
    Serial.print(";");
    Serial.print("57:");
    Serial.print(config.tireInches);
    Serial.print(";");
    Serial.print("58:");
    Serial.print(config.diffRatio);
    Serial.print(";");
    Serial.print("59:");
    Serial.print(config.rearDiffTeeth);
    Serial.print(";");
    Serial.print("60:");
    Serial.print(config.nextShiftDelay);
    Serial.print(";");
    Serial.print("61:");
    Serial.print(config.maxSlip);
    Serial.print(";");
    Serial.print("62:");
    Serial.print(config.stallSpeed);
    Serial.print(";");
    Serial.print("63:");
    Serial.print(config.batteryLimit);
    Serial.print(";");
    Serial.print("64:");
    Serial.print(config.firstTccGear);
    Serial.print(";");
    Serial.print("65:");
    Serial.print(config.triggerWheelTeeth);
    Serial.print(";");
    Serial.print("66:");
    Serial.print(config.tpsAgre);
    Serial.print(";");
    Serial.print("67:");
    Serial.println(config.transSloppy);
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

                if (asset == 7777)
                {
                    Serial.println("Ending communication..");
                    configMode = false;
                }
                else if (asset == 8989)
                {
                    getFeatures();
                    getConfig();
                    getGears();
                }
                else if (asset == 60000)
                {
                    downGear = false;
                    upGear = false;
                    configSet = false;
                    tpsInit0 = false;  
                    tpsInit1 = false;  
                    featureSet = true;
                }
                else if (asset == 50000)
                {
                    downGear = false;
                    upGear = false;
                    featureSet = false;
                    tpsInit0 = false;  
                    tpsInit1 = false;  
                    configSet = true;
                }
                else if (asset == 440)
                {
                    configSet = false;
                    featureSet = false;
                    downGear = false;
                    tpsInit0 = false;  
                    tpsInit1 = false;  
                    upGear = true;
                }
                else if (asset == 550)
                {
                    configSet = false;
                    featureSet = false;
                    upGear = false;
                    tpsInit0 = false;  
                    tpsInit1 = false;  
                    downGear = true;
                }
                else if (asset == 1100) {
                     configSet = false;
                    featureSet = false;
                    upGear = false;
                    downGear = false;
                    tpsInit0 = true;  
                    tpsInit1 = false;                 
                }
                else if (asset == 2200) {
                     configSet = false;
                    featureSet = false;
                    upGear = false;
                    downGear = false;
                    tpsInit0 = false;  
                    tpsInit1 = true;                      
                }

                if (featureSet)
                {
                    setFeatures(asset, value);
                }

                if (configSet)
                {
                    if (asset == 58 || asset == 61 || asset == 67)
                    {
                        float fvalue = atof(separator);
                        setConfigFloat(asset, fvalue);
                        // Serial.print(asset);
                    }
                    else
                    {
                        setConfig(asset, value);
                    }
                }
                if (upGear)
                {
                    setUpGear(asset, value);
                }
                if (downGear)
                {
                    setDownGear(asset, value);
                }
                if (tpsInit0) {
                    tpsInit(0);
                }
                if (tpsInit1) {
                    tpsInit(1);
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
*/
void serialWatch(Task *me)
{
    // put your main code here, to run repeatedly:
    pollConfigMode();
    if (configMode)
    {
        serialConfig();
    }
}
