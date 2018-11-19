struct ConfigParam
{
    int boostMax, boostDrop, boostSpring, fuelMaxRPM, tireWidth, tireProfile, tireInches, maxRPM, nextShiftDelay, stallSpeed, maxSlip, batteryLimit, firstTccGear;
    float diffRatio;
};

struct ConfigParam readConfig();
