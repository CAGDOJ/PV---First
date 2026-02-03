#include "PVModel.hpp"

namespace wrench_pvfirst {
namespace energy {

/**
 * Constructor implementation
 */
PVModel::PVModel(double area_m2,
                 double module_efficiency,
                 double system_efficiency)
    : area_m2_(area_m2),
      module_efficiency_(module_efficiency),
      system_efficiency_(system_efficiency) {
    // No additional initialization required
}

/**
 * Compute instantaneous photovoltaic power
 */
double PVModel::computePower(double irradiance_w_m2) const {
    return irradiance_w_m2 *
           area_m2_ *
           module_efficiency_ *
           system_efficiency_;
}

/**
 * Compute photovoltaic energy over a timestep
 */
double PVModel::computeEnergy(double irradiance_w_m2,
                              double delta_t_seconds) const {
    double power_watts = computePower(irradiance_w_m2);

    // Convert energy from Joules to kilowatt-hours (kWh)
    return (power_watts * delta_t_seconds) / 3'600'000.0;
}

} // namespace energy
} // namespace wrench_pvfirst
