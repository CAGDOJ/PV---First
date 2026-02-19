#include "SolarModel.hpp"
#include <cmath>

double SolarModel::computeIrradiance(double latitude,
                                     int dayOfYear,
                                     double hour)
{
    double Gmax = 1000.0;

    double decl =
        23.45 * sin((360.0 / 365.0) *
        (284 + dayOfYear) *
        M_PI / 180.0);

    double latRad = latitude * M_PI / 180.0;
    double decRad = decl * M_PI / 180.0;

    double hourAngle =
        (hour - 12.0) * 15.0 * M_PI / 180.0;

    double sinAlpha =
        sin(latRad) * sin(decRad) +
        cos(latRad) * cos(decRad) *
        cos(hourAngle);

    if (sinAlpha < 0) return 0.0;

    return Gmax * sinAlpha;
}
