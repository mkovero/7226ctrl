struct ConfigParam
{
    int boostMax, boostDrop, boostSpring, fuelMaxRPM, tireWidth, tireProfile, tireInches, maxRPM, nextShiftDelay, stallSpeed, maxSlip, batteryLimit, firstTccGear, rearDiffTeeth, triggerWheelTeeth;
    float diffRatio;
};

struct ConfigParam readConfig();
