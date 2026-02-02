#include "HPCPowerModel.hpp"

namespace wrench_pvfirst {
namespace energy {

/**
 * Constructor implementation
 */
HPCPowerModel::HPCPowerModel(double power_watts)
    : power_watts_(power_watts) {
    // No additional initialization required
}

/**
 * Compute energy consumption over a timestep
 */
double HPCPowerModel::computeEnergy(double delta_t_seconds) const {
    // Convert energy from Joules to kilowatt-hours (kWh)
    return (power_watts_ * delta_t_seconds) / 3'600'000.0;
}

/**
 * Getter implementation
 */
double HPCPowerModel::getPowerWatts() const {
    return power_watts_;
}

} // namespace energy
} // namespace wrench_pvfirst
