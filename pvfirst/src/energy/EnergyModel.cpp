#include "EnergyModel.hpp"
#include <algorithm>

EnergyModel::EnergyModel(double carbonIntensity)
    : CI_grid(carbonIntensity)
{}

void EnergyModel::update(double P_job,
                         double P_pv,
                         double delta_t_seconds)
{
    double delta_t_hours = delta_t_seconds / 3600.0;

    double E_interval = P_job * delta_t_hours;

    double E_pv_interval = std::min(P_job, P_pv) * delta_t_hours;

    double E_grid_interval = E_interval - E_pv_interval;

    stats.E_total += E_interval;
    stats.E_pv    += E_pv_interval;
    stats.E_grid  += E_grid_interval;

    stats.CO2 += E_grid_interval * CI_grid;
}

EnergyStats EnergyModel::getStats() const {
    return stats;
}
