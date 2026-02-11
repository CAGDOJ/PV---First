#include "EnergyModel.hpp" // Inclui o cabeçalho do modelo energético

EnergyModel::EnergyModel(double carbonIntensity, double deltaT) // Construtor
    : CI_grid(carbonIntensity), delta_t(deltaT) {} // Inicializa membros

void EnergyModel::update(double P_job, double P_pv) {
    // Energia total consumida no intervalo
    double E_interval = P_job * delta_t; // kWh

    // Energia fotovoltaica utilizada (PV-First)
    double E_pv_interval = std::min(P_job, P_pv) * delta_t; // kWh

    // Energia da rede elétrica
    double E_grid_interval = E_interval - E_pv_interval; // kWh

    // Acumula estatísticas
    stats.E_total += E_interval; // kWh
    stats.E_pv    += E_pv_interval; // kWh
    stats.E_grid  += E_grid_interval; // kWh
    stats.CO2     += E_grid_interval * CI_grid; // gCO2
}

EnergyStats EnergyModel::getStats() const {
    return stats; // Retorna as estatísticas acumuladas
}
