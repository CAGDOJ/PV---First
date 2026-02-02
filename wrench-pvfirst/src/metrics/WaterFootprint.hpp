#ifndef WATER_FOOTPRINT_HPP
#define WATER_FOOTPRINT_HPP

/**
 * Water Footprint Model
 * Modelo de Pegada Hídrica
 *
 * EN:
 * This class implements a model to estimate the indirect water footprint
 * associated with electricity consumption from the power grid.
 * The model converts grid energy usage (kWh) into water consumption (liters)
 * using a predefined water intensity factor.
 *
 * PT:
 * Esta classe implementa um modelo para estimar a pegada hídrica indireta
 * associada ao consumo de energia elétrica da rede. O modelo converte o
 * consumo de energia da rede (kWh) em consumo de água (litros) utilizando
 * um fator de intensidade hídrica.
 */

namespace wrench_pvfirst {
namespace metrics {

class WaterFootprint {
public:
    /**
     * Constructor
     *
     * EN:
     * Initializes the water footprint model.
     *
     * PT:
     * Inicializa o modelo de pegada hídrica.
     *
     * @param water_liters_per_kwh Water intensity factor (L/kWh)
     */
    explicit WaterFootprint(double water_liters_per_kwh);

    /**
     * Compute water footprint.
     *
     * EN:
     * Computes the indirect water consumption associated with grid
     * electricity usage.
     *
     * W(t) = E_grid(t) * WF
     *
     * PT:
     * Calcula o consumo indireto de água associado ao uso de energia
     * elétrica da rede.
     *
     * @param grid_energy_kwh Energy drawn from the grid (kWh)
     * @return Water consumption (liters)
     */
    double compute(double grid_energy_kwh) const;

    /**
     * Getter
     *
     * EN:
     * Returns the water intensity factor.
     *
     * PT:
     * Retorna o fator de intensidade hídrica.
     */
    double getWaterIntensity() const;

private:
    // Water consumption factor (liters per kWh)
    double water_liters_per_kwh_;
};

} // namespace metrics
} // namespace wrench_pvfirst

#endif // WATER_FOOTPRINT_HPP
