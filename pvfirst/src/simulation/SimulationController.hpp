#pragma once

#include "energy/EnergyModel.hpp"

#include <string>

class SimulationController
{
public:
    SimulationController();
    void run();

private:
    double askJobFlops();
    double parseJobInput(const std::string& input);

    EnergyModel model;
};
