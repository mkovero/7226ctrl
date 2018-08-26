struct ConfigParam
{
    int boostMax, boostDrop, boostSpring, fuelMaxRPM, tireWidth, tireProfile, tireInches, maxRPM;
    float diffRatio;
};

struct ConfigParam readConfig();
