#include <Arduino.h>
#include <EEPROM.h>
#include "include/eeprom.h"

// start reading from the first byte (address 0) of the EEPROM

int readEEPROM(int mapId, int xVal, int yVal)
{
    int modVal = 0;
    int address = mapId + xVal + yVal;
    if (debugEnabled)
    {
        Serial.print(F("[EEPROM->EEPROM] EEPROM: reading from address:"));
        Serial.println(address);
    }
    modVal = EEPROM.read(address);
    if (modVal == 0)
    {
        modVal = 100;
    };

    return modVal;
}

void writeEEPROM(int mapId, int xVal, int yVal, int modVal)
{
    int address = mapId + xVal + yVal;
    if (debugEnabled)
    {
        Serial.print(F("[EEPROM->EEPROM] writing to address:"));
        Serial.println(address);
    }
    EEPROM.write(address, modVal);
}

void resetEEPROM()
{
    for (int i = 0; i < EEPROM.length(); i++)
        EEPROM.write(i, 0);
}
