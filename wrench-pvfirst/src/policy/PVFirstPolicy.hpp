#ifndef PV_FIRST_POLICY_HPP
#define PV_FIRST_POLICY_HPP

/**
 * PV-first Energy Policy
 * Política Energética PV-first
 *
 * EN:
 * This class implements a PV-first energy management policy. The policy
 * prioritizes the use of photovoltaic (PV) energy to satisfy the energy
 * demand of an HPC node before drawing energy from the electrical grid.
 *
 * PT:
 * Esta classe implementa a política de gerenciamento energético PV-first.
 * A política prioriza o uso da energia fotovoltaica para atender à demanda
 * energética de um nó HPC antes de recorrer à rede elétrica.
 */

namespace wrench_pvfirst {
namespace policy {

class PVFirstPolicy {
public:
    /**
     * Constructor
     *
     * EN:
     * Default constructor. No internal state is required.
     *
     * PT:
     * Construtor padrão. Nenhum estado interno é necessário.
     */
    PVFirstPolicy() = default;

    /**
     * Apply PV-first policy.
     *
     * EN:
     * Determines how much energy demand is supplied by PV generation
     * and how much must be supplied by the electrical grid.
     *
     * E_PV_used   = min(E_PV_available, E_demand)
     * E_grid_used = E_demand - E_PV_used
     *
     * PT:
     * Determina quanto da demanda energética é suprida pela geração
     * fotovoltaica e quanto deve ser suprido pela rede elétrica.
     *
     * @param pv_energy_kwh     Available photovoltaic energy (kWh)
     * @param demand_energy_kwh Energy demand of the HPC node (kWh)
     * @param pv_used_kwh       Output: PV energy actually used (kWh)
     * @param grid_used_kwh     Output: Grid energy used (kWh)
     */
    void apply(double pv_energy_kwh,
               double demand_energy_kwh,
               double &pv_used_kwh,
               double &grid_used_kwh) const;
};

} // namespace policy
} // namespace wrench_pvfirst

#endif // PV_FIRST_POLICY_HPP
