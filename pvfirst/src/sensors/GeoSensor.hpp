#pragma once
#include <string>

struct GPSData {
    double latitude;
    double longitude;
    std::string city;
};

class GeoSensor {
public:
    GPSData getLocation();
};
