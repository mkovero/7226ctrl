// Electric steering wheel control
// Basically its just resistor ladder we're reading here and sending values over HC-11 radio transmitter
// Unfortunately it is best to use strings for messaging as they are not so easily mixed with interfering radio traffic.
double lastPress;

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

    if (millis() - lastPress > 500)
    {
        if (rightSide > 860 && rightSide < 890)
        {
            lastPress = millis();
            Serial1.print("VolUP");
        }
        if (rightSide > 730 && rightSide < 790)
        {
            lastPress = millis();
            Serial1.print("VolDOWN");
        }
        if (leftSide > 860 && leftSide < 890)
        {
            lastPress = millis();
            Serial1.print("ArrowUP");
        }
        if (leftSide > 730 && leftSide < 790)
        {
            lastPress = millis();
            Serial1.print("ArrowDOWN");
        }
        if (rightSide > 600 && rightSide < 670)
        {
            lastPress = millis();
            Serial1.print("PickupPhone");
        }
        if (rightSide > 430 && rightSide < 490)
        {
            lastPress = millis();
            Serial1.print("HangPhone");
        }
        if (leftSide > 600 && leftSide < 670)
        {
            lastPress = millis();
            Serial1.print("MenuNext");
        }
        if (leftSide > 430 && leftSide < 490)
        {
            lastPress = millis();
            Serial1.print("MenuPrev");
        }
        if (leftSide < 400 || rightSide < 400)
        {
            Serial.print("TOOT");
        }
        Serial1.flush();
        //if (debugEnabled) { Serial.print(rightSide); Serial.print("-"); Serial.println(leftSide); }
    }
}