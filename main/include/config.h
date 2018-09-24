struct ConfigParam
{
    int boostMax, boostDrop, boostSpring, fuelMaxRPM, tireWidth, tireProfile, tireInches, maxRPM, nextShiftDelay;
    float diffRatio;
};

struct ConfigParam readConfig();
