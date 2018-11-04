struct ConfigParam
{
    int boostMax, boostDrop, boostSpring, fuelMaxRPM, tireWidth, tireProfile, tireInches, maxRPM, nextShiftDelay, stallSpeed, maxSlip, batteryLimit;
    float diffRatio;
};

struct ConfigParam readConfig();
