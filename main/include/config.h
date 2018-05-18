struct ConfigParam
{
    int boostMax, boostDrop, fuelMaxRPM, tireWidth, tireProfile, tireInches, maxRPM;
    float diffRatio;
};

struct ConfigParam readConfig();
