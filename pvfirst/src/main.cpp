#include "simulation/SimulationController.hpp"

#include <exception>
#include <iostream>
#include <xbt/log.h>

int main()
{
    // aqui eu abaixo o log do plugin de energia para nao poluir a saida
    xbt_log_control_set("host_energy.thres:critical");

    try {
        SimulationController controller;
        controller.run();
    }
    catch (const std::exception& e) {
        std::cerr << "\n============================================================\n";
        std::cerr << "O programa parou porque encontrou um erro.\n";
        std::cerr << "Detalhe: " << e.what() << "\n";
        std::cerr << "============================================================\n";
        return 1;
    }
    catch (...) {
        std::cerr << "\n============================================================\n";
        std::cerr << "O programa parou por causa de um erro inesperado.\n";
        std::cerr << "============================================================\n";
        return 1;
    }

    return 0;
}