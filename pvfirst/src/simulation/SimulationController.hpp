#ifndef SIMULATION_CONTROLLER_HPP
#define SIMULATION_CONTROLLER_HPP

#include <string>
#include "energy/EnergyModel.hpp"
#include "sensors/GeoSensor.hpp"
#include "sensors/SolarModel.hpp"


class SimulationController {
public:
    SimulationController();
    void run();

private:
    EnergyModel model;

    double parseTimeInput(const std::string& input);
};

#endif
