#include <Arduino.h>
#include <EEPROM.h>

// start reading from the first byte (address 0) of the EEPROM


int readEEPROM(int mapId, int xVal, int yVal)
{
    int modVal = 0;
    byte value;

    // Data stored in eeprom is
    // [mapId][xVal][yVal][modVal] = 4bytes
    // mapId is always greater than 100.
    int address = mapId + xVal + yVal;
    Serial.print("EEPROM: reading from address:");
    Serial.println(address);
    modVal = EEPROM.read(address);

    return modVal;

    if (address == EEPROM.length())
    {
        address = 0;
    }

    /***
    As the EEPROM sizes are powers of two, wrapping (preventing overflow) of an
    EEPROM address is also doable by a bitwise and of the length - 1.

    ++address &= EEPROM.length() - 1;
  ***/
}

int writeEEPROM(int mapId, int xVal, int yVal, int modVal)
{
    byte value;

    // Data stored in eeprom is
    // [mapId][xVal][yVal][modVal] = 4bytes
    // mapId is greater than 10000.
    int address = mapId + xVal + yVal;
    Serial.print("EEPROM: writing to address:");
    Serial.println(address);
    EEPROM.write(address, modVal);

    if (address == EEPROM.length())
    {
        address = 0;
    }

    /***
    As the EEPROM sizes are powers of two, wrapping (preventing overflow) of an
    EEPROM address is also doable by a bitwise and of the length - 1.

    ++address &= EEPROM.length() - 1;
  ***/
}

