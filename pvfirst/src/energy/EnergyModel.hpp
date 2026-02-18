#ifndef ENERGY_MODEL_HPP
#define ENERGY_MODEL_HPP

struct EnergyStats {
    double E_total = 0.0; 
    double E_pv    = 0.0;
    double E_grid  = 0.0;
    double CO2     = 0.0;
};

class EnergyModel {
public:
    EnergyModel(double carbonIntensity);

    void update(double P_job,
                double P_pv,
                double delta_t_seconds);

    EnergyStats getStats() const;

private:
    double CI_grid;
    EnergyStats stats;
};

#endif
