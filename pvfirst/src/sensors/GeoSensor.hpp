#pragma once
#include <string>

struct GPSData {
    double latitude  = -1.4558;
    double longitude = -48.4902;
    std::string city = "Belem";
};

class GeoSensor {
public:
    GPSData getLocation();
};
