#ifndef ENERGY_MANAGER_HPP
#define ENERGY_MANAGER_HPP

#include <memory>

// WRENCH / SimGrid core
#include <wrench-dev.h>

// Energy models
#include "../energy/PVModel.hpp"
#include "../energy/HPCPowerModel.hpp"

// Policy
#include "../policy/PVFirstPolicy.hpp"

// Environmental metric
#include "../metrics/WaterFootprint.hpp"

/**
 * EnergyManager
 * Gerenciador Energético
 *
 * EN:
 * This class integrates energy models, energy policies, and environmental
 * metrics with the WRENCH simulation clock. It is responsible for computing
 * energy generation, energy consumption, applying the PV-first policy, and
 * estimating the indirect water footprint at each simulation time advance.
 *
 * PT:
 * Esta classe integra modelos energéticos, políticas energéticas e métricas
 * ambientais ao relógio de simulação do WRENCH. Ela é responsável por calcular
 * a geração e o consumo de energia, aplicar a política PV-first e estimar a
 * pegada hídrica indireta a cada avanço temporal da simulação.
 */

namespace wrench_pvfirst {
namespace simulation {

class EnergyManager {
public:
    /**
     * Constructor
     *
     * EN:
     * Initializes the EnergyManager and synchronizes it with the current
     * WRENCH simulation time.
     *
     * PT:
     * Inicializa o EnergyManager e o sincroniza com o tempo atual da
     * simulação do WRENCH.
     */
    EnergyManager(
        std::shared_ptr<wrench_pvfirst::energy::PVModel> pv_model,
        std::shared_ptr<wrench_pvfirst::energy::HPCPowerModel> hpc_power_model,
        std::shared_ptr<wrench_pvfirst::policy::PVFirstPolicy> pv_policy,
        std::shared_ptr<wrench_pvfirst::metrics::WaterFootprint> water_metric
    );

    /**
     * Update method
     *
     * EN:
     * This method must be called whenever the WRENCH simulation time advances.
     * It computes the elapsed time, energy generation, energy consumption,
     * applies the PV-first policy, and updates environmental metrics.
     *
     * PT:
     * Este método deve ser chamado sempre que o tempo da simulação do WRENCH
     * avança. Ele calcula o intervalo de tempo, a geração e o consumo de
     * energia, aplica a política PV-first e atualiza as métricas ambientais.
     *
     * @param irradiance_w_m2 Current solar irradiance (W/m²)
     */
    void update(double irradiance_w_m2);

    // --- Getters (for logging and analysis) ---

    double getTotalPvEnergyKWh() const;
    double getTotalGridEnergyKWh() const;
    double getTotalWaterConsumptionLiters() const;

private:
    // Energy models
    std::shared_ptr<wrench_pvfirst::energy::PVModel> pv_model_;
    std::shared_ptr<wrench_pvfirst::energy::HPCPowerModel> hpc_power_model_;

    // Policy
    std::shared_ptr<wrench_pvfirst::policy::PVFirstPolicy> pv_policy_;

    // Environmental metric
    std::shared_ptr<wrench_pvfirst::metrics::WaterFootprint> water_metric_;

    // Time tracking (seconds)
    double last_update_time_seconds_;

    // Accumulated metrics
    double total_pv_energy_kwh_;
    double total_grid_energy_kwh_;
    double total_water_liters_;
};

} // namespace simulation
} // namespace wrench_pvfirst

#endif // ENERGY_MANAGER_HPP
