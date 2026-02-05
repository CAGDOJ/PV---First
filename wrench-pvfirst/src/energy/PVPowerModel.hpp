#ifndef HPC_POWER_MODEL_HPP
#define HPC_POWER_MODEL_HPP

/**
 * HPC Power Consumption Model
 * Modelo de Consumo Energético de HPC
 *
 * EN:
 * This class implements a power consumption model for an HPC compute node.
 * It represents the average power draw of a host during task execution
 * and is intended to be coupled to the WRENCH simulation clock by an
 * external energy manager.
 *
 * PT:
 * Esta classe implementa um modelo de consumo energético para um nó de
 * Computação de Alto Desempenho (HPC). O modelo representa o consumo médio
 * de potência do host durante a execução de tarefas e deve ser acoplado
 * ao relógio de simulação do WRENCH por meio de um gerenciador externo.
 */

namespace wrench_pvfirst {
namespace energy {

class HPCPowerModel {
public:
    /**
     * Constructor
     *
     * EN:
     * Initializes the HPC power model.
     *
     * PT:
     * Inicializa o modelo de consumo energético do nó HPC.
     *
     * @param power_watts Average power consumption of the host (W)
     */
    explicit HPCPowerModel(double power_watts);

    /**
     * Compute energy consumption over a timestep.
     *
     * EN:
     * Computes the energy consumed by the HPC node during a given
     * simulation timestep.
     *
     * PVPowerModel(t) = (P_host * Δt) (Adicionar uma costante de Energia\variavel )(deixar pra abranger)
     *
     * PT:
     * Calcula a energia consumida pelo nó HPC durante um intervalo
     * de tempo da simulação.
     *
     * @param delta_t_seconds Timestep duration (seconds)
     * @return Energy consumption (kWh)
     */
    double computeEnergy(double delta_t_seconds) const;

    /**
     * Getter
     *
     * EN:
     * Returns the average power consumption of the host.
     *
     * PT:
     * Retorna a potência média consumida pelo host.
     */
    double getPowerWatts() const;

private:
    // Average power consumption of the HPC node (W)
    double power_watts_;
};

} // namespace energy
} // namespace wrench_pvfirst

#endif // HPC_POWER_MODEL_HPP
