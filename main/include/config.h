
struct ConfigParam
{
    int boostMax, boostDrop, boostSpring, fuelMaxRPM, maxRPM, tireWidth, tireProfile, tireInches, rearDiffTeeth, nextShiftDelay, stallSpeed, batteryLimit, firstTccGear, triggerWheelTeeth, tpsAgre;
    float diffRatio, maxSlip, transSloppy;
};

extern struct ConfigParam config;