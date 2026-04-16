#pragma once

struct WeatherImpact
{
    double cloudFactor       = 1.0;
    double rainFactor        = 1.0;
    double tempFactor        = 1.0;
    double windCoolingFactor = 1.0;

    double temperature = 28.0;
    double cloudCover  = 0.0;
    double rainAmount  = 0.0;
    double windSpeed   = 0.0;
};

class MetarSensor {
public:
    WeatherImpact getWeatherImpact(double latitude,
                                   double longitude);
};
