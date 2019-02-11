#include <Arduino.h>

#define INPUT_SIZE 32

char input[INPUT_SIZE + 1];

void modifyConfig(int asset, int value)
{
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
    while (Serial.available())
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