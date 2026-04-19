#pragma once

#include "energy/EnergyModel.hpp"

#include <string>

// Aqui eu concentrei os parametros do painel em uma struct simples.
// A ideia e deixar o experimento mais modular sem mudar a estrutura do projeto.
//
// Para trocar o painel, basta alterar estes valores:
// - panelMaterial: monocrystalline, polycrystalline, thinfilm
// - panelFaceType: monofacial, bifacial
// - panelAreaM2
// - baseEfficiency
// - bifacialGainFactor
struct PVConfig
{
    std::string panelMaterial = "monocrystalline";
    std::string panelFaceType = "monofacial";

    double panelAreaM2 = 10.0;
    double baseEfficiency = 0.20;
    double bifacialGainFactor = 1.10;
};

// Aqui ficam os parametros gerais do experimento.
// O jobFlops continua entrando pelo usuario durante a execucao,
// mas eu deixei um valor padrao para o caso de apertar Enter.
struct SimulationConfig
{
    double defaultJobFlops = 5e10;
    double gridCarbonIntensity = 100.0;

    PVConfig pv;
};

class SimulationController
{
public:
    SimulationController();
    void run();

private:
    double askJobFlops();
    double parseJobInput(const std::string& input);

    // A ordem importa aqui.
    // Eu deixei config antes de model porque o EnergyModel usa o fator de CO2 da config.
    SimulationConfig config;
    EnergyModel model;
};