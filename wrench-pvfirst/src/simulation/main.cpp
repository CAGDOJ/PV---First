#include <iostream> // Inclui biblioteca para entrada e saída padrão
#include "energy/EnergyModel.hpp" // Inclui o cabeçalho do modelo energético

int main() {
    EnergyModel model(400.0, 1.0); // 400 gCO2/kWh, Δt = 1h

    model.update(2.0, 1.0); // P_job=2kW, P_PV=1kW
    model.update(2.0, 0.5); // P_job=2kW, P_PV=0.5kW

    EnergyStats s = model.getStats(); // Obtém estatísticas acumuladas  

    std::cout << "E_total = " << s.E_total << " kWh\n"; // Energia total consumida
    std::cout << "E_pv    = " << s.E_pv << " kWh\n"; // Energia suprida por fotovoltaica
    std::cout << "E_grid = " << s.E_grid << " kWh\n"; // Energia suprida pela rede elétrica
    std::cout << "CO2    = " << s.CO2 << " gCO2\n"; // Emissões de CO2

    return 0; // Indica que o programa terminou com sucesso
}
