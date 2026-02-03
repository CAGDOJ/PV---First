#include <iostream>
#include <memory>

// WRENCH core
#include <wrench-dev.h>

// Energy infrastructure
#include "../energy/PVModel.hpp"
#include "../energy/HPCPowerModel.hpp"
#include "../policy/PVFirstPolicy.hpp"
#include "../metrics/WaterFootprint.hpp"
#include "../simulation/EnergyManager.hpp"

using namespace wrench;
using namespace wrench_pvfirst;

int main(int argc, char **argv) {

    // 1. Initialize WRENCH simulation
    auto simulation = wrench::Simulation::create();
    simulation->init(&argc, argv);

    std::cout << "[INFO] WRENCH simulation initialized" << std::endl;

    // 2. Load a minimal SimGrid platform
    simulation->instantiatePlatform(argv[1]);
    std::cout << "[INFO] Platform loaded" << std::endl;

    // 3. Create energy-related models
    auto pv_model = std::make_shared<energy::PVModel>(10.0, 0.18, 0.90);
    auto hpc_power_model = std::make_shared<energy::HPCPowerModel>(200.0);
    auto pv_policy = std::make_shared<policy::PVFirstPolicy>();
    auto water_metric = std::make_shared<metrics::WaterFootprint>(2.0);

    // 4. Create the EnergyManager
    simulation::EnergyManager energy_manager(
        pv_model,
        hpc_power_model,
        pv_policy,
        water_metric
    );
    std::cout << "[INFO] EnergyManager created" << std::endl;

    // 5. Simple irradiance profile (synthetic)
    double irradiance_profile[] = {0.0, 200.0, 500.0, 800.0, 600.0, 300.0, 0.0};
    int profile_size = sizeof(irradiance_profile) / sizeof(double);

    // 6. Advance simulation time manually
    for (int i = 0; i < profile_size; i++) {
        simulation->sleep(60.0);
        energy_manager.update(irradiance_profile[i]);
        std::cout << "[INFO] Time = "
                  << wrench::Simulation::getCurrentTime()
                  << " s | Irradiance = "
                  << irradiance_profile[i]
                  << " W/m²"
                  << std::endl;
    }

    // 7. Print final results
    std::cout << "\n===== Simulation Results =====" << std::endl;
    std::cout << "Total PV energy generated (kWh): "
              << energy_manager.getTotalPvEnergyKWh() << std::endl;
    std::cout << "Total grid energy consumed (kWh): "
              << energy_manager.getTotalGridEnergyKWh() << std::endl;
    std::cout << "Total water consumption (liters): "
              << energy_manager.getTotalWaterConsumptionLiters() << std::endl;
    std::cout << "================================" << std::endl;

    return 0;
}