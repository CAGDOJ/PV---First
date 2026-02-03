#include "EnergyManager.hpp"

namespace wrench_pvfirst {
namespace simulation {

/**
 * Constructor implementation
 */
EnergyManager::EnergyManager(
    std::shared_ptr<wrench_pvfirst::energy::PVModel> pv_model,
    std::shared_ptr<wrench_pvfirst::energy::HPCPowerModel> hpc_power_model,
    std::shared_ptr<wrench_pvfirst::policy::PVFirstPolicy> pv_policy,
    std::shared_ptr<wrench_pvfirst::metrics::WaterFootprint> water_metric
)
    : pv_model_(pv_model),
      hpc_power_model_(hpc_power_model),
      pv_policy_(pv_policy),
      water_metric_(water_metric),
      // Synchronize with WRENCH simulation clock
      last_update_time_seconds_(
          wrench::Simulation::getCurrentSimulationTime()
      ),
      total_pv_energy_kwh_(0.0),
      total_grid_energy_kwh_(0.0),
      total_water_liters_(0.0) {}

/**
 * Update method implementation
 */
void EnergyManager::update(double irradiance_w_m2) {

    // ------------------------------------------------------------
    // 1. Get current simulation time from WRENCH
    // ------------------------------------------------------------
    double current_time_seconds =
        wrench::Simulation::getCurrentSimulationTime();

    // ------------------------------------------------------------
    // 2. Compute elapsed time since last update
    // ------------------------------------------------------------
    double delta_t_seconds =
        current_time_seconds - last_update_time_seconds_;

    // ------------------------------------------------------------
    // 3. Safety check (discrete-event simulation)
    // ------------------------------------------------------------
    if (delta_t_seconds <= 0.0) {
        return;
    }

    // ------------------------------------------------------------
    // 4. Compute energy demand of the HPC node
    // ------------------------------------------------------------
    double hpc_demand_kwh =
        hpc_power_model_->computeEnergy(delta_t_seconds);

    // ------------------------------------------------------------
    // 5. Compute photovoltaic energy generation
    // ------------------------------------------------------------
    double pv_energy_kwh =
        pv_model_->computeEnergy(irradiance_w_m2, delta_t_seconds);

    // ------------------------------------------------------------
    // 6. Apply PV-first policy
    // ------------------------------------------------------------
    double pv_used_kwh = 0.0;
    double grid_used_kwh = 0.0;

    pv_policy_->apply(
        pv_energy_kwh,
        hpc_demand_kwh,
        pv_used_kwh,
        grid_used_kwh
    );

    // ------------------------------------------------------------
    // 7. Update accumulated energy metrics
    // ------------------------------------------------------------
    total_pv_energy_kwh_ += pv_used_kwh;
    total_grid_energy_kwh_ += grid_used_kwh;

    // ------------------------------------------------------------
    // 8. Compute and accumulate water footprint
    // ------------------------------------------------------------
    double water_liters =
        water_metric_->compute(grid_used_kwh);

    total_water_liters_ += water_liters;

    // ------------------------------------------------------------
    // 9. Update internal time reference
    // ------------------------------------------------------------
    last_update_time_seconds_ = current_time_seconds;
}

// --- Getters ---

double EnergyManager::getTotalPvEnergyKWh() const {
    return total_pv_energy_kwh_;
}

double EnergyManager::getTotalGridEnergyKWh() const {
    return total_grid_energy_kwh_;
}

double EnergyManager::getTotalWaterConsumptionLiters() const {
    return total_water_liters_;
}

} // namespace simulation
} // namespace wrench_pvfirst
