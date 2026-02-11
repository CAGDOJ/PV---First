#ifndef ENERGY_MODEL_HPP // ENERGY_MODEL_HPP
#define ENERGY_MODEL_HPP // ENERGY_MODEL_HPP

#include <algorithm> // Para std::min

/**
 * @brief Estrutura para armazenar métricas energéticas de um job.
 */
struct EnergyStats {
    double E_total = 0.0;   // Energia total consumida (kWh)
    double E_pv    = 0.0;   // Energia suprida por fotovoltaica (kWh)
    double E_grid  = 0.0;   // Energia suprida pela rede elétrica (kWh)
    double CO2     = 0.0;   // Emissões de CO2 (gCO2)
};

/**
 * @brief Modelo energético baseado na política PV-First.
 *
 * Implementa diretamente o modelo matemático descrito no Capítulo 3
 * da dissertação.
 */
class EnergyModel {
public:
    /**
     * @param carbonIntensity Intensidade de carbono da rede elétrica (gCO2/kWh)
     * @param deltaT Intervalo de discretização temporal (horas)
     */
    EnergyModel(double carbonIntensity, double deltaT);

    /**
     * @brief Atualiza o consumo energético em um intervalo de tempo.
     *
     * @param P_job Potência demandada pelo job (kW)
     * @param P_pv Potência fotovoltaica disponível (kW)
     */
    void update(double P_job, double P_pv);

    /**
     * @brief Retorna as métricas energéticas acumuladas.
     */
    EnergyStats getStats() const;

private:
    double CI_grid;   // Intensidade de carbono da rede
    double delta_t;   // Passo de tempo (h)

    EnergyStats stats;
};

#endif // ENERGY_MODEL_HPP
