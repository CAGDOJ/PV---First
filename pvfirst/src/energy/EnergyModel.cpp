#include "EnergyModel.hpp"

EnergyModel::EnergyModel(double carbonIntensity)
    : CI_grid(carbonIntensity)
{
}

void EnergyModel::update(double P_job,
                         double P_pv,
                         double delta_t_seconds)
{
    // Aqui eu mantenho o raciocinio mais direto:
    // 1) o job pede uma certa potencia media
    // 2) a placa tem uma potencia disponivel naquele instante
    // 3) a politica PV-First decide o que vem da placa e o que sobra para a rede
    // 4) no fim eu transformo isso em energia no intervalo do job

    double delta_t_hours = delta_t_seconds / 3600.0;

    PowerSplit split = policy.apply(P_job, P_pv);

    double E_interval      = P_job * delta_t_hours;
    double E_pv_interval   = split.pv * delta_t_hours;
    double E_grid_interval = split.grid * delta_t_hours;

    stats.E_total += E_interval;
    stats.E_pv    += E_pv_interval;
    stats.E_grid  += E_grid_interval;

    // O CO2 so entra em cima do que veio da rede.
    stats.CO2 += E_grid_interval * CI_grid;
}

EnergyStats EnergyModel::getStats() const
{
    return stats;
}
