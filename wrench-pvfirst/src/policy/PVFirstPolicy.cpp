#include "PVFirstPolicy.hpp"

namespace wrench_pvfirst {
namespace policy {

/**
 * Apply PV-first policy implementation
 */
void PVFirstPolicy::apply(double pv_energy_kwh,
                          double demand_energy_kwh,
                          double &pv_used_kwh,
                          double &grid_used_kwh) const {
    // ------------------------------------------------------------------
    // EN:
    // The PV-first policy always prioritizes photovoltaic energy.
    // If PV energy is sufficient, the grid is not used.
    // Otherwise, the grid supplies the remaining demand.
    //
    // PT:
    // A política PV-first sempre prioriza a energia fotovoltaica.
    // Se a energia PV for suficiente, a rede não é utilizada.
    // Caso contrário, a rede supre a demanda restante.
    // ------------------------------------------------------------------

    if (pv_energy_kwh >= demand_energy_kwh) {
        // PV can fully satisfy the demand
        pv_used_kwh = demand_energy_kwh;
        grid_used_kwh = 0.0;
    } else {
        // PV partially satisfies the demand
        pv_used_kwh = pv_energy_kwh;
        grid_used_kwh = demand_energy_kwh - pv_energy_kwh;
    }
}

} // namespace policy
} // namespace wrench_pvfirst
