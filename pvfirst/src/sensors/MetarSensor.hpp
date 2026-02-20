#pragma once

struct WeatherImpact
{
    double cloudFactor;
    double rainFactor;
    double tempFactor;
    double windCoolingFactor;

    // 🔹 valores reais medidos
    double temperature;
    double cloudCover;
    double rainAmount;
    double windSpeed;
};


class MetarSensor {
public:
    WeatherImpact getWeatherImpact(double latitude,
                                    double longitude);
};
