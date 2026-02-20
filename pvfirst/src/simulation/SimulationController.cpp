#include "SimulationController.hpp"
#include "sensors/GeoSensor.hpp"
#include "sensors/SolarModel.hpp"
#include "sensors/MetarSensor.hpp"

#include <iostream>
#include <string>
#include <cctype>
#include <ctime>
#include <fstream>


SimulationController::SimulationController()
    : model(100.0) // intensidade média carbono (gCO2/kWh)
{}

double SimulationController::parseTimeInput(const std::string& input)
{
    double value = std::stod(input.substr(0, input.size() - 1));
    char unit = std::toupper(input.back());

    switch(unit)
    {
        case 'S': return value;
        case 'M': return value * 60.0;
        case 'H': return value * 3600.0;
        default:
            std::cout << "Unidade invalida. Usando segundos.\n";
            return value;
    }
}

void SimulationController::run()
{
    std::string input;

    std::cout << "Digite o tempo de simulacao (ex: 10S, 5M, 2H): ";
    std::cin >> input;

    double total_seconds = parseTimeInput(input);

    std::cout << "\nSimulando "
              << total_seconds
              << " segundos...\n";

    // =====================================================
    // 1️⃣ GPS AUTOMÁTICO
    // =====================================================

    GeoSensor geo;
    GPSData gps = geo.getLocation();

    std::cout << "Local detectado: "
              << gps.city
              << " (Lat: " << gps.latitude
              << ", Lon: " << gps.longitude
              << ")\n";

    // =====================================================
    // 2️⃣ DATA E HORA LOCAL
    // =====================================================

    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);

    int dayOfYear = local->tm_yday + 1;

    int hourInt = local->tm_hour;
    int minuteInt = local->tm_min;

    double hour =
        hourInt + minuteInt / 60.0;

    std::cout << "Hora local: "
              << (hourInt < 10 ? "0" : "")
              << hourInt << ":"
              << (minuteInt < 10 ? "0" : "")
              << minuteInt << "\n";

    std::cout << "Dia do ano: "
              << dayOfYear << "\n";

    // =====================================================
    // 3️⃣ MODELO SOLAR
    // =====================================================

    SolarModel solar;

    double G =
        solar.computeIrradiance(
            gps.latitude,
            dayOfYear,
            hour);

    std::cout << "Irradiancia teorica: "
              << G << " W/m2\n";

    // =====================================================
    // 4️⃣ WEATHER GLOBAL
    // =====================================================

    MetarSensor metar;

    WeatherImpact impact =
        metar.getWeatherImpact(
            gps.latitude,
            gps.longitude);

        std::cout << "\n===== CONDICOES METEOROLOGICAS =====\n";

        // 🌥 NUVENS
        if (impact.cloudFactor == 1.0)
            std::cout << "Cobertura de nuvens: Ceu limpo (sem reducao de irradiancia)\n";
        else
            std::cout << "Cobertura de nuvens: Reducao de "
                    << (1.0 - impact.cloudFactor) * 100
                    << "% na irradiancia\n";

        // 🌧 CHUVA
        if (impact.rainFactor == 1.0)
            std::cout << "Chuva: Nao esta chovendo\n";
        else
            std::cout << "Chuva: Reducao significativa na geracao devido a precipitacao\n";

        // 🌡 TEMPERATURA
        if (impact.tempFactor == 1.0)
            std::cout << "Temperatura: Sem perda termica significativa\n";
        else
            std::cout << "Temperatura: Perda de "
                    << (1.0 - impact.tempFactor) * 100
                    << "% na eficiencia do modulo\n";

        // 💨 VENTO
        if (impact.windCoolingFactor == 1.0)
            std::cout << "Vento: Sem impacto significativo\n";
        else
            std::cout << "Vento: Ganho de "
                    << (impact.windCoolingFactor - 1.0) * 100
                    << "% por resfriamento do modulo\n";



    // =====================================================
    // 5️⃣ APLICAR IMPACTOS
    // =====================================================

    G *= impact.cloudFactor;
    G *= impact.rainFactor;

    std::cout << "Irradiancia ajustada: "
              << G << " W/m2\n";

    // =====================================================
    // 6️⃣ EFICIENCIA FINAL
    // =====================================================

    double baseEfficiency = 0.20;

    double efficiency =
        baseEfficiency *
        impact.tempFactor *
        impact.windCoolingFactor;

    std::cout << "Eficiencia final: "
              << efficiency << "\n";

    // =====================================================
    // 7️⃣ POTENCIA FV
    // =====================================================

    double area = 10.0;

    double P_pv =
        efficiency * area * G / 1000.0;

    std::cout << "Potencia PV estimada: "
              << P_pv << " kW\n";

    // =====================================================
    // 8️⃣ ENERGY MODEL
    // =====================================================

    double P_job = 2.0;

    model.update(P_job,
                 P_pv,
                 total_seconds);

    EnergyStats stats =
        model.getStats();

    std::cout << "\n===== RESULTADO =====\n";
    std::cout << "Energia Total: "
              << stats.E_total
              << " kWh\n";

    std::cout << "Energia PV: "
              << stats.E_pv
              << " kWh\n";

    std::cout << "Energia Grid: "
              << stats.E_grid
              << " kWh\n";

    std::cout << "Emissoes CO2: "
              << stats.CO2
              << " gCO2\n";

    std::ofstream file("results.csv", std::ios::app);

    file << stats.E_total << ","
        << stats.E_pv << ","
        << stats.E_grid << ","
        << stats.CO2 << std::endl;

    file.close();

}
