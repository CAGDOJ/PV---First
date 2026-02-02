#ifndef PV_MODEL_HPP
#define PV_MODEL_HPP

/**
 * Photovoltaic Energy Generation Model
 * Modelo de Geração de Energia Fotovoltaica
 *
 * EN:
 * This class implements a photovoltaic (PV) energy generation model.
 * It converts solar irradiance into electrical power and energy using
 * classical PV equations. The model is independent from the simulation
 * environment and must be coupled to the WRENCH simulation clock by
 * an external manager.
 *
 * PT:
 * Esta classe implementa um modelo de geração de energia fotovoltaica (PV).
 * O modelo converte irradiância solar em potência e energia elétrica
 * utilizando equações fotovoltaicas clássicas. Este modelo é independente
 * do ambiente de simulação e deve ser acoplado ao relógio do WRENCH por
 * meio de um gerenciador externo.
 */

namespace wrench_pvfirst {
namespace energy {

class PVModel {
public:
    /**
     * Constructor
     *
     * EN:
     * Initializes the photovoltaic model parameters.
     *
     * PT:
     * Inicializa os parâmetros do modelo fotovoltaico.
     *
     * @param area_m2 Total PV panel area (m²)
     * @param module_efficiency PV module efficiency (0–1)
     * @param system_efficiency Overall system efficiency (0–1)
     */
    PVModel(double area_m2,
            double module_efficiency,
            double system_efficiency);

    /**
     * Compute instantaneous photovoltaic power.
     *
     * EN:
     * Computes the PV power based on solar irradiance.
     *
     * P_PV(t) = G(t) * A * η_module * η_system
     *
     * PT:
     * Calcula a potência fotovoltaica com base na irradiância solar.
     *
     * @param irradiance_w_m2 Solar irradiance (W/m²)
     * @return Photovoltaic power (W)
     */
    double computePower(double irradiance_w_m2) const;

    /**
     * Compute photovoltaic energy over a timestep.
     *
     * EN:
     * Computes the PV energy generated during a simulation timestep.
     *
     * E_PV(t) = (P_PV(t) * Δt) / 3,600,000
     *
     * PT:
     * Calcula a energia fotovoltaica gerada durante um intervalo de tempo
     * da simulação.
     *
     * @param irradiance_w_m2 Solar irradiance (W/m²)
     * @param delta_t_seconds Timestep duration (seconds)
     * @return Generated energy (kWh)
     */
    double computeEnergy(double irradiance_w_m2,
                         double delta_t_seconds) const;

private:
    // Total photovoltaic panel area (m²)
    double area_m2_;

    // PV module efficiency (0–1)
    double module_efficiency_;

    // Overall system efficiency (0–1)
    double system_efficiency_;
};

} // namespace energy
} // namespace wrench_pvfirst

#endif // PV_MODEL_HPP
