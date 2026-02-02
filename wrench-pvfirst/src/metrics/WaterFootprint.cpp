#include "WaterFootprint.hpp"

namespace wrench_pvfirst {
namespace metrics {

/**
 * Constructor implementation
 */
WaterFootprint::WaterFootprint(double water_liters_per_kwh)
    : water_liters_per_kwh_(water_liters_per_kwh) {
    // No additional initialization required
}

/**
 * Compute water footprint
 */
double WaterFootprint::compute(double grid_energy_kwh) const {
    // Indirect water consumption is proportional to grid energy usage
    return grid_energy_kwh * water_liters_per_kwh_;
}

/**
 * Getter implementation
 */
double WaterFootprint::getWaterIntensity() const {
    return water_liters_per_kwh_;
}

} // namespace metrics
} // namespace wrench_pvfirst
