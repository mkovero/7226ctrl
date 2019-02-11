#include <Arduino.h>

#define INPUT_SIZE 32
long lastActiveConfig = 120000;
boolean configMode = false;
// config.myVersion = 20190211

char input[INPUT_SIZE + 1];

void pollConfigMode()
{
    if (!configMode)
    {
        if (Serial.available() > 0)
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

            char *command = strchr(input, ':');
            while (command != 0)
            {
                int asset = atoi(command);
                ++command;
                int value = atoi(command);
            }
            if (asset == 13371337)
            {
                if (value == config.myVersion)
                {
                    Serial.print("69696969:");
                    Serial.println(config.myVersion);
                    configMode = true;
                    lastActiveConfig = millis();
                    if (debugEnabled)
                    {
                        Serial.println("Serial connection accepted");
                    }
                }
                else
                {
                    Serial.print("66666666:");
                    Serial.println(config.myVersion);
                    if (debugEnabled)
                    {
                        Serial.println("Serial connection failed, version mismatch");
                    }
                }
            }
        }
    }
    else
    {
        if (millis() - lastActiveConfig > config.serialTimeout)
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
void modifyConfig(int asset, int value)
{
    lastActiveConfig = millis();
    switch (asset)
    {
    case 1:
        // first setting
        // config.blaa = value;
        break;
    case 2:
        // second setting
        // config.foo = value;
        break;
    default:
        break;
    }
}

void serialConfig()
{
    int key = 0;

    // Start receiving command from Serial
    if (Serial.available())
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
                modifyConfig(asset, value);
            }

            command = strtok(0, ";");
        }
    }
}