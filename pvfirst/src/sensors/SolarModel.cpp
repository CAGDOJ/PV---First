#include "SolarModel.hpp"
#include <cmath>

double SolarModel::computeIrradiance(double latitude,
                                     int dayOfYear,
                                     double hour)
{
    // O que eu fiz aqui foi manter um modelo solar simples e didatico.
    // Ele usa:
    // - a latitude do local
    // - o dia do ano
    // - a hora local
    // para estimar uma irradiancia teorica maxima naquele instante.

    double Gmax = 1000.0;

    double decl =
        23.45 * std::sin((360.0 / 365.0) *
        (284 + dayOfYear) *
        M_PI / 180.0);

    double latRad = latitude * M_PI / 180.0;
    double decRad = decl * M_PI / 180.0;

    double hourAngle =
        (hour - 12.0) * 15.0 * M_PI / 180.0;

    double sinAlpha =
        std::sin(latRad) * std::sin(decRad) +
        std::cos(latRad) * std::cos(decRad) *
        std::cos(hourAngle);

    if (sinAlpha < 0.0)
        return 0.0;

    return Gmax * sinAlpha;
}
