#pragma once

#include <string>

struct SimGridJobConfig
{
    std::string platformPath = "simgrid/platform.xml";
    std::string hostName     = "hpc-node";
    double jobFlops          = 5e10;
};

struct SimGridJobResult
{
    std::string hostName;
    double jobFlops        = 0.0;
    double durationSeconds = 0.0;
    double energyJoules    = 0.0;
    double energyKWh       = 0.0;
    double averagePowerKW  = 0.0;
    double hostSpeedFlops  = 0.0;
};

class SimGridJobRunner
{
public:
    SimGridJobResult run(const SimGridJobConfig& config);
};