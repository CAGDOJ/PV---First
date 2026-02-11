#include "SimulationController.hpp"
#include <iostream>
#include <string>
#include <cctype>

SimulationController::SimulationController()
    : model(100.0) // Média Brasil ~100 gCO2/kWh
{}

double SimulationController::parseTimeInput(const std::string& input) {

    double value = std::stod(input.substr(0, input.size() - 1));
    char unit = std::toupper(input.back());

    switch(unit) {
        case 'S': return value;
        case 'M': return value * 60.0;
        case 'H': return value * 3600.0;
        default:
            std::cout << "Unidade invalida. Usando segundos.\n";
            return value;
    }
}

void SimulationController::run() {

    std::string input;

    std::cout << "Digite o tempo de simulacao (ex: 10S, 5M, 2H): ";
    std::cin >> input;

    double total_seconds = parseTimeInput(input);

    std::cout << "\nSimulando " << total_seconds << " segundos...\n";

    // Exemplo fixo de potência
    double P_job = 2.0;  // kW
    double P_pv  = 1.0;  // kW

    model.update(P_job, P_pv, total_seconds);

    EnergyStats stats = model.getStats();

    std::cout << "\n===== RESULTADO =====\n";
    std::cout << "Tempo simulado: " << total_seconds << " segundos\n";
    std::cout << "Energia Total: " << stats.E_total << " kWh\n";
    std::cout << "Energia PV: " << stats.E_pv << " kWh\n";
    std::cout << "Energia Grid: " << stats.E_grid << " kWh\n";
    std::cout << "Emissoes CO2: " << stats.CO2 << " gCO2\n";
}
