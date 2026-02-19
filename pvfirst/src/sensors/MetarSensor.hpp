#pragma once

struct WeatherImpact {
    double cloudFactor;        // redução por nuvem
    double rainFactor;         // redução por chuva
    double tempFactor;         // impacto térmico
    double windCoolingFactor;  // melhora por vento
};

class MetarSensor {
public:
    WeatherImpact getWeatherImpact(double latitude,
                                    double longitude);
};
