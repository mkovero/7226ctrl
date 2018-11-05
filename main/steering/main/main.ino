#include <Arduino.h>
// Electric steering wheel control
// Basically its just resistor ladder we're reading here and sending values over HC-11 radio transmitter
// Unfortunately it is best to use strings for messaging as they are not so easily mixed with interfering radio traffic.
double lastPress;
boolean debugEnabled = true;

void setup()
{
    Serial1.begin(9600); // Serial port to HC11
    pinMode(A0, INPUT);  // Resistor ladder members
    pinMode(A1, INPUT);
}

void loop()
{
    int rightSide = analogRead(A0);
    int leftSide = analogRead(A1);
    if ((millis() - lastPress > 500))
    {
        if (rightSide > 860 && rightSide < 890)
        {
            lastPress = millis();
            Serial1.write(100);
        }
        if (rightSide > 730 && rightSide < 790)
        {
            lastPress = millis();
            Serial1.write(101);
        }
        if (leftSide > 860 && leftSide < 890)
        {
            lastPress = millis();
            Serial1.write(200);
        }
        if (leftSide > 730 && leftSide < 790)
        {
            lastPress = millis();
            Serial1.write(201);
        }
        if ((rightSide > 600 && rightSide < 670) && (leftSide > 600 && leftSide < 670)) {
            lastPress = millis();
            Serial1.write(249); 
        }
        if ((rightSide > 600 && rightSide < 670) && (leftSide < 600 && leftSide > 670))
        {
            lastPress = millis();
            Serial1.write(150);
        }
        if (rightSide > 430 && rightSide < 490)
        {
            lastPress = millis();
            Serial1.write(151);
        }
        if (leftSide > 600 && leftSide < 670) 
        {
            lastPress = millis();
            Serial1.write(160);
        }
        if (leftSide > 430 && leftSide < 490)
        {
            lastPress = millis();
            Serial1.write(161);
        }
        if (leftSide < 400 || rightSide < 400)
        {
            Serial1.write(55);
        }
        Serial1.flush();
    }
    delay(100);
    //if (debugEnabled) { Serial.print(rightSide); Serial.print("-"); Serial.println(leftSide); }
}
