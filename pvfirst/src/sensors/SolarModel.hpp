#pragma once

class SolarModel {
public:
    double computeIrradiance(double latitude,
                             int dayOfYear,
                             double hour);
};
