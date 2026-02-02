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

    // ------------------------------------------------------------
    // 1. Initialize WRENCH simulation
    // ------------------------------------------------------------
    Simulation simulation;
    simulation.init(&argc, argv);

    std::cout << "[INFO] WRENCH simulation initialized" << std::endl;

    // ------------------------------------------------------------
    // 2. Load a minimal SimGrid platform
    // ------------------------------------------------------------
    // NOTE:
    // You MUST provide a platform XML file when running.
    // Example:
    // ./pvfirst_example platform.xml
    //
    // The platform defines hosts and network.
    simulation.instantiatePlatform(argv[1]);

    std::cout << "[INFO] Platform loaded" << std::endl;

    // ------------------------------------------------------------
    // 3. Create energy-related models
    // ------------------------------------------------------------
    auto pv_model = std::make_shared<energy::PVModel>(
        10.0,   // PV area (m²)
        0.18,   // Module efficiency
        0.90    // System efficiency
    );

    auto hpc_power_model = std::make_shared<energy::HPCPowerModel>(
        200.0   // Average power consumption (W)
    );

    auto pv_policy = std::make_shared<policy::PVFirstPolicy>();

    auto water_metric = std::make_shared<metrics::WaterFootprint>(
        2.0     // Liters per kWh
    );

    // ------------------------------------------------------------
    // 4. Create the EnergyManager (core integration point)
    // ------------------------------------------------------------
    simulation::EnergyManager energy_manager(
        pv_model,
        hpc_power_model,
        pv_policy,
        water_metric
    );

    std::cout << "[INFO] EnergyManager created" << std::endl;

    // ------------------------------------------------------------
    // 5. Simple irradiance profile (synthetic)
    // ------------------------------------------------------------
    // This simulates a day-like irradiance evolution
    double irradiance_profile[] = {
        0.0, 200.0, 500.0, 800.0, 600.0, 300.0, 0.0
    };

    int profile_size = sizeof(irradiance_profile) / sizeof(double);

    // ------------------------------------------------------------
    // 6. Advance simulation time manually
    // ------------------------------------------------------------
    // NOTE:
    // This is a SIMPLE example.
    // In real WRENCH simulations, time advances due to workflow events.
    for (int i = 0; i < profile_size; i++) {

        // Advance simulation time by 60 seconds
        simulation.sleep(60.0);

        // Update energy manager using current irradiance
        energy_manager.update(irradiance_profile[i]);

        std::cout << "[INFO] Time = "
                  << Simulation::getCurrentSimulationTime()
                  << " s | Irradiance = "
                  << irradiance_profile[i]
                  << " W/m²"
                  << std::endl;
    }

    // ------------------------------------------------------------
    // 7. Print final results
    // ------------------------------------------------------------
    std::cout << "\n===== Simulation Results =====" << std::endl;
    std::cout << "Total PV energy generated (kWh): "
              << energy_manager.getTotalPvEnergyKWh()
              << std::endl;

    std::cout << "Total grid energy consumed (kWh): "
              << energy_manager.getTotalGridEnergyKWh()
              << std::endl;

    std::cout << "Total water consumption (liters): "
              << energy_manager.getTotalWaterConsumptionLiters()
              << std::endl;

    std::cout << "================================" << std::endl;

    return 0;
}
